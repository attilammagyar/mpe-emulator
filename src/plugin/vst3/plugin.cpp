/*
 * This file is part of MPE Emulator.
 * Copyright (C) 2023, 2024, 2025  Attila M. Magyar
 *
 * MPE Emulator is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * MPE Emulator is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <algorithm>
#include <cstddef>
#include <string>

#include <vst3sdk/base/source/fstreamer.h>
#include <vst3sdk/base/source/fstring.h>
#include <vst3sdk/pluginterfaces/base/fstrdefs.h>
#include <vst3sdk/pluginterfaces/base/funknown.h>
#include <vst3sdk/pluginterfaces/base/futils.h>
#include <vst3sdk/pluginterfaces/base/ibstream.h>
#include <vst3sdk/pluginterfaces/base/ustring.h>
#include <vst3sdk/pluginterfaces/vst/ivstevents.h>
#include <vst3sdk/pluginterfaces/vst/ivsthostapplication.h>
#include <vst3sdk/pluginterfaces/vst/ivstparameterchanges.h>
#include <vst3sdk/public.sdk/source/vst/vstaudioprocessoralgo.h>
#include <vst3sdk/public.sdk/source/vst/vsteventshelper.h>

#include "plugin/vst3/plugin.hpp"

#include "gui/gui.hpp"

#if SMTG_OS_LINUX
#include "plugin/vst3/plugin-xcb.cpp"
#elif SMTG_OS_WINDOWS
#include "plugin/vst3/plugin-win32.cpp"
#else
#error "Unsupported OS, currently MPE Emulator can be compiled only for Linux and Windows. (Or did something go wrong with the SMTG_OS_LINUX and SMTG_OS_WINDOWS macros?)"
#endif

#include "midi.hpp"
#include "serializer.hpp"
#include "strings.hpp"


using namespace Steinberg;


#define MPE_EMULATOR_VST3_SEND_MSG(msg_id, attr_setter, attr_name, attr_value)  \
    do {                                                                        \
        IPtr<Vst::IMessage> message = owned(allocateMessage());                 \
                                                                                \
        if (message) {                                                          \
            message->setMessageID(msg_id);                                      \
                                                                                \
            Vst::IAttributeList* attributes = message->getAttributes();         \
                                                                                \
            if (attributes) {                                                   \
                attributes->attr_setter((attr_name), (attr_value));             \
                sendMessage(message);                                           \
            }                                                                   \
        }                                                                       \
    } while (false)


#define MPE_EMULATOR_VST3_SEND_EMPTY_MSG(msg_id)                            \
    do {                                                                    \
        IPtr<Vst::IMessage> message = owned(allocateMessage());             \
                                                                            \
        if (message) {                                                      \
            message->setMessageID(msg_id);                                  \
            sendMessage(message);                                           \
        }                                                                   \
    } while (false)


namespace MpeEmulator
{

FUID const Vst3Plugin::Processor::ID(0x56535441, 0x4d50456d, 0x7065656d, 0x756c6174);

FUID const Vst3Plugin::Controller::ID(0x56534541, 0x4d50456d, 0x7065656d, 0x756c6174);


ViewRect const Vst3Plugin::GUI::rect(0, 0, MpeEmulator::GUI::WIDTH, MpeEmulator::GUI::HEIGHT);


Vst3Plugin::Event::Event()
    : time_offset(0.0),
    velocity_or_value(0.0),
    type(Type::UNDEFINED),
    note_or_ctl(0),
    channel(0)
{
}


Vst3Plugin::Event::Event(
        Type const type,
        double const time_offset,
        Midi::Byte const note_or_ctl,
        Midi::Channel const channel,
        double const velocity_or_value
) : time_offset(time_offset),
    velocity_or_value(velocity_or_value),
    type(type),
    note_or_ctl(note_or_ctl),
    channel(channel & 0x0f)
{
}


bool Vst3Plugin::Event::operator<(Event const& event) const noexcept
{
    return time_offset < event.time_offset;
}


FUnknown* Vst3Plugin::Processor::createInstance(void* unused)
{
    return (Vst::IAudioProcessor*)new Processor();
}


Vst3Plugin::Processor::Processor()
    : proxy(),
    events(8192),
    sample_rate(44100.0)
{
    setControllerClass(Controller::ID);
}


tresult PLUGIN_API Vst3Plugin::Processor::initialize(FUnknown* context)
{
    tresult result = AudioEffect::initialize(context);

    if (result != kResultTrue) {
        return kResultFalse;
    }

    addEventInput(STR16("Event Input"), 1);
    addEventOutput(STR16("Event Output"), 16);

    /*
    Audio output is not actually used, but some hosts are known to not run
    plugins properly that have 0 audio channels.
    */
    addAudioOutput(STR16("AudioOutput"), Vst::SpeakerArr::kStereo);

    return kResultOk;
}


tresult PLUGIN_API Vst3Plugin::Processor::setBusArrangements(
    Vst::SpeakerArrangement* inputs,
    int32 number_of_inputs,
    Vst::SpeakerArrangement* outputs,
    int32 number_of_outputs
) {
    if (
            number_of_outputs >= 1 && outputs[0] == Vst::SpeakerArr::kStereo
    )
    {
        return AudioEffect::setBusArrangements(
            inputs, number_of_inputs, outputs, number_of_outputs
        );
    }

    return kResultFalse;
}


tresult PLUGIN_API Vst3Plugin::Processor::connect(IConnectionPoint* other)
{
    tresult result = Vst::AudioEffect::connect(other);
    share_proxy();

    return result;
}


tresult PLUGIN_API Vst3Plugin::Processor::notify(Vst::IMessage* message)
{
    if (!message) {
        return kInvalidArgument;
    }

    if (FIDStringsEqual(message->getMessageID(), MSG_CTL_READY)) {
        share_proxy();
    }

    return Vst::AudioEffect::notify(message);
}


tresult PLUGIN_API Vst3Plugin::Processor::canProcessSampleSize(int32 symbolic_sample_size)
{
    if (
            symbolic_sample_size == Vst::SymbolicSampleSizes::kSample64
            || symbolic_sample_size == Vst::SymbolicSampleSizes::kSample32
    ) {
        return kResultTrue;
    }

    return kResultFalse;
}


tresult PLUGIN_API Vst3Plugin::Processor::setupProcessing(Vst::ProcessSetup& setup)
{
    double const sample_rate = (double)setup.sampleRate;

    this->sample_rate = sample_rate > 0.0 ? sample_rate : 44100.0;

    return AudioEffect::setupProcessing(setup);
}


tresult PLUGIN_API Vst3Plugin::Processor::setProcessing(TBool state)
{
    reset_for_state_change(state);

    return kResultOk;
}


void Vst3Plugin::Processor::reset_for_state_change(TBool const new_state) noexcept
{
    if (new_state) {
        proxy.resume();
    } else {
        proxy.suspend();
    }
}


tresult PLUGIN_API Vst3Plugin::Processor::setActive(TBool state)
{
    reset_for_state_change(state);

    return AudioEffect::setActive(state);
}


void Vst3Plugin::Processor::share_proxy() noexcept
{
    MPE_EMULATOR_VST3_SEND_MSG(
        MSG_SHARE_PROXY, setInt, MSG_SHARE_PROXY_PROXY, (int64)&proxy
    );
}


tresult PLUGIN_API Vst3Plugin::Processor::process(Vst::ProcessData& data)
{
    proxy.begin_processing();
    collect_note_events(data);
    collect_param_change_events(data);
    std::sort(events.begin(), events.end());
    process_events();
    events.clear();

    if (data.outputEvents != NULL) {
        generate_out_events(*data.outputEvents, std::max(0, data.numSamples - 1));
    }

    if (proxy.is_dirty()) {
        proxy.clear_dirty_flag();
        MPE_EMULATOR_VST3_SEND_EMPTY_MSG(MSG_PROXY_DIRTY);
    }

    if (data.numOutputs == 0 || data.numSamples < 1) {
        return kResultOk;
    }

    generate_samples(data);

    return kResultOk;
}


void Vst3Plugin::Processor::collect_param_change_events(
        Vst::ProcessData& data
) noexcept {
    if (!data.inputParameterChanges) {
        return;
    }

    int32 numParamsChanged = data.inputParameterChanges->getParameterCount();

    for (int32 i = 0; i < numParamsChanged; i++) {
        Vst::IParamValueQueue* param_queue = (
            data.inputParameterChanges->getParameterData(i)
        );

        if (!param_queue) {
            continue;
        }

        Vst::ParamID const param_tag = param_queue->getParameterId();
        Proxy::ControllerId const ctl_id = (Proxy::ControllerId)param_tag;

        switch (param_tag) {
            case (Vst::ParamID)Vst::ControllerNumbers::kPitchBend:
                collect_param_change_events_as_midi_ctl(
                    param_queue, Event::Type::PITCH_WHEEL, 0
                );
                break;

            case (Vst::ParamID)Vst::ControllerNumbers::kAfterTouch:
                collect_param_change_events_as_midi_ctl(
                    param_queue, Event::Type::CHANNEL_PRESSURE, 0
                );
                break;

            default:
                if (0 <= ctl_id && ctl_id <= Proxy::ControllerId::MAX_MIDI_CC) {
                    collect_param_change_events_as_midi_ctl(
                        param_queue, Event::Type::CONTROL_CHANGE, (Midi::Byte)param_tag
                    );
                } else {
                    collect_param_change_events_as_exported_param(
                        param_queue, param_tag
                    );
                }
                break;
        }
    }
}


void Vst3Plugin::Processor::collect_param_change_events_as_midi_ctl(
        Vst::IParamValueQueue* const param_queue,
        Event::Type const event_type,
        Midi::Byte const midi_controller
) noexcept {
    int32 const number_of_points = param_queue->getPointCount();

    Vst::ParamValue value;
    int32 sample_offset;

    for (int32 i = 0; i < number_of_points; ++i) {
        if (param_queue->getPoint(i, sample_offset, value) != kResultTrue) {
            continue;
        }

        events.push_back(
            Event(
                event_type,
                (double)sample_offset / sample_rate,
                midi_controller,
                0,
                (double)value
            )
        );
    }
}


void Vst3Plugin::Processor::collect_param_change_events_as_exported_param(
        Vst::IParamValueQueue* const param_queue,
        Vst::ParamID const param_tag
) noexcept {
    Proxy::ParamId const param_id = vst3_param_tag_to_proxy_param_id(param_tag);

    if (param_id == Proxy::ParamId::INVALID_PARAM_ID) {
        return;
    }

    int32 const number_of_points = param_queue->getPointCount();

    if (number_of_points <= 0) {
        return;
    }

    Vst::ParamValue value;
    int32 sample_offset;

    for (int32 i = 0; i < number_of_points; ++i) {
        if (param_queue->getPoint(i, sample_offset, value) != kResultTrue) {
            continue;
        }

        events.push_back(
            Event(
                Event::Type::PARAM_CHANGE,
                (double)sample_offset / sample_rate,
                (Midi::Byte)vst3_param_tag_to_proxy_param_id(param_tag),
                0,
                (double)value
            )
        );
    }
}


void Vst3Plugin::Processor::collect_note_events(Vst::ProcessData& data) noexcept
{
    Vst::IEventList* input_events = data.inputEvents;

    if (!input_events) {
        return;
    }

    int32 count = input_events->getEventCount();
    Vst::Event event;

    for (int32 i = 0; i < count; ++i) {
        if (input_events->getEvent(i, event) != kResultTrue) {
            continue;
        }

        switch (event.type) {
            case Vst::Event::EventTypes::kNoteOnEvent:
                events.push_back(
                    Event(
                        Event::Type::NOTE_ON,
                        (double)event.sampleOffset / sample_rate,
                        (Midi::Byte)event.noteOn.pitch,
                        (Midi::Channel)(event.noteOn.channel & 0xff),
                        (double)event.noteOn.velocity
                    )
                );
                break;

            case Vst::Event::EventTypes::kNoteOffEvent:
                events.push_back(
                    Event(
                        Event::Type::NOTE_OFF,
                        (double)event.sampleOffset / sample_rate,
                        (Midi::Byte)event.noteOff.pitch,
                        (Midi::Channel)(event.noteOff.channel & 0xff),
                        (double)event.noteOff.velocity
                    )
                );
                break;

            case Vst::Event::EventTypes::kPolyPressureEvent:
                events.push_back(
                    Event(
                        Event::Type::NOTE_PRESSURE,
                        (double)event.sampleOffset / sample_rate,
                        (Midi::Byte)event.polyPressure.pitch,
                        (Midi::Channel)(event.polyPressure.channel & 0xff),
                        (double)event.polyPressure.pressure
                    )
                );
                break;

            default:
                break;
        }
    }
}


void Vst3Plugin::Processor::process_events() noexcept
{
    for (std::vector<Event>::const_iterator it = events.begin(); it != events.end(); ++it) {
        process_event(*it);
    }
}


void Vst3Plugin::Processor::process_event(Event const& event) noexcept
{
    switch (event.type) {
        case Event::Type::NOTE_ON: {
            Midi::Byte const velocity = float_to_midi_byte(event.velocity_or_value);

            if (velocity == 0) {
                proxy.note_off(event.time_offset, event.channel, event.note_or_ctl, 64);
            } else {
                proxy.note_on(event.time_offset, event.channel, event.note_or_ctl, velocity);
            }

            break;
        }

        case Event::Type::NOTE_PRESSURE:
            proxy.aftertouch(
                event.time_offset,
                event.channel,
                event.note_or_ctl,
                float_to_midi_byte(event.velocity_or_value)
            );
            break;

        case Event::Type::NOTE_OFF:
            proxy.note_off(
                event.time_offset,
                event.channel,
                event.note_or_ctl,
                float_to_midi_byte(event.velocity_or_value)
            );
            break;

        case Event::Type::PITCH_WHEEL:
            proxy.pitch_wheel_change(
                event.time_offset,
                0,
                float_to_midi_word(event.velocity_or_value)
            );
            break;

        case Event::Type::CONTROL_CHANGE:
            proxy.control_change(
                event.time_offset,
                0,
                event.note_or_ctl,
                float_to_midi_byte(event.velocity_or_value)
            );
            break;

        case Event::Type::CHANNEL_PRESSURE:
            proxy.channel_pressure(
                event.time_offset,
                0,
                float_to_midi_byte(event.velocity_or_value)
            );
            break;

        case Event::Type::PARAM_CHANGE:
            proxy.process_message(
                Proxy::MessageType::SET_PARAM, (Proxy::ParamId)event.note_or_ctl, event.velocity_or_value
            );
            break;

        default:
            break;
    }
}


Midi::Byte Vst3Plugin::Processor::float_to_midi_byte(
        double const number
) const noexcept {
    return std::min(
        (Midi::Byte)127,
        std::max((Midi::Byte)0, (Midi::Byte)std::round(number * 127.0))
    );
}


Midi::Word Vst3Plugin::Processor::float_to_midi_word(
        double const number
) const noexcept {
    return std::min(
        (Midi::Word)16383,
        std::max((Midi::Word)0, (Midi::Word)std::round(number * 16383.0))
    );
}


void Vst3Plugin::Processor::generate_out_events(
        Vst::IEventList& queue,
        int32 const last_sample_offset
) noexcept {
    Vst::Event vst_event;

    for (Proxy::OutEvents::const_iterator i = proxy.out_events.begin(); i != proxy.out_events.end(); ++i) {
        Midi::Event const& midi_event(*i);
        int32 const sample_offset = (
            midi_event.get_sample_offset<int32>(sample_rate, last_sample_offset)
        );

        switch (midi_event.command) {
            case Midi::NOTE_OFF:
                Vst::Helpers::init(
                    vst_event,
                    Vst::Event::EventTypes::kNoteOffEvent,
                    0,
                    sample_offset,
                    0,
                    Vst::Event::EventFlags::kIsLive
                );
                vst_event.noteOff.channel = midi_event.channel;
                vst_event.noteOff.pitch = midi_event.data_1;
                vst_event.noteOff.velocity = Midi::byte_to_float<float>(midi_event.data_2);
                vst_event.noteOff.noteId = -1;
                vst_event.noteOff.tuning = 0.0f;

                break;

            case Midi::NOTE_ON:
                Vst::Helpers::init(
                    vst_event,
                    Vst::Event::EventTypes::kNoteOnEvent,
                    0,
                    sample_offset,
                    0,
                    Vst::Event::EventFlags::kIsLive
                );
                vst_event.noteOn.channel = midi_event.channel;
                vst_event.noteOn.pitch = midi_event.data_1;
                vst_event.noteOn.tuning = 0.0f;
                vst_event.noteOn.velocity = Midi::byte_to_float<float>(midi_event.data_2);
                vst_event.noteOn.length = 0;
                vst_event.noteOn.noteId = -1;

                break;

            case Midi::CONTROL_CHANGE:
                initialize_cc_event(
                    vst_event,
                    sample_offset,
                    last_sample_offset,
                    midi_event.data_1,
                    midi_event.channel,
                    midi_event.data_2,
                    0,
                    midi_event.is_pre_note_on_setup
                );

                break;

            case Midi::CHANNEL_PRESSURE:
                initialize_cc_event(
                    vst_event,
                    sample_offset,
                    last_sample_offset,
                    Vst::ControllerNumbers::kAfterTouch,
                    midi_event.channel,
                    midi_event.data_1,
                    0,
                    midi_event.is_pre_note_on_setup
                );

                break;

            case Midi::PITCH_BEND_CHANGE:
                initialize_cc_event(
                    vst_event,
                    sample_offset,
                    last_sample_offset,
                    Vst::ControllerNumbers::kPitchBend,
                    midi_event.channel,
                    midi_event.data_1,
                    midi_event.data_2,
                    midi_event.is_pre_note_on_setup
                );

                break;

            default:
                MPE_EMULATOR_ASSERT_NOT_REACHED();
                continue;
        }

        queue.addEvent(vst_event);
    }
}


void Vst3Plugin::Processor::initialize_cc_event(
        Vst::Event& vst_event,
        int32 const sample_offset,
        int32 const last_sample_offset,
        uint8 const control_number,
        uint8 const channel,
        int8 const value_1,
        int8 const value_2,
        bool const is_pre_note_on_setup
) const noexcept {
    Vst::Helpers::initLegacyMIDICCOutEvent(
        vst_event, control_number, channel, value_1, value_2
    );

    /*
    The VST 3 protocol sends MIDI CC, channel pressure, and pitch bend events as
    parameter automation, separately from the note events, which messes up the
    sequentiality.

    The MPE specs recommend sending Note On setup control events before the
    Note On event itself, but since it's not mandatory, we should be fine with
    sending them with the same sample offset, and let the VST 3 implementation
    and the synth pick and order.

    Unfortunately, some commercial synths seem to ignore parameter automation
    events (and therefore the control events that are sent via parameters as a
    result of the Vst::LegacyMIDICCOutEvent events that we produce here) before
    encountering a Note On. In order to make sure that all synths receive at
    least one Note On setup sequence after the corresponding Note On event, we
    defer the repeated Note On setup events by 1 sample. In order to avoid this
    deferred Note On setup overwriting any actual, non-setup control event, we
    need to also defer all other control events as well.

    (Well, if a Note On event comes right at the very last sample of a block,
    then there's not much we can do, we can only hope that the ordering will
    not be messed up noticably.)
    */
    vst_event.sampleOffset = std::max(
        last_sample_offset,
        sample_offset + is_pre_note_on_setup ? 0 : 1
    );
}


void Vst3Plugin::Processor::generate_samples(Vst::ProcessData& data) noexcept
{
    if (processSetup.symbolicSampleSize == Vst::SymbolicSampleSizes::kSample64) {
        double** out_samples = (
            (double**)getChannelBuffersPointer(processSetup, data.outputs[0])
        );

        std::fill_n(out_samples[0], data.numSamples, 0.0);
        std::fill_n(out_samples[1], data.numSamples, 0.0);
    } else if (processSetup.symbolicSampleSize == Vst::SymbolicSampleSizes::kSample32) {
        float** out_samples = (
            (float**)getChannelBuffersPointer(processSetup, data.outputs[0])
        );

        std::fill_n(out_samples[0], data.numSamples, 0.0f);
        std::fill_n(out_samples[1], data.numSamples, 0.0f);
    } else {
        return;
    }
}


uint32 PLUGIN_API Vst3Plugin::Processor::getLatencySamples()
{
    return 0;
}


uint32 PLUGIN_API Vst3Plugin::Processor::getTailSamples()
{
    return Vst::kNoTail;
}


tresult PLUGIN_API Vst3Plugin::Processor::setState(IBStream* state)
{
    if (state == NULL) {
        return kResultFalse;
    }

    Serializer::import_settings_in_gui_thread(proxy, read_stream(state));
    proxy.push_message(
        Proxy::MessageType::CLEAR_DIRTY_FLAG,
        Proxy::ParamId::INVALID_PARAM_ID,
        0.0
    );

    return kResultOk;
}


std::string Vst3Plugin::read_stream(IBStream* stream)
{
    /*
    Not using FStreamer::readString8(), because we need the entire string here,
    and that method stops at line breaks.
    */

    char* buffer = new char[Serializer::MAX_SIZE];
    size_t i;
    int32 bytes_read;
    char8 c;

    for (i = 0; i != Serializer::MAX_SIZE; ++i) {
        stream->read(&c, sizeof(char8), &bytes_read);

        if (bytes_read != sizeof(char8) || c == '\x00') {
            break;
        }

        buffer[i] = c;
    }

    if (i >= Serializer::MAX_SIZE) {
        i = Serializer::MAX_SIZE - 1;
    }

    buffer[i] = '\x00';

    std::string const result(buffer, (std::string::size_type)i);

    delete[] buffer;

    return result;
}


tresult PLUGIN_API Vst3Plugin::Processor::getState(IBStream* state)
{
    if (state == NULL) {
        return kResultFalse;
    }

    std::string const& serialized = Serializer::serialize(proxy);
    int32 const size = serialized.size();
    int32 numBytesWritten;

    state->write((void*)serialized.c_str(), size, &numBytesWritten);

    if (numBytesWritten != size) {
        return kResultFalse;
    }

    return kResultOk;
}


Vst3Plugin::GUI::GUI(Proxy& proxy)
    : CPluginView(&rect),
    proxy(proxy),
    gui(NULL),
    run_loop(NULL),
    event_handler(NULL),
    timer_handler(NULL)
{
}


Vst3Plugin::GUI::~GUI()
{
    if (gui != NULL) {
        delete gui;

        gui = NULL;
    }
}


tresult PLUGIN_API Vst3Plugin::GUI::isPlatformTypeSupported(FIDString type)
{
    if (FIDStringsEqual(type, MPE_EMULATOR_VST3_GUI_PLATFORM)) {
        return kResultTrue;
    }

    return kResultFalse;
}


tresult PLUGIN_API Vst3Plugin::GUI::canResize()
{
    return kResultFalse;
}


void Vst3Plugin::GUI::attachedToParent()
{
    show_if_needed();
}


void Vst3Plugin::GUI::show_if_needed()
{
    if (!isAttached()) {
        return;
    }

    initialize();
}


FUnknown* Vst3Plugin::Controller::createInstance(void* unused)
{
    return (IEditController*)new Vst3Plugin::Controller();
}


Vst3Plugin::Controller::Controller() : proxy(NULL)
{
}


Vst3Plugin::Controller::~Controller()
{
}


tresult PLUGIN_API Vst3Plugin::Controller::initialize(FUnknown* context)
{
    tresult result = EditControllerEx1::initialize(context);

    if (result != kResultTrue) {
        return result;
    }

    FUnknownPtr<Vst::IVst3WrapperMPESupport> mpe_support(getHostContext());

    if (mpe_support) {
        mpe_support->enableMPEInputProcessing(false);
    }

    addUnit(
        new Vst::Unit(
            STR("Root"), Vst::kRootUnitId, Vst::kNoParentUnitId, Vst::kNoProgramListId
        )
    );

    constexpr int midi_cc_begin = (int)Proxy::ControllerId::BANK_SELECT;
    constexpr int midi_cc_end = (int)Proxy::ControllerId::MAX_MIDI_CC + 1;
    constexpr int cc_sound_5 = (int)Proxy::ControllerId::SOUND_5;

    for (int cc = midi_cc_begin; cc != midi_cc_end; ++cc) {
        parameters.addParameter(
            create_midi_ctl_param(
                (Proxy::ControllerId)cc,
                (Vst::ParamID)cc,
                cc == cc_sound_5 ? 0.5 : 0.0
            )
        );
    }

    parameters.addParameter(
        create_midi_ctl_param(
            Proxy::ControllerId::PITCH_WHEEL,
            (Vst::ParamID)Vst::ControllerNumbers::kPitchBend,
            0.5
        )
    );

    parameters.addParameter(
        create_midi_ctl_param(
            Proxy::ControllerId::CHANNEL_PRESSURE,
            (Vst::ParamID)Vst::ControllerNumbers::kAfterTouch,
            0.0
        )
    );

    constexpr int param_begin = (int)Proxy::ParamId::MCM;
    constexpr int param_end = (int)Proxy::ParamId::INVALID_PARAM_ID;

    Proxy const dummy_proxy;

    for (int param_id = param_begin; param_id != param_end; ++param_id) {
        parameters.addParameter(
            create_exported_param(dummy_proxy, (Proxy::ParamId)param_id)
        );
    }

    parameters.addParameter(set_up_patch_changed_param());

    return result;
}


Vst::ParamID Vst3Plugin::proxy_param_id_to_vst3_param_tag(
        Proxy::ParamId const param_id
) {
    return 4096 + (uint32)param_id;
}


Proxy::ParamId Vst3Plugin::vst3_param_tag_to_proxy_param_id(
        Vst::ParamID const param_tag
) {
    if (param_tag < 4096 || param_tag >= 4096 + Proxy::PARAM_ID_COUNT) {
        return Proxy::ParamId::INVALID_PARAM_ID;
    }

    return (Proxy::ParamId)(param_tag - 4096);
}


Vst::Parameter* Vst3Plugin::Controller::create_midi_ctl_param(
        Proxy::ControllerId const controller_id,
        Vst::ParamID const param_tag,
        double const default_value
) const {
    Vst::RangeParameter* param = new Vst::RangeParameter(
        USTRING(Strings::CONTROLLERS_LONG[(size_t)controller_id]),
        param_tag,
        USTRING("%"),
        0.0,
        100.0,
        default_value * 100.0,
        0,
        Vst::ParameterInfo::kCanAutomate,
        Vst::kRootUnitId,
        USTRING(Strings::CONTROLLERS_SHORT[(size_t)controller_id])
    );
    param->setPrecision(1);

    return param;
}


Vst::Parameter* Vst3Plugin::Controller::create_exported_param(
        Proxy const& proxy,
        Proxy::ParamId const param_id
) const {
    size_t number_of_options;
    char const* const* const options = Strings::get_options(param_id, number_of_options);

    if (options == NULL) {
        Vst::RangeParameter* param = new Vst::RangeParameter(
            USTRING(Strings::PARAMS[(size_t)param_id]),
            proxy_param_id_to_vst3_param_tag(param_id),
            USTRING("%"),
            0.0,
            100.0,
            100.0 * proxy.get_param_default_ratio(param_id),
            0,
            Vst::ParameterInfo::kCanAutomate,
            Vst::kRootUnitId,
            USTRING(proxy.get_param_name(param_id).c_str())
        );
        param->setPrecision(1);

        return param;
    } else {
        Vst::StringListParameter* param = new Vst::StringListParameter(
            USTRING(Strings::PARAMS[(size_t)param_id]),
            proxy_param_id_to_vst3_param_tag(param_id),
            NULL,
            Vst::ParameterInfo::kIsList,
            Vst::kRootUnitId,
            USTRING(proxy.get_param_name(param_id).c_str())
        );

        for (size_t i = param_id == Proxy::ParamId::Z1CHN ? 1 : 0; i != number_of_options; ++i) {
            param->appendString(USTRING(options[i]));
        }

        return param;
    }
}


Vst::Parameter* Vst3Plugin::Controller::set_up_patch_changed_param() const
{
    Vst::RangeParameter* param = new Vst::RangeParameter(
        USTRING("Patch Changed"),
        SETTINGS_CHANGED_PARAM_ID,
        USTRING("%"),
        0.0,
        100.0,
        0.0,
        0,
        Vst::ParameterInfo::kIsReadOnly,
        Vst::kRootUnitId,
        USTRING("Changed")
    );
    param->setPrecision(1);

    return param;
}


tresult PLUGIN_API Vst3Plugin::Controller::getMidiControllerAssignment(
        int32 bus_index,
        int16 channel,
        Vst::CtrlNumber midi_controller_number,
        Vst::ParamID& id
) {
    if (bus_index == 0 && midi_controller_number < Vst::kCountCtrlNumber) {
        if (
                (0 <= (int)midi_controller_number && midi_controller_number <= (int)Proxy::MAX_MIDI_CC)
                || midi_controller_number == Vst::ControllerNumbers::kPitchBend
                || midi_controller_number == Vst::ControllerNumbers::kAfterTouch
        ) {
            id = midi_controller_number;

            return kResultTrue;
        }
    }

    return kResultFalse;
}


tresult PLUGIN_API Vst3Plugin::Controller::connect(IConnectionPoint* other)
{
    tresult result = EditControllerEx1::connect(other);

    MPE_EMULATOR_VST3_SEND_EMPTY_MSG(MSG_CTL_READY);

    return result;
}


tresult PLUGIN_API Vst3Plugin::Controller::notify(Vst::IMessage* message)
{
    if (!message) {
        return kInvalidArgument;
    }

    if (FIDStringsEqual(message->getMessageID(), MSG_SHARE_PROXY)) {
        int64 proxy_ptr;

        if (message->getAttributes()->getInt(MSG_SHARE_PROXY_PROXY, proxy_ptr) == kResultOk) {
            proxy = (Proxy*)proxy_ptr;
            update_params();

            return kResultOk;
        }
    } else if (FIDStringsEqual(message->getMessageID(), MSG_PROXY_DIRTY)) {
        if (proxy != NULL) {
            update_params();
        }

        /*
        Calling setDirty(true) would suffice, the dummy parameter dance is done
        only to keep parameter behaviour in sync with the FST plugin.
        */
        Vst::ParamValue const new_value = getParamNormalized(SETTINGS_CHANGED_PARAM_ID) + 0.01;

        setParamNormalized(SETTINGS_CHANGED_PARAM_ID, new_value < 1.0 ? new_value : 0.0);
        setDirty(true);

        return kResultOk;
    }

    return EditControllerEx1::notify(message);
}


void Vst3Plugin::Controller::update_params()
{
    constexpr int param_begin = (int)Proxy::ParamId::MCM;
    constexpr int param_end = (int)Proxy::ParamId::INVALID_PARAM_ID;

    for (int i = param_begin; i != param_end; ++i) {
        Proxy::ParamId const param_id = (Proxy::ParamId)i;
        setParamNormalized(
            proxy_param_id_to_vst3_param_tag(param_id),
            proxy->get_param_ratio_atomic(param_id)
        );
    }
}


IPlugView* PLUGIN_API Vst3Plugin::Controller::createView(FIDString name)
{
    if (FIDStringsEqual(name, Vst::ViewType::kEditor)) {
        if (proxy == NULL) {
            return NULL;
        }

        GUI* gui = new GUI(*proxy);

        return gui;
    }

    return NULL;
}


tresult PLUGIN_API Vst3Plugin::Controller::setComponentState(IBStream* state)
{
    return kResultOk;
}

}


BEGIN_FACTORY_DEF(
    MpeEmulator::Constants::COMPANY_NAME,
    MpeEmulator::Constants::COMPANY_WEB,
    MpeEmulator::Constants::COMPANY_EMAIL
)

    DEF_CLASS2(
        INLINE_UID_FROM_FUID(MpeEmulator::Vst3Plugin::Processor::ID),
        PClassInfo::kManyInstances,
        kVstAudioEffectClass,
        MpeEmulator::Constants::PLUGIN_NAME,
        0,
        Vst::PlugType::kInstrumentSynth,
        MpeEmulator::Constants::PLUGIN_VERSION_STR,
        kVstVersionString,
        MpeEmulator::Vst3Plugin::Processor::createInstance
    )

    DEF_CLASS2(
        INLINE_UID_FROM_FUID(MpeEmulator::Vst3Plugin::Controller::ID),
        PClassInfo::kManyInstances,
        kVstComponentControllerClass,
        "MPEEmulatorController",
        0,
        "",
        MpeEmulator::Constants::PLUGIN_VERSION_STR,
        kVstVersionString,
        MpeEmulator::Vst3Plugin::Controller::createInstance
    )

END_FACTORY
