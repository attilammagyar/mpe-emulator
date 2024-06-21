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

#ifndef MPE_EMULATOR__PLUGIN__FST__PLUGIN_HPP
#define MPE_EMULATOR__PLUGIN__FST__PLUGIN_HPP

#include <string>
#include <bitset>

#include <fst/fst.h>

#include "gui/gui.hpp"

#include "bank.hpp"
#include "common.hpp"
#include "midi.hpp"
#include "spscqueue.hpp"
#include "proxy.hpp"


namespace MpeEmulator
{

class FstPlugin : public Midi::EventHandler
{
    public:
        class Parameter
        {
            public:
                Parameter();

                Parameter(
                    char const* short_name,
                    char const* long_name,
                    Proxy::ParamId const param_id = Proxy::ParamId::INVALID_PARAM_ID,
                    Proxy::ControllerId const controller_id = Proxy::ControllerId::INVALID_CONTROLLER_ID
                );

                Parameter(Parameter const& parameter) = default;
                Parameter(Parameter&& parameter) = default;

                Parameter& operator=(Parameter const& parameter) noexcept = default;
                Parameter& operator=(Parameter&& parameter) noexcept = default;

                char const* get_short_name() const noexcept;
                char const* get_long_name() const noexcept;
                Proxy::ParamId get_param_id() const noexcept;
                Proxy::ControllerId get_controller_id() const noexcept;
                bool is_midi_cc_helper() const noexcept;
                bool is_exported_param() const noexcept;

                // bool needs_host_update() const noexcept; /* See FstPlugin::generate_samples() */

                float get_value(Proxy const& proxy) const noexcept;
                float get_last_set_value() const noexcept;
                void set_value(float const value) noexcept;

            private:
                char const* short_name;
                char const* long_name;
                Proxy::ParamId param_id;
                Proxy::ControllerId controller_id;
                // int change_index; /* See FstPlugin::generate_samples() */
                float value;
        };

        static constexpr size_t NUMBER_OF_PARAMETERS = (
            + (size_t)Proxy::ControllerId::MAX_MIDI_CC
            + 1                                         /* Pitch Wheel */
            + 1                                         /* Channel Pressure */
            + (size_t)Proxy::ParamId::PARAM_ID_COUNT
            + 1                                         /* Dummy Parameter */
        );

        static constexpr size_t PATCH_CHANGED_PARAMETER_INDEX = NUMBER_OF_PARAMETERS - 1;
        static constexpr char const* PATCH_CHANGED_PARAMETER_SHORT_NAME = "Changed";
        static constexpr char const* PATCH_CHANGED_PARAMETER_LONG_NAME = "Settings Changed";

        typedef Parameter Parameters[NUMBER_OF_PARAMETERS];

        /*
        Audio inputs and outputs are not actually used, but some hosts are known
        to not run plugins properly that have 0 audio channels.
        */
        static constexpr VstInt32 IN_CHANNELS = 2;
        static constexpr VstInt32 OUT_CHANNELS = 2;

        static constexpr VstInt32 VERSION = MpeEmulator::Constants::PLUGIN_VERSION_INT;

        static constexpr char const* FST_H_VERSION = (
            "FST "
            MPE_EMULATOR_TO_STRING(FST_MAJOR_VERSION)
            "."
            MPE_EMULATOR_TO_STRING(FST_MINOR_VERSION)
            "."
            MPE_EMULATOR_TO_STRING(FST_MICRO_VERSION)
        );

        static AEffect* create_instance(
            audioMasterCallback const host_callback_ptr,
            GUI::PlatformData const platform_data
        ) noexcept;

        static VstIntPtr VSTCALLBACK dispatch(
            AEffect* effect,
            VstInt32 op_code,
            VstInt32 index,
            VstIntPtr ivalue,
            void* pointer,
            float fvalue
        );

        static void VSTCALLBACK process_accumulating(
            AEffect* effect,
            float** indata,
            float** outdata,
            VstInt32 frames
        );

        static void VSTCALLBACK process_replacing(
            AEffect* effect,
            float** indata,
            float** outdata,
            VstInt32 frames
        );

        static void VSTCALLBACK process_double_replacing(
            AEffect* effect,
            double** indata,
            double** outdata,
            VstInt32 frames
        );

        static float VSTCALLBACK get_parameter(
            AEffect* effect,
            VstInt32 index
        );

        static void VSTCALLBACK set_parameter(
            AEffect* effect,
            VstInt32 index,
            float fvalue
        );

        static void populate_parameters(
            Proxy& proxy,
            Parameters& parameters
        ) noexcept;

        FstPlugin(
            AEffect* const effect,
            audioMasterCallback const host_callback_ptr,
            GUI::PlatformData const platform_data
        ) noexcept;

        ~FstPlugin();

        VstInt32 get_latency_samples() const noexcept;
        void initialize() noexcept;
        void need_idle() noexcept;
        VstIntPtr idle() noexcept;
        void set_sample_rate(double const new_sample_rate) noexcept;
        void set_block_size(VstIntPtr const new_block_size) noexcept;
        void suspend() noexcept;
        void resume() noexcept;
        void process_vst_events(VstEvents const* const events) noexcept;
        void process_vst_midi_event(VstMidiEvent const* const event) noexcept;

        template<typename NumberType>
        void generate_samples(
            VstInt32 const sample_count,
            NumberType** out_samples
        ) noexcept;

        void generate_and_add_samples(
            VstInt32 const sample_count,
            float const* const* const in_samples,
            float** out_samples
        ) noexcept;

        VstIntPtr get_chunk(void** chunk, bool is_preset) noexcept;
        void set_chunk(void const* chunk, VstIntPtr const size, bool is_preset) noexcept;

        void note_on(
            double const time_offset,
            Midi::Channel const channel,
            Midi::Note const note,
            Midi::Byte const velocity
        ) noexcept;

        void control_change(
            double const time_offset,
            Midi::Channel const channel,
            Midi::Controller const controller,
            Midi::Byte const new_value
        ) noexcept;

        void program_change(
            double const time_offset,
            Midi::Channel const channel,
            Midi::Byte const new_program
        ) noexcept;

        void channel_pressure(
            double const time_offset,
            Midi::Channel const channel,
            Midi::Byte const pressure
        ) noexcept;

        void pitch_wheel_change(
            double const time_offset,
            Midi::Channel const channel,
            Midi::Word const new_value
        ) noexcept;

        VstIntPtr get_program() noexcept;
        void set_program(size_t index) noexcept;

        VstIntPtr get_program_name(char* name, size_t index) noexcept;
        void get_program_name(char* name) noexcept;
        void set_program_name(const char* name);

        float get_parameter(size_t index) noexcept;
        void set_parameter(size_t index, float value) noexcept;
        bool is_automatable(size_t index) noexcept;

        void get_param_label(size_t index, char* buffer) noexcept;
        void get_param_display(size_t index, char* buffer) noexcept;
        void get_param_name(size_t index, char* buffer) noexcept;

        void open_gui(GUI::PlatformWidget parent_window);
        void gui_idle();
        void close_gui();

        Proxy proxy;

    private:
        static constexpr double HOST_CC_UI_UPDATE_FREQUENCY = 6.0;
        static constexpr double HOST_CC_UI_UPDATE_FREQUENCY_INV = (
            1.0 / HOST_CC_UI_UPDATE_FREQUENCY
        );

        static constexpr double BANK_UPDATE_FREQUENCY = 3.0;
        static constexpr double BANK_UPDATE_FREQUENCY_INV = (
            1.0 / BANK_UPDATE_FREQUENCY
        );

        static constexpr size_t OUT_EVENTS_BUFFER_SIZE = 16384;

        enum MessageType {
            NONE = 0,

            /* from GUI to Audio */
            CHANGE_PROGRAM = 1,
            RENAME_PROGRAM = 2,
            CHANGE_PARAM = 3,
            IMPORT_PATCH = 4,
            IMPORT_BANK = 5,

            /* from Audio to GUI */
            PROGRAM_CHANGED = 6,
            BANK_CHANGED = 7,
            PARAMS_CHANGED = 8,
            PROXY_WAS_DIRTY = 9,
        };

        class Message
        {
            public:
                Message();

                explicit Message(
                    MessageType const type,
                    size_t const index = 0,
                    std::string const& serialized_data = ""
                );

                Message(size_t const index, double const new_value);

                Message(Message const& message) = default;
                Message(Message&& message) = default;

                Message& operator=(Message const& message) = default;
                Message& operator=(Message&& message) = default;

                MessageType get_type() const noexcept;

                size_t get_index() const noexcept;
                std::string const& get_serialized_data() const noexcept;

                double get_new_value() const noexcept;

            private:
                std::string serialized_data;
                double new_value;
                size_t index;
                MessageType type;
        };

        struct VstEvents_
        {
            int numEvents;
            VstIntPtr _pad;
            VstMidiEvent* events[OUT_EVENTS_BUFFER_SIZE];
        };

        VstIntPtr host_callback(
            VstInt32 op_code,
            VstInt32 index = 0,
            VstIntPtr ivalue = 0,
            void* pointer = NULL,
            float fvalue = 0.0f
        ) const noexcept;

        void clear_received_midi_cc() noexcept;

        void prepare_processing(VstInt32 const sample_count) noexcept;
        void finalize_processing(VstInt32 const sample_count) noexcept;
        void send_out_events(VstInt32 const last_sample_offset) noexcept;

        template<typename SampleType>
        void render_silence(
            VstInt32 const sample_count,
            SampleType** out_samples
        ) noexcept;

        void update_host_display() noexcept;

        void process_internal_messages_in_audio_thread(
            SPSCQueue<Message>& messages
        ) noexcept;

        void process_internal_messages_in_gui_thread() noexcept;

        void handle_change_program(size_t const new_program) noexcept;
        void handle_rename_program(std::string const& name) noexcept;

        void handle_change_param(
            size_t const index,
            double const new_value
        ) noexcept;

        void handle_import_patch(std::string const& patch) noexcept;
        void handle_import_bank(std::string const& serialized_bank) noexcept;

        void handle_program_changed(
            size_t const new_program,
            std::string const& patch
        ) noexcept;

        void handle_bank_changed(std::string const& serialized_bank) noexcept;
        void handle_params_changed() noexcept;
        void handle_proxy_was_dirty() noexcept;

        Parameters parameters;

        AEffect* const effect;
        audioMasterCallback const host_callback_ptr;
        GUI::PlatformData const platform_data;

        ERect window_rect;
        std::bitset<Proxy::ControllerId::CONTROLLER_ID_COUNT> midi_cc_received;
        GUI* gui;
        SPSCQueue<Message> to_audio_messages;
        SPSCQueue<Message> to_audio_string_messages;
        SPSCQueue<Message> to_gui_messages;
        Bank bank;
        Bank program_names;
        VstEvents_ out_events;
        VstMidiEvent out_event_buffer[OUT_EVENTS_BUFFER_SIZE];
        std::string serialized_bank;
        std::string current_patch;
        double sample_rate;
        size_t current_program_index;
        VstInt32 min_samples_before_next_cc_ui_update;
        VstInt32 remaining_samples_before_next_cc_ui_update;
        VstInt32 min_samples_before_next_bank_update;
        VstInt32 remaining_samples_before_next_bank_update;
        VstInt32 prev_logged_op_code;
        char program_name[kVstMaxProgNameLen];
        bool had_midi_cc_event;
        bool received_midi_cc_cleared;
        bool need_bank_update;
        bool need_host_update;
};

}

#endif
