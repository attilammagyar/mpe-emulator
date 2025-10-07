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

#ifndef MPE_EMULATOR__MIDI_HPP
#define MPE_EMULATOR__MIDI_HPP

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>

#ifdef MPE_EMULATOR_ASSERTIONS
#include <cstdio>
#include <cstring>
#include <string>
#endif

#include "common.hpp"


namespace MpeEmulator { namespace Midi
{

typedef uint8_t Byte;
typedef uint16_t Word;

typedef Byte Note;
typedef Byte Channel;
typedef Byte Controller;
typedef Byte Command;


template<typename FloatType>
inline Byte float_to_byte(FloatType const value) noexcept
{
    return (Byte)std::min(127, std::max(0, (int)std::round(value * 127.0f)));
}


template<typename FloatType>
inline Word float_to_word(FloatType const value) noexcept
{
    return (Word)std::min(16383, std::max(0, (int)std::round(value * 16383.0)));
}


template<typename FloatType>
inline FloatType byte_to_float(Byte const value) noexcept
{
    constexpr FloatType scale = (FloatType)(1.0 / 127.0);

    return std::min((FloatType)1.0, (FloatType)value * scale);
}


template<typename FloatType>
inline FloatType word_to_float(Word const value) noexcept
{
    constexpr FloatType scale = (FloatType)(1.0 / 16383.0);

    return std::min((FloatType)1.0, (FloatType)value * scale);
}


class EventHandler
{
    public:
        EventHandler() : running_status(0) {}

        void note_off(
            double const time_offset,
            Channel const channel,
            Note const note,
            Byte const velocity
        ) noexcept {}

        void note_on(
            double const time_offset,
            Channel const channel,
            Note const note,
            Byte const velocity
        ) noexcept {}

        void aftertouch(
            double const time_offset,
            Channel const channel,
            Note const note,
            Byte const pressure
        ) noexcept {}

        void control_change(
            double const time_offset,
            Channel const channel,
            Controller const controller,
            Byte const new_value
        ) noexcept {}

        void program_change(
            double const time_offset,
            Channel const channel,
            Byte const new_program
        ) noexcept {}

        void channel_pressure(
            double const time_offset,
            Channel const channel,
            Byte const pressure
        ) noexcept {}

        void pitch_wheel_change(
            double const time_offset,
            Channel const channel,
            Word const new_value
        ) noexcept {}

        void channel_mode(
            double const time_offset,
            Channel const channel,
            Byte const message,
            Byte const data
        ) noexcept {}

        Byte running_status;
};


template<class EventHandlerClass>
class EventDispatcher
{
    public:
        /**
         * \brief Parse and dispatch the events found in the buffer.
         *
         * \sa dispatch_event()
         */
        static size_t dispatch_events(
            EventHandlerClass& event_handler,
            double const time_offset,
            Byte const* const buffer,
            size_t const buffer_size
        ) noexcept;

        /**
         * \brief Parse and dispatch the first event that can be read from the
         *        buffer.
         *
         * If the \c running_status member of the \c EventHandlerClass object
         * indicates a previously established valid running status (MSB is 1),
         * then data bytes (MSB is 0) at the beginning of the buffer are parsed
         * as if a new event with the same status byte was received. Otherwise
         * data bytes at the beginning of the buffer are skipped over.
         *
         * \return Number of bytes processed.
         */
        static size_t dispatch_event(
            EventHandlerClass& event_handler,
            double const time_offset,
            Byte const* const buffer,
            size_t const buffer_size
        ) noexcept;

    private:
        static constexpr Byte STATUS_MASK = 0x80;
        static constexpr Byte MESSAGE_TYPE_MASK = 0xf0;
        static constexpr Byte CHANNEL_MASK = 0x0f;

        static bool is_status_byte(Byte const byte) noexcept;
        static bool is_data_byte(Byte const byte) noexcept;

        static size_t process_note_off(
            EventHandlerClass& event_handler,
            double const time_offset,
            Byte const channel,
            Byte const* const buffer,
            size_t const buffer_size,
            size_t& next_byte
        ) noexcept;

        static size_t process_note_on(
            EventHandlerClass& event_handler,
            double const time_offset,
            Byte const channel,
            Byte const* const buffer,
            size_t const buffer_size,
            size_t& next_byte
        ) noexcept;

        static size_t process_aftertouch(
            EventHandlerClass& event_handler,
            double const time_offset,
            Byte const channel,
            Byte const* const buffer,
            size_t const buffer_size,
            size_t& next_byte
        ) noexcept;

        static size_t process_control_change(
            EventHandlerClass& event_handler,
            double const time_offset,
            Byte const channel,
            Byte const* const buffer,
            size_t const buffer_size,
            size_t& next_byte
        ) noexcept;

        static size_t process_program_change(
            EventHandlerClass& event_handler,
            double const time_offset,
            Byte const channel,
            Byte const* const buffer,
            size_t const buffer_size,
            size_t& next_byte
        ) noexcept;

        static size_t process_channel_pressure(
            EventHandlerClass& event_handler,
            double const time_offset,
            Byte const channel,
            Byte const* const buffer,
            size_t const buffer_size,
            size_t& next_byte
        ) noexcept;

        static size_t process_pitch_bend_change(
            EventHandlerClass& event_handler,
            double const time_offset,
            Byte const channel,
            Byte const* const buffer,
            size_t const buffer_size,
            size_t& next_byte
        ) noexcept;

        static bool parse_data_byte(
            Byte const* const buffer,
            size_t const buffer_size,
            size_t& next_byte,
            Byte& byte
        ) noexcept;

        static bool parse_data_bytes(
            Byte const* const buffer,
            size_t const buffer_size,
            size_t& next_byte,
            Byte& byte_1,
            Byte& byte_2
        ) noexcept;

        static size_t skip_remaining_data_bytes(
            Byte const* const buffer,
            size_t const buffer_size,
            size_t& next_byte
        ) noexcept;
};


constexpr Channel CHANNEL_MAX                           = 15;
constexpr Channel CHANNELS                              = CHANNEL_MAX + 1;
constexpr Channel INVALID_CHANNEL                       = 255;

constexpr Note NOTE_MAX                                 = 127;
constexpr Note NOTES                                    = NOTE_MAX + 1;
constexpr Note INVALID_NOTE                             = 255;

constexpr Controller DATA_ENTRY_MSB                     = 0x06;
constexpr Controller RPN_LSB                            = 0x64;
constexpr Controller RPN_MSB                            = 0x65;
constexpr Controller MAX_CONTROLLER_ID                  = 0x7f;

constexpr Command NOTE_OFF                              = 0x80;
constexpr Command NOTE_ON                               = 0x90;
constexpr Command AFTERTOUCH                            = 0xa0;
constexpr Command CONTROL_CHANGE                        = 0xb0;
constexpr Command PROGRAM_CHANGE                        = 0xc0;
constexpr Command CHANNEL_PRESSURE                      = 0xd0;
constexpr Command PITCH_BEND_CHANGE                     = 0xe0;

constexpr Command CONTROL_CHANGE_ALL_SOUND_OFF          = 0x78;


class Event
{
    public:
        Event() noexcept
            : time_offset(0.0),
            value(0.0),
            command(NOTE_OFF),
            channel(0),
            data_1(0),
            data_2(0)
        {
        }

        Event(
                double const time_offset,
                Command const command,
                Channel const channel,
                Byte const data_1 = 0,
                Byte const data_2 = 0,
                double const value = 0.0,
                bool const is_pre_note_on_setup = false
        ) noexcept
            : time_offset(time_offset),
            value(value),
            command(command),
            channel(channel),
            data_1(data_1),
            data_2(data_2),
            is_pre_note_on_setup(is_pre_note_on_setup)
        {
        }

        Event(Event const& event) noexcept = default;
        Event(Event&& event) noexcept = default;

        Event& operator=(Event const& event) noexcept = default;
        Event& operator=(Event&& event) noexcept = default;

        template<typename Integer>
        Integer get_sample_offset(
                double const sample_rate,
                Integer const last_sample_offset
        ) const noexcept {
            return std::min(
                last_sample_offset,
                std::max(0, (Integer)std::round(time_offset * sample_rate))
            );
        }

#ifdef MPE_EMULATOR_ASSERTIONS
        std::string to_string() const noexcept
        {
            constexpr size_t buffer_size = 128;
            char const* command_str;
            char buffer[buffer_size];

            switch (this->command) {
                case NOTE_OFF:          command_str = "NOTE_OFF"; break;
                case NOTE_ON:           command_str = "NOTE_ON"; break;
                case AFTERTOUCH:        command_str = "AFTERTOUCH"; break;
                case CONTROL_CHANGE:    command_str = "CONTROL_CHANGE"; break;
                case PROGRAM_CHANGE:    command_str = "PROGRAM_CHANGE"; break;
                case CHANNEL_PRESSURE:  command_str = "CHANNEL_PRESSURE"; break;
                case PITCH_BEND_CHANGE: command_str = "PITCH_BEND_CHANGE"; break;
                default:                command_str = "UNDEFINED"; break;
            }

            snprintf(
                buffer,
                buffer_size,
                "t=%.3f cmd=%s ch=%hhu d1=0x%02hhx d2=0x%02hhx (v=%.3f)%s",
                time_offset,
                command_str,
                channel,
                data_1,
                data_2,
                value,
                is_pre_note_on_setup ? " pre-NOTE_ON setup" : ""
            );

            return std::string(buffer);
        }
#endif

        double time_offset;
        double value;
        Command command;
        Channel channel;
        Byte data_1;
        Byte data_2;
        bool is_pre_note_on_setup;
};


template<class EventHandlerClass>
size_t EventDispatcher<EventHandlerClass>::dispatch_events(
        EventHandlerClass& event_handler,
        double const time_offset,
        Byte const* const buffer,
        size_t const buffer_size
) noexcept {
    size_t next_byte = 0;

    while (next_byte != buffer_size) {
        next_byte += dispatch_event(
            event_handler, time_offset, &buffer[next_byte], buffer_size - next_byte
        );
    }

    return next_byte;
}


template<class EventHandlerClass>
size_t EventDispatcher<EventHandlerClass>::dispatch_event(
        EventHandlerClass& event_handler,
        double const time_offset,
        Byte const* const buffer,
        size_t const buffer_size
) noexcept {
    size_t next_byte = 0;

    if (buffer_size < 1) {
        return next_byte;
    }

    Byte status = buffer[next_byte];

    if (is_status_byte(status)) {
        ++next_byte;
        event_handler.running_status = status;
    } else {
        status = event_handler.running_status;

        if (!is_status_byte(status)) {
            return skip_remaining_data_bytes(buffer, buffer_size, next_byte);
        }
    }

    Command msg_type = status & MESSAGE_TYPE_MASK;
    Channel channel = status & CHANNEL_MASK;

    switch (msg_type) {
        case NOTE_OFF:
            return process_note_off(
                event_handler, time_offset, channel, buffer, buffer_size, next_byte
            );

        case NOTE_ON:
            return process_note_on(
                event_handler, time_offset, channel, buffer, buffer_size, next_byte
            );

        case AFTERTOUCH:
            return process_aftertouch(
                event_handler, time_offset, channel, buffer, buffer_size, next_byte
            );

        case CONTROL_CHANGE:
            return process_control_change(
                event_handler, time_offset, channel, buffer, buffer_size, next_byte
            );

        case PROGRAM_CHANGE:
            return process_program_change(
                event_handler, time_offset, channel, buffer, buffer_size, next_byte
            );

        case CHANNEL_PRESSURE:
            return process_channel_pressure(
                event_handler, time_offset, channel, buffer, buffer_size, next_byte
            );

        case PITCH_BEND_CHANGE:
            return process_pitch_bend_change(
                event_handler, time_offset, channel, buffer, buffer_size, next_byte
            );

        default:
            return skip_remaining_data_bytes(buffer, buffer_size, next_byte);
    }

    return next_byte;
}


template<class EventHandlerClass>
bool EventDispatcher<EventHandlerClass>::is_status_byte(Byte const byte) noexcept
{
    return (byte & STATUS_MASK) != 0;
}


template<class EventHandlerClass>
bool EventDispatcher<EventHandlerClass>::is_data_byte(Byte const byte) noexcept
{
    return (byte & STATUS_MASK) == 0;
}


template<class EventHandlerClass>
size_t EventDispatcher<EventHandlerClass>::process_note_off(
        EventHandlerClass& event_handler,
        double const time_offset,
        Byte const channel,
        Byte const* const buffer,
        size_t const buffer_size,
        size_t& next_byte
) noexcept {
    Byte note;
    Byte velocity;

    if (!parse_data_bytes(buffer, buffer_size, next_byte, note, velocity)) {
        return next_byte;
    }

    event_handler.note_off(time_offset, channel, (Note)note, velocity);

    return next_byte;
}


template<class EventHandlerClass>
size_t EventDispatcher<EventHandlerClass>::process_note_on(
        EventHandlerClass& event_handler,
        double const time_offset,
        Byte const channel,
        Byte const* const buffer,
        size_t const buffer_size,
        size_t& next_byte
) noexcept {
    Byte note;
    Byte velocity;

    if (!parse_data_bytes(buffer, buffer_size, next_byte, note, velocity)) {
        return next_byte;
    }

    if (velocity == 0) {
        event_handler.note_off(time_offset, channel, (Note)note, 64);
    } else {
        event_handler.note_on(time_offset, channel, (Note)note, velocity);
    }

    return next_byte;
}


template<class EventHandlerClass>
size_t EventDispatcher<EventHandlerClass>::process_aftertouch(
        EventHandlerClass& event_handler,
        double const time_offset,
        Byte const channel,
        Byte const* const buffer,
        size_t const buffer_size,
        size_t& next_byte
) noexcept {
    Byte note;
    Byte pressure;

    if (!parse_data_bytes(buffer, buffer_size, next_byte, note, pressure)) {
        return next_byte;
    }

    event_handler.aftertouch(time_offset, channel, (Note)note, pressure);

    return next_byte;
}


template<class EventHandlerClass>
size_t EventDispatcher<EventHandlerClass>::process_control_change(
        EventHandlerClass& event_handler,
        double const time_offset,
        Byte const channel,
        Byte const* const buffer,
        size_t const buffer_size,
        size_t& next_byte
) noexcept {
    Byte d1;
    Byte d2;

    if (!parse_data_bytes(buffer, buffer_size, next_byte, d1, d2)) {
        return next_byte;
    }

    if (d1 < CONTROL_CHANGE_ALL_SOUND_OFF) {
        /*
        Interpretation of MIDI CC messages that belong to special controllers
        (e.g. sustain pedal) is left for the event handler. This aligns with the
        restrictions that are imposed by hosts which swallow most of the raw CC
        messages and instead, require plugins to export parameters that can be
        assigned to MIDI controllers (for example, FL Studio 21).
        */

        event_handler.control_change(time_offset, channel, (Controller)d1, d2);
    } else {
        event_handler.channel_mode(time_offset, channel, d1, d2);
    }

    return next_byte;
}


template<class EventHandlerClass>
size_t EventDispatcher<EventHandlerClass>::process_program_change(
        EventHandlerClass& event_handler,
        double const time_offset,
        Byte const channel,
        Byte const* const buffer,
        size_t const buffer_size,
        size_t& next_byte
) noexcept {
    Byte new_program;

    if (!parse_data_byte(buffer, buffer_size, next_byte, new_program)) {
        return next_byte;
    }

    event_handler.program_change(time_offset, channel, new_program);

    return next_byte;
}


template<class EventHandlerClass>
size_t EventDispatcher<EventHandlerClass>::process_channel_pressure(
        EventHandlerClass& event_handler,
        double const time_offset,
        Byte const channel,
        Byte const* const buffer,
        size_t const buffer_size,
        size_t& next_byte
) noexcept {
    Byte pressure;

    if (!parse_data_byte(buffer, buffer_size, next_byte, pressure)) {
        return next_byte;
    }

    event_handler.channel_pressure(time_offset, channel, pressure);

    return next_byte;
}


template<class EventHandlerClass>
size_t EventDispatcher<EventHandlerClass>::process_pitch_bend_change(
        EventHandlerClass& event_handler,
        double const time_offset,
        Byte const channel,
        Byte const* const buffer,
        size_t const buffer_size,
        size_t& next_byte
) noexcept {
    Byte lsb;
    Byte msb;

    if (!parse_data_bytes(buffer, buffer_size, next_byte, lsb, msb)) {
        return next_byte;
    }

    event_handler.pitch_wheel_change(time_offset, channel, (msb << 7) | lsb);

    return next_byte;
}


template<class EventHandlerClass>
bool EventDispatcher<EventHandlerClass>::parse_data_bytes(
        Byte const* const buffer,
        size_t const buffer_size,
        size_t& next_byte,
        Byte& byte_1,
        Byte& byte_2
) noexcept {
    return (
        parse_data_byte(buffer, buffer_size, next_byte, byte_1)
        && parse_data_byte(buffer, buffer_size, next_byte, byte_2)
    );
}


template<class EventHandlerClass>
bool EventDispatcher<EventHandlerClass>::parse_data_byte(
        Byte const* const buffer,
        size_t const buffer_size,
        size_t& next_byte,
        Byte& byte
) noexcept {
    if (next_byte >= buffer_size || !is_data_byte(buffer[next_byte])) {
        return false;
    }

    byte = buffer[next_byte];
    ++next_byte;

    return true;
}


template<class EventHandlerClass>
size_t EventDispatcher<EventHandlerClass>::skip_remaining_data_bytes(
        Byte const* const buffer,
        size_t const buffer_size,
        size_t& next_byte
) noexcept {
    while (next_byte < buffer_size && is_data_byte(buffer[next_byte])) {
        ++next_byte;
    }

    return next_byte;
}

} }

#endif
