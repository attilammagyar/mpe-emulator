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

#include "test.cpp"

#include <algorithm>
#include <cstddef>
#include <iterator>

#include "note_stack.cpp"


using namespace MpeEmulator;


#define assert_empty(note_stack)                                            \
    do {                                                                    \
        assert_true(note_stack.is_empty());                                 \
        assert_eq(Midi::INVALID_NOTE, note_stack.top());                    \
        assert_eq(Midi::INVALID_NOTE, note_stack.pop());                    \
    } while (false)


#define assert_top(expected_note, note_stack)                               \
    do {                                                                    \
        assert_false(note_stack.is_empty());                                \
        assert_true(note_stack.is_top(expected_note));                      \
        assert_eq(expected_note, note_stack.top());                         \
    } while (false)


#define assert_extreme(extreme_type, expected_note, note_stack)             \
    do {                                                                    \
        assert_false(note_stack.is_empty());                                \
        assert_eq(expected_note, note_stack.extreme_type());                \
    } while (false)


#define assert_oldest(expected_note, note_stack)                            \
    assert_extreme(oldest, expected_note, note_stack)


#define assert_lowest(expected_note, note_stack)                            \
    assert_extreme(lowest, expected_note, note_stack)


#define assert_highest(expected_note, note_stack)                           \
    assert_extreme(highest, expected_note, note_stack)


#define assert_pop(                                                         \
        expected_popped_note,                                               \
        expected_top_note_after_pop,                                        \
        note_stack                                                          \
)                                                                           \
    do {                                                                    \
        assert_true(note_stack.is_top(expected_popped_note));               \
        assert_eq(expected_popped_note, note_stack.top());                  \
        assert_eq(expected_popped_note, note_stack.pop());                  \
                                                                            \
        assert_true(note_stack.is_top(expected_top_note_after_pop));        \
        assert_eq(expected_top_note_after_pop, note_stack.top());           \
    } while (false)


TEST(note_stack_is_created_empty, {
    NoteStack note_stack;

    assert_empty(note_stack);
})


TEST(when_a_note_is_pushed_on_the_stack_then_stack_is_no_longer_empty_and_the_note_is_on_the_top, {
    NoteStack note_stack;

    note_stack.push(57);

    assert_pop(57, Midi::INVALID_NOTE, note_stack);
    assert_empty(note_stack);
})


TEST(pushing_an_invalid_note_is_no_op, {
    NoteStack note_stack;

    note_stack.push(Midi::NOTE_MAX + 1);

    assert_empty(note_stack);
})


TEST(note_stack_is_a_lifo_container, {
    NoteStack note_stack;

    note_stack.push(57);
    note_stack.push(59);
    note_stack.push(48);
    note_stack.push(50);
    note_stack.push(52);

    assert_pop(52, 50, note_stack);
    assert_pop(50, 48, note_stack);
    assert_pop(48, 59, note_stack);
    assert_pop(59, 57, note_stack);
    assert_pop(57, Midi::INVALID_NOTE, note_stack);
    assert_empty(note_stack);
})


TEST(when_a_note_stack_is_cleared_then_it_will_become_empty, {
    NoteStack note_stack;

    note_stack.push(57);
    note_stack.push(59);
    note_stack.push(48);
    note_stack.push(50);
    note_stack.push(52);
    note_stack.clear();

    assert_empty(note_stack);
})


TEST(removing_from_empty_stack_is_no_op, {
    NoteStack note_stack;

    note_stack.remove(57);

    assert_empty(note_stack);
})


TEST(removing_an_invalid_note_is_no_op, {
    NoteStack note_stack;

    note_stack.remove(Midi::NOTE_MAX + 1);

    assert_empty(note_stack);
})


TEST(removing_note_which_is_not_in_the_stack_is_no_op, {
    NoteStack note_stack;

    note_stack.push(57);
    note_stack.push(59);
    note_stack.push(48);

    note_stack.remove(52);

    assert_pop(48, 59, note_stack);
    assert_pop(59, 57, note_stack);
    assert_pop(57, Midi::INVALID_NOTE, note_stack);
    assert_empty(note_stack);
})


TEST(top_note_can_be_removed, {
    NoteStack note_stack;

    note_stack.push(57);
    note_stack.push(59);
    note_stack.push(48);
    note_stack.push(50);
    note_stack.push(52);

    note_stack.remove(52);

    assert_pop(50, 48, note_stack);
    assert_pop(48, 59, note_stack);
    assert_pop(59, 57, note_stack);
    assert_pop(57, Midi::INVALID_NOTE, note_stack);
    assert_empty(note_stack);
})


TEST(stack_can_be_emptied_by_removing_notes_from_the_top, {
    NoteStack note_stack;

    note_stack.push(57);
    note_stack.push(59);
    note_stack.push(48);
    note_stack.push(50);
    note_stack.push(52);

    note_stack.remove(52);
    assert_top(50, note_stack);

    note_stack.remove(50);
    assert_top(48, note_stack);

    note_stack.remove(48);
    assert_top(59, note_stack);

    note_stack.remove(59);
    assert_top(57, note_stack);

    note_stack.remove(57);
    assert_empty(note_stack);
})


TEST(first_note_can_be_removed, {
    NoteStack note_stack;

    note_stack.push(57);
    note_stack.push(59);
    note_stack.push(48);
    note_stack.push(50);
    note_stack.push(52);

    note_stack.remove(57);

    assert_pop(52, 50, note_stack);
    assert_pop(50, 48, note_stack);
    assert_pop(48, 59, note_stack);
    assert_pop(59, Midi::INVALID_NOTE, note_stack);
    assert_empty(note_stack);
})


TEST(note_stack_can_be_emptied_by_removing_notes_from_the_bottom, {
    NoteStack note_stack;

    note_stack.push(57);
    note_stack.push(59);
    note_stack.push(48);
    note_stack.push(50);
    note_stack.push(52);

    note_stack.remove(57);
    assert_top(52, note_stack);

    note_stack.remove(59);
    assert_top(52, note_stack);

    note_stack.remove(48);
    assert_top(52, note_stack);

    note_stack.remove(50);
    assert_top(52, note_stack);

    note_stack.remove(52);
    assert_empty(note_stack);
})


TEST(note_can_be_removed_from_the_middle, {
    NoteStack note_stack;

    note_stack.push(57);
    note_stack.push(59);
    note_stack.push(48);
    note_stack.push(50);
    note_stack.push(52);

    note_stack.remove(48);

    assert_pop(52, 50, note_stack);
    assert_pop(50, 59, note_stack);
    assert_pop(59, 57, note_stack);
    assert_pop(57, Midi::INVALID_NOTE, note_stack);
    assert_empty(note_stack);
})


TEST(all_notes_can_be_removed_from_the_middle, {
    NoteStack note_stack;

    note_stack.push(57);
    note_stack.push(59);
    note_stack.push(48);
    note_stack.push(50);
    note_stack.push(52);

    note_stack.remove(48);
    assert_top(52, note_stack);

    note_stack.remove(59);
    assert_top(52, note_stack);

    note_stack.remove(50);

    assert_pop(52, 57, note_stack);
    assert_pop(57, Midi::INVALID_NOTE, note_stack);
    assert_empty(note_stack);
})


TEST(all_notes_can_be_removed_starting_from_the_middle, {
    NoteStack note_stack;

    note_stack.push(57);
    note_stack.push(59);
    note_stack.push(48);
    note_stack.push(50);
    note_stack.push(52);

    note_stack.remove(48);
    assert_top(52, note_stack);

    note_stack.remove(59);
    assert_top(52, note_stack);

    note_stack.remove(50);
    assert_top(52, note_stack);

    note_stack.remove(52);
    assert_top(57, note_stack);

    note_stack.remove(57);
    assert_empty(note_stack);
})


TEST(removing_note_which_is_already_removed_is_no_op, {
    NoteStack note_stack;

    note_stack.push(57);
    note_stack.push(59);
    note_stack.push(48);

    note_stack.remove(59);
    note_stack.remove(59);
    assert_top(48, note_stack);

    note_stack.remove(48);
    note_stack.remove(48);
    assert_top(57, note_stack);

    note_stack.remove(57);
    note_stack.remove(57);
    assert_empty(note_stack);
})


TEST(when_a_note_is_pushed_multiple_times_then_only_the_last_instance_remains, {
    NoteStack note_stack;

    note_stack.push(50);
    note_stack.push(57);
    note_stack.push(50);
    note_stack.push(52);
    note_stack.push(59);
    note_stack.push(52);
    note_stack.push(52);
    note_stack.push(48);
    note_stack.push(52);
    note_stack.push(52);
    note_stack.push(50);
    note_stack.push(52);
    note_stack.push(52);

    assert_pop(52, 50, note_stack);
    assert_pop(50, 48, note_stack);
    assert_pop(48, 59, note_stack);
    assert_pop(59, 57, note_stack);
    assert_pop(57, Midi::INVALID_NOTE, note_stack);
    assert_empty(note_stack);
})


TEST(stays_consistent_after_many_operations, {
    NoteStack note_stack;

    note_stack.push(48);
    note_stack.push(59);
    note_stack.push(57);
    note_stack.push(59);

    note_stack.remove(48);

    assert_pop(59, 57, note_stack);

    note_stack.push(50);
    note_stack.push(52);
    assert_pop(52, 50, note_stack);

    note_stack.remove(50);
    assert_top(57, note_stack);

    note_stack.push(48);
    note_stack.push(59);
    note_stack.push(48);
    note_stack.push(50);
    assert_pop(50, 48, note_stack);
    assert_pop(48, 59, note_stack);

    note_stack.remove(59);
    assert_top(57, note_stack);

    note_stack.push(48);
    note_stack.push(59);
    note_stack.push(50);
    note_stack.push(48);
    note_stack.push(52);
    note_stack.push(50);
    note_stack.push(52);

    assert_pop(52, 50, note_stack);
    assert_pop(50, 48, note_stack);
    assert_pop(48, 59, note_stack);
    assert_pop(59, 57, note_stack);
    assert_pop(57, Midi::INVALID_NOTE, note_stack);

    assert_empty(note_stack);
})


TEST(keeps_track_of_highest_and_lowest_note, {
    NoteStack note_stack;

    note_stack.push(33);
    assert_oldest(33, note_stack);
    assert_lowest(33, note_stack);
    assert_highest(33, note_stack);

    note_stack.push(57);
    assert_oldest(33, note_stack);
    assert_lowest(33, note_stack);
    assert_highest(57, note_stack);

    note_stack.push(81);
    assert_oldest(33, note_stack);
    assert_lowest(33, note_stack);
    assert_highest(81, note_stack);

    note_stack.pop();
    assert_oldest(33, note_stack);
    assert_lowest(33, note_stack);
    assert_highest(57, note_stack);

    note_stack.push(45);
    assert_oldest(33, note_stack);
    assert_lowest(33, note_stack);
    assert_highest(57, note_stack);

    note_stack.remove(33);
    assert_oldest(57, note_stack);
    assert_lowest(45, note_stack);
    assert_highest(57, note_stack);

    note_stack.pop();
    assert_oldest(57, note_stack);
    assert_lowest(57, note_stack);
    assert_highest(57, note_stack);
})


TEST(can_find_note, {
    NoteStack note_stack;

    assert_false(note_stack.find(Midi::INVALID_NOTE));
    assert_false(note_stack.find(60));

    note_stack.push(50);

    assert_false(note_stack.find(60));
    assert_true(note_stack.find(50));

    note_stack.push(72);

    assert_true(note_stack.find(72));

    note_stack.pop();
})


TEST(can_make_statistics_about_channels, {
    NoteStack note_stack;
    NoteStack::ChannelStats stats;
    NoteStack::ChannelsByNotes channels_by_notes;

    std::fill_n(channels_by_notes, Midi::NOTES, Midi::INVALID_CHANNEL);

    note_stack.make_stats(channels_by_notes, stats);
    assert_eq("lo=0xff hi=0xff old=0xff new=0xff", stats.to_string());

    note_stack.push(60);
    channels_by_notes[60] = 1;
    note_stack.make_stats(channels_by_notes, stats);
    assert_eq("lo=0x01 hi=0x01 old=0x01 new=0x01", stats.to_string());

    note_stack.push(72);
    channels_by_notes[72] = 2;
    note_stack.make_stats(channels_by_notes, stats);
    assert_eq("lo=0x01 hi=0x02 old=0x01 new=0x02", stats.to_string());

    note_stack.push(64);
    channels_by_notes[64] = 3;
    note_stack.make_stats(channels_by_notes, stats);
    assert_eq("lo=0x01 hi=0x02 old=0x01 new=0x03", stats.to_string());

    note_stack.push(48);
    channels_by_notes[48] = 4;
    note_stack.make_stats(channels_by_notes, stats);
    assert_eq("lo=0x04 hi=0x02 old=0x01 new=0x04", stats.to_string());

    note_stack.push(67);
    channels_by_notes[67] = 5;
    note_stack.make_stats(channels_by_notes, stats);
    assert_eq("lo=0x04 hi=0x02 old=0x01 new=0x05", stats.to_string());
})


TEST(can_collect_active_channels, {
    constexpr Midi::Channel expected_channels[] = {0, 5, 15};

    NoteStack note_stack;
    NoteStack::Channels active_channels;
    NoteStack::ChannelsByNotes channels_by_notes;

    size_t active_channels_count = 0;

    std::fill_n(active_channels, Midi::CHANNELS, Midi::INVALID_CHANNEL);
    std::fill_n(channels_by_notes, Midi::NOTES, Midi::INVALID_CHANNEL);

    note_stack.push(48);
    channels_by_notes[48] = 2;

    note_stack.push(59);
    channels_by_notes[59] = 5;

    note_stack.push(48);
    channels_by_notes[48] = 0;

    note_stack.push(50);
    channels_by_notes[50] = 15;

    note_stack.push(60);
    channels_by_notes[60] = 10;
    note_stack.remove(60);

    note_stack.collect_active_channels(
        channels_by_notes, active_channels, active_channels_count
    );
    std::sort(std::begin(active_channels), std::end(active_channels));

    assert_eq(3, active_channels_count);
    assert_eq(expected_channels, active_channels, active_channels_count);
})
