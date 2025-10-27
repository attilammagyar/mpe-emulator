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

#ifndef MPE_EMULATOR__NOTE_STACK_CPP
#define MPE_EMULATOR__NOTE_STACK_CPP

#include <algorithm>

#ifdef MPE_EMULATOR_ASSERTIONS
#include <cstdio>
#include <cstring>
#endif

#include "note_stack.hpp"


namespace MpeEmulator
{

template<bool skip_updating_extremes>
NoteStackTpl<skip_updating_extremes>::NoteStackTpl() noexcept
{
    clear();
}


template<bool skip_updating_extremes>
void NoteStackTpl<skip_updating_extremes>::clear() noexcept
{
    std::fill_n(next, ITEMS, Midi::INVALID_NOTE);
    std::fill_n(previous, ITEMS, Midi::INVALID_NOTE);

    head = Midi::INVALID_NOTE;
    oldest_ = Midi::INVALID_NOTE;
    lowest_ = Midi::INVALID_NOTE;
    highest_ = Midi::INVALID_NOTE;
}


template<bool skip_updating_extremes>
bool NoteStackTpl<skip_updating_extremes>::is_empty() const noexcept
{
    return head == Midi::INVALID_NOTE;
}


template<bool skip_updating_extremes>
bool NoteStackTpl<skip_updating_extremes>::is_top(Midi::Note const note) const noexcept
{
    return head == note;
}


template<bool skip_updating_extremes>
bool NoteStackTpl<skip_updating_extremes>::find(Midi::Note const note) const noexcept
{
    if (MPE_EMULATOR_UNLIKELY(is_invalid(note))) {
        return false;
    }

    return is_already_pushed(note);
}


template<bool skip_updating_extremes>
Midi::Note NoteStackTpl<skip_updating_extremes>::top() const noexcept
{
    return head;
}


template<bool skip_updating_extremes>
Midi::Note NoteStackTpl<skip_updating_extremes>::oldest() const noexcept
{
    return oldest_;
}


template<bool skip_updating_extremes>
Midi::Note NoteStackTpl<skip_updating_extremes>::lowest() const noexcept
{
    return lowest_;
}


template<bool skip_updating_extremes>
Midi::Note NoteStackTpl<skip_updating_extremes>::highest() const noexcept
{
    return highest_;
}


template<bool skip_updating_extremes>
void NoteStackTpl<skip_updating_extremes>::push(Midi::Note const note) noexcept
{
    if (MPE_EMULATOR_UNLIKELY(is_invalid(note))) {
        return;
    }

    if (oldest_ == Midi::INVALID_NOTE) {
        oldest_ = note;
    }

    if (is_already_pushed(note)) {
        remove<false>(note);
    }

    if (head != Midi::INVALID_NOTE) {
        previous[head] = note;
    }

    next[note] = head;
    head = note;

    if constexpr (!skip_updating_extremes) {
        if (lowest_ == Midi::INVALID_NOTE || note < lowest_) {
            lowest_ = note;
        }

        if (highest_ == Midi::INVALID_NOTE || note > highest_) {
            highest_ = note;
        }
    }
}


template<bool skip_updating_extremes>
bool NoteStackTpl<skip_updating_extremes>::is_invalid(Midi::Note const note) const noexcept
{
    return note > Midi::NOTE_MAX;
}


template<bool skip_updating_extremes>
bool NoteStackTpl<skip_updating_extremes>::is_already_pushed(Midi::Note const note) const noexcept
{
    return head == note || previous[note] != Midi::INVALID_NOTE;
}


template<bool skip_updating_extremes>
Midi::Note NoteStackTpl<skip_updating_extremes>::pop() noexcept
{
    if (is_empty()) {
        return Midi::INVALID_NOTE;
    }

    Midi::Note const note = head;

    head = next[note];

    if (head != Midi::INVALID_NOTE) {
        previous[head] = Midi::INVALID_NOTE;
    }

    next[note] = Midi::INVALID_NOTE;
    update_extremes_after_remove(note);

    return note;
}


template<bool skip_updating_extremes>
void NoteStackTpl<skip_updating_extremes>::update_extremes_after_remove(Midi::Note const changed_note) noexcept
{
    if constexpr (skip_updating_extremes) {
        return;
    }

    if (is_empty()) {
        lowest_ = Midi::INVALID_NOTE;
        highest_ = Midi::INVALID_NOTE;

        return;
    }

    bool can_skip = true;

    if (changed_note == lowest_) {
        lowest_ = Midi::INVALID_NOTE;
        can_skip = false;
    }

    if (changed_note == highest_) {
        highest_ = Midi::INVALID_NOTE;
        can_skip = false;
    }

    if (can_skip) {
        return;
    }

    Midi::Note note = head;

    for (size_t i = 0; note != Midi::INVALID_NOTE && i != ITEMS; ++i) {
        if (lowest_ == Midi::INVALID_NOTE || note < lowest_) {
            lowest_ = note;
        }

        if (highest_ == Midi::INVALID_NOTE || note > highest_) {
            highest_ = note;
        }

        note = next[note];
    }
}


template<bool skip_updating_extremes>
void NoteStackTpl<skip_updating_extremes>::remove(Midi::Note const note) noexcept
{
    if (MPE_EMULATOR_UNLIKELY(is_invalid(note))) {
        return;
    }

    remove<true>(note);
}


template<bool skip_updating_extremes>
template<bool should_update_extremes>
void NoteStackTpl<skip_updating_extremes>::remove(Midi::Note const note) noexcept
{
    Midi::Note const next_note = next[note];
    Midi::Note const previous_note = previous[note];

    if (note == oldest_) {
        oldest_ = previous_note;
    }

    if (next_note != Midi::INVALID_NOTE) {
        previous[next_note] = previous_note;
    }

    if (note == head) {
        head = next_note;
    } else {
        if (previous_note != Midi::INVALID_NOTE) {
            next[previous_note] = next_note;
        }

        next[note] = Midi::INVALID_NOTE;
        previous[note] = Midi::INVALID_NOTE;
    }

    if constexpr (should_update_extremes) {
        update_extremes_after_remove(note);
    }
}


template<bool skip_updating_extremes>
void NoteStackTpl<skip_updating_extremes>::collect_active_channels(
        ChannelsByNotes const& channels_by_notes,
        Channels& channels,
        size_t& count
) const noexcept {
    count = 0;
    Midi::Note note = head;

    for (size_t i = 0; note != Midi::INVALID_NOTE && i != ITEMS && count != Midi::CHANNELS; ++i) {
        channels[count++] = channels_by_notes[note];
        note = next[note];
    }
}


template<bool skip_updating_extremes>
void NoteStackTpl<skip_updating_extremes>::make_stats(
        ChannelsByNotes const& channels_by_notes,
        ChannelStats& stats
) const noexcept {
    if (is_empty()) {
        stats.lowest = Midi::INVALID_CHANNEL;
        stats.highest = Midi::INVALID_CHANNEL;
        stats.oldest = Midi::INVALID_CHANNEL;
        stats.newest = Midi::INVALID_CHANNEL;
    } else {
        if constexpr (skip_updating_extremes) {
            stats.lowest = Midi::INVALID_CHANNEL;
            stats.highest = Midi::INVALID_CHANNEL;
        } else {
            stats.lowest =  channels_by_notes[lowest_];
            stats.highest = channels_by_notes[highest_];
        }

        stats.oldest = channels_by_notes[oldest_];
        stats.newest = channels_by_notes[head];
    }
}


template<bool skip_updating_extremes>
NoteStackTpl<skip_updating_extremes>::ChannelStats::ChannelStats() noexcept
    : lowest(Midi::INVALID_CHANNEL),
    highest(Midi::INVALID_CHANNEL),
    oldest(Midi::INVALID_CHANNEL),
    newest(Midi::INVALID_CHANNEL)
{
}


#ifdef MPE_EMULATOR_ASSERTIONS
template<bool skip_updating_extremes>
std::string NoteStackTpl<skip_updating_extremes>::ChannelStats::to_string() const noexcept
{
    constexpr size_t buffer_size = 64;
    char buffer[buffer_size];

    snprintf(
        buffer,
        buffer_size,
        "lo=0x%02hhx hi=0x%02hhx old=0x%02hhx new=0x%02hhx",
        lowest,
        highest,
        oldest,
        newest
    );

    return std::string(buffer);
}
#endif


// template<bool skip_updating_extremes>
// void NoteStackTpl<skip_updating_extremes>::dump() const noexcept
// {
    // fprintf(stderr, "  top=\t%hhx\n", head);
    // fprintf(stderr, "  lowest=\t%hhx\n", lowest_);
    // fprintf(stderr, "  highest=\t%hhx\n", highest_);

    // fprintf(stderr, "  next=\t[");

    // for (Midi::Note i = 0; i != ITEMS; ++i) {
        // if (next[i] != Midi::INVALID_NOTE) {
            // fprintf(stderr, "%hhx-->%hhx, ", i, next[i]);
        // }
    // }

    // fprintf(stderr, "]\n  prev=\t[");

    // for (Midi::Note i = 0; i != ITEMS; ++i) {
        // if (previous[i] != Midi::INVALID_NOTE) {
            // fprintf(stderr, "%hhx-->%hhx, ", i, previous[i]);
        // }
    // }

    // fprintf(stderr, "]\n\n");
// }

}

#endif
