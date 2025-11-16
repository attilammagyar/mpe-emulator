/*
 * This file is part of MPE Emulator.
 * Copyright (C) 2024, 2025  Attila M. Magyar
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

#include <cstddef>
#include <cstdio>
#include <cstring>
#include <string>

#include "test.cpp"

#include "common.hpp"
#include "midi.hpp"


using namespace MpeEmulator;


class MidiEventLogger : public Midi::EventHandler
{
    public:
        void note_off(
                double const time_offset,
                Midi::Channel const channel,
                Midi::Note const note,
                Midi::Byte const velocity
        ) noexcept MPE_EMULATOR_OVERRIDE {
            log_event("NOTE_OFF", time_offset, channel, note, velocity);
        }

        void note_on(
                double const time_offset,
                Midi::Channel const channel,
                Midi::Note const note,
                Midi::Byte const velocity
        ) noexcept MPE_EMULATOR_OVERRIDE {
            log_event("NOTE_ON", time_offset, channel, note, velocity);
        }

        void aftertouch(
                double const time_offset,
                Midi::Channel const channel,
                Midi::Note const note,
                Midi::Byte const pressure
        ) noexcept MPE_EMULATOR_OVERRIDE {
            log_event("AFTERTOUCH", time_offset, channel, note, pressure);
        }

        void control_change(
                double const time_offset,
                Midi::Channel const channel,
                Midi::Controller const controller,
                Midi::Byte const new_value
        ) noexcept MPE_EMULATOR_OVERRIDE {
            log_event("CONTROL_CHANGE", time_offset, channel, controller, new_value);
        }

        void program_change(
                double const time_offset,
                Midi::Channel const channel,
                Midi::Byte const new_program
        ) noexcept MPE_EMULATOR_OVERRIDE {
            log_event("PROGRAM_CHANGE", time_offset, channel, new_program);
        }

        void channel_pressure(
                double const time_offset,
                Midi::Channel const channel,
                Midi::Byte const pressure
        ) noexcept MPE_EMULATOR_OVERRIDE {
            log_event("CHANNEL_PRESSURE", time_offset, channel, pressure);
        }

        void pitch_wheel_change(
                double const time_offset,
                Midi::Channel const channel,
                Midi::Word const new_value
        ) noexcept MPE_EMULATOR_OVERRIDE {
            log_event("PITCH_WHEEL", time_offset, channel, new_value);
        }

        void channel_mode(
                double const time_offset,
                Midi::Channel const channel,
                Midi::Byte const message,
                Midi::Byte const data
        ) noexcept MPE_EMULATOR_OVERRIDE {
            log_event("CHANNEL_MODE", time_offset, channel, message, data);
        }

        std::string events;

    private:
        void log_event(
                char const* const event_name,
                double const time_offset,
                Midi::Channel const channel
        ) {
            char buffer[128];

            snprintf(
                buffer, 128, "%s %.1f 0x%02hhx\n", event_name, time_offset, channel
            );

            events += buffer;
        }

        void log_event(
                char const* const event_name,
                double const time_offset,
                Midi::Channel const channel,
                Midi::Byte const byte
        ) {
            char buffer[128];

            snprintf(
                buffer,
                128,
                "%s %.1f 0x%02hhx 0x%02hhx\n",
                event_name,
                time_offset,
                channel,
                byte
            );

            events += buffer;
        }

        void log_event(
                char const* const event_name,
                double const time_offset,
                Midi::Channel const channel,
                Midi::Byte const byte_1,
                Midi::Byte const byte_2
        ) {
            char buffer[128];

            snprintf(
                buffer,
                128,
                "%s %.1f 0x%02hhx 0x%02hhx 0x%02hhx\n",
                event_name,
                time_offset,
                channel,
                byte_1,
                byte_2
            );

            events += buffer;
        }

        void log_event(
                char const* const event_name,
                double const time_offset,
                Midi::Channel const channel,
                Midi::Word const word
        ) {
            char buffer[128];

            snprintf(
                buffer,
                128,
                "%s %.1f 0x%02hhx 0x%04hx\n",
                event_name,
                time_offset,
                channel,
                word
            );

            events += buffer;
        }
};


void assert_all_bytes_were_processed(
        size_t const buffer_size,
        size_t const processed_bytes
) {
    assert_eq((int)buffer_size, (int)processed_bytes);
}


std::string parse_midi(
        double const time_offset,
        char const* const buffer,
        size_t buffer_size = 0
) {
    MidiEventLogger logger;
    size_t const size = buffer_size == 0 ? strlen(buffer) : buffer_size;
    size_t const processed_bytes = (
        Midi::EventDispatcher<MidiEventLogger>::dispatch_events(
            logger, time_offset, (Midi::Byte const*)buffer, size
        )
    );

    assert_all_bytes_were_processed(size, processed_bytes);

    return logger.events;
}


TEST(parses_known_midi_messages_and_ignores_unknown_and_invalid_ones, {
    assert_eq("NOTE_OFF 1.0 0x06 0x42 0x70\n", parse_midi(1.0, "\x86\x42\x70"));
    assert_eq("NOTE_ON 2.0 0x06 0x42 0x70\n", parse_midi(2.0, "\x96\x42\x70"));
    assert_eq("AFTERTOUCH 3.0 0x06 0x42 0x70\n", parse_midi(3.0, "\xa6\x42\x70"));
    assert_eq("CONTROL_CHANGE 4.0 0x06 0x01 0x70\n", parse_midi(4.0, "\xb6\x01\x70"));
    assert_eq("PROGRAM_CHANGE 5.0 0x06 0x01\n", parse_midi(5.0, "\xc6\x01"));
    assert_eq("CHANNEL_PRESSURE 6.0 0x06 0x42\n", parse_midi(6.0, "\xd6\x42"));
    assert_eq("PITCH_WHEEL 7.0 0x06 0x0abc\n", parse_midi(7.0, "\xe6\x3c\x15"));
    assert_eq("CHANNEL_MODE 8.0 0x06 0x78 0x00\n", parse_midi(8.0, "\xb6\x78\x00", 3));
    assert_eq("CHANNEL_MODE 9.0 0x06 0x79 0x42\n", parse_midi(9.0, "\xb6\x79\x42"));
    assert_eq(
        "NOTE_ON 10.0 0x06 0x42 0x70\n",
        parse_midi(10.0, "\x01\xff\x7f\x7f\x86\x99\xff\x96\x42\x70\xff")
    );
})


TEST(running_status, {
    assert_eq(
        (
            "NOTE_ON 1.0 0x07 0x61 0x70\n"
            "NOTE_OFF 1.0 0x07 0x61 0x40\n"
            "NOTE_ON 1.0 0x07 0x62 0x71\n"
            "NOTE_ON 1.0 0x07 0x63 0x72\n"
            "NOTE_ON 1.0 0x07 0x64 0x73\n"
            "CONTROL_CHANGE 1.0 0x07 0x01 0x60\n"
            "CONTROL_CHANGE 1.0 0x07 0x01 0x61\n"
            "CONTROL_CHANGE 1.0 0x07 0x01 0x62\n"
        ),
        parse_midi(
            1.0,
            (
                "\x97\x61\x70"
                    "\x61\x00"
                    "\x62\x71"
                    "\x63\x72"
                    "\x64\x73"
                "\xb7\x01\x60"
                    "\x01\x61"
                    "\x01\x62"
            ),
            18
        )
    );
})


TEST(type_conversions, {
    assert_eq(0, Midi::float_to_byte<float>(-0.1f));
    assert_eq(0, Midi::float_to_byte<float>(0.0f));
    assert_eq(63, Midi::float_to_byte<float>(63.0f / 127.0f));
    assert_eq(127, Midi::float_to_byte<float>(1.0f));
    assert_eq(127, Midi::float_to_byte<float>(1.1f));

    assert_eq(0, Midi::float_to_word<float>(-0.1f));
    assert_eq(0, Midi::float_to_word<float>(0.0f));
    assert_eq(8192, Midi::float_to_word<float>(0.5f));
    assert_eq(16383, Midi::float_to_word<float>(1.0f));
    assert_eq(16383, Midi::float_to_word<float>(1.1f));

    assert_eq(0.0f, Midi::byte_to_float<float>(0), 0.0001f);
    assert_eq(63.0f / 127.0f, Midi::byte_to_float<float>(63), 0.0001f);
    assert_eq(1.0f, Midi::byte_to_float<float>(127), 0.0001f);
    assert_eq(1.0f, Midi::byte_to_float<float>(128), 0.0001f);

    assert_eq(0.0f, Midi::word_to_float<float>(0), 0.0001f);
    assert_eq(0.5f, Midi::word_to_float<float>(8192), 0.0001f);
    assert_eq(1.0f, Midi::word_to_float<float>(16383), 0.0001f);
    assert_eq(1.0f, Midi::word_to_float<float>(16384), 0.0001f);
})


void assert_event_sample_offset(
        int const expected_offset,
        double const time_offset,
        double const sample_rate,
        int const last_sample_offset
) {
    Midi::Event event(time_offset, Midi::NOTE_OFF, 1);

    assert_eq(
        expected_offset,
        event.get_sample_offset(sample_rate, last_sample_offset),
        "time_offset=%f, sample_rate=%f, last_sample_offset=%d",
        time_offset,
        sample_rate,
        last_sample_offset
    );
}


TEST(event_time_offset_to_sample_offset_conversion, {
    assert_event_sample_offset(0, 0.0, 44100.0, 255);
    assert_event_sample_offset(0, -0.0, 44100.0, 255);
    assert_event_sample_offset(0, -1.0, 44100.0, 255);
    assert_event_sample_offset(0, 0.000001, 44100.0, 255);
    assert_event_sample_offset(255, 1.0, 44100.0, 255);
    assert_event_sample_offset(100, 0.01, 10000.0, 255);
    assert_event_sample_offset(100, 0.999999, 100.0, 999);
})
