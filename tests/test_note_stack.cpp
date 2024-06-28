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
        Midi::Note note = 0;                                                \
        Midi::Channel channel = 0;                                          \
        Midi::Byte velocity = 255;                                          \
                                                                            \
        assert_true(note_stack.is_empty());                                 \
                                                                            \
        note_stack.top(note, channel, velocity);                            \
        assert_eq(Midi::INVALID_NOTE, note);                                \
        assert_eq(Midi::INVALID_CHANNEL, channel);                          \
        assert_eq(0, velocity);                                             \
                                                                            \
        note = 0;                                                           \
        velocity = 255;                                                     \
        channel = 0;                                                        \
        note_stack.pop(note, channel, velocity);                            \
        assert_eq(Midi::INVALID_NOTE, note);                                \
        assert_eq(Midi::INVALID_CHANNEL, channel);                          \
        assert_eq(0, velocity);                                             \
    } while (false)

#define assert_top(                                                         \
        expected_note,                                                      \
        expected_channel,                                                   \
        expected_velocity,                                                  \
        note_stack                                                          \
)                                                                           \
    do {                                                                    \
        Midi::Note note = Midi::INVALID_NOTE;                               \
        Midi::Channel channel = Midi::INVALID_CHANNEL;                      \
        Midi::Byte velocity = 255;                                          \
                                                                            \
        assert_false(note_stack.is_empty());                                \
        assert_true(note_stack.is_top(expected_note));                      \
                                                                            \
        note_stack.top(note, channel);                                      \
        assert_eq(expected_note, note);                                     \
        assert_eq(expected_channel, channel);                               \
                                                                            \
        note = Midi::INVALID_NOTE;                                          \
        channel = Midi::INVALID_CHANNEL;                                    \
                                                                            \
        note_stack.top(note, channel, velocity);                            \
        assert_eq(expected_note, note);                                     \
        assert_eq(expected_channel, channel);                               \
        assert_eq(expected_velocity, velocity);                             \
    } while (false)

#define assert_extreme(                                                     \
        extreme_type,                                                       \
        expected_note,                                                      \
        expected_channel,                                                   \
        expected_velocity,                                                  \
        note_stack                                                          \
)                                                                           \
    do {                                                                    \
        Midi::Note note = Midi::INVALID_NOTE;                               \
        Midi::Channel channel = Midi::INVALID_CHANNEL;                      \
        Midi::Byte velocity = 255;                                          \
                                                                            \
        assert_false(note_stack.is_empty());                                \
                                                                            \
        note_stack.extreme_type(note, channel);                             \
        assert_eq(expected_note, note);                                     \
        assert_eq(expected_channel, channel);                               \
                                                                            \
        note = Midi::INVALID_NOTE;                                          \
        channel = Midi::INVALID_CHANNEL;                                    \
                                                                            \
        note_stack.extreme_type(note, channel, velocity);                   \
        assert_eq(expected_note, note);                                     \
        assert_eq(expected_channel, channel);                               \
        assert_eq(expected_velocity, velocity);                             \
    } while (false)

#define assert_oldest(                                                      \
        expected_note,                                                      \
        expected_channel,                                                   \
        expected_velocity,                                                  \
        note_stack                                                          \
)                                                                           \
    assert_extreme(                                                         \
        oldest,                                                             \
        expected_note,                                                      \
        expected_channel,                                                   \
        expected_velocity,                                                  \
        note_stack                                                          \
    )

#define assert_lowest(                                                      \
        expected_note,                                                      \
        expected_channel,                                                   \
        expected_velocity,                                                  \
        note_stack                                                          \
)                                                                           \
    assert_extreme(                                                         \
        lowest,                                                             \
        expected_note,                                                      \
        expected_channel,                                                   \
        expected_velocity,                                                  \
        note_stack                                                          \
    )

#define assert_highest(                                                     \
        expected_note,                                                      \
        expected_channel,                                                   \
        expected_velocity,                                                  \
        note_stack                                                          \
)                                                                           \
    assert_extreme(                                                         \
        highest,                                                            \
        expected_note,                                                      \
        expected_channel,                                                   \
        expected_velocity,                                                  \
        note_stack                                                          \
    )

#define assert_pop(                                                         \
        expected_popped_note,                                               \
        expected_popped_channel,                                            \
        expected_popped_velocity,                                           \
        expected_top_note_after_pop,                                        \
        expected_top_channel_after_pop,                                     \
        expected_top_velocity_after_pop,                                    \
        note_stack                                                          \
)                                                                           \
    do {                                                                    \
        Midi::Note note = Midi::INVALID_NOTE;                               \
        Midi::Channel channel = Midi::INVALID_CHANNEL;                      \
        Midi::Byte velocity = 255;                                          \
                                                                            \
        assert_true(note_stack.is_top(expected_popped_note));               \
        note_stack.top(note, channel, velocity);                            \
        assert_eq(expected_popped_note, note);                              \
        assert_eq(expected_popped_channel, channel);                        \
        assert_eq(expected_popped_velocity, velocity);                      \
                                                                            \
        note = Midi::INVALID_NOTE;                                          \
        channel = Midi::INVALID_CHANNEL;                                    \
        velocity = 255;                                                     \
        note_stack.pop(note, channel, velocity);                            \
        assert_eq(expected_popped_note, note);                              \
        assert_eq(expected_popped_channel, channel);                        \
        assert_eq(expected_popped_velocity, velocity);                      \
                                                                            \
        note = Midi::INVALID_NOTE;                                          \
        channel = Midi::INVALID_CHANNEL;                                    \
        velocity = 255;                                                     \
        note_stack.top(note, channel, velocity);                            \
        assert_true(note_stack.is_top(expected_top_note_after_pop));        \
        assert_eq(expected_top_note_after_pop, note);                       \
        assert_eq(expected_top_channel_after_pop, channel);                 \
        assert_eq(expected_top_velocity_after_pop, velocity);               \
    } while (false)


TEST(note_stack_is_created_empty, {
    NoteStack note_stack;

    assert_empty(note_stack);
})


TEST(when_a_note_is_pushed_on_the_stack_then_stack_is_no_longer_empty_and_the_note_is_on_the_top, {
    NoteStack note_stack;

    note_stack.push(57, 5, 63);

    assert_pop(57, 5, 63, Midi::INVALID_NOTE, Midi::INVALID_CHANNEL, 0, note_stack);
    assert_empty(note_stack);
})


TEST(pushing_an_invalid_note_is_no_op, {
    NoteStack note_stack;

    note_stack.push(Midi::NOTE_MAX + 1, 5, 63);

    assert_empty(note_stack);
})


TEST(note_stack_is_a_lifo_container, {
    NoteStack note_stack;

    note_stack.push(57, 1, 33);
    note_stack.push(59, 2, 43);
    note_stack.push(48, 3, 53);
    note_stack.push(50, 4, 63);
    note_stack.push(52, 5, 73);

    assert_pop(52, 5, 73, 50, 4, 63, note_stack);
    assert_pop(50, 4, 63, 48, 3, 53, note_stack);
    assert_pop(48, 3, 53, 59, 2, 43, note_stack);
    assert_pop(59, 2, 43, 57, 1, 33, note_stack);
    assert_pop(57, 1, 33, Midi::INVALID_NOTE, Midi::INVALID_CHANNEL, 0, note_stack);
    assert_empty(note_stack);
})


TEST(when_a_note_stack_is_cleared_then_it_will_become_empty, {
    NoteStack note_stack;

    note_stack.push(57, 1, 63);
    note_stack.push(59, 2, 63);
    note_stack.push(48, 3, 63);
    note_stack.push(50, 4, 63);
    note_stack.push(52, 5, 63);
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

    note_stack.push(57, 1, 63);
    note_stack.push(59, 2, 63);
    note_stack.push(48, 3, 63);

    note_stack.remove(52);

    assert_pop(48, 3, 63, 59, 2, 63, note_stack);
    assert_pop(59, 2, 63, 57, 1, 63, note_stack);
    assert_pop(57, 1, 63, Midi::INVALID_NOTE, Midi::INVALID_CHANNEL, 0, note_stack);
    assert_empty(note_stack);
})


TEST(top_note_can_be_removed, {
    NoteStack note_stack;

    note_stack.push(57, 1, 63);
    note_stack.push(59, 2, 63);
    note_stack.push(48, 3, 63);
    note_stack.push(50, 4, 63);
    note_stack.push(52, 5, 63);

    note_stack.remove(52);

    assert_pop(50, 4, 63, 48, 3, 63, note_stack);
    assert_pop(48, 3, 63, 59, 2, 63, note_stack);
    assert_pop(59, 2, 63, 57, 1, 63, note_stack);
    assert_pop(57, 1, 63, Midi::INVALID_NOTE, Midi::INVALID_CHANNEL, 0, note_stack);
    assert_empty(note_stack);
})


TEST(stack_can_be_emptied_by_removing_notes_from_the_top, {
    NoteStack note_stack;

    note_stack.push(57, 1, 63);
    note_stack.push(59, 2, 63);
    note_stack.push(48, 3, 63);
    note_stack.push(50, 4, 63);
    note_stack.push(52, 5, 63);

    note_stack.remove(52);
    assert_top(50, 4, 63, note_stack);

    note_stack.remove(50);
    assert_top(48, 3, 63, note_stack);

    note_stack.remove(48);
    assert_top(59, 2, 63, note_stack);

    note_stack.remove(59);
    assert_top(57, 1, 63, note_stack);

    note_stack.remove(57);
    assert_empty(note_stack);
})


TEST(first_note_can_be_removed, {
    NoteStack note_stack;

    note_stack.push(57, 1, 63);
    note_stack.push(59, 2, 63);
    note_stack.push(48, 3, 63);
    note_stack.push(50, 4, 63);
    note_stack.push(52, 5, 63);

    note_stack.remove(57);

    assert_pop(52, 5, 63, 50, 4, 63, note_stack);
    assert_pop(50, 4, 63, 48, 3, 63, note_stack);
    assert_pop(48, 3, 63, 59, 2, 63, note_stack);
    assert_pop(59, 2, 63, Midi::INVALID_NOTE, Midi::INVALID_CHANNEL, 0, note_stack);
    assert_empty(note_stack);
})


TEST(note_stack_can_be_emptied_by_removing_notes_from_the_bottom, {
    NoteStack note_stack;

    note_stack.push(57, 1, 63);
    note_stack.push(59, 2, 63);
    note_stack.push(48, 3, 63);
    note_stack.push(50, 4, 63);
    note_stack.push(52, 5, 63);

    note_stack.remove(57);
    assert_top(52, 5, 63, note_stack);

    note_stack.remove(59);
    assert_top(52, 5, 63, note_stack);

    note_stack.remove(48);
    assert_top(52, 5, 63, note_stack);

    note_stack.remove(50);
    assert_top(52, 5, 63, note_stack);

    note_stack.remove(52);
    assert_empty(note_stack);
})


TEST(note_can_be_removed_from_the_middle, {
    NoteStack note_stack;

    note_stack.push(57, 1, 63);
    note_stack.push(59, 2, 63);
    note_stack.push(48, 3, 63);
    note_stack.push(50, 4, 63);
    note_stack.push(52, 5, 63);

    note_stack.remove(48);

    assert_pop(52, 5, 63, 50, 4, 63, note_stack);
    assert_pop(50, 4, 63, 59, 2, 63, note_stack);
    assert_pop(59, 2, 63, 57, 1, 63, note_stack);
    assert_pop(57, 1, 63, Midi::INVALID_NOTE, Midi::INVALID_CHANNEL, 0, note_stack);
    assert_empty(note_stack);
})


TEST(all_notes_can_be_removed_from_the_middle, {
    NoteStack note_stack;

    note_stack.push(57, 1, 63);
    note_stack.push(59, 2, 63);
    note_stack.push(48, 3, 63);
    note_stack.push(50, 4, 63);
    note_stack.push(52, 5, 63);

    note_stack.remove(48);
    assert_top(52, 5, 63, note_stack);

    note_stack.remove(59);
    assert_top(52, 5, 63, note_stack);

    note_stack.remove(50);

    assert_pop(52, 5, 63, 57, 1, 63, note_stack);
    assert_pop(57, 1, 63, Midi::INVALID_NOTE, Midi::INVALID_CHANNEL, 0, note_stack);
    assert_empty(note_stack);
})


TEST(all_notes_can_be_removed_starting_from_the_middle, {
    NoteStack note_stack;

    note_stack.push(57, 1, 63);
    note_stack.push(59, 2, 63);
    note_stack.push(48, 3, 63);
    note_stack.push(50, 4, 63);
    note_stack.push(52, 5, 63);

    note_stack.remove(48);
    assert_top(52, 5, 63, note_stack);

    note_stack.remove(59);
    assert_top(52, 5, 63, note_stack);

    note_stack.remove(50);
    assert_top(52, 5, 63, note_stack);

    note_stack.remove(52);
    assert_top(57, 1, 63, note_stack);

    note_stack.remove(57);
    assert_empty(note_stack);
})


TEST(removing_note_which_is_already_removed_is_no_op, {
    NoteStack note_stack;

    note_stack.push(57, 1, 63);
    note_stack.push(59, 2, 63);
    note_stack.push(48, 3, 63);

    note_stack.remove(59);
    note_stack.remove(59);
    assert_top(48, 3, 63, note_stack);

    note_stack.remove(48);
    note_stack.remove(48);
    assert_top(57, 1, 63, note_stack);

    note_stack.remove(57);
    note_stack.remove(57);
    assert_empty(note_stack);
})


TEST(when_a_note_is_pushed_multiple_times_then_only_the_last_instance_remains, {
    NoteStack note_stack;

    note_stack.push(50, 1, 23);
    note_stack.push(57, 2, 23);
    note_stack.push(50, 3, 33);
    note_stack.push(52, 4, 33);
    note_stack.push(59, 5, 23);
    note_stack.push(52, 6, 43);
    note_stack.push(52, 7, 53);
    note_stack.push(48, 8, 23);
    note_stack.push(52, 9, 63);
    note_stack.push(52, 10, 73);
    note_stack.push(50, 11, 43);
    note_stack.push(52, 12, 83);
    note_stack.push(52, 13, 93);

    assert_pop(52, 13, 93, 50, 11, 43, note_stack);
    assert_pop(50, 11, 43, 48, 8, 23, note_stack);
    assert_pop(48, 8, 23, 59, 5, 23, note_stack);
    assert_pop(59, 5, 23, 57, 2, 23, note_stack);
    assert_pop(57, 2, 23, Midi::INVALID_NOTE, Midi::INVALID_CHANNEL, 0, note_stack);
    assert_empty(note_stack);
})


TEST(stays_consistent_after_many_operations, {
    NoteStack note_stack;

    note_stack.push(48, 1, 23);
    note_stack.push(59, 2, 23);
    note_stack.push(57, 3, 23);
    note_stack.push(59, 4, 33);

    note_stack.remove(48);

    assert_pop(59, 4, 33, 57, 3, 23, note_stack);

    note_stack.push(50, 5, 23);
    note_stack.push(52, 6, 23);
    assert_pop(52, 6, 23, 50, 5, 23, note_stack);

    note_stack.remove(50);
    assert_top(57, 3, 23, note_stack);

    note_stack.push(48, 7, 33);
    note_stack.push(59, 8, 43);
    note_stack.push(48, 9, 43);
    note_stack.push(50, 10, 33);
    assert_pop(50, 10, 33, 48, 9, 43, note_stack);
    assert_pop(48, 9, 43, 59, 8, 43, note_stack);

    note_stack.remove(59);
    assert_top(57, 3, 23, note_stack);

    note_stack.push(48, 11, 53);
    note_stack.push(59, 12, 53);
    note_stack.push(50, 13, 43);
    note_stack.push(48, 14, 63);
    note_stack.push(52, 15, 33);
    note_stack.push(50, 1, 53);
    note_stack.push(52, 2, 43);

    assert_pop(52, 2, 43, 50, 1, 53, note_stack);
    assert_pop(50, 1, 53, 48, 14, 63, note_stack);
    assert_pop(48, 14, 63, 59, 12, 53, note_stack);
    assert_pop(59, 12, 53, 57, 3, 23, note_stack);
    assert_pop(57, 3, 23, Midi::INVALID_NOTE, Midi::INVALID_CHANNEL, 0, note_stack);

    assert_empty(note_stack);
})


TEST(keeps_track_of_highest_and_lowest_note, {
    NoteStack note_stack;
    Midi::Note note = Midi::INVALID_NOTE;
    Midi::Channel channel = Midi::INVALID_CHANNEL;
    Midi::Byte velocity = 255;

    note_stack.push(33, 1, 127);
    assert_oldest(33, 1, 127, note_stack);
    assert_lowest(33, 1, 127, note_stack);
    assert_highest(33, 1, 127, note_stack);

    note_stack.push(57, 2, 111);
    assert_oldest(33, 1, 127, note_stack);
    assert_lowest(33, 1, 127, note_stack);
    assert_highest(57, 2, 111, note_stack);

    note_stack.push(81, 3, 100);
    assert_oldest(33, 1, 127, note_stack);
    assert_lowest(33, 1, 127, note_stack);
    assert_highest(81, 3, 100, note_stack);

    note_stack.pop(note, channel, velocity);
    assert_oldest(33, 1, 127, note_stack);
    assert_lowest(33, 1, 127, note_stack);
    assert_highest(57, 2, 111, note_stack);

    note_stack.push(45, 4, 110);
    assert_oldest(33, 1, 127, note_stack);
    assert_lowest(33, 1, 127, note_stack);
    assert_highest(57, 2, 111, note_stack);

    note_stack.remove(33);
    assert_oldest(57, 2, 111, note_stack);
    assert_lowest(45, 4, 110, note_stack);
    assert_highest(57, 2, 111, note_stack);

    note_stack.pop(note, channel, velocity);
    assert_oldest(57, 2, 111, note_stack);
    assert_lowest(57, 2, 111, note_stack);
    assert_highest(57, 2, 111, note_stack);
})


TEST(can_find_note, {
    NoteStack note_stack;
    Midi::Note note = Midi::INVALID_NOTE;
    Midi::Channel channel = Midi::INVALID_CHANNEL;
    Midi::Byte velocity = 255;

    assert_false(note_stack.find(Midi::INVALID_NOTE, channel, velocity));
    assert_false(note_stack.find(60, channel, velocity));

    note_stack.push(50, 1, 100);

    assert_false(note_stack.find(60, channel, velocity));
    assert_true(note_stack.find(50, channel, velocity));
    assert_eq(1, channel);
    assert_eq(100, velocity);

    note_stack.push(72, 2, 110);

    assert_true(note_stack.find(72, channel, velocity));
    assert_eq(2, channel);
    assert_eq(110, velocity);

    note_stack.pop(note, channel, velocity);
})


TEST(can_make_statistics_about_channels, {
    NoteStack note_stack;
    NoteStack::ChannelStats stats;

    note_stack.make_stats(stats);
    assert_eq("lo=0xff hi=0xff old=0xff new=0xff", stats.to_string());

    note_stack.push(60, 1, 127);
    note_stack.make_stats(stats);
    assert_eq("lo=0x01 hi=0x01 old=0x01 new=0x01", stats.to_string());

    note_stack.push(72, 2, 127);
    note_stack.make_stats(stats);
    assert_eq("lo=0x01 hi=0x02 old=0x01 new=0x02", stats.to_string());

    note_stack.push(64, 3, 127);
    note_stack.make_stats(stats);
    assert_eq("lo=0x01 hi=0x02 old=0x01 new=0x03", stats.to_string());

    note_stack.push(48, 4, 127);
    note_stack.make_stats(stats);
    assert_eq("lo=0x04 hi=0x02 old=0x01 new=0x04", stats.to_string());

    note_stack.push(67, 5, 127);
    note_stack.make_stats(stats);
    assert_eq("lo=0x04 hi=0x02 old=0x01 new=0x05", stats.to_string());
})


TEST(can_collect_active_channels, {
    constexpr Midi::Channel expected_channels[] = {0, 5, 15};

    NoteStack note_stack;
    NoteStack::Channels active_channels;
    size_t active_channels_count = 0;

    std::fill_n(active_channels, Midi::CHANNELS, Midi::INVALID_CHANNEL);

    note_stack.push(48, 2, 33);
    note_stack.push(59, 5, 43);
    note_stack.push(48, 0, 43);
    note_stack.push(50, 15, 33);
    note_stack.push(60, 10, 33);
    note_stack.remove(60);

    note_stack.get_active_channels(active_channels, active_channels_count);
    std::sort(std::begin(active_channels), std::end(active_channels));

    assert_eq(3, active_channels_count);
    assert_eq(expected_channels, active_channels, active_channels_count);
})
