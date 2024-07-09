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

        typedef Midi::Channel Channels[Midi::CHANNELS];
        typedef Midi::Channel ChannelsByNotes[Midi::NOTES];

        NoteStack() noexcept;

        void clear() noexcept;
        bool is_empty() const noexcept;
        bool is_top(Midi::Note const note) const noexcept;
        bool find(Midi::Note const note) const noexcept;
        Midi::Note top() const noexcept;
        Midi::Note oldest() const noexcept;
        Midi::Note lowest() const noexcept;
        Midi::Note highest() const noexcept;
        void push(Midi::Note const note) noexcept;
        Midi::Note pop() noexcept;
        void remove(Midi::Note const note) noexcept;

        void collect_active_channels(
            ChannelsByNotes const& channels_by_notes,
            Channels& channels,
            size_t& count
        ) const noexcept;

        void make_stats(
            ChannelsByNotes const& channels_by_notes,
            ChannelStats& stats
        ) const noexcept;

    private:
        static constexpr size_t ITEMS = Midi::NOTES;

        // void dump() const noexcept;

        bool is_invalid(Midi::Note const note) const noexcept;

        template<bool should_update_extremes>
        void remove(Midi::Note const note) noexcept;

        void update_extremes_after_remove(Midi::Note const changed_note) noexcept;

        bool is_already_pushed(Midi::Note const note) const noexcept;

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
        Midi::Note next[ITEMS];
        Midi::Note previous[ITEMS];

        Midi::Note head;
        Midi::Note oldest_;
        Midi::Note lowest_;
        Midi::Note highest_;
};

}

#endif
