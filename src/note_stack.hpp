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

#ifndef MPE_EMULATOR__NOTE_STACK_HPP
#define MPE_EMULATOR__NOTE_STACK_HPP

#include <cstddef>
#include <vector>

#ifdef MPE_EMULATOR_ASSERTIONS
#include <string>
#endif

#include "common.hpp"
#include "midi.hpp"


namespace MpeEmulator
{

/**
 * \brief A stack (LIFO) for unique \c Midi::Channel and \c Midi::Note pairs
 *        where all operations cost O(1) most of the time, including removing an
 *        element by value from the middle.
 */
class NoteStack
{
    public:
        class ChannelStats
        {
            public:
                ChannelStats() noexcept;
                ChannelStats(ChannelStats const& stats) = default;
                ChannelStats(ChannelStats&& stats) = default;

                ChannelStats& operator=(ChannelStats const& stats) noexcept = default;
                ChannelStats& operator=(ChannelStats&& stats) noexcept = default;

#ifdef MPE_EMULATOR_ASSERTIONS
                std::string to_string() const noexcept;
#endif

                Midi::Channel lowest;
                Midi::Channel highest;
                Midi::Channel oldest;
                Midi::Channel newest;
        };

        NoteStack() noexcept;

        void clear() noexcept;
        bool is_empty() const noexcept;
        bool is_top(Midi::Note const note) const noexcept;

        bool find(
            Midi::Note const note,
            Midi::Channel& channel,
            Midi::Byte& velocity
        ) const noexcept;

        void top(
            Midi::Note& note,
            Midi::Channel& channel,
            Midi::Byte& velocity
        ) const noexcept;

        void top(Midi::Note& note, Midi::Channel& channel) const noexcept;

        void oldest(
            Midi::Note& note,
            Midi::Channel& channel,
            Midi::Byte& velocity
        ) const noexcept;

        void oldest(Midi::Note& note, Midi::Channel& channel) const noexcept;

        void lowest(
            Midi::Note& note,
            Midi::Channel& channel,
            Midi::Byte& velocity
        ) const noexcept;

        void lowest(Midi::Note& note, Midi::Channel& channel) const noexcept;

        void highest(
            Midi::Note& note,
            Midi::Channel& channel,
            Midi::Byte& velocity
        ) const noexcept;

        void highest(Midi::Note& note, Midi::Channel& channel) const noexcept;

        void push(
            Midi::Note const note,
            Midi::Channel const channel,
            Midi::Byte const velocity
        ) noexcept;

        void pop(
            Midi::Note& note,
            Midi::Channel& channel,
            Midi::Byte& velocity
        ) noexcept;

        void remove(Midi::Note const note) noexcept;

        Midi::Word get_active_channels() const noexcept;

        void make_stats(ChannelStats& stats) const noexcept;

    private:
        static constexpr Midi::Word INVALID_ITEM = Midi::INVALID_NOTE;

        static constexpr size_t ITEMS = Midi::NOTES;

        // void dump() const noexcept;

        bool is_invalid(Midi::Note const note) const noexcept;

        template<bool should_update_extremes>
        void remove(Midi::Byte const item) noexcept;

        void update_extremes(Midi::Byte const changed_item) noexcept;

        bool is_already_pushed(Midi::Note const note) const noexcept;

        Midi::Byte velocities[ITEMS];
        Midi::Channel channels[ITEMS];

        /*
        Since we have a small, finite number of possible elements, and they are
        unique, we can represent the LIFO container as a pair of arrays which
        contain respectively the next and previous pointers of a finite sized
        doubly linked list, and we can use the values themselves as indices
        within the arrays. This way we can both add, remove, and look up
        elements at any position of the container in constant time.

        In other words:

            next[X] = Y if and only if Y is the next element after X
            previous[Y] = X if and only if next[X] = Y
        */
        Midi::Byte next[ITEMS];
        Midi::Byte previous[ITEMS];

        Midi::Byte head;
        Midi::Byte oldest_;
        Midi::Byte lowest_;
        Midi::Byte highest_;
};

}

#endif
