/*
 * This file is part of MPE Emulator.
 * Copyright (C) 2023, 2024  Attila M. Magyar
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

#ifndef MPE_EMULATOR__PLUGIN__VST3__PLUGIN_HPP
#define MPE_EMULATOR__PLUGIN__VST3__PLUGIN_HPP

#include <string>
#include <vector>

#include <vst3sdk/pluginterfaces/gui/iplugview.h>
#include <vst3sdk/pluginterfaces/vst/ivstmessage.h>
#include <vst3sdk/pluginterfaces/vst/ivstmidicontrollers.h>
#include <vst3sdk/pluginterfaces/vst/vsttypes.h>
#include <vst3sdk/public.sdk/source/common/pluginview.h>
#include <vst3sdk/public.sdk/source/main/pluginfactory.h>
#include <vst3sdk/public.sdk/source/vst/vstaudioeffect.h>
#include <vst3sdk/public.sdk/source/vst/vsteditcontroller.h>

#include "gui/gui.hpp"

#include "common.hpp"
#include "midi.hpp"
#include "proxy.hpp"


using namespace Steinberg;


namespace MpeEmulator
{

class Vst3Plugin
{
    public:
        static constexpr char const* MSG_CTL_READY = "MpeEmulatorCtl";

        static constexpr char const* MSG_SHARE_PROXY = "MpeEmulatorProxy";
        static constexpr char const* MSG_SHARE_PROXY_PROXY = "Proxy";

        static constexpr char const* MSG_PROXY_DIRTY = "MpeEmulatorDirty";

        static constexpr Vst::ParamID SETTINGS_CHANGED_PARAM_ID = 8192;

        static std::string read_stream(IBStream* stream);

        class Event
        {
            public:
                enum Type {
                    UNDEFINED = 0,
                    NOTE_ON = 1,
                    NOTE_PRESSURE = 2,
                    NOTE_OFF = 3,
                    PITCH_WHEEL = 4,
                    CONTROL_CHANGE = 5,
                    CHANNEL_PRESSURE = 6,
                    PARAM_CHANGE = 7,
                };

                Event();
                Event(
                    Type const type,
                    double const time_offset,
                    Midi::Byte const note_or_ctl,
                    Midi::Channel const channel,
                    double const velocity_or_value
                );
                Event(Event const& event) = default;
                Event(Event&& event) = default;

                Event& operator=(Event const& event) noexcept = default;
                Event& operator=(Event&& event) noexcept = default;
                bool operator<(Event const& event) const noexcept;

                double time_offset;
                double velocity_or_value;
                Type type;
                Midi::Byte note_or_ctl;
                Midi::Channel channel;
        };

        class Processor : public Vst::AudioEffect
        {
            public:
                static FUID const ID;

                static FUnknown* createInstance(void* unused);

                Processor();

                tresult PLUGIN_API initialize(FUnknown* context) SMTG_OVERRIDE;

                tresult PLUGIN_API setBusArrangements(
                    Vst::SpeakerArrangement* inputs,
                    int32 number_of_inputs,
                    Vst::SpeakerArrangement* outputs,
                    int32 number_of_outputs
                ) SMTG_OVERRIDE;

                tresult PLUGIN_API connect(IConnectionPoint* other) SMTG_OVERRIDE;
                tresult PLUGIN_API notify(Vst::IMessage* message) SMTG_OVERRIDE;

                tresult PLUGIN_API canProcessSampleSize(int32 symbolic_sample_size) SMTG_OVERRIDE;
                tresult PLUGIN_API setupProcessing(Vst::ProcessSetup& setup) SMTG_OVERRIDE;
                tresult PLUGIN_API setActive(TBool state) SMTG_OVERRIDE;
                tresult PLUGIN_API process(Vst::ProcessData& data) SMTG_OVERRIDE;

                uint32 PLUGIN_API getLatencySamples() SMTG_OVERRIDE;
                uint32 PLUGIN_API getTailSamples() SMTG_OVERRIDE;

                tresult PLUGIN_API setState(IBStream* state) SMTG_OVERRIDE;
                tresult PLUGIN_API getState(IBStream* state) SMTG_OVERRIDE;

            private:
                void share_proxy() noexcept;

                void collect_param_change_events(Vst::ProcessData& data) noexcept;

                void collect_param_change_events_as_midi_ctl(
                    Vst::IParamValueQueue* const param_queue,
                    Event::Type const event_type,
                    Midi::Byte const midi_controller
                ) noexcept;

                void collect_param_change_events_as_exported_param(
                    Vst::IParamValueQueue* const param_queue,
                    Vst::ParamID const param_tag
                ) noexcept;

                void collect_note_events(Vst::ProcessData& data) noexcept;
                void process_events() noexcept;
                void process_event(Event const& event) noexcept;
                void generate_out_events(
                    Vst::IEventList& queue,
                    int32 const last_sample_offset
                ) noexcept;

                void initialize_cc_event(
                    Vst::Event& vst_event,
                    int32 const sample_offset,
                    int32 const last_sample_offset,
                    uint8 const control_number,
                    uint8 const channel,
                    int8 const value_1,
                    int8 const value_2,
                    bool const is_pre_note_on_setup
                ) const noexcept;

                Midi::Byte float_to_midi_byte(double const number) const noexcept;
                Midi::Word float_to_midi_word(double const number) const noexcept;

                void generate_samples(Vst::ProcessData& data) noexcept;

                void import_patch(std::string const& serialized) noexcept;

                Proxy proxy;
                std::vector<Event> events;
                double sample_rate;
                size_t new_program;

            public:
                /* No need to check the SDK. */
                // cppcheck-suppress unknownMacro
                OBJ_METHODS(Processor, Vst::AudioEffect)
                DEFINE_INTERFACES
                END_DEFINE_INTERFACES(Vst::AudioEffect)
                REFCOUNT_METHODS(Vst::AudioEffect)
        };

        class Controller;

        class GUI : public CPluginView
        {
            public:
                explicit GUI(Proxy& proxy);

                virtual ~GUI();

                tresult PLUGIN_API isPlatformTypeSupported(FIDString type) SMTG_OVERRIDE;
                tresult PLUGIN_API canResize() SMTG_OVERRIDE;

                virtual void attachedToParent() override;
                virtual void removedFromParent() override;

            private:
                static ViewRect const rect;

                void initialize();
                void show_if_needed();

                Proxy& proxy;
                MpeEmulator::GUI* gui;

                void* run_loop;
                void* event_handler;
                void* timer_handler;
        };

        class Controller : public Vst::EditControllerEx1, public Vst::IMidiMapping
        {
            public:
                static FUID const ID;

                static FUnknown* createInstance(void* unused);

                Controller();

                virtual ~Controller();

                tresult PLUGIN_API initialize(FUnknown* context) SMTG_OVERRIDE;

                tresult PLUGIN_API connect(IConnectionPoint* other) SMTG_OVERRIDE;
                tresult PLUGIN_API notify(Vst::IMessage* message) SMTG_OVERRIDE;

                tresult PLUGIN_API getMidiControllerAssignment(
                    int32 bus_index,
                    int16 channel,
                    Vst::CtrlNumber midi_controller_number,
                    Vst::ParamID& id
                ) SMTG_OVERRIDE;

                IPlugView* PLUGIN_API createView(FIDString name) SMTG_OVERRIDE;

                tresult PLUGIN_API setComponentState(IBStream* state) SMTG_OVERRIDE;

            private:
                Vst::Parameter* create_midi_ctl_param(
                    Proxy::ControllerId const controller_id,
                    Vst::ParamID const param_id
                ) const;

                Vst::Parameter* create_exported_param(
                    Proxy const& proxy,
                    Proxy::ParamId const param_id
                ) const;

                Vst::Parameter* set_up_patch_changed_param() const;

                void update_params();

                Proxy* proxy;

            public:
                OBJ_METHODS(Controller, Vst::EditControllerEx1)
                DEFINE_INTERFACES
                    DEF_INTERFACE(Vst::IMidiMapping)
                END_DEFINE_INTERFACES(Vst::EditControllerEx1)
                REFCOUNT_METHODS(Vst::EditControllerEx1)
        };

    private:
        static Vst::ParamID proxy_param_id_to_vst3_param_tag(
            Proxy::ParamId const param_id
        );

        static Proxy::ParamId vst3_param_tag_to_proxy_param_id(
            Vst::ParamID const param_tag
        );
};

}

#endif
