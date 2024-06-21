/*
 * This file is part of MPE Emulator.
 * Copyright (C) 2023, 2024  Attila M. Magyar
 * Copyright (C) 2023  Patrik Ehringer
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

#include <cstring>

#include "plugin/fst/plugin.hpp"

// #include "debug.hpp"
#include "serializer.hpp"
#include "spscqueue.cpp"
#include "strings.hpp"


namespace MpeEmulator
{

static constexpr int FST_OP_CODE_NAMES_LEN = 78;

static constexpr char const* FST_OP_CODE_NAMES[FST_OP_CODE_NAMES_LEN] = {
    "Open",                         /*    0 */
    "Close",                        /*    1 */
    "SetProgram",                   /*    2 */
    "GetProgram",                   /*    3 */
    "SetProgramName",               /*    4 */
    "GetProgramName",               /*    5 */
    "GetParamLabel",                /*    6 */
    "GetParamDisplay",              /*    7 */
    "GetParamName",                 /*    8 */
    "UNKNOWN-9",                    /*    9 */
    "SetSampleRate",                /*   10 */
    "SetBlockSize",                 /*   11 */
    "MainsChanged",                 /*   12 */
    "EditGetRect",                  /*   13 */
    "EditOpen",                     /*   14 */
    "EditClose",                    /*   15 */
    "UNKNOWN-16",                   /*   16 */
    "UNKNOWN-17",                   /*   17 */
    "UNKNOWN-18",                   /*   18 */
    "EditIdle",                     /*   19 */
    "UNKNOWN-20",                   /*   20 */
    "UNKNOWN-21",                   /*   21 */
    "Identify",                     /*   22 */
    "GetChunk",                     /*   23 */
    "SetChunk",                     /*   24 */
    "ProcessEvents",                /*   25 */
    "CanBeAutomated",               /*   26 */
    "String2Parameter",             /*   27 */
    "UNKNOWN-28",                   /*   28 */
    "GetProgramNameIndexed",        /*   29 */
    "UNKNOWN-30",                   /*   30 */
    "UNKNOWN-31",                   /*   31 */
    "UNKNOWN-32",                   /*   32 */
    "GetInputProperties",           /*   33 */
    "GetOutputProperties",          /*   34 */
    "GetPlugCategory",              /*   35 */
    "UNKNOWN-36",                   /*   36 */
    "UNKNOWN-37",                   /*   37 */
    "UNKNOWN-38",                   /*   38 */
    "UNKNOWN-39",                   /*   39 */
    "UNKNOWN-40",                   /*   40 */
    "UNKNOWN-41",                   /*   41 */
    "SetSpeakerArrangement",        /*   42 */
    "UNKNOWN-43",                   /*   43 */
    "UNKNOWN-44",                   /*   44 */
    "GetEffectName",                /*   45 */
    "UNKNOWN-46",                   /*   46 */
    "GetVendorString",              /*   47 */
    "GetProductString",             /*   48 */
    "GetVendorVersion",             /*   49 */
    "VendorSpecific",               /*   50 */
    "CanDo",                        /*   51 */
    "UNKNOWN-52",                   /*   52 */
    "Idle",                         /*   53, see the comment at the bottom */
    "UNKNOWN-54",                   /*   54 */
    "UNKNOWN-55",                   /*   55 */
    "UNKNOWN-56",                   /*   56 */
    "UNKNOWN-57",                   /*   57 */
    "GetVstVersion",                /*   58 */
    "UNKNOWN-59",                   /*   59 */
    "UNKNOWN-60",                   /*   60 */
    "UNKNOWN-61",                   /*   61 */
    "UNKNOWN-62",                   /*   62 */
    "GetCurrentMidiProgram",        /*   63 */
    "UNKNOWN-64",                   /*   64 */
    "UNKNOWN-65",                   /*   65 */
    "GetMidiNoteName",              /*   66 */
    "UNKNOWN-67",                   /*   67 */
    "UNKNOWN-68",                   /*   68 */
    "GetSpeakerArrangement",        /*   69 */
    "ShellGetNextPlugin",           /*   70 */
    "StartProcess",                 /*   71 */
    "StopProcess",                  /*   72 */
    "SetTotalSampleToProcess",      /*   73 */
    "UNKNOWN-74",                   /*   74 */
    "UNKNOWN-75",                   /*   75 */
    "UNKNOWN-76",                   /*   76 */
    "SetProcessPrecision",          /*   77 */
};


AEffect* FstPlugin::create_instance(
        audioMasterCallback const host_callback_ptr,
        GUI::PlatformData const platform_data
) noexcept {
    AEffect* effect = new AEffect();

    FstPlugin* fst_plugin = new FstPlugin(
        effect, host_callback_ptr, platform_data
    );

    memset(effect, 0, sizeof(AEffect));

    effect->magic = kEffectMagic;
    effect->dispatcher = &dispatch;
    effect->process = &process_accumulating;
    effect->getParameter = &get_parameter;
    effect->setParameter = &set_parameter;
    effect->numPrograms = (VstInt32)Bank::NUMBER_OF_PROGRAMS;
    effect->numParams = (VstInt32)NUMBER_OF_PARAMETERS;
    effect->numInputs = FstPlugin::IN_CHANNELS;
    effect->numOutputs = FstPlugin::OUT_CHANNELS;
    effect->flags = (
        effFlagsHasEditor
        | effFlagsIsSynth
        | effFlagsCanReplacing
        | effFlagsCanDoubleReplacing
        | effFlagsProgramChunks
    );
    effect->initialDelay = fst_plugin->get_latency_samples();
    effect->object = (void*)fst_plugin;
    effect->uniqueID = CCONST('A', 'M', 'P', 'E');
    effect->version = FstPlugin::VERSION;
    effect->processReplacing = &process_replacing;
    effect->processDoubleReplacing = &process_double_replacing;

    return effect;
}


VstIntPtr VSTCALLBACK FstPlugin::dispatch(
        AEffect* effect,
        VstInt32 op_code,
        VstInt32 index,
        VstIntPtr ivalue,
        void* pointer,
        float fvalue
) {
    constexpr VstInt32 effIdle = 53; /* see the comment at the bottom */

    MpeEmulator::FstPlugin* fst_plugin = (MpeEmulator::FstPlugin*)effect->object;

    // if (
            // true
            // && op_code != effEditIdle
            // && (op_code != effProcessEvents || fst_plugin->prev_logged_op_code != effProcessEvents)
            // && op_code != effIdle
            // && op_code != effGetProgram
            // && op_code != effGetProgramName
            // && op_code != effGetProductString
            // && op_code != 67
            // && op_code != 68
            // && op_code != effEditGetRect
            // && op_code != effGetProgramNameIndexed
    // ) {
        // fst_plugin->prev_logged_op_code = op_code;

        // MPE_EMULATOR_DEBUG(
            // "plugin=%p, op_code=%d, op_code_name=%s, index=%d, ivalue=%d, fvalue=%f",
            // effect->object,
            // (int)op_code,
            // ((op_code < FST_OP_CODE_NAMES_LEN) ? FST_OP_CODE_NAMES[op_code] : "???"),
            // (int)index,
            // (int)ivalue,
            // fvalue
        // );
    // }

    switch (op_code) {
        case effProcessEvents:
            fst_plugin->process_vst_events((VstEvents*)pointer);
            return 1;

        case effOpen:
            fst_plugin->initialize();
            return 0;

        case effClose:
            delete fst_plugin;
            return 0;

        case effSetProgram:
            fst_plugin->set_program((size_t)ivalue);
            return 0;

        case effGetProgram:
            return fst_plugin->get_program();

        case effSetProgramName:
            fst_plugin->set_program_name((const char*)pointer);
            return 0;

        case effGetProgramName:
            fst_plugin->get_program_name((char*)pointer);
            return 0;

        case effGetProgramNameIndexed:
            return fst_plugin->get_program_name((char*)pointer, (size_t)index);

        case effGetParamLabel:
            fst_plugin->get_param_label((size_t)index, (char*)pointer);
            return 0;

        case effGetParamDisplay:
            fst_plugin->get_param_display((size_t)index, (char*)pointer);
            return 0;

        case effGetParamName:
            fst_plugin->get_param_name((size_t)index, (char*)pointer);
            return 0;

        case effCanBeAutomated:
            return fst_plugin->is_automatable((size_t)index) ? 1 : 0;

        case effSetSampleRate:
            fst_plugin->set_sample_rate((double)fvalue);
            return 0;

        case effSetBlockSize:
            fst_plugin->set_block_size(ivalue);
            return 0;

        case effMainsChanged:
            if (ivalue) {
                fst_plugin->resume();
            } else {
                fst_plugin->suspend();
            }
            return 0;

        case effEditGetRect:
            *((ERect**)pointer) = &fst_plugin->window_rect;
            return (VstIntPtr)pointer;

        case effEditOpen:
            fst_plugin->open_gui((GUI::PlatformWidget)pointer);
            return 1;

        case effEditIdle:
            fst_plugin->gui_idle();
            return 0;

        case effEditClose:
            fst_plugin->close_gui();
            return 0;

        case effGetChunk:
            return fst_plugin->get_chunk((void**)pointer, index ? true : false);

        case effSetChunk:
            fst_plugin->set_chunk((void const*)pointer, ivalue, index ? true : false);
            return 0;

        case effGetPlugCategory:
            return kPlugCategSynth;

        case effGetEffectName:
        case effGetProductString:
            strncpy((char*)pointer, Constants::PLUGIN_NAME, 16);
            return 1;

        case effGetVendorString:
            strncpy((char*)pointer, Constants::COMPANY_NAME, 24);
            return 1;

        case effGetVendorVersion:
            return FstPlugin::VERSION;

        case effGetVstVersion:
            return kVstVersion;

        case effIdentify:
            return CCONST('N', 'v', 'E', 'f');

        case effCanDo:
            /*
            Can-do queries based on JUCE.
            */
            if (
                    strcmp("receiveVstMidiEvent", (char const*)pointer) == 0
                    || strcmp("receiveVstMidiEvents", (char const*)pointer) == 0
                    || strcmp("receiveVstEvents", (char const*)pointer) == 0
                    || strcmp("sendVstEvents", (char const*)pointer) == 0
                    || strcmp("sendVstMidiEvent", (char const*)pointer) == 0
                    || strcmp("sendVstMidiEvents", (char const*)pointer) == 0
            ) {
                return 1;
            }

            if (strcmp("openCloseAnyThread", (char const*)pointer) == 0) {
                return -1;
            }

            // MPE_EMULATOR_DEBUG(
                // "op_code=%d, op_code_name=%s, index=%d, ivalue=%d, fvalue=%f, pointer=%s",
                // (int)op_code,
                // ((op_code < FST_OP_CODE_NAMES_LEN) ? FST_OP_CODE_NAMES[op_code] : "???"),
                // (int)index,
                // (int)ivalue,
                // fvalue,
                // (char*)pointer
            // );
            return 0;

        case effIdle:
            return fst_plugin->idle();

        default:
            return 0;
    }

    return 0;
}


void VSTCALLBACK FstPlugin::process_accumulating(
        AEffect* effect,
        float** indata,
        float** outdata,
        VstInt32 frames
) {
    MpeEmulator::FstPlugin* fst_plugin = (MpeEmulator::FstPlugin*)effect->object;

    fst_plugin->generate_and_add_samples(frames, indata, outdata);
}


void VSTCALLBACK FstPlugin::process_replacing(
        AEffect* effect,
        float** indata,
        float** outdata,
        VstInt32 frames
) {
    MpeEmulator::FstPlugin* fst_plugin = (MpeEmulator::FstPlugin*)effect->object;

    fst_plugin->generate_samples<float>(frames, outdata);
}


void VSTCALLBACK FstPlugin::process_double_replacing(
        AEffect* effect,
        double** indata,
        double** outdata,
        VstInt32 frames
) {
    MpeEmulator::FstPlugin* fst_plugin = (MpeEmulator::FstPlugin*)effect->object;

    fst_plugin->generate_samples<double>(frames, outdata);
}


float VSTCALLBACK FstPlugin::get_parameter(AEffect* effect, VstInt32 index)
{
    MpeEmulator::FstPlugin* fst_plugin = (MpeEmulator::FstPlugin*)effect->object;

    return fst_plugin->get_parameter((size_t)index);
}


void VSTCALLBACK FstPlugin::set_parameter(
        AEffect* effect,
        VstInt32 index,
        float fvalue
) {
    MpeEmulator::FstPlugin* fst_plugin = (MpeEmulator::FstPlugin*)effect->object;

    fst_plugin->set_parameter((size_t)index, fvalue);
}


void FstPlugin::populate_parameters(
        Proxy& proxy,
        Parameters& parameters
) noexcept {
    size_t index = 0;

    constexpr int midi_cc_begin = (int)Proxy::ControllerId::BANK_SELECT;
    constexpr int midi_cc_end = (int)Proxy::ControllerId::MAX_MIDI_CC + 1;

    for (int controller_id = midi_cc_begin; controller_id != midi_cc_end; ++controller_id) {
        parameters[index++] = Parameter(
            Strings::CONTROLLERS_SHORT[controller_id],
            Strings::CONTROLLERS_LONG[controller_id],
            Proxy::ParamId::INVALID_PARAM_ID,
            (Proxy::ControllerId)controller_id
        );
    }

    Parameter pitch_wheel(
        Strings::CONTROLLERS_SHORT[Proxy::ControllerId::PITCH_WHEEL],
        Strings::CONTROLLERS_LONG[Proxy::ControllerId::PITCH_WHEEL],
        Proxy::ParamId::INVALID_PARAM_ID,
        Proxy::ControllerId::PITCH_WHEEL
    );
    pitch_wheel.set_value(0.5f);

    parameters[index++] = pitch_wheel;

    parameters[index++] = Parameter(
        Strings::CONTROLLERS_SHORT[Proxy::ControllerId::CHANNEL_PRESSURE],
        Strings::CONTROLLERS_LONG[Proxy::ControllerId::CHANNEL_PRESSURE],
        Proxy::ParamId::INVALID_PARAM_ID,
        Proxy::ControllerId::CHANNEL_PRESSURE
    );

    constexpr int param_begin = (int)Proxy::ParamId::MCM;
    constexpr int param_end = (int)Proxy::ParamId::INVALID_PARAM_ID;

    for (int param_id = param_begin; param_id != param_end; ++param_id) {
        parameters[index++] = Parameter(
            proxy.get_param_name((Proxy::ParamId)param_id).c_str(),
            Strings::PARAMS[param_id],
            (Proxy::ParamId)param_id,
            Proxy::ControllerId::INVALID_CONTROLLER_ID
        );
    }

    MPE_EMULATOR_ASSERT(index == PATCH_CHANGED_PARAMETER_INDEX);

    Parameter patch_changed = Parameter(
        PATCH_CHANGED_PARAMETER_SHORT_NAME,
        PATCH_CHANGED_PARAMETER_LONG_NAME,
        Proxy::ParamId::INVALID_PARAM_ID,
        Proxy::ControllerId::INVALID_CONTROLLER_ID
    );

    patch_changed.set_value(0.0f);

    parameters[PATCH_CHANGED_PARAMETER_INDEX] = patch_changed;
}


FstPlugin::FstPlugin(
        AEffect* const effect,
        audioMasterCallback const host_callback_ptr,
        GUI::PlatformData const platform_data
) noexcept
    : proxy(),
    effect(effect),
    host_callback_ptr(host_callback_ptr),
    platform_data(platform_data),
    gui(NULL),
    to_audio_messages(1024),
    to_audio_string_messages(256),
    to_gui_messages(1024),
    serialized_bank(""),
    current_patch(""),
    sample_rate(44100.0),
    current_program_index(0),
    min_samples_before_next_cc_ui_update(8192),
    remaining_samples_before_next_cc_ui_update(0),
    min_samples_before_next_bank_update(16384),
    remaining_samples_before_next_bank_update(0),
    prev_logged_op_code(-1),
    had_midi_cc_event(false),
    need_bank_update(false),
    need_host_update(false)
{
    clear_received_midi_cc();

    window_rect.top = 0;
    window_rect.left = 0;
    window_rect.bottom = GUI::HEIGHT;
    window_rect.right = GUI::WIDTH;

    populate_parameters(proxy, parameters);

    serialized_bank = bank.serialize();
    current_patch = bank[current_program_index].serialize();

    program_names.import_names(serialized_bank);
}


FstPlugin::~FstPlugin()
{
    close_gui();
}


void FstPlugin::process_internal_messages_in_audio_thread(
        SPSCQueue<FstPlugin::Message>& messages
) noexcept {
    SPSCQueue<Message>::SizeType const message_count = messages.length();

    for (size_t i = 0; i != message_count; ++i) {
        Message message;

        if (!messages.pop(message)) {
            continue;
        }

        switch (message.get_type()) {
            case MessageType::CHANGE_PROGRAM:
                handle_change_program(message.get_index());
                break;

            case MessageType::RENAME_PROGRAM:
                handle_rename_program(message.get_serialized_data());
                break;

            case MessageType::CHANGE_PARAM:
                handle_change_param(
                    message.get_index(), message.get_new_value()
                );
                break;

            case MessageType::IMPORT_PATCH:
                handle_import_patch(message.get_serialized_data());
                break;

            case MessageType::IMPORT_BANK:
                handle_import_bank(message.get_serialized_data());
                break;

            default:
                break;
        }
    }
}


void FstPlugin::handle_change_program(size_t const new_program) noexcept
{
    if (new_program >= Bank::NUMBER_OF_PROGRAMS) {
        return;
    }

    size_t const old_program = bank.get_current_program_index();

    if (new_program == old_program) {
        return;
    }

    std::string const new_patch(bank[new_program].serialize());

    proxy.process_messages();
    bank[old_program].import(Serializer::serialize(proxy));
    Serializer::import_settings_in_audio_thread(proxy, new_patch);
    proxy.clear_dirty_flag();
    bank.set_current_program_index(new_program);

    need_bank_update = true;
}


void FstPlugin::handle_rename_program(std::string const& name) noexcept
{
    size_t const current_program_index = bank.get_current_program_index();
    Bank::Program& current_program = bank[current_program_index];

    current_program.set_name(name);

    need_bank_update = true;
}


void FstPlugin::handle_change_param(
        size_t const index,
        double const new_value
) noexcept {
    if (index >= NUMBER_OF_PARAMETERS || index == PATCH_CHANGED_PARAMETER_INDEX) {
        return;
    }

    Parameter& param = parameters[index];

    if (param.is_exported_param()) {
        proxy.process_message(
            Proxy::MessageType::SET_PARAM, param.get_param_id(), new_value
        );

        return;
    }

    Proxy::ControllerId controller_id = param.get_controller_id();

    if (midi_cc_received[(size_t)controller_id]) {
        return;
    }

    /*
    Some hosts (e.g. FL Studio 21) swallow most MIDI CC messages, and
    the only way to make physical knobs and faders on a MIDI keyboard
    work in the plugin is to export parameters to which those MIDI CC
    messages can be assigned in the host, and then interpret the
    changes of these parameters as if the corresponding MIDI CC message
    had been received.
    */

    switch (controller_id) {
        case Proxy::ControllerId::PITCH_WHEEL:
            proxy.pitch_wheel_change(
                0.0, 0, Midi::float_to_word<double>(new_value)
            );

            break;

        case Proxy::ControllerId::CHANNEL_PRESSURE:
            proxy.channel_pressure(
                0.0, 0, Midi::float_to_byte<double>(new_value)
            );

            break;

        case Proxy::ControllerId::MIDI_LEARN:
        case Proxy::ControllerId::NONE:
            MPE_EMULATOR_ASSERT_NOT_REACHED();

            break;

        default:
            proxy.control_change(
                0.0,
                0,
                param.get_controller_id(),
                Midi::float_to_byte<double>(new_value)
            );

            break;
    }
}


void FstPlugin::handle_import_patch(std::string const& patch) noexcept
{
    size_t const current_program = bank.get_current_program_index();

    Serializer::import_settings_in_audio_thread(proxy, patch);
    proxy.clear_dirty_flag();

    std::string const& serialized_patch(Serializer::serialize(proxy));

    bank[current_program].import(serialized_patch);

    need_bank_update = true;
}


void FstPlugin::handle_import_bank(std::string const& serialized_bank) noexcept
{
    size_t const current_program = bank.get_current_program_index();

    bank.import(serialized_bank);

    Serializer::import_settings_in_audio_thread(
        proxy, bank[current_program].serialize()
    );
    proxy.clear_dirty_flag();

    need_bank_update = true;
}


void FstPlugin::process_internal_messages_in_gui_thread() noexcept
{
    SPSCQueue<Message>::SizeType const message_count = to_gui_messages.length();

    for (size_t i = 0; i != message_count; ++i) {
        Message message;

        if (!to_gui_messages.pop(message)) {
            continue;
        }

        switch (message.get_type()) {
            case MessageType::PROGRAM_CHANGED:
                handle_program_changed(
                    message.get_index(), message.get_serialized_data()
                );
                break;

            case MessageType::BANK_CHANGED:
                handle_bank_changed(message.get_serialized_data());
                break;

            case MessageType::PARAMS_CHANGED:
                handle_params_changed();
                break;

            case MessageType::PROXY_WAS_DIRTY:
                handle_proxy_was_dirty();
                break;

            default:
                break;
        }
    }
}


void FstPlugin::handle_program_changed(
        size_t const new_program,
        std::string const& patch
) noexcept {
    Bank::Program program;

    current_program_index = new_program;
    current_patch = patch;

    program.import(patch);
    program_names[current_program_index].set_name(program.get_name());
}


void FstPlugin::handle_bank_changed(std::string const& serialized_bank) noexcept
{
    this->serialized_bank = serialized_bank;

    program_names.import_names(serialized_bank);
}


void FstPlugin::handle_params_changed() noexcept
{
    need_host_update = true;
}


void FstPlugin::handle_proxy_was_dirty() noexcept
{
    Parameter& dirty = parameters[PATCH_CHANGED_PARAMETER_INDEX];

    float const new_value = dirty.get_value(proxy) + 0.01f;

    dirty.set_value(new_value < 1.0f ? new_value : 0.0f);
    need_host_update = true;

    VstInt32 const index = (VstInt32)PATCH_CHANGED_PARAMETER_INDEX;

    host_callback(audioMasterBeginEdit, index);
    host_callback(audioMasterAutomate, index, 0, NULL, new_value);
    host_callback(audioMasterEndEdit, index);
}


VstInt32 FstPlugin::get_latency_samples() const noexcept
{
    return 0;
}


void FstPlugin::initialize() noexcept
{
    need_idle();
}


void FstPlugin::need_idle() noexcept
{
    /* see the comment at the bottom */
    constexpr int audioMasterNeedIdle = 14;

    host_callback(audioMasterNeedIdle);
}


VstIntPtr FstPlugin::idle() noexcept
{
    process_internal_messages_in_gui_thread();
    update_host_display();

    return 1;
}


void FstPlugin::set_sample_rate(double const new_sample_rate) noexcept
{
    sample_rate = new_sample_rate;

    process_internal_messages_in_gui_thread();

    if (new_sample_rate > HOST_CC_UI_UPDATE_FREQUENCY) {
        min_samples_before_next_cc_ui_update = 1 + (VstInt32)(
            new_sample_rate * HOST_CC_UI_UPDATE_FREQUENCY_INV
        );
        remaining_samples_before_next_cc_ui_update = min_samples_before_next_cc_ui_update;

        min_samples_before_next_bank_update = 1 + (VstInt32)(
            new_sample_rate * BANK_UPDATE_FREQUENCY_INV
        );
        remaining_samples_before_next_bank_update = min_samples_before_next_bank_update;
    }

    proxy.running_status = 0;
    this->running_status = 0;
}


void FstPlugin::set_block_size(VstIntPtr const new_block_size) noexcept
{
    process_internal_messages_in_gui_thread();
    proxy.running_status = 0;
    this->running_status = 0;
}


void FstPlugin::suspend() noexcept
{
    process_internal_messages_in_gui_thread();
    need_idle();
    proxy.suspend();
    proxy.running_status = 0;
    this->running_status = 0;
}


void FstPlugin::resume() noexcept
{
    proxy.resume();
    proxy.begin_processing();
    proxy.running_status = 0;
    this->running_status = 0;
    host_callback(audioMasterWantMidi, 0, 1);
    process_internal_messages_in_gui_thread();
    need_idle();
}


void FstPlugin::process_vst_events(VstEvents const* const events) noexcept
{
    clear_received_midi_cc();

    for (VstInt32 i = 0; i < events->numEvents; ++i) {
        VstEvent* event = events->events[i];

        if (event->type == kVstMidiType) {
            process_vst_midi_event((VstMidiEvent*)event);
        }
    }

    if (had_midi_cc_event && remaining_samples_before_next_cc_ui_update == 0) {
        had_midi_cc_event = false;
        remaining_samples_before_next_cc_ui_update = min_samples_before_next_cc_ui_update;
        to_gui_messages.push(Message(MessageType::PARAMS_CHANGED));
    }
}


VstIntPtr FstPlugin::host_callback(
        VstInt32 op_code,
        VstInt32 index,
        VstIntPtr ivalue,
        void* pointer,
        float fvalue
) const noexcept {
    if (host_callback_ptr == NULL) {
        return 0;
    }

    return host_callback_ptr(effect, op_code, index, ivalue, pointer, fvalue);
}


void FstPlugin::clear_received_midi_cc() noexcept
{
    midi_cc_received.reset();
    received_midi_cc_cleared = true;
}


void FstPlugin::process_vst_midi_event(VstMidiEvent const* const event) noexcept
{
    double const time_offset = (double)event->deltaFrames / sample_rate;

    Midi::Byte const* const midi_bytes = (Midi::Byte const*)event->midiData;

    Midi::EventDispatcher<FstPlugin>::dispatch_event(
        *this, time_offset, midi_bytes, 4
    );
    Midi::EventDispatcher<Proxy>::dispatch_event(
        proxy, time_offset, midi_bytes, 4
    );
}


template<typename SampleType>
void FstPlugin::generate_samples(
        VstInt32 const sample_count,
        SampleType** out_samples
) noexcept {
    if (sample_count < 1) {
        return;
    }

    prepare_processing(sample_count);

    for (VstInt32 i = 0; i != OUT_CHANNELS; ++i) {
        std::fill_n(out_samples[i], sample_count, (SampleType)0.0);
    }

    finalize_processing(sample_count);

    /*
    It would be nice to notify the host about param changes that originate from
    the plugin, but since the CC helper parameters are only ever changed by us
    during processing due to MIDI CC messages, we don't want the host to record
    them both as MIDI CC and as parameter automation.

    Also, since parameter handling seems to be done in the GUI thread and
    generate_samples() is run in the audio thread, calling audioMasterAutomate
    would cross threads, which is probably unsafe in most hosts.
    */
}


void FstPlugin::prepare_processing(VstInt32 const sample_count) noexcept
{
    if (!received_midi_cc_cleared) {
        clear_received_midi_cc();
    }

    received_midi_cc_cleared = false;

    process_internal_messages_in_audio_thread(to_audio_string_messages);
    process_internal_messages_in_audio_thread(to_audio_messages);

    if (had_midi_cc_event) {
        if (remaining_samples_before_next_cc_ui_update >= sample_count) {
            remaining_samples_before_next_cc_ui_update -= sample_count;
        } else {
            remaining_samples_before_next_cc_ui_update = 0;
        }
    }

    proxy.process_messages();
}


void FstPlugin::finalize_processing(VstInt32 const sample_count) noexcept
{
    send_out_events((int)std::max(0, sample_count - 1));
    proxy.begin_processing();

    if (remaining_samples_before_next_bank_update >= sample_count) {
        remaining_samples_before_next_bank_update -= sample_count;

        return;
    } else if (remaining_samples_before_next_bank_update > 0) {
        remaining_samples_before_next_bank_update = 0;

        return;
    }

    bool const is_dirty = proxy.is_dirty();

    if (MPE_EMULATOR_LIKELY(!(is_dirty || need_bank_update))) {
        return;
    }

    remaining_samples_before_next_bank_update = min_samples_before_next_bank_update;
    need_bank_update = false;
    proxy.clear_dirty_flag();

    size_t const current_program = bank.get_current_program_index();
    std::string const& current_patch = Serializer::serialize(proxy);

    bank[current_program].import(current_patch);

    std::string const& serialized_bank(bank.serialize());

    to_gui_messages.push(
        Message(MessageType::PROGRAM_CHANGED, current_program, current_patch)
    );
    to_gui_messages.push(Message(MessageType::BANK_CHANGED, 0, serialized_bank));

    if (is_dirty) {
        to_gui_messages.push(Message(MessageType::PROXY_WAS_DIRTY));
    }
}


void FstPlugin::send_out_events(int const last_sample_offset) noexcept
{
    size_t next_vst_event_idx = 0;

    memset(&out_events, 0, sizeof(VstEvents_));

    out_events.numEvents = 0;

    for (Proxy::OutEvents::const_iterator i = proxy.out_events.begin(); i != proxy.out_events.end(); ++i) {
        Midi::Event const& midi_event(*i);
        VstMidiEvent* const vst_midi_event = &out_event_buffer[next_vst_event_idx];

        memset(vst_midi_event, 0, sizeof(VstMidiEvent));

        vst_midi_event->type = kVstMidiType;
        vst_midi_event->byteSize = sizeof(VstMidiEvent);
        vst_midi_event->deltaFrames = (
            midi_event.get_sample_offset<int>(sample_rate, last_sample_offset)
        );
        vst_midi_event->midiData[0] = midi_event.command | midi_event.channel;
        vst_midi_event->midiData[1] = midi_event.data_1;
        vst_midi_event->midiData[2] = midi_event.data_2;

        out_events.events[next_vst_event_idx] = vst_midi_event;

        ++next_vst_event_idx;

        if (next_vst_event_idx == OUT_EVENTS_BUFFER_SIZE) {
            out_events.numEvents = (int)next_vst_event_idx;
            host_callback(audioMasterProcessEvents, 0, 0, (void*)&out_events);
            next_vst_event_idx = 0;
        }
    }

    if (next_vst_event_idx != 0) {
        out_events.numEvents = (int)next_vst_event_idx;
        host_callback(audioMasterProcessEvents, 0, 0, (void*)&out_events);
    }
}


void FstPlugin::update_host_display() noexcept
{
    if (need_host_update) {
        need_host_update = false;
        host_callback(audioMasterUpdateDisplay);
    }
}


void FstPlugin::generate_and_add_samples(
        VstInt32 const sample_count,
        float const* const* const in_samples,
        float** out_samples
) noexcept {
    if (sample_count < 1) {
        return;
    }

    prepare_processing(sample_count);
    finalize_processing(sample_count);
}


VstIntPtr FstPlugin::get_chunk(void** chunk, bool is_preset) noexcept
{
    process_internal_messages_in_gui_thread();

    if (is_preset) {
        Bank::Program program;

        program.import(current_patch);
        program.set_name(program_names[current_program_index].get_name());

        current_patch = program.serialize();

        *chunk = (void*)current_patch.c_str();

        return (VstIntPtr)current_patch.length();
    } else {
        *chunk = (void*)serialized_bank.c_str();

        return (VstIntPtr)serialized_bank.length();
    }
}


void FstPlugin::set_chunk(void const* chunk, VstIntPtr const size, bool is_preset) noexcept
{
    process_internal_messages_in_gui_thread();

    std::string buffer((char const*)chunk, (std::string::size_type)size);

    if (is_preset) {
        current_patch = buffer;

        Bank::Program program;
        program.import(current_patch);

        std::string const& name(program.get_name());

        program_names[current_program_index].set_name(name);

        to_audio_string_messages.push(
            Message(MessageType::IMPORT_PATCH, 0, current_patch)
        );
        to_audio_string_messages.push(
            Message(MessageType::RENAME_PROGRAM, 0, name)
        );
    } else {
        serialized_bank = buffer;

        program_names.import_names(serialized_bank);

        to_audio_string_messages.push(
            Message(MessageType::IMPORT_BANK, 0, serialized_bank)
        );
    }
}


void FstPlugin::note_on(
        double const time_offset,
        Midi::Channel const channel,
        Midi::Note const note,
        Midi::Byte const velocity
) noexcept {
}


void FstPlugin::control_change(
        double const time_offset,
        Midi::Channel const channel,
        Midi::Controller const controller,
        Midi::Byte const new_value
) noexcept {
    constexpr Midi::Controller max_cc = (
        (Midi::Controller)Proxy::ControllerId::MAX_MIDI_CC
    );

    had_midi_cc_event = true;

    if (controller <= max_cc) {
        midi_cc_received[(size_t)controller] = true;
    }
}


void FstPlugin::program_change(
        double const time_offset,
        Midi::Channel const channel,
        Midi::Byte const new_program
) noexcept {
}


void FstPlugin::channel_pressure(
        double const time_offset,
        Midi::Channel const channel,
        Midi::Byte const pressure
) noexcept {
    had_midi_cc_event = true;
}


void FstPlugin::pitch_wheel_change(
        double const time_offset,
        Midi::Channel const channel,
        Midi::Word const new_value
) noexcept {
    had_midi_cc_event = true;
}


VstIntPtr FstPlugin::get_program() noexcept
{
    return current_program_index;
}


void FstPlugin::set_program(size_t index) noexcept
{
    current_program_index = index;

    to_audio_messages.push(Message(MessageType::CHANGE_PROGRAM, index));
}


VstIntPtr FstPlugin::get_program_name(char* name, size_t index) noexcept
{
    process_internal_messages_in_gui_thread();

    if (index >= Bank::NUMBER_OF_PROGRAMS) {
        return 0;
    }

    strncpy(
        name,
        program_names[index].get_name().c_str(),
        kVstMaxProgNameLen - 1
    );
    name[kVstMaxProgNameLen - 1] = '\x00';

    return 1;
}


void FstPlugin::get_program_name(char* name) noexcept
{
    process_internal_messages_in_gui_thread();

    strncpy(
        name,
        program_names[current_program_index].get_name().c_str(),
        kVstMaxProgNameLen - 1
    );
    name[kVstMaxProgNameLen - 1] = '\x00';
}


void FstPlugin::set_program_name(const char* name)
{
    process_internal_messages_in_gui_thread();

    to_audio_string_messages.push(Message(MessageType::RENAME_PROGRAM, 0, name));
    program_names[current_program_index].set_name(name);
}


void FstPlugin::open_gui(GUI::PlatformWidget parent_window)
{
    process_internal_messages_in_gui_thread();

    close_gui();
    gui = new GUI(FST_H_VERSION, platform_data, parent_window, proxy, false);
    gui->show();
}


void FstPlugin::gui_idle()
{
    process_internal_messages_in_gui_thread();
    update_host_display();

    /*
    Some hosts (e.g. Ardour 5.12.0) send an effEditIdle message before sending
    the first effEditOpen.
    */
    if (gui == NULL) {
        return;
    }

    gui->idle();
}


void FstPlugin::close_gui()
{
    process_internal_messages_in_gui_thread();

    if (gui == NULL) {
        need_idle();

        return;
    }

    delete gui;

    gui = NULL;
    need_idle();
}


FstPlugin::Parameter::Parameter()
    : short_name("unknown"),
    long_name("unknown"),
    param_id(Proxy::ParamId::INVALID_PARAM_ID),
    controller_id(Proxy::ControllerId::INVALID_CONTROLLER_ID),
    // change_index(-1), /* See FstPlugin::generate_samples() */
    value(0.0f)
{
}


FstPlugin::Parameter::Parameter(
        char const* const short_name,
        char const* const long_name,
        Proxy::ParamId const param_id,
        Proxy::ControllerId const controller_id
) : short_name(short_name),
    long_name(long_name),
    param_id(param_id),
    controller_id(controller_id),
    // change_index(-1), /* See FstPlugin::generate_samples() */
    value(0.0f)
{
}


char const* FstPlugin::Parameter::get_short_name() const noexcept
{
    return short_name;
}


char const* FstPlugin::Parameter::get_long_name() const noexcept
{
    return long_name;
}


Proxy::ParamId FstPlugin::Parameter::get_param_id() const noexcept
{
    return param_id;
}


Proxy::ControllerId FstPlugin::Parameter::get_controller_id() const noexcept
{
    return controller_id;
}


bool FstPlugin::Parameter::is_midi_cc_helper() const noexcept
{
    return controller_id != Proxy::ControllerId::INVALID_CONTROLLER_ID;
}


bool FstPlugin::Parameter::is_exported_param() const noexcept
{
    return param_id != Proxy::ParamId::INVALID_PARAM_ID;
}


/* See FstPlugin::generate_samples() */
// bool FstPlugin::Parameter::needs_host_update() const noexcept
// {
    // return change_index != midi_controller->get_change_index();
// }


float FstPlugin::Parameter::get_value(Proxy const& proxy) const noexcept
{
    if (is_exported_param()) {
        /* See FstPlugin::generate_samples() */
        // change_index = ...

        return proxy.get_param_ratio_atomic(param_id);
    }

    return get_last_set_value();
}


float FstPlugin::Parameter::get_last_set_value() const noexcept
{
    return this->value;
}


void FstPlugin::Parameter::set_value(float const value) noexcept
{
    this->value = value;
}


float FstPlugin::get_parameter(size_t index) noexcept
{
    return parameters[index].get_value(proxy);
}


void FstPlugin::set_parameter(size_t index, float value) noexcept
{
    if (index == PATCH_CHANGED_PARAMETER_INDEX) {
        return;
    }

    Parameter& param = parameters[index];

    param.set_value(value);

    to_audio_messages.push(Message(index, (double)value));
}


bool FstPlugin::is_automatable(size_t index) noexcept
{
    return (
        index != PATCH_CHANGED_PARAMETER_INDEX && index < NUMBER_OF_PARAMETERS
    );
}


void FstPlugin::get_param_label(size_t index, char* buffer) noexcept
{
    process_internal_messages_in_gui_thread();

    buffer[0] = '\x00';
}


void FstPlugin::get_param_display(size_t index, char* buffer) noexcept
{
    process_internal_messages_in_gui_thread();

    Parameter const& param = parameters[index];
    float const value = param.get_value(proxy);

    if (param.is_exported_param()) {
        Strings::param_ratio_to_str(
            proxy, param.get_param_id(), value, buffer, kVstMaxParamStrLen
        );
    } else {
        snprintf(buffer, kVstMaxParamStrLen, "%.2f%%", value * 100.0f);
        buffer[kVstMaxParamStrLen - 1] = '\x00';
    }
}


void FstPlugin::get_param_name(size_t index, char* buffer) noexcept
{
    process_internal_messages_in_gui_thread();

    strncpy(buffer, parameters[index].get_short_name(), kVstMaxParamStrLen);
    buffer[kVstMaxParamStrLen - 1] = '\x00';
}


FstPlugin::Message::Message() : Message(MessageType::NONE)
{
}


FstPlugin::Message::Message(
        MessageType const type,
        size_t const index,
        std::string const& serialized_data
) : serialized_data(serialized_data),
    new_value(0.0),
    index(index),
    type(type)
{
}


FstPlugin::Message::Message(size_t const index, double const new_value)
    : serialized_data(""),
    new_value(new_value),
    index(index),
    type(MessageType::CHANGE_PARAM)
{
}


FstPlugin::MessageType FstPlugin::Message::get_type() const noexcept
{
    return type;
}


size_t FstPlugin::Message::get_index() const noexcept
{
    return index;
}


std::string const& FstPlugin::Message::get_serialized_data() const noexcept
{
    return serialized_data;
}


double FstPlugin::Message::get_new_value() const noexcept
{
    return new_value;
}


/*

Origin of the opcode values for `effIdle` and `audioMasterNeedIdle`
-------------------------------------------------------------------

https://git.iem.at/zmoelnig/FST/-/issues/24

The values for `effIdle` and `audioMasterNeedIdle` were discovered using trial
and error.

REAPER 6.79 keeps sending opcode `53` regularly to the plugin from the UI
thread, even when the plugin's window is closed, and even when the plugin is
bypassed and its track is muted. This seems to suggest that 53 might be the
missing value of `effIdle` in `fst.h`, since other opcodes that one would
expect to be used this frequently (like `effEditIdle` and `effProcessEvent`)
are already known, and there aren't any other unknown opcode names which would
make a lot of sense to be called repeatedly. (Though there might be
unknown-unknown opcodes, but let's be optimistic.)

One way to confirm this would be to see how this opcode interacts with
`audioMasterNeedIdle` which seems to be related, based on its name. The problem
is that the value for `audioMasterNeedIdle` is also unknown.

Let's take a look at JUCE: `audioMasterNeedIdle` is handled in
`modules/juce_audio_processors/format_types/juce_VSTPluginFormat.cpp` like this:

    pointer_sized_int handleCallback (int32 opcode, ...)
    {
        switch (opcode)
        {
            // ...
            case Vst2::audioMasterNeedIdle:                 startTimer (50); break;
            // ...
        }

        return 0;
    }

None of the parameters are used besides the opcode, so there's a chance that we
can get away with calling the host with all other parameters set to `0` and
`NULL`, which means that it's worth trying to brute-force the value for this
opcode. (The highest known host opcode is below `50` in `fst.h`, and
apparently, they are numbered sequentially, and there are around 30 unknown
host opcodes, so the correct value will probably be under `100`.)

How do we know if we found `audioMasterNeedIdle`?

According to `./modules/juce_events/timers/juce_Timer.h`, the `startTimer()`
method will start a timer which regularly calls the `timerCallback()` virtual
method, which in this case looks like this:

    void timerCallback() override
    {
        if (dispatch (Vst2::effIdle, 0, 0, nullptr, 0) == 0)
            stopTimer();
    }

So indeed, `audioMasterNeedIdle` and `effIdle` are related: after receiving a
call with `audioMasterNeedIdle`, the host is supposed to regularly send
`effIdle` messages to the plugin, and keep doing so as long as the plugin
responds with a non-zero value. Once the plugin returns `0`, the timer should
be stopped, and `effIdle` should not get called until the plugin requests it
again.

The problem is that it's impossible to correlate a brute-force guess candidate
for `audioMasterNeedIdle` with opcode `53` if REAPER keeps sending `53` all the
time, even if the plugin responds with `0` to unknown opcodes.

REAPER behaving so differently from what's seen in JUCE probably has to do with
some buggy plugins, and if so, then hopefully it is the result of multiple
tweaks and bugfixes over the years. Fortunately, REAPER's entire changelog is
available on their website (https://www.reaper.fm/whatsnew.txt). Let's see if
it mentions any changes around VST plugins and idle-ness:

    $ grep -i 'vst.*idle' whatsnew.txt
    + VST: fix UI idle processing for bridged VST2 on Linux
    + VST: bumped effEditIdle rate back up to 10hz
    + VST: extraneous effIdle for plugins that dont request it
    + VST: better compatibility with plug-ins that require effIdle
    + VST: updated idle processing behavior, vst 2.3 startprocess/stopprocess support
    + vst: support for plug-ins that require audioMasterIdle

Two of these look interesting with regards to `effIdle`: the "VST: better
compatibility with plug-ins that require effIdle" change belongs to the `v2.001
- October 12 2007` release, and the "VST: extraneous effIdle for plugins that
dont request it" change was released in the `v2.004 - October 19 2007` version.

Both of these versions can still be downloaded from REAPER's old version archive
(https://reaper.fm/download-old.php?ver=2x), and both come as 32 bit
Windows-only binaries, packed into a standard NSIS installer. One advantage of
these installers is that they can be extracted even on a Linux system using the
`7z x reaper2001-install.exe` command, without having to actually run them and
accept any license terms that might prohibit debugging.

Fortunately again, `v2.001` runs well enough in Wine (https://www.winehq.org/)
on Linux. Hopefully this old version with its early `effIdle` support is closer
to the behaviour that's seen in JUCE.

To thest this, the following plugin named `idle.cpp` was made. It only does
some basic housekeeping, the interesting part is that it will attempt to send
`audioMasterNeedIdle` messages when it's initialized, or its GUI is closed
(when the GUI is open, we don't really need `effIdle`, since we have
`effEditIdle`), or when it's suspended or resumed:

    #include <algorithm>
    #include <cstdio>
    #include <windows.h>

    #include "fst.h"

    #define _IDLE_LOG(msg, ...) \
        do { \
            fprintf(stderr, "%s:%d/%s():\t", __FILE__, __LINE__, __FUNCTION__); \
            fprintf(stderr, "TID=%#x\t", (unsigned int)GetCurrentThreadId()); \
            fprintf(stderr, msg, ##__VA_ARGS__); \
            fprintf(stderr, "\n"); \
        } while (false)

    namespace Idle {

    HINSTANCE dll_instance = NULL;
    constexpr int OP_CODE_NAMES_LEN = 78;
    constexpr char const* OP_CODE_NAMES[OP_CODE_NAMES_LEN] = {
        "Open",                         //    0
        "Close",                        //    1
        "SetProgram",                   //    2
        "GetProgram",                   //    3
        "SetProgramName",               //    4
        "GetProgramName",               //    5
        "GetParamLabel",                //    6
        "GetParamDisplay",              //    7
        "GetParamName",                 //    8
        "UNKNOWN-9",                    //    9
        "SetSampleRate",                //   10
        "SetBlockSize",                 //   11
        "MainsChanged",                 //   12
        "EditGetRect",                  //   13
        "EditOpen",                     //   14
        "EditClose",                    //   15
        "UNKNOWN-16",                   //   16
        "UNKNOWN-17",                   //   17
        "UNKNOWN-18",                   //   18
        "EditIdle",                     //   19
        "UNKNOWN-20",                   //   20
        "UNKNOWN-21",                   //   21
        "Identify",                     //   22
        "GetChunk",                     //   23
        "SetChunk",                     //   24
        "ProcessEvents",                //   25
        "CanBeAutomated",               //   26
        "String2Parameter",             //   27
        "UNKNOWN-28",                   //   28
        "GetProgramNameIndexed",        //   29
        "UNKNOWN-30",                   //   30
        "UNKNOWN-31",                   //   31
        "UNKNOWN-32",                   //   32
        "GetInputProperties",           //   33
        "GetOutputProperties",          //   34
        "GetPlugCategory",              //   35
        "UNKNOWN-36",                   //   36
        "UNKNOWN-37",                   //   37
        "UNKNOWN-38",                   //   38
        "UNKNOWN-39",                   //   39
        "UNKNOWN-40",                   //   40
        "UNKNOWN-41",                   //   41
        "SetSpeakerArrangement",        //   42
        "UNKNOWN-43",                   //   43
        "UNKNOWN-44",                   //   44
        "GetEffectName",                //   45
        "UNKNOWN-46",                   //   46
        "GetVendorString",              //   47
        "GetProductString",             //   48
        "GetVendorVersion",             //   49
        "VendorSpecific",               //   50
        "CanDo",                        //   51
        "UNKNOWN-52",                   //   52
        "UNKNOWN-53",                   //   53
        "UNKNOWN-54",                   //   54
        "UNKNOWN-55",                   //   55
        "UNKNOWN-56",                   //   56
        "UNKNOWN-57",                   //   57
        "GetVstVersion",                //   58
        "UNKNOWN-59",                   //   59
        "UNKNOWN-60",                   //   60
        "UNKNOWN-61",                   //   61
        "UNKNOWN-62",                   //   62
        "GetCurrentMidiProgram",        //   63
        "UNKNOWN-64",                   //   64
        "UNKNOWN-65",                   //   65
        "GetMidiNoteName",              //   66
        "UNKNOWN-67",                   //   67
        "UNKNOWN-68",                   //   68
        "GetSpeakerArrangement",        //   69
        "ShellGetNextPlugin",           //   70
        "StartProcess",                 //   71
        "StopProcess",                  //   72
        "SetTotalSampleToProcess",      //   73
        "UNKNOWN-74",                   //   74
        "UNKNOWN-75",                   //   75
        "UNKNOWN-76",                   //   76
        "SetProcessPrecision",          //   77
    };

    class Plugin
    {
        public:
            static constexpr VstInt32 VERSION = 999000;
            static constexpr int WIDTH = 640;
            static constexpr int HEIGHT = 480;

            static AEffect* create_instance(audioMasterCallback host_callback)
            {
                AEffect* effect = new AEffect();

                Plugin* plugin = new Plugin(effect, host_callback);

                memset(effect, 0, sizeof(AEffect));

                effect->dispatcher = &dispatch;
                effect->flags = (
                    effFlagsHasEditor
                    | effFlagsIsSynth
                    | effFlagsCanReplacing
                    | effFlagsCanDoubleReplacing
                    | effFlagsProgramChunks
                );
                effect->magic = kEffectMagic;
                effect->numInputs = 0;
                effect->numOutputs = 2;
                effect->numPrograms = 0;
                effect->numParams = 0;
                effect->object = (void*)plugin;
                effect->process = &process_accumulating;
                effect->processReplacing = &process_replacing;
                effect->processDoubleReplacing = &process_double_replacing;
                effect->getParameter = &get_parameter;
                effect->setParameter = &set_parameter;
                effect->uniqueID = CCONST('i', 'd', 'l', 'e');
                effect->version = VERSION;

                return effect;
            }

            static VstIntPtr VSTCALLBACK dispatch(
                    AEffect* effect,
                    VstInt32 op_code,
                    VstInt32 index,
                    VstIntPtr ivalue,
                    void* pointer,
                    float fvalue
            ) {
                Plugin* plugin = (Idle::Plugin*)effect->object;

                _IDLE_LOG(
                    "plugin=%p, op_code=%d, op_code_name=%s, index=%d, ivalue=%d, fvalue=%f",
                    effect->object,
                    (int)op_code,
                    ((op_code < OP_CODE_NAMES_LEN) ? OP_CODE_NAMES[op_code] : "???"),
                    (int)index,
                    (int)ivalue,
                    fvalue
                );

                switch (op_code) {
                    case effOpen:
                    case effEditClose:
                        plugin->need_idle();
                        return 0;

                    case effMainsChanged:
                        plugin->need_idle();
                        return 0;

                    case 53:
                        return plugin->idle();

                    case effProcessEvents:
                        return 1;

                    case effClose:
                        delete plugin;
                        return 0;

                    case effEditGetRect:
                        *((ERect**)pointer) = &plugin->window_rect;
                        return (VstIntPtr)pointer;

                    case effEditOpen:
                        plugin->open_gui((HWND)pointer);
                        return 1;

                    case effGetChunk:
                        return plugin->get_chunk((void**)pointer, index ? true : false);

                    case effGetPlugCategory:
                        return kPlugCategSynth;

                    case effGetEffectName:
                    case effGetProductString:
                        strncpy((char*)pointer, "Idle", 8);
                        return 1;

                    case effGetVendorString:
                        strncpy((char*)pointer, "Idle", 24);
                        return 1;

                    case effGetVendorVersion:
                        return VERSION;

                    case effGetVstVersion:
                        return kVstVersion;

                    case effIdentify:
                        return CCONST('N', 'v', 'E', 'f');

                    case effCanDo:
                        if (strcmp("receiveVstMidiEvent", (char const*)pointer) == 0) {
                            return 1;
                        }

                        return 0;

                    default:
                        return 0;
                }
            }

            static void VSTCALLBACK process_accumulating(
                    AEffect* effect,
                    float** indata,
                    float** outdata,
                    VstInt32 frames
            ) {
            }

            static void VSTCALLBACK process_replacing(
                    AEffect* effect,
                    float** indata,
                    float** outdata,
                    VstInt32 frames
            ) {
                std::fill_n(outdata[0], frames, 0.0f);
                std::fill_n(outdata[1], frames, 0.0f);
            }

            static void VSTCALLBACK process_double_replacing(
                    AEffect* effect,
                    double** indata,
                    double** outdata,
                    VstInt32 frames
            ) {
                std::fill_n(outdata[0], frames, 0.0);
                std::fill_n(outdata[1], frames, 0.0);
            }

            static float VSTCALLBACK get_parameter(AEffect* effect, VstInt32 index)
            {
                return 0.0f;
            }

            static void VSTCALLBACK set_parameter(
                    AEffect* effect,
                    VstInt32 index,
                    float fvalue
            ) {
            }

            Plugin(AEffect* effect, audioMasterCallback host_callback)
                : effect(effect),
                host_callback(host_callback),
                chunk("IDLE"),
                window(NULL),
                counter(-1)
            {
                window_rect.top = 0;
                window_rect.left = 0;
                window_rect.bottom = HEIGHT;
                window_rect.right = WIDTH;
            }

            ~Plugin()
            {
                close_gui();
            }

            void need_idle()
            {
            }

            VstIntPtr idle() noexcept
            {
                return 0;
            }

            VstIntPtr get_chunk(void** buffer, bool is_preset)
            {
                *buffer = (void*)chunk;
                return 5;
            }

            void open_gui(HWND parent_window)
            {
                window = CreateWindow(
                    TEXT("STATIC"),         // lpClassName
                    TEXT("idle"),           // lpWindowName
                    WS_CHILD | WS_VISIBLE,  // dwStyle
                    0,                      // x
                    0,                      // y
                    WIDTH,                  // nWidth
                    HEIGHT,                 // nHeight
                    parent_window,          // hWndParent
                    NULL,                   // hMenu
                    NULL,                   // hInstance
                    NULL                    // lpParam
                );
            }

            void close_gui()
            {
                if (window != NULL) {
                    DestroyWindow(window);
                    window = NULL;
                }
            }

        private:
            AEffect* const effect;
            audioMasterCallback const host_callback;
            char const* chunk;
            ERect window_rect;
            HWND window;
            int counter;
    };

    }

    extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
    {
        _IDLE_LOG("loaded");
        Idle::dll_instance = hinstDLL;
        return TRUE;
    }

    extern "C" __declspec(dllexport) AEffect* VSTPluginMain(audioMasterCallback host_callback)
    {
        _IDLE_LOG("creating plugin instance");
        return Idle::Plugin::create_instance(host_callback);
    }

This can be cross-compiled on Linux into a 32 bit DLL with the following
`Makefile`, using MinGW:

    idle.dll: idle.o idle.def
        /usr/bin/i686-w64-mingw32-g++ \
            -Wall \
            -shared \
            -static \
            $^ \
            -o idle.dll \
            -lgdi32 -luser32 -lkernel32 -municode -lcomdlg32 -lole32

    idle.o: idle.cpp fst.h
        /usr/bin/i686-w64-mingw32-g++ \
            -DFST_DONT_DEPRECATE_UNKNOWN \
            -D OEMRESOURCE \
            -Wall -Werror \
            -c $< -o $@

The `idle.def` file looks like this:

    LIBRARY idle.dll
    EXPORTS
        VSTPluginMain   @1

But when tested with this old version of REAPER, the plugin never gets
instantiated!

There's a clue in JUCE in
`modules/juce_audio_plugin_client/juce_audio_plugin_client_VST2.cpp`:

    #if ! defined (JUCE_64BIT) && JUCE_MSVC // (can't compile this on win64, but it's not needed anyway with VST2.4)
     extern "C" __declspec (dllexport) int main (Vst2::audioMasterCallback audioMaster)
     {
         return (int) pluginEntryPoint (audioMaster);
     }
    #endif

Apparently, these very old 32 bit plugins might use `main()` instead of
`VSTPluginMain()`, so let's change `idle.cpp`:

    extern "C" __declspec(dllexport) int main(audioMasterCallback host_callback)
    {
        _IDLE_LOG("creating plugin instance");
        return (int)Idle::Plugin::create_instance(host_callback);
    }

And `idle.def`:

    LIBRARY idle.dll
    EXPORTS
        main   @1

This requires adding `-Wno-main` to the compiler options as well in the
`Makefile`:

    idle.o: idle.cpp fst.h
        /usr/bin/i686-w64-mingw32-g++ \
            -DFST_DONT_DEPRECATE_UNKNOWN \
            -D OEMRESOURCE \
            -Wall -Werror \
            -Wno-main \
            -c $< -o $@

Now the plugin is loaded successfully, and it doesn't get flooded with
opcode `53` messages, the brute-force trial and error process can begin.

The idea is to try to invoke `audioMasterNeedIdle` from `effOpen`,
`effEditClose`, and `effMainsChanged`, and count how many `53` opcodes are
received after that. We are going to respond with `1` to opcode `53` messages,
but when the counter reaches `10`, we are going to start responding with `0`.
Assuming that this early version of REAPER behaves similarly to what's seen in
JUCE, the correct guess for `audioMasterNeedIdle` should result in exactly 11
invocations of opcode `53`. (That is, if `effIdle` is really `53`.)

    void need_idle()
    {
        constexpr int audioMasterNeedIdle = ???;

        counter = 0;

        _IDLE_LOG("trying; opcode=%d", audioMasterNeedIdle);

        VstIntPtr result = host_callback(
            effect, audioMasterNeedIdle, 0, 0, NULL, 0.0f
        );

        _IDLE_LOG("  result; result=%lld", (long long int)result);
    }

    VstIntPtr idle() noexcept
    {
        _IDLE_LOG("counter=%d", counter);

        if (counter < 0) {
            // we don't have an ongoing need_idle() test at the moment
            return 0;
        }

        if (counter < 10) {
            ++counter;
            return 1;
        }

        counter = -1;

        return 0;
    }

Long story short, `audioMasterNeedIdle = 14` results in exactly 11
invocations of opcode `53`, confirming that `effIdle = 53`.

*/

}
