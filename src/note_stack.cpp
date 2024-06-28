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
#include <cstring>
#endif

#include "note_stack.hpp"


namespace MpeEmulator
{

NoteStack::NoteStack() noexcept
{
    clear();
}


void NoteStack::clear() noexcept
{
    std::fill_n(next, ITEMS, INVALID_ITEM);
    std::fill_n(previous, ITEMS, INVALID_ITEM);
    std::fill_n(channels, ITEMS, Midi::INVALID_CHANNEL);
    std::fill_n(velocities, ITEMS, 0);

    head = INVALID_ITEM;
    oldest_ = INVALID_ITEM;
    lowest_ = INVALID_ITEM;
    highest_ = INVALID_ITEM;
}


bool NoteStack::is_empty() const noexcept
{
    return head == INVALID_ITEM;
}


bool NoteStack::is_top(Midi::Note const note) const noexcept
{
    return head == note;
}


bool NoteStack::find(
        Midi::Note const note,
        Midi::Channel& channel,
        Midi::Byte& velocity
) const noexcept {
    if (MPE_EMULATOR_UNLIKELY(is_invalid(note))) {
        return false;
    }

    Midi::Byte const item = note;

    if (!is_already_pushed(item)) {
        channel = Midi::INVALID_CHANNEL;
        velocity = 0;

        return false;
    }

    velocity = velocities[item];
    channel = channels[item];

    return true;
}


void NoteStack::top(
        Midi::Note& note,
        Midi::Channel& channel,
        Midi::Byte& velocity
) const noexcept {
    top(note, channel);
    velocity = is_empty() ? 0 : velocities[head];
}


void NoteStack::top(Midi::Note& note, Midi::Channel& channel) const noexcept
{
    note = head;
    channel = is_empty() ? Midi::INVALID_CHANNEL : channels[head];
}


void NoteStack::oldest(
        Midi::Note& note,
        Midi::Channel& channel,
        Midi::Byte& velocity
) const noexcept {
    oldest(note, channel);
    velocity = oldest_ == INVALID_ITEM ? 0 : velocities[oldest_];
}


void NoteStack::oldest(Midi::Note& note, Midi::Channel& channel) const noexcept
{
    note = oldest_;
    channel = (
        oldest_ == INVALID_ITEM ? Midi::INVALID_CHANNEL : channels[oldest_]
    );
}


void NoteStack::lowest(
        Midi::Note& note,
        Midi::Channel& channel,
        Midi::Byte& velocity
) const noexcept {
    lowest(note, channel);
    velocity = lowest_ == INVALID_ITEM ? 0 : velocities[lowest_];
}


void NoteStack::lowest(Midi::Note& note, Midi::Channel& channel) const noexcept
{
    note = lowest_;
    channel = (
        lowest_ == INVALID_ITEM ? Midi::INVALID_CHANNEL : channels[lowest_]
    );
}


void NoteStack::highest(
        Midi::Note& note,
        Midi::Channel& channel,
        Midi::Byte& velocity
) const noexcept {
    highest(note, channel);
    velocity = highest_ == INVALID_ITEM ? 0 : velocities[highest_];
}


void NoteStack::highest(Midi::Note& note, Midi::Channel& channel) const noexcept
{
    note = highest_;
    channel = (
        highest_ == INVALID_ITEM ? Midi::INVALID_CHANNEL : channels[highest_]
    );
}


void NoteStack::push(
        Midi::Note const note,
        Midi::Channel const channel,
        Midi::Byte const velocity
) noexcept {
    if (MPE_EMULATOR_UNLIKELY(is_invalid(note))) {
        return;
    }

    Midi::Byte const item = note;

    if (oldest_ == INVALID_ITEM) {
        oldest_ = item;
    }

    if (is_already_pushed(item)) {
        remove<false>(item);
    }

    if (head != INVALID_ITEM) {
        previous[head] = item;
    }

    next[item] = head;
    head = item;
    velocities[item] = velocity;
    channels[item] = channel;

    if (lowest_ == INVALID_ITEM || note < lowest_) {
        lowest_ = item;
    }

    if (highest_ == INVALID_ITEM || note > highest_) {
        highest_ = item;
    }
}


bool NoteStack::is_invalid(Midi::Note const note) const noexcept
{
    return note > Midi::NOTE_MAX;
}


bool NoteStack::is_already_pushed(Midi::Byte const item) const noexcept
{
    return head == item || previous[item] != INVALID_ITEM;
}


void NoteStack::pop(
        Midi::Note& note,
        Midi::Channel& channel,
        Midi::Byte& velocity
) noexcept {
    if (is_empty()) {
        note = Midi::INVALID_NOTE;
        channel = Midi::INVALID_CHANNEL;
        velocity = 0;

        return;
    }

    Midi::Byte const item = head;

    head = next[item];

    if (head != INVALID_ITEM) {
        previous[head] = INVALID_ITEM;
    }

    next[item] = INVALID_ITEM;

    velocity = velocities[item];
    channel = channels[item];
    note = item;

    update_extremes(item);
}


void NoteStack::update_extremes(Midi::Byte const changed_item) noexcept
{
    if (is_empty()) {
        lowest_ = INVALID_ITEM;
        highest_ = INVALID_ITEM;

        return;
    }

    if (changed_item == lowest_) {
        lowest_ = INVALID_ITEM;
    }

    if (changed_item == highest_) {
        highest_ = INVALID_ITEM;
    }

    Midi::Byte item = head;

    for (size_t i = 0; item != INVALID_ITEM && i != ITEMS; ++i) {
        if (lowest_ == INVALID_ITEM || item < lowest_) {
            lowest_ = item;
        }

        if (highest_ == INVALID_ITEM || item > highest_) {
            highest_ = item;
        }

        item = next[item];
    }
}


void NoteStack::remove(Midi::Note const note) noexcept
{
    if (MPE_EMULATOR_UNLIKELY(is_invalid(note))) {
        return;
    }

    remove<true>(note);
}


template<bool should_update_extremes>
void NoteStack::remove(Midi::Byte const item) noexcept
{
    Midi::Byte const next_item = next[item];
    Midi::Byte const previous_item = previous[item];

    if (item == oldest_) {
        oldest_ = previous_item;
    }

    if (next_item != INVALID_ITEM) {
        previous[next_item] = previous_item;
    }

    if (item == head) {
        head = next_item;
    } else {
        if (previous_item != INVALID_ITEM) {
            next[previous_item] = next_item;
        }

        next[item] = INVALID_ITEM;
        previous[item] = INVALID_ITEM;
    }

    if constexpr (should_update_extremes) {
        update_extremes(item);
    }
}


void NoteStack::get_active_channels(Channels& channels, size_t& count) const noexcept
{
    count = 0;
    Midi::Byte item = head;

    for (size_t i = 0; item != INVALID_ITEM && i != ITEMS && count != Midi::CHANNELS; ++i) {
        channels[count++] = this->channels[item];
        item = next[item];
    }
}


void NoteStack::make_stats(ChannelStats& stats) const noexcept
{
    if (is_empty()) {
        stats.lowest = Midi::INVALID_CHANNEL;
        stats.highest = Midi::INVALID_CHANNEL;
        stats.oldest = Midi::INVALID_CHANNEL;
        stats.newest = Midi::INVALID_CHANNEL;
    } else {
        stats.lowest =  channels[lowest_];
        stats.highest = channels[highest_];
        stats.oldest = channels[oldest_];
        stats.newest = channels[head];
    }
}


NoteStack::ChannelStats::ChannelStats() noexcept
    : lowest(Midi::INVALID_CHANNEL),
    highest(Midi::INVALID_CHANNEL),
    oldest(Midi::INVALID_CHANNEL),
    newest(Midi::INVALID_CHANNEL)
{
}


#ifdef MPE_EMULATOR_ASSERTIONS
std::string NoteStack::ChannelStats::to_string() const noexcept
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


// void NoteStack::dump() const noexcept
// {
    // fprintf(stderr, "  top=\t%hhx\n", head);
    // fprintf(stderr, "  lowest=\t%hhx\n", lowest_);
    // fprintf(stderr, "  highest=\t%hhx\n", highest_);

    // fprintf(stderr, "  next=\t[");

    // for (Midi::Byte i = 0; i != ITEMS; ++i) {
        // if (next[i] != INVALID_ITEM) {
            // fprintf(stderr, "%hhx-->%hhx, ", i, next[i]);
        // }
    // }

    // fprintf(stderr, "]\n  prev=\t[");

    // for (Midi::Byte i = 0; i != ITEMS; ++i) {
        // if (previous[i] != INVALID_ITEM) {
            // fprintf(stderr, "%hhx-->%hhx, ", i, previous[i]);
        // }
    // }

    // fprintf(stderr, "]\n\n");
// }

}

#endif
