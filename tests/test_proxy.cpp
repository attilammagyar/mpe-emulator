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

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <string>
#include <vector>

#include "test.cpp"

#include "math.hpp"

#include "proxy.cpp"


using namespace MpeEmulator;


constexpr Proxy::MessageType SET_PARAM = Proxy::MessageType::SET_PARAM;
constexpr Proxy::MessageType REFRESH_PARAM = Proxy::MessageType::REFRESH_PARAM;
constexpr Proxy::MessageType CLEAR = Proxy::MessageType::CLEAR;
constexpr Proxy::MessageType CLEAR_DIRTY_FLAG = Proxy::MessageType::CLEAR_DIRTY_FLAG;


std::string out_events_to_string(Proxy const& proxy)
{
    std::string result("");

    for (Proxy::OutEvents::const_iterator i = proxy.out_events.begin(); i != proxy.out_events.end(); ++i) {
        result += i->to_string() + "\n";
    }

    return result;
}


template<size_t expected_events_count>
void assert_out_events(
        /*
        Cppcheck 2.7 seems to fail to understand that `expected_events` is a const ref.
        */
        std::array<char const* const, expected_events_count> const& expected_events, // cppcheck-suppress constParameter
        Proxy const& proxy
) {
    std::string expected_str("");

    for (size_t i = 0; i != expected_events_count; ++i) {
        expected_str += expected_events[i];
        expected_str += "\n";
    }

    assert_eq(expected_str.c_str(), out_events_to_string(proxy).c_str());
}


TEST(communication_with_the_gui_is_lock_free, {
    Proxy proxy;

    assert_true(proxy.is_lock_free());
});


void set_param(Proxy& proxy, Proxy::ParamId const param_id, double const ratio)
{
    proxy.push_message(SET_PARAM, param_id, ratio);
}


TEST(can_look_up_param_id_by_name, {
    Proxy proxy;
    double avg_collisions;
    double avg_bucket_size;
    unsigned int max_collisions;

    proxy.get_param_id_hash_table_statistics(
        max_collisions, avg_collisions, avg_bucket_size
    );

    assert_lte((int)max_collisions, 2);
    assert_lte(avg_bucket_size, 1.3);
    assert_lte(avg_collisions, 2.1);

    assert_eq(Proxy::ParamId::INVALID_PARAM_ID, proxy.get_param_id(""));
    assert_eq(Proxy::ParamId::INVALID_PARAM_ID, proxy.get_param_id(" \n"));
    assert_eq(Proxy::ParamId::INVALID_PARAM_ID, proxy.get_param_id("NO_SUCH_PARAM"));

    for (int i = 0; i != Proxy::ParamId::PARAM_ID_COUNT; ++i) {
        std::string const name = proxy.get_param_name((Proxy::ParamId)i);
        Proxy::ParamId const param_id = proxy.get_param_id(name);
        assert_eq((Proxy::ParamId)i, param_id, "i=%d, name=\"%s\"", i, name);
    }
})


void assert_message_dirtiness(
        Proxy& proxy,
        Proxy::MessageType const message_type,
        bool const expected_dirtiness
) {
    assert_false(
        proxy.is_dirty(),
        "Expected proxy not to be dirty before sending message; message=%d",
        (int)message_type
    );

    proxy.push_message(message_type, Proxy::ParamId::Z1ANC, 0.123);
    assert_false(
        proxy.is_dirty(),
        "Expected proxy not to become dirty before processing message; message=%d",
        (int)message_type
    );

    proxy.process_messages();

    if (expected_dirtiness) {
        assert_true(
            proxy.is_dirty(),
            "Expected proxy to become dirty after processing message; message=%d",
            (int)message_type
        );
    } else {
        assert_false(
            proxy.is_dirty(),
            "Expected proxy not to become dirty after processing message; message=%d",
            (int)message_type
        );
    }

    proxy.clear_dirty_flag();
    assert_false(
        proxy.is_dirty(),
        "Expected proxy not to remain dirty after clearing the flag; message=%d",
        (int)message_type
    );
}


TEST(when_proxy_config_changes_then_proxy_becomes_dirty, {
    Proxy proxy;

    assert_message_dirtiness(proxy, SET_PARAM, true);
    assert_message_dirtiness(proxy, REFRESH_PARAM, false);
    assert_message_dirtiness(proxy, CLEAR, true);

    proxy.push_message(SET_PARAM, Proxy::ParamId::Z1CHN, 0.5);
    assert_message_dirtiness(proxy, CLEAR_DIRTY_FLAG, false);
})


TEST(can_process_messages_synchronously, {
    Proxy proxy;
    Proxy::Message message(SET_PARAM, Proxy::ParamId::Z1ANC, 0.123);

    assert_false(proxy.is_dirty());

    proxy.process_message(message);
    assert_true(proxy.is_dirty());

    proxy.clear_dirty_flag();
    assert_false(proxy.is_dirty());

    assert_eq(
        0.123, proxy.get_param_ratio_atomic(Proxy::ParamId::Z1ANC), 0.000001
    );
})


void turn_off_reset_for_all_rules(Proxy& proxy)
{
    for (size_t i = 0; i != Proxy::RULES; ++i) {
        proxy.rules[i].reset.set_value(Proxy::Reset::RST_OFF);
    }
}


TEST(when_sending_mcm_is_turned_off_then_does_not_send_mcm_on_reset, {
    Proxy proxy;

    turn_off_reset_for_all_rules(proxy);

    proxy.send_mcm.set_value(Proxy::Toggle::OFF);
    proxy.zone_type.set_value(Proxy::ZoneType::ZT_UPPER);
    proxy.channels.set_value(10);

    proxy.suspend();
    proxy.resume();
    proxy.begin_processing();

    assert_out_events<0>({}, proxy);
})


TEST(when_proxy_is_suspended_then_new_events_are_ignored, {
    Proxy proxy;

    proxy.send_mcm.set_value(Proxy::Toggle::ON);
    proxy.zone_type.set_value(Proxy::ZoneType::ZT_UPPER);
    proxy.channels.set_value(10);
    proxy.begin_processing();
    proxy.begin_processing();

    proxy.suspend();
    proxy.control_change(1.0, 5, 1, 110);
    proxy.pitch_wheel_change(2.0, 6, 10000);
    proxy.channel_pressure(3.0, 7, 30);
    proxy.note_on(1.0, 1, 60, 96);
    proxy.note_off(6.0, 1, 60, 96);

    assert_out_events<0>({}, proxy);

    proxy.send_mcm.set_value(Proxy::Toggle::OFF);
    proxy.resume();
    proxy.begin_processing();

    assert_out_events<0>({}, proxy);
})


TEST(when_proxy_is_resumed_then_previous_notes_are_turned_off, {
    Proxy proxy;

    proxy.note_on(1.0, 1, 60, 96);
    proxy.suspend();
    proxy.resume();
    proxy.begin_processing();

    assert_out_events<3>(
        {
            "t=0.000 cmd=CONTROL_CHANGE ch=0 d1=0x40 d2=0x00 (v=0.000)",
            "t=0.000 cmd=CONTROL_CHANGE ch=1 d1=0x40 d2=0x00 (v=0.000)",
            "t=0.000 cmd=NOTE_OFF ch=1 d1=0x3c d2=0x40 (v=0.504)",
        },
        proxy
    );
})


TEST(when_sending_mcm_is_turned_on_then_sends_mcm_on_reset, {
    Proxy proxy;

    turn_off_reset_for_all_rules(proxy);

    proxy.send_mcm.set_value(Proxy::Toggle::ON);
    proxy.zone_type.set_value(Proxy::ZoneType::ZT_UPPER);
    proxy.channels.set_value(10);

    proxy.suspend();
    proxy.resume();
    proxy.begin_processing();

    assert_out_events<6>(
        {
            "t=0.000 cmd=CONTROL_CHANGE ch=15 d1=0x65 d2=0x00 (v=0.000)",
            "t=0.000 cmd=CONTROL_CHANGE ch=15 d1=0x64 d2=0x06 (v=0.000)",
            "t=0.000 cmd=CONTROL_CHANGE ch=15 d1=0x06 d2=0x0a (v=0.000)",
            "t=0.000 cmd=CONTROL_CHANGE ch=0 d1=0x65 d2=0x00 (v=0.000)",
            "t=0.000 cmd=CONTROL_CHANGE ch=0 d1=0x64 d2=0x06 (v=0.000)",
            "t=0.000 cmd=CONTROL_CHANGE ch=0 d1=0x06 d2=0x00 (v=0.000)",
        },
        proxy
    );
})


TEST(resetting_clears_out_events, {
    Proxy proxy;

    turn_off_reset_for_all_rules(proxy);

    proxy.send_mcm.set_value(Proxy::Toggle::ON);
    proxy.zone_type.set_value(Proxy::ZoneType::ZT_UPPER);
    proxy.channels.set_value(10);

    proxy.suspend();
    proxy.resume();
    proxy.suspend();
    proxy.resume();
    proxy.suspend();
    proxy.resume();
    proxy.begin_processing();

    assert_out_events<6>(
        {
            "t=0.000 cmd=CONTROL_CHANGE ch=15 d1=0x65 d2=0x00 (v=0.000)",
            "t=0.000 cmd=CONTROL_CHANGE ch=15 d1=0x64 d2=0x06 (v=0.000)",
            "t=0.000 cmd=CONTROL_CHANGE ch=15 d1=0x06 d2=0x0a (v=0.000)",
            "t=0.000 cmd=CONTROL_CHANGE ch=0 d1=0x65 d2=0x00 (v=0.000)",
            "t=0.000 cmd=CONTROL_CHANGE ch=0 d1=0x64 d2=0x06 (v=0.000)",
            "t=0.000 cmd=CONTROL_CHANGE ch=0 d1=0x06 d2=0x00 (v=0.000)",
        },
        proxy
    );
})


TEST(when_settings_are_changed_then_processing_triggers_reset, {
    Proxy proxy;

    turn_off_reset_for_all_rules(proxy);

    proxy.send_mcm.set_value(Proxy::Toggle::ON);
    proxy.suspend();
    proxy.resume();
    proxy.begin_processing();

    proxy.note_on(0.0, 1, 60, 96);

    proxy.zone_type.set_value(Proxy::ZoneType::ZT_UPPER);
    proxy.channels.set_value(10);
    proxy.begin_processing();

    assert_out_events<9>(
        {
            "t=0.000 cmd=CONTROL_CHANGE ch=0 d1=0x40 d2=0x00 (v=0.000)",
            "t=0.000 cmd=CONTROL_CHANGE ch=1 d1=0x40 d2=0x00 (v=0.000)",
            "t=0.000 cmd=NOTE_OFF ch=1 d1=0x3c d2=0x40 (v=0.504)",
            "t=0.000 cmd=CONTROL_CHANGE ch=15 d1=0x65 d2=0x00 (v=0.000)",
            "t=0.000 cmd=CONTROL_CHANGE ch=15 d1=0x64 d2=0x06 (v=0.000)",
            "t=0.000 cmd=CONTROL_CHANGE ch=15 d1=0x06 d2=0x0a (v=0.000)",
            "t=0.000 cmd=CONTROL_CHANGE ch=0 d1=0x65 d2=0x00 (v=0.000)",
            "t=0.000 cmd=CONTROL_CHANGE ch=0 d1=0x64 d2=0x06 (v=0.000)",
            "t=0.000 cmd=CONTROL_CHANGE ch=0 d1=0x06 d2=0x00 (v=0.000)",
        },
        proxy
    );
})


TEST(when_settings_remain_unchanged_then_processing_does_not_trigger_reset, {
    Proxy proxy;

    proxy.suspend();
    proxy.resume();

    proxy.send_mcm.set_value(Proxy::Toggle::ON);
    proxy.zone_type.set_value(Proxy::ZoneType::ZT_UPPER);
    proxy.channels.set_value(10);

    proxy.begin_processing();
    proxy.begin_processing();
    proxy.begin_processing();

    assert_out_events<0>({}, proxy);
})


TEST(when_sending_mcm_is_turned_off_then_does_not_send_mcm_on_config_change, {
    Proxy proxy;

    turn_off_reset_for_all_rules(proxy);

    proxy.suspend();
    proxy.resume();
    proxy.begin_processing();

    set_param(proxy, Proxy::ParamId::MCM, 0.0);
    set_param(proxy, Proxy::ParamId::Z1TYP, 1.0);
    set_param(proxy, Proxy::ParamId::Z1CHN, proxy.channels.value_to_ratio(10));

    proxy.begin_processing();

    assert_out_events<0>({}, proxy);
})


TEST(when_sending_mcm_is_turned_on_then_sends_mcm_on_config_change, {
    Proxy proxy;

    turn_off_reset_for_all_rules(proxy);

    proxy.suspend();
    proxy.resume();
    proxy.begin_processing();

    set_param(proxy, Proxy::ParamId::MCM, 1.0);
    set_param(proxy, Proxy::ParamId::Z1TYP, 1.0);
    set_param(proxy, Proxy::ParamId::Z1CHN, proxy.channels.value_to_ratio(10));

    proxy.begin_processing();

    assert_out_events<6>(
        {
            "t=0.000 cmd=CONTROL_CHANGE ch=15 d1=0x65 d2=0x00 (v=0.000)",
            "t=0.000 cmd=CONTROL_CHANGE ch=15 d1=0x64 d2=0x06 (v=0.000)",
            "t=0.000 cmd=CONTROL_CHANGE ch=15 d1=0x06 d2=0x0a (v=0.000)",
            "t=0.000 cmd=CONTROL_CHANGE ch=0 d1=0x65 d2=0x00 (v=0.000)",
            "t=0.000 cmd=CONTROL_CHANGE ch=0 d1=0x64 d2=0x06 (v=0.000)",
            "t=0.000 cmd=CONTROL_CHANGE ch=0 d1=0x06 d2=0x00 (v=0.000)",
        },
        proxy
    );
})


TEST(when_a_cc_does_not_match_any_rule_then_it_is_sent_unmodified_via_the_manager_channel, {
    Proxy proxy;

    for (size_t i = 0; i != Proxy::RULES; ++i) {
        proxy.rules[i].in_cc.set_value(Proxy::ControllerId::NONE);
    }

    proxy.begin_processing();
    proxy.control_change(1.0, 5, 7, 110);
    proxy.pitch_wheel_change(2.0, 6, 10000);
    proxy.channel_pressure(3.0, 7, 30);

    assert_out_events<3>(
        {
            "t=1.000 cmd=CONTROL_CHANGE ch=0 d1=0x07 d2=0x6e (v=0.866)",
            "t=2.000 cmd=PITCH_BEND_CHANGE ch=0 d1=0x10 d2=0x4e (v=0.610)",
            "t=3.000 cmd=CHANNEL_PRESSURE ch=0 d1=0x1e d2=0x00 (v=0.236)",
        },
        proxy
    );
})


TEST(when_the_target_of_a_cc_is_global_then_it_is_sent_via_the_manager_channel, {
    Proxy proxy;

    proxy.rules[0].in_cc.set_value(Proxy::ControllerId::PITCH_WHEEL);
    proxy.rules[0].out_cc.set_value(Proxy::ControllerId::PITCH_WHEEL);
    proxy.rules[0].target.set_value(Proxy::Target::TRG_GLOBAL);

    proxy.rules[1].in_cc.set_value(Proxy::ControllerId::CHANNEL_PRESSURE);
    proxy.rules[1].out_cc.set_value(Proxy::ControllerId::CHANNEL_PRESSURE);
    proxy.rules[1].target.set_value(Proxy::Target::TRG_GLOBAL);

    proxy.rules[2].in_cc.set_value(Proxy::ControllerId::MODULATION_WHEEL);
    proxy.rules[2].out_cc.set_value(Proxy::ControllerId::SOUND_5);
    proxy.rules[2].target.set_value(Proxy::Target::TRG_GLOBAL);

    proxy.begin_processing();
    proxy.control_change(1.0, 5, 1, 110);
    proxy.pitch_wheel_change(2.0, 6, 10000);
    proxy.channel_pressure(3.0, 7, 30);

    assert_out_events<3>(
        {
            "t=1.000 cmd=CONTROL_CHANGE ch=0 d1=0x4a d2=0x6e (v=0.866)",
            "t=2.000 cmd=PITCH_BEND_CHANGE ch=0 d1=0x10 d2=0x4e (v=0.610)",
            "t=3.000 cmd=CHANNEL_PRESSURE ch=0 d1=0x1e d2=0x00 (v=0.236)",
        },
        proxy
    );
})


void test_out_cc_none(Proxy::Target const target)
{
    Proxy proxy;

    turn_off_reset_for_all_rules(proxy);

    proxy.rules[0].in_cc.set_value(Proxy::ControllerId::PITCH_WHEEL);
    proxy.rules[0].out_cc.set_value(Proxy::ControllerId::NONE);
    proxy.rules[0].target.set_value(target);

    proxy.rules[1].in_cc.set_value(Proxy::ControllerId::CHANNEL_PRESSURE);
    proxy.rules[1].out_cc.set_value(Proxy::ControllerId::NONE);
    proxy.rules[1].target.set_value(target);

    proxy.rules[2].in_cc.set_value(Proxy::ControllerId::MODULATION_WHEEL);
    proxy.rules[2].out_cc.set_value(Proxy::ControllerId::NONE);
    proxy.rules[2].target.set_value(target);

    proxy.note_on(0.0, 1, 60, 96);

    proxy.begin_processing();
    proxy.control_change(1.0, 5, 1, 110);
    proxy.pitch_wheel_change(2.0, 6, 10000);
    proxy.channel_pressure(3.0, 7, 30);

    assert_out_events<0>({}, proxy);
}


TEST(when_the_out_cc_of_a_rule_is_none_then_its_in_cc_is_swallowed, {
    test_out_cc_none(Proxy::Target::TRG_GLOBAL);
    test_out_cc_none(Proxy::Target::TRG_NEWEST);
})


TEST(repeated_cc_events_are_sent_only_once_on_the_manager_channel, {
    Proxy proxy;

    for (size_t i = 0; i != Proxy::RULES; ++i) {
        proxy.rules[i].in_cc.set_value(Proxy::ControllerId::NONE);
    }

    proxy.begin_processing();
    proxy.control_change(1.0, 5, 7, 110);
    proxy.control_change(1.0, 5, 7, 110);
    proxy.control_change(1.0, 5, 7, 110);
    proxy.control_change(1.0, 5, 7, 110);
    proxy.control_change(1.0, 5, 7, 110);

    assert_out_events<1>(
        {"t=1.000 cmd=CONTROL_CHANGE ch=0 d1=0x07 d2=0x6e (v=0.866)"},
        proxy
    );
})


TEST(allocates_new_channel_for_each_note, {
    Proxy proxy;

    turn_off_reset_for_all_rules(proxy);

    proxy.zone_type.set_value(Proxy::ZoneType::ZT_UPPER);
    proxy.channels.set_value(10);
    proxy.begin_processing();

    proxy.note_on(1.0, 1, 60, 96);
    proxy.note_on(2.0, 2, 72, 111);
    proxy.note_on(3.0, 2, 84, 127);

    assert_out_events<3>(
        {
            "t=1.000 cmd=NOTE_ON ch=14 d1=0x3c d2=0x60 (v=0.756)",
            "t=2.000 cmd=NOTE_ON ch=13 d1=0x48 d2=0x6f (v=0.874)",
            "t=3.000 cmd=NOTE_ON ch=12 d1=0x54 d2=0x7f (v=1.000)",
        },
        proxy
    );
})


TEST(can_ignore_new_notes_when_running_out_of_available_channels, {
    Proxy proxy;

    turn_off_reset_for_all_rules(proxy);

    proxy.zone_type.set_value(Proxy::ZoneType::ZT_UPPER);
    proxy.channels.set_value(3);
    proxy.excess_note_handling.set_value(Proxy::ExcessNoteHandling::ENH_IGNORE);
    proxy.begin_processing();

    proxy.note_on(1.0, 1, 60, 96);
    proxy.note_on(2.0, 2, 72, 111);
    proxy.note_on(3.0, 2, 84, 127);
    proxy.note_on(4.0, 3, 96, 127);
    proxy.note_on(5.0, 4, 98, 127);

    assert_out_events<3>(
        {
            "t=1.000 cmd=NOTE_ON ch=14 d1=0x3c d2=0x60 (v=0.756)",
            "t=2.000 cmd=NOTE_ON ch=13 d1=0x48 d2=0x6f (v=0.874)",
            "t=3.000 cmd=NOTE_ON ch=12 d1=0x54 d2=0x7f (v=1.000)",
        },
        proxy
    );
})


TEST(when_excess_notes_are_to_be_ignored_then_ignores_repeated_note_on, {
    Proxy proxy;

    turn_off_reset_for_all_rules(proxy);

    proxy.zone_type.set_value(Proxy::ZoneType::ZT_UPPER);
    proxy.excess_note_handling.set_value(Proxy::ExcessNoteHandling::ENH_IGNORE);
    proxy.begin_processing();

    proxy.note_on(1.0, 1, 60, 96);
    proxy.note_on(2.0, 2, 72, 111);
    proxy.note_on(3.0, 2, 84, 127);
    proxy.note_on(4.0, 1, 60, 127);
    proxy.note_on(5.0, 2, 60, 127);

    assert_out_events<3>(
        {
            "t=1.000 cmd=NOTE_ON ch=14 d1=0x3c d2=0x60 (v=0.756)",
            "t=2.000 cmd=NOTE_ON ch=13 d1=0x48 d2=0x6f (v=0.874)",
            "t=3.000 cmd=NOTE_ON ch=12 d1=0x54 d2=0x7f (v=1.000)",
        },
        proxy
    );
})


TEST(can_steal_channel_when_running_out_of_available_channels, {
    Proxy proxy;

    turn_off_reset_for_all_rules(proxy);

    proxy.zone_type.set_value(Proxy::ZoneType::ZT_UPPER);
    proxy.channels.set_value(3);
    proxy.excess_note_handling.set_value(
        Proxy::ExcessNoteHandling::ENH_STEAL_OLDEST
    );
    proxy.begin_processing();

    proxy.note_on(1.0, 1, 60, 96);
    proxy.note_on(2.0, 2, 72, 111);
    proxy.note_on(3.0, 2, 84, 127);
    proxy.note_on(4.0, 3, 96, 115);
    proxy.note_on(5.0, 4, 98, 120);

    assert_out_events<7>(
        {
            "t=1.000 cmd=NOTE_ON ch=14 d1=0x3c d2=0x60 (v=0.756)",
            "t=2.000 cmd=NOTE_ON ch=13 d1=0x48 d2=0x6f (v=0.874)",
            "t=3.000 cmd=NOTE_ON ch=12 d1=0x54 d2=0x7f (v=1.000)",
            "t=4.000 cmd=NOTE_OFF ch=14 d1=0x3c d2=0x40 (v=0.504)",
            "t=4.000 cmd=NOTE_ON ch=14 d1=0x60 d2=0x73 (v=0.906)",
            "t=5.000 cmd=NOTE_OFF ch=13 d1=0x48 d2=0x40 (v=0.504)",
            "t=5.000 cmd=NOTE_ON ch=13 d1=0x62 d2=0x78 (v=0.945)",
        },
        proxy
    );
})


TEST(when_excess_notes_are_to_steal_channels_then_repeated_note_on_replaces_existing_note, {
    Proxy proxy;

    turn_off_reset_for_all_rules(proxy);

    proxy.zone_type.set_value(Proxy::ZoneType::ZT_UPPER);
    proxy.excess_note_handling.set_value(Proxy::ExcessNoteHandling::ENH_STEAL_HIGHEST);
    proxy.begin_processing();

    proxy.note_on(1.0, 1, 60, 96);
    proxy.note_on(2.0, 2, 72, 111);
    proxy.note_on(3.0, 2, 84, 127);
    proxy.note_on(4.0, 1, 60, 127);
    proxy.note_on(5.0, 2, 60, 110);

    assert_out_events<7>(
        {
            "t=1.000 cmd=NOTE_ON ch=14 d1=0x3c d2=0x60 (v=0.756)",
            "t=2.000 cmd=NOTE_ON ch=13 d1=0x48 d2=0x6f (v=0.874)",
            "t=3.000 cmd=NOTE_ON ch=12 d1=0x54 d2=0x7f (v=1.000)",
            "t=4.000 cmd=NOTE_OFF ch=14 d1=0x3c d2=0x40 (v=0.504)",
            "t=4.000 cmd=NOTE_ON ch=14 d1=0x3c d2=0x7f (v=1.000)",
            "t=5.000 cmd=NOTE_OFF ch=14 d1=0x3c d2=0x40 (v=0.504)",
            "t=5.000 cmd=NOTE_ON ch=14 d1=0x3c d2=0x6e (v=0.866)",
        },
        proxy
    );
})


void test_global_cc_reset(Proxy::Reset const reset)
{
    Proxy proxy;

    proxy.rules[0].in_cc.set_value(Proxy::ControllerId::PITCH_WHEEL);
    proxy.rules[0].out_cc.set_value(Proxy::ControllerId::PITCH_WHEEL);
    proxy.rules[0].init_value.set_ratio(0.5);
    proxy.rules[0].target.set_value(Proxy::Target::TRG_GLOBAL);
    proxy.rules[0].reset.set_value(reset);

    proxy.rules[1].in_cc.set_value(Proxy::ControllerId::MODULATION_WHEEL);
    proxy.rules[1].out_cc.set_value(Proxy::ControllerId::CHANNEL_PRESSURE);
    proxy.rules[1].init_value.set_ratio(0.2);
    proxy.rules[1].target.set_value(Proxy::Target::TRG_GLOBAL);
    proxy.rules[1].reset.set_value(reset);

    proxy.rules[2].in_cc.set_value(Proxy::ControllerId::VOLUME);
    proxy.rules[2].out_cc.set_value(Proxy::ControllerId::SOUND_5);
    proxy.rules[2].init_value.set_ratio(0.3);
    proxy.rules[2].target.set_value(Proxy::Target::TRG_GLOBAL);
    proxy.rules[2].reset.set_value(reset);

    proxy.rules[3].in_cc.set_value(Proxy::ControllerId::CHANNEL_PRESSURE);
    proxy.rules[3].out_cc.set_value(Proxy::ControllerId::MODULATION_WHEEL);
    proxy.rules[3].init_value.set_ratio(0.123);
    proxy.rules[3].target.set_value(Proxy::Target::TRG_NEWEST);
    proxy.rules[3].reset.set_value(reset);

    proxy.pitch_wheel_change(0.0, 0, 16383);
    proxy.channel_pressure(0.0, 0, 127);
    proxy.control_change(0.0, 0, Proxy::ControllerId::MODULATION_WHEEL, 127);
    proxy.control_change(0.0, 0, Proxy::ControllerId::VOLUME, 127);

    proxy.suspend();
    proxy.resume();
    proxy.begin_processing();

    assert_out_events<3>(
        {
            "t=0.000 cmd=PITCH_BEND_CHANGE ch=0 d1=0x00 d2=0x40 (v=0.500)",
            "t=0.000 cmd=CHANNEL_PRESSURE ch=0 d1=0x19 d2=0x00 (v=0.200)",
            "t=0.000 cmd=CONTROL_CHANGE ch=0 d1=0x4a d2=0x26 (v=0.300)",
        },
        proxy
    );
}


TEST(when_reset_is_on_for_a_global_cc_event_then_its_initial_value_is_sent_on_reset, {
    test_global_cc_reset(Proxy::Reset::RST_INIT);
    test_global_cc_reset(Proxy::Reset::RST_LAST);
})


TEST(when_reset_is_set_to_init_for_a_non_global_cc_event_then_its_initial_value_is_sent_before_note_on, {
    Proxy proxy;

    proxy.zone_type.set_value(Proxy::ZoneType::ZT_UPPER);
    proxy.channels.set_value(2);
    proxy.excess_note_handling.set_value(
        Proxy::ExcessNoteHandling::ENH_STEAL_OLDEST
    );

    proxy.rules[0].in_cc.set_value(Proxy::ControllerId::PITCH_WHEEL);
    proxy.rules[0].out_cc.set_value(Proxy::ControllerId::PITCH_WHEEL);
    proxy.rules[0].init_value.set_ratio(0.5);
    proxy.rules[0].target.set_value(Proxy::Target::TRG_NEWEST);
    proxy.rules[0].reset.set_value(Proxy::Reset::RST_INIT);

    proxy.rules[1].in_cc.set_value(Proxy::ControllerId::MODULATION_WHEEL);
    proxy.rules[1].out_cc.set_value(Proxy::ControllerId::CHANNEL_PRESSURE);
    proxy.rules[1].init_value.set_ratio(0.2);
    proxy.rules[1].target.set_value(Proxy::Target::TRG_HIGHEST);
    proxy.rules[1].reset.set_value(Proxy::Reset::RST_INIT);

    proxy.rules[2].in_cc.set_value(Proxy::ControllerId::VOLUME);
    proxy.rules[2].out_cc.set_value(Proxy::ControllerId::SOUND_5);
    proxy.rules[2].init_value.set_ratio(0.5);
    proxy.rules[2].target.set_value(Proxy::Target::TRG_LOWEST_ABOVE_ANCHOR);
    proxy.rules[2].reset.set_value(Proxy::Reset::RST_INIT);

    proxy.begin_processing();

    proxy.note_on(1.0, 1, 60, 96);
    proxy.note_on(2.0, 2, 72, 111);
    proxy.note_on(3.0, 2, 84, 127);

    assert_out_events<26>(
        {
            "t=1.000 cmd=PITCH_BEND_CHANGE ch=14 d1=0x00 d2=0x40 (v=0.500) pre-NOTE_ON setup",
            "t=1.000 cmd=CHANNEL_PRESSURE ch=14 d1=0x19 d2=0x00 (v=0.200) pre-NOTE_ON setup",
            "t=1.000 cmd=CONTROL_CHANGE ch=14 d1=0x4a d2=0x40 (v=0.500) pre-NOTE_ON setup",
            "t=1.000 cmd=NOTE_ON ch=14 d1=0x3c d2=0x60 (v=0.756)",
            "t=1.000 cmd=PITCH_BEND_CHANGE ch=14 d1=0x00 d2=0x40 (v=0.500)",
            "t=1.000 cmd=CHANNEL_PRESSURE ch=14 d1=0x19 d2=0x00 (v=0.200)",
            "t=1.000 cmd=CONTROL_CHANGE ch=14 d1=0x4a d2=0x40 (v=0.500)",
            "t=2.000 cmd=PITCH_BEND_CHANGE ch=14 d1=0x00 d2=0x40 (v=0.500)",
            "t=2.000 cmd=PITCH_BEND_CHANGE ch=13 d1=0x00 d2=0x40 (v=0.500) pre-NOTE_ON setup",
            "t=2.000 cmd=CHANNEL_PRESSURE ch=14 d1=0x19 d2=0x00 (v=0.200)",
            "t=2.000 cmd=CHANNEL_PRESSURE ch=13 d1=0x19 d2=0x00 (v=0.200) pre-NOTE_ON setup",
            "t=2.000 cmd=CONTROL_CHANGE ch=13 d1=0x4a d2=0x40 (v=0.500) pre-NOTE_ON setup",
            "t=2.000 cmd=NOTE_ON ch=13 d1=0x48 d2=0x6f (v=0.874)",
            "t=2.000 cmd=PITCH_BEND_CHANGE ch=13 d1=0x00 d2=0x40 (v=0.500)",
            "t=2.000 cmd=CHANNEL_PRESSURE ch=13 d1=0x19 d2=0x00 (v=0.200)",
            "t=2.000 cmd=CONTROL_CHANGE ch=13 d1=0x4a d2=0x40 (v=0.500)",
            "t=3.000 cmd=NOTE_OFF ch=14 d1=0x3c d2=0x40 (v=0.504)",
            "t=3.000 cmd=PITCH_BEND_CHANGE ch=13 d1=0x00 d2=0x40 (v=0.500)",
            "t=3.000 cmd=PITCH_BEND_CHANGE ch=14 d1=0x00 d2=0x40 (v=0.500) pre-NOTE_ON setup",
            "t=3.000 cmd=CHANNEL_PRESSURE ch=13 d1=0x19 d2=0x00 (v=0.200)",
            "t=3.000 cmd=CHANNEL_PRESSURE ch=14 d1=0x19 d2=0x00 (v=0.200) pre-NOTE_ON setup",
            "t=3.000 cmd=CONTROL_CHANGE ch=14 d1=0x4a d2=0x40 (v=0.500) pre-NOTE_ON setup",
            "t=3.000 cmd=NOTE_ON ch=14 d1=0x54 d2=0x7f (v=1.000)",
            "t=3.000 cmd=PITCH_BEND_CHANGE ch=14 d1=0x00 d2=0x40 (v=0.500)",
            "t=3.000 cmd=CHANNEL_PRESSURE ch=14 d1=0x19 d2=0x00 (v=0.200)",
            "t=3.000 cmd=CONTROL_CHANGE ch=14 d1=0x4a d2=0x40 (v=0.500)",
        },
        proxy
    );
})


TEST(when_reset_is_set_to_last_for_a_non_global_cc_event_then_its_last_value_is_sent_before_note_on, {
    Proxy proxy;

    proxy.zone_type.set_value(Proxy::ZoneType::ZT_UPPER);
    proxy.channels.set_value(2);
    proxy.excess_note_handling.set_value(
        Proxy::ExcessNoteHandling::ENH_STEAL_OLDEST
    );

    proxy.rules[0].in_cc.set_value(Proxy::ControllerId::PITCH_WHEEL);
    proxy.rules[0].out_cc.set_value(Proxy::ControllerId::PITCH_WHEEL);
    proxy.rules[0].init_value.set_ratio(0.5);
    proxy.rules[0].target.set_value(Proxy::Target::TRG_NEWEST);
    proxy.rules[0].reset.set_value(Proxy::Reset::RST_LAST);

    proxy.rules[1].in_cc.set_value(Proxy::ControllerId::MODULATION_WHEEL);
    proxy.rules[1].out_cc.set_value(Proxy::ControllerId::CHANNEL_PRESSURE);
    proxy.rules[1].init_value.set_ratio(0.2);
    proxy.rules[1].target.set_value(Proxy::Target::TRG_HIGHEST);
    proxy.rules[1].reset.set_value(Proxy::Reset::RST_LAST);

    proxy.rules[2].in_cc.set_value(Proxy::ControllerId::VOLUME);
    proxy.rules[2].out_cc.set_value(Proxy::ControllerId::SOUND_5);
    proxy.rules[2].init_value.set_ratio(0.5);
    proxy.rules[2].target.set_value(Proxy::Target::TRG_LOWEST_ABOVE_ANCHOR);
    proxy.rules[2].reset.set_value(Proxy::Reset::RST_LAST);

    proxy.begin_processing();

    proxy.pitch_wheel_change(0.0, 0, 16383);
    proxy.control_change(0.0, 0, Proxy::ControllerId::MODULATION_WHEEL, 127);
    proxy.control_change(0.0, 0, Proxy::ControllerId::VOLUME, 127);

    proxy.begin_processing();

    proxy.note_on(1.0, 1, 60, 96);
    proxy.note_on(2.0, 2, 72, 111);
    proxy.note_on(3.0, 2, 84, 127);

    assert_out_events<26>(
        {
            "t=1.000 cmd=PITCH_BEND_CHANGE ch=14 d1=0x7f d2=0x7f (v=1.000) pre-NOTE_ON setup",
            "t=1.000 cmd=CHANNEL_PRESSURE ch=14 d1=0x7f d2=0x00 (v=1.000) pre-NOTE_ON setup",
            "t=1.000 cmd=CONTROL_CHANGE ch=14 d1=0x4a d2=0x7f (v=1.000) pre-NOTE_ON setup",
            "t=1.000 cmd=NOTE_ON ch=14 d1=0x3c d2=0x60 (v=0.756)",
            "t=1.000 cmd=PITCH_BEND_CHANGE ch=14 d1=0x7f d2=0x7f (v=1.000)",
            "t=1.000 cmd=CHANNEL_PRESSURE ch=14 d1=0x7f d2=0x00 (v=1.000)",
            "t=1.000 cmd=CONTROL_CHANGE ch=14 d1=0x4a d2=0x7f (v=1.000)",
            "t=2.000 cmd=PITCH_BEND_CHANGE ch=14 d1=0x7f d2=0x7f (v=1.000)",
            "t=2.000 cmd=PITCH_BEND_CHANGE ch=13 d1=0x7f d2=0x7f (v=1.000) pre-NOTE_ON setup",
            "t=2.000 cmd=CHANNEL_PRESSURE ch=14 d1=0x7f d2=0x00 (v=1.000)",
            "t=2.000 cmd=CHANNEL_PRESSURE ch=13 d1=0x7f d2=0x00 (v=1.000) pre-NOTE_ON setup",
            "t=2.000 cmd=CONTROL_CHANGE ch=13 d1=0x4a d2=0x7f (v=1.000) pre-NOTE_ON setup",
            "t=2.000 cmd=NOTE_ON ch=13 d1=0x48 d2=0x6f (v=0.874)",
            "t=2.000 cmd=PITCH_BEND_CHANGE ch=13 d1=0x7f d2=0x7f (v=1.000)",
            "t=2.000 cmd=CHANNEL_PRESSURE ch=13 d1=0x7f d2=0x00 (v=1.000)",
            "t=2.000 cmd=CONTROL_CHANGE ch=13 d1=0x4a d2=0x7f (v=1.000)",
            "t=3.000 cmd=NOTE_OFF ch=14 d1=0x3c d2=0x40 (v=0.504)",
            "t=3.000 cmd=PITCH_BEND_CHANGE ch=13 d1=0x7f d2=0x7f (v=1.000)",
            "t=3.000 cmd=PITCH_BEND_CHANGE ch=14 d1=0x7f d2=0x7f (v=1.000) pre-NOTE_ON setup",
            "t=3.000 cmd=CHANNEL_PRESSURE ch=13 d1=0x7f d2=0x00 (v=1.000)",
            "t=3.000 cmd=CHANNEL_PRESSURE ch=14 d1=0x7f d2=0x00 (v=1.000) pre-NOTE_ON setup",
            "t=3.000 cmd=CONTROL_CHANGE ch=14 d1=0x4a d2=0x7f (v=1.000) pre-NOTE_ON setup",
            "t=3.000 cmd=NOTE_ON ch=14 d1=0x54 d2=0x7f (v=1.000)",
            "t=3.000 cmd=PITCH_BEND_CHANGE ch=14 d1=0x7f d2=0x7f (v=1.000)",
            "t=3.000 cmd=CHANNEL_PRESSURE ch=14 d1=0x7f d2=0x00 (v=1.000)",
            "t=3.000 cmd=CONTROL_CHANGE ch=14 d1=0x4a d2=0x7f (v=1.000)",
        },
        proxy
    );
})


TEST(excess_note_off_events_are_ignored, {
    Proxy proxy;

    turn_off_reset_for_all_rules(proxy);

    proxy.zone_type.set_value(Proxy::ZoneType::ZT_UPPER);
    proxy.channels.set_value(3);
    proxy.excess_note_handling.set_value(Proxy::ExcessNoteHandling::ENH_IGNORE);
    proxy.begin_processing();

    proxy.note_off(0.5, 1, 60, 64);
    proxy.note_on(1.0, 1, 60, 96);
    proxy.note_on(2.0, 2, 72, 111);
    proxy.note_on(3.0, 2, 84, 127);
    proxy.note_on(4.0, 3, 96, 127);
    proxy.note_on(5.0, 4, 98, 127);
    proxy.note_off(6.0, 1, 60, 96);
    proxy.note_off(7.0, 2, 72, 111);
    proxy.note_off(8.0, 2, 84, 127);
    proxy.note_off(9.0, 3, 96, 127);
    proxy.note_off(10.0, 4, 98, 127);

    assert_out_events<6>(
        {
            "t=1.000 cmd=NOTE_ON ch=14 d1=0x3c d2=0x60 (v=0.756)",
            "t=2.000 cmd=NOTE_ON ch=13 d1=0x48 d2=0x6f (v=0.874)",
            "t=3.000 cmd=NOTE_ON ch=12 d1=0x54 d2=0x7f (v=1.000)",
            "t=6.000 cmd=NOTE_OFF ch=14 d1=0x3c d2=0x60 (v=0.756)",
            "t=7.000 cmd=NOTE_OFF ch=13 d1=0x48 d2=0x6f (v=0.874)",
            "t=8.000 cmd=NOTE_OFF ch=12 d1=0x54 d2=0x7f (v=1.000)",
        },
        proxy
    );
})


TEST(note_off_velocity_can_be_overridden_with_note_on_velocity, {
    Proxy proxy;

    turn_off_reset_for_all_rules(proxy);

    proxy.channels.set_value(1);
    proxy.begin_processing();

    proxy.excess_note_handling.set_value(
        Proxy::ExcessNoteHandling::ENH_STEAL_OLDEST
    );
    proxy.override_release_velocity.set_value(Proxy::Toggle::ON);
    proxy.note_on(0.0, 0, 60, 16);
    proxy.begin_processing();

    proxy.note_on(1.0, 0, 62, 32);
    proxy.note_off(2.0, 0, 62, 48);

    assert_out_events<3>(
        {
            "t=1.000 cmd=NOTE_OFF ch=1 d1=0x3c d2=0x10 (v=0.126)",
            "t=1.000 cmd=NOTE_ON ch=1 d1=0x3e d2=0x20 (v=0.252)",
            "t=2.000 cmd=NOTE_OFF ch=1 d1=0x3e d2=0x20 (v=0.252)",
        },
        proxy
    );
})


TEST(when_a_note_is_released_then_its_channel_can_be_allocated_for_new_notes, {
    Proxy proxy;

    turn_off_reset_for_all_rules(proxy);

    proxy.channels.set_value(2);
    proxy.excess_note_handling.set_value(Proxy::ExcessNoteHandling::ENH_IGNORE);
    proxy.begin_processing();

    proxy.note_on(0.0, 0, 60, 127);
    proxy.note_on(1.0, 0, 62, 127);
    proxy.note_off(2.0, 0, 62, 127);
    proxy.begin_processing();

    proxy.note_on(3.0, 0, 63, 96);

    assert_out_events<1>(
        {"t=3.000 cmd=NOTE_ON ch=2 d1=0x3f d2=0x60 (v=0.756)"},
        proxy
    );
})


TEST(when_the_target_of_a_cc_is_not_global_then_it_is_sent_only_on_the_channel_of_the_selected_note, {
    Proxy proxy;

    proxy.rules[0].in_cc.set_value(Proxy::ControllerId::PITCH_WHEEL);
    proxy.rules[0].out_cc.set_value(Proxy::ControllerId::PITCH_WHEEL);
    proxy.rules[0].target.set_value(Proxy::Target::TRG_HIGHEST);

    proxy.rules[1].in_cc.set_value(Proxy::ControllerId::CHANNEL_PRESSURE);
    proxy.rules[1].out_cc.set_value(Proxy::ControllerId::CHANNEL_PRESSURE);
    proxy.rules[1].target.set_value(Proxy::Target::TRG_NEWEST);

    proxy.rules[2].in_cc.set_value(Proxy::ControllerId::MODULATION_WHEEL);
    proxy.rules[2].out_cc.set_value(Proxy::ControllerId::SOUND_5);
    proxy.rules[2].target.set_value(Proxy::Target::TRG_OLDEST);

    proxy.rules[3].in_cc.set_value(Proxy::ControllerId::VOLUME);
    proxy.rules[3].out_cc.set_value(Proxy::ControllerId::VOLUME);
    proxy.rules[3].target.set_value(Proxy::Target::TRG_LOWEST);

    proxy.note_on(0.1, 0, 60, 127);     /* channel=1, oldest */
    proxy.note_on(0.2, 0, 67, 127);     /* channel=2, highest */
    proxy.note_on(0.3, 0, 48, 127);     /* channel=3, lowest */
    proxy.note_on(0.4, 0, 64, 127);     /* channel=4, newest */
    proxy.begin_processing();

    proxy.control_change(1.0, 5, 1, 110);
    proxy.pitch_wheel_change(2.0, 6, 10000);
    proxy.control_change(3.0, 8, 7, 96);
    proxy.channel_pressure(4.0, 7, 30);

    assert_out_events<4>(
        {
            "t=1.000 cmd=CONTROL_CHANGE ch=1 d1=0x4a d2=0x6e (v=0.866)",
            "t=2.000 cmd=PITCH_BEND_CHANGE ch=2 d1=0x10 d2=0x4e (v=0.610)",
            "t=3.000 cmd=CONTROL_CHANGE ch=3 d1=0x07 d2=0x60 (v=0.756)",
            "t=4.000 cmd=CHANNEL_PRESSURE ch=4 d1=0x1e d2=0x00 (v=0.236)",
        },
        proxy
    );
})


TEST(target_of_a_cc_may_be_below_the_anchor, {
    Proxy proxy;

    proxy.anchor.set_value(72);

    proxy.rules[0].in_cc.set_value(Proxy::ControllerId::PITCH_WHEEL);
    proxy.rules[0].out_cc.set_value(Proxy::ControllerId::PITCH_WHEEL);
    proxy.rules[0].target.set_value(Proxy::Target::TRG_HIGHEST_BELOW_ANCHOR);

    proxy.rules[1].in_cc.set_value(Proxy::ControllerId::CHANNEL_PRESSURE);
    proxy.rules[1].out_cc.set_value(Proxy::ControllerId::CHANNEL_PRESSURE);
    proxy.rules[1].target.set_value(Proxy::Target::TRG_NEWEST_BELOW_ANCHOR);

    proxy.rules[2].in_cc.set_value(Proxy::ControllerId::MODULATION_WHEEL);
    proxy.rules[2].out_cc.set_value(Proxy::ControllerId::SOUND_5);
    proxy.rules[2].target.set_value(Proxy::Target::TRG_OLDEST_BELOW_ANCHOR);

    proxy.rules[3].in_cc.set_value(Proxy::ControllerId::VOLUME);
    proxy.rules[3].out_cc.set_value(Proxy::ControllerId::VOLUME);
    proxy.rules[3].target.set_value(Proxy::Target::TRG_LOWEST_BELOW_ANCHOR);

    proxy.note_on(0.1, 0, 60, 127);     /* channel=1, oldest below anchor */
    proxy.note_on(0.2, 0, 67, 127);     /* channel=2, highest below anchor */
    proxy.note_on(0.3, 0, 48, 127);     /* channel=3, lowest below anchor */
    proxy.note_on(0.4, 0, 64, 127);     /* channel=4, newest below anchor */
    proxy.note_on(0.5, 0, 72, 127);     /* channel=5, above anchor */
    proxy.begin_processing();

    proxy.control_change(1.0, 5, 1, 110);
    proxy.pitch_wheel_change(2.0, 6, 10000);
    proxy.control_change(3.0, 8, 7, 96);
    proxy.channel_pressure(4.0, 7, 30);

    assert_out_events<4>(
        {
            "t=1.000 cmd=CONTROL_CHANGE ch=1 d1=0x4a d2=0x6e (v=0.866)",
            "t=2.000 cmd=PITCH_BEND_CHANGE ch=2 d1=0x10 d2=0x4e (v=0.610)",
            "t=3.000 cmd=CONTROL_CHANGE ch=3 d1=0x07 d2=0x60 (v=0.756)",
            "t=4.000 cmd=CHANNEL_PRESSURE ch=4 d1=0x1e d2=0x00 (v=0.236)",
        },
        proxy
    );
})


TEST(target_of_a_cc_may_be_above_the_anchor, {
    Proxy proxy;

    proxy.anchor.set_value(37);

    proxy.rules[0].in_cc.set_value(Proxy::ControllerId::PITCH_WHEEL);
    proxy.rules[0].out_cc.set_value(Proxy::ControllerId::PITCH_WHEEL);
    proxy.rules[0].target.set_value(Proxy::Target::TRG_HIGHEST_ABOVE_ANCHOR);

    proxy.rules[1].in_cc.set_value(Proxy::ControllerId::CHANNEL_PRESSURE);
    proxy.rules[1].out_cc.set_value(Proxy::ControllerId::CHANNEL_PRESSURE);
    proxy.rules[1].target.set_value(Proxy::Target::TRG_NEWEST_ABOVE_ANCHOR);

    proxy.rules[2].in_cc.set_value(Proxy::ControllerId::MODULATION_WHEEL);
    proxy.rules[2].out_cc.set_value(Proxy::ControllerId::SOUND_5);
    proxy.rules[2].target.set_value(Proxy::Target::TRG_OLDEST_ABOVE_ANCHOR);

    proxy.rules[3].in_cc.set_value(Proxy::ControllerId::VOLUME);
    proxy.rules[3].out_cc.set_value(Proxy::ControllerId::VOLUME);
    proxy.rules[3].target.set_value(Proxy::Target::TRG_LOWEST_ABOVE_ANCHOR);

    proxy.note_on(0.1, 0, 60, 127);     /* channel=1, oldest above anchor */
    proxy.note_on(0.2, 0, 67, 127);     /* channel=2, highest above anchor */
    proxy.note_on(0.3, 0, 48, 127);     /* channel=3, lowest above anchor */
    proxy.note_on(0.4, 0, 64, 127);     /* channel=4, newest above anchor */
    proxy.note_on(0.5, 0, 36, 127);     /* channel=5, below anchor */
    proxy.begin_processing();

    proxy.control_change(1.0, 5, 1, 110);
    proxy.pitch_wheel_change(2.0, 6, 10000);
    proxy.control_change(3.0, 8, 7, 96);
    proxy.channel_pressure(4.0, 7, 30);

    assert_out_events<4>(
        {
            "t=1.000 cmd=CONTROL_CHANGE ch=1 d1=0x4a d2=0x6e (v=0.866)",
            "t=2.000 cmd=PITCH_BEND_CHANGE ch=2 d1=0x10 d2=0x4e (v=0.610)",
            "t=3.000 cmd=CONTROL_CHANGE ch=3 d1=0x07 d2=0x60 (v=0.756)",
            "t=4.000 cmd=CHANNEL_PRESSURE ch=4 d1=0x1e d2=0x00 (v=0.236)",
        },
        proxy
    );
})


TEST(when_cc_target_is_below_the_anchor_but_all_notes_are_above_it_then_cc_is_dropped, {
    Proxy proxy;

    proxy.anchor.set_value(72);

    proxy.rules[0].in_cc.set_value(Proxy::ControllerId::PITCH_WHEEL);
    proxy.rules[0].out_cc.set_value(Proxy::ControllerId::PITCH_WHEEL);
    proxy.rules[0].target.set_value(Proxy::Target::TRG_HIGHEST_BELOW_ANCHOR);

    proxy.rules[1].in_cc.set_value(Proxy::ControllerId::CHANNEL_PRESSURE);
    proxy.rules[1].out_cc.set_value(Proxy::ControllerId::CHANNEL_PRESSURE);
    proxy.rules[1].target.set_value(Proxy::Target::TRG_NEWEST_BELOW_ANCHOR);

    proxy.rules[2].in_cc.set_value(Proxy::ControllerId::MODULATION_WHEEL);
    proxy.rules[2].out_cc.set_value(Proxy::ControllerId::SOUND_5);
    proxy.rules[2].target.set_value(Proxy::Target::TRG_OLDEST_BELOW_ANCHOR);

    proxy.rules[3].in_cc.set_value(Proxy::ControllerId::VOLUME);
    proxy.rules[3].out_cc.set_value(Proxy::ControllerId::VOLUME);
    proxy.rules[3].target.set_value(Proxy::Target::TRG_LOWEST_BELOW_ANCHOR);

    proxy.note_on(0.1, 0, 60, 127);
    proxy.note_on(0.2, 0, 67, 127);
    proxy.note_on(0.3, 0, 48, 127);
    proxy.note_on(0.4, 0, 64, 127);
    proxy.note_on(0.5, 0, 72, 127);
    proxy.note_off(0.6, 0, 60, 64);
    proxy.note_off(0.7, 0, 67, 64);
    proxy.note_off(0.8, 0, 48, 64);
    proxy.note_off(0.9, 0, 64, 64);
    proxy.begin_processing();

    proxy.control_change(1.0, 5, 1, 110);
    proxy.pitch_wheel_change(2.0, 6, 10000);
    proxy.control_change(3.0, 8, 7, 96);
    proxy.channel_pressure(4.0, 7, 30);

    assert_out_events<0>({}, proxy);
})


TEST(when_cc_target_is_above_the_anchor_but_all_notes_are_below_it_then_cc_is_dropped, {
    Proxy proxy;

    proxy.anchor.set_value(37);

    proxy.rules[0].in_cc.set_value(Proxy::ControllerId::PITCH_WHEEL);
    proxy.rules[0].out_cc.set_value(Proxy::ControllerId::PITCH_WHEEL);
    proxy.rules[0].target.set_value(Proxy::Target::TRG_HIGHEST_ABOVE_ANCHOR);

    proxy.rules[1].in_cc.set_value(Proxy::ControllerId::CHANNEL_PRESSURE);
    proxy.rules[1].out_cc.set_value(Proxy::ControllerId::CHANNEL_PRESSURE);
    proxy.rules[1].target.set_value(Proxy::Target::TRG_NEWEST_ABOVE_ANCHOR);

    proxy.rules[2].in_cc.set_value(Proxy::ControllerId::MODULATION_WHEEL);
    proxy.rules[2].out_cc.set_value(Proxy::ControllerId::SOUND_5);
    proxy.rules[2].target.set_value(Proxy::Target::TRG_OLDEST_ABOVE_ANCHOR);

    proxy.rules[3].in_cc.set_value(Proxy::ControllerId::VOLUME);
    proxy.rules[3].out_cc.set_value(Proxy::ControllerId::VOLUME);
    proxy.rules[3].target.set_value(Proxy::Target::TRG_LOWEST_ABOVE_ANCHOR);

    proxy.note_on(0.1, 0, 60, 127);
    proxy.note_on(0.2, 0, 67, 127);
    proxy.note_on(0.3, 0, 48, 127);
    proxy.note_on(0.4, 0, 64, 127);
    proxy.note_on(0.5, 0, 36, 127);
    proxy.note_off(0.6, 0, 60, 64);
    proxy.note_off(0.7, 0, 67, 64);
    proxy.note_off(0.8, 0, 48, 64);
    proxy.note_off(0.9, 0, 64, 64);
    proxy.begin_processing();

    proxy.control_change(1.0, 5, 1, 110);
    proxy.pitch_wheel_change(2.0, 6, 10000);
    proxy.control_change(3.0, 8, 7, 96);
    proxy.channel_pressure(4.0, 7, 30);

    assert_out_events<0>({}, proxy);
})


TEST(when_reset_is_set_to_init_and_cc_target_changes_then_cc_is_reset_for_previous_note, {
    Proxy proxy;

    turn_off_reset_for_all_rules(proxy);

    proxy.rules[0].in_cc.set_value(Proxy::ControllerId::PITCH_WHEEL);
    proxy.rules[0].out_cc.set_value(Proxy::ControllerId::PITCH_WHEEL);
    proxy.rules[0].target.set_value(Proxy::Target::TRG_OLDEST);
    proxy.rules[0].init_value.set_ratio(0.5);
    proxy.rules[0].reset.set_value(Proxy::Reset::RST_INIT);

    proxy.rules[1].in_cc.set_value(Proxy::ControllerId::CHANNEL_PRESSURE);
    proxy.rules[1].out_cc.set_value(Proxy::ControllerId::CHANNEL_PRESSURE);
    proxy.rules[1].target.set_value(Proxy::Target::TRG_NEWEST);
    proxy.rules[1].init_value.set_ratio(0.2);
    proxy.rules[1].reset.set_value(Proxy::Reset::RST_INIT);

    proxy.note_on(0.0, 0, 48, 127);     /* channel=1, oldest */
    proxy.note_on(1.0, 0, 60, 127);     /* channel=2, newest */
    proxy.channel_pressure(2.0, 0, 100);
    proxy.pitch_wheel_change(3.0, 0, 16383);
    proxy.begin_processing();

    proxy.note_on(0.0, 0, 72, 127);     /* channel=3, newest */
    proxy.channel_pressure(1.0, 0, 127);

    assert_out_events<7>(
        {
            "t=0.000 cmd=PITCH_BEND_CHANGE ch=3 d1=0x00 d2=0x40 (v=0.500) pre-NOTE_ON setup",
            "t=0.000 cmd=CHANNEL_PRESSURE ch=2 d1=0x19 d2=0x00 (v=0.200)",
            "t=0.000 cmd=CHANNEL_PRESSURE ch=3 d1=0x19 d2=0x00 (v=0.200) pre-NOTE_ON setup",
            "t=0.000 cmd=NOTE_ON ch=3 d1=0x48 d2=0x7f (v=1.000)",
            "t=0.000 cmd=PITCH_BEND_CHANGE ch=3 d1=0x00 d2=0x40 (v=0.500)",
            "t=0.000 cmd=CHANNEL_PRESSURE ch=3 d1=0x19 d2=0x00 (v=0.200)",
            "t=1.000 cmd=CHANNEL_PRESSURE ch=3 d1=0x7f d2=0x00 (v=1.000)",
        },
        proxy
    );
})


TEST(when_reset_is_set_to_last_and_cc_target_changes_then_cc_is_reset_for_previous_note, {
    Proxy proxy;

    turn_off_reset_for_all_rules(proxy);

    proxy.rules[0].in_cc.set_value(Proxy::ControllerId::PITCH_WHEEL);
    proxy.rules[0].out_cc.set_value(Proxy::ControllerId::PITCH_WHEEL);
    proxy.rules[0].target.set_value(Proxy::Target::TRG_OLDEST);
    proxy.rules[0].init_value.set_ratio(0.5);
    proxy.rules[0].reset.set_value(Proxy::Reset::RST_LAST);

    proxy.rules[1].in_cc.set_value(Proxy::ControllerId::CHANNEL_PRESSURE);
    proxy.rules[1].out_cc.set_value(Proxy::ControllerId::CHANNEL_PRESSURE);
    proxy.rules[1].target.set_value(Proxy::Target::TRG_NEWEST);
    proxy.rules[1].init_value.set_ratio(0.2);
    proxy.rules[1].reset.set_value(Proxy::Reset::RST_LAST);

    proxy.note_on(0.0, 0, 48, 127);     /* channel=1, oldest */
    proxy.note_on(1.0, 0, 60, 127);     /* channel=2, newest */
    proxy.channel_pressure(2.0, 0, 127);
    proxy.pitch_wheel_change(3.0, 0, 16383);
    proxy.begin_processing();

    proxy.note_on(0.0, 0, 72, 127);     /* channel=3, newest */
    proxy.channel_pressure(1.0, 0, 96);

    assert_out_events<7>(
        {
            "t=0.000 cmd=PITCH_BEND_CHANGE ch=3 d1=0x7f d2=0x7f (v=1.000) pre-NOTE_ON setup",
            "t=0.000 cmd=CHANNEL_PRESSURE ch=2 d1=0x7f d2=0x00 (v=1.000)",
            "t=0.000 cmd=CHANNEL_PRESSURE ch=3 d1=0x7f d2=0x00 (v=1.000) pre-NOTE_ON setup",
            "t=0.000 cmd=NOTE_ON ch=3 d1=0x48 d2=0x7f (v=1.000)",
            "t=0.000 cmd=PITCH_BEND_CHANGE ch=3 d1=0x7f d2=0x7f (v=1.000)",
            "t=0.000 cmd=CHANNEL_PRESSURE ch=3 d1=0x7f d2=0x00 (v=1.000)",
            "t=1.000 cmd=CHANNEL_PRESSURE ch=3 d1=0x60 d2=0x00 (v=0.756)",
        },
        proxy
    );
})


TEST(when_the_in_cc_of_a_rule_is_midi_learn_then_it_is_replaced_with_the_first_controller_message, {
    Proxy proxy;

    proxy.rules[0].in_cc.set_value(Proxy::ControllerId::MIDI_LEARN);
    proxy.rules[0].out_cc.set_value(Proxy::ControllerId::PITCH_WHEEL);
    proxy.rules[0].target.set_value(Proxy::Target::TRG_OLDEST);

    proxy.rules[1].in_cc.set_value(Proxy::ControllerId::MIDI_LEARN);
    proxy.rules[1].out_cc.set_value(Proxy::ControllerId::SOUND_5);
    proxy.rules[1].target.set_value(Proxy::Target::TRG_NEWEST);

    proxy.note_on(0.0, 0, 48, 127);     /* channel=1, oldest */
    proxy.note_on(1.0, 0, 60, 127);     /* channel=3, newest */
    proxy.begin_processing();

    proxy.channel_pressure(0.0, 0, 96);

    assert_out_events<2>(
        {
            "t=0.000 cmd=PITCH_BEND_CHANGE ch=1 d1=0x60 d2=0x60 (v=0.756)",
            "t=0.000 cmd=CONTROL_CHANGE ch=2 d1=0x4a d2=0x60 (v=0.756)",
        },
        proxy
    );

    assert_eq(
        (int)Proxy::ControllerId::CHANNEL_PRESSURE,
        (int)proxy.rules[0].in_cc.get_value()
    );
    assert_eq(
        (int)Proxy::ControllerId::CHANNEL_PRESSURE,
        (int)proxy.rules[1].in_cc.get_value()
    );
})


TEST(cc_can_be_inverted, {
    Proxy proxy;

    turn_off_reset_for_all_rules(proxy);

    proxy.rules[1].in_cc.set_value(Proxy::ControllerId::CHANNEL_PRESSURE);
    proxy.rules[1].out_cc.set_value(Proxy::ControllerId::CHANNEL_PRESSURE);
    proxy.rules[1].target.set_value(Proxy::Target::TRG_GLOBAL);
    proxy.rules[1].invert.set_value(Proxy::Toggle::ON);

    proxy.begin_processing();

    proxy.channel_pressure(0.0, 0, 96);

    assert_out_events<1>(
        {"t=0.000 cmd=CHANNEL_PRESSURE ch=0 d1=0x1f d2=0x00 (v=0.244)"},
        proxy
    );
})


TEST(cc_can_be_distorted, {
    Proxy proxy;

    turn_off_reset_for_all_rules(proxy);

    proxy.rules[1].in_cc.set_value(Proxy::ControllerId::CHANNEL_PRESSURE);
    proxy.rules[1].out_cc.set_value(Proxy::ControllerId::CHANNEL_PRESSURE);
    proxy.rules[1].target.set_value(Proxy::Target::TRG_GLOBAL);
    proxy.rules[1].distortion_type.set_value(
        Math::DistortionShape::DIST_SHAPE_SMOOTH_SHARP
    );
    proxy.rules[1].distortion_level.set_ratio(1.0);

    proxy.begin_processing();

    proxy.channel_pressure(0.0, 0, 10);

    assert_out_events<1>(
        {"t=0.000 cmd=CHANNEL_PRESSURE ch=0 d1=0x00 d2=0x00 (v=0.000)"},
        proxy
    );
})


TEST(cc_midpoint_can_be_shifted, {
    Proxy proxy;

    turn_off_reset_for_all_rules(proxy);

    proxy.rules[0].in_cc.set_value(Proxy::ControllerId::PITCH_WHEEL);
    proxy.rules[0].out_cc.set_value(Proxy::ControllerId::PITCH_WHEEL);
    proxy.rules[0].target.set_value(Proxy::Target::TRG_GLOBAL);
    proxy.rules[0].midpoint.set_ratio(0.75);

    proxy.begin_processing();

    proxy.pitch_wheel_change(0.0, 0, 4096);     /* 25% */
    proxy.pitch_wheel_change(1.0, 0, 12288);    /* 75% */

    assert_out_events<2>(
        {
            "t=0.000 cmd=PITCH_BEND_CHANGE ch=0 d1=0x00 d2=0x30 (v=0.375)",
            "t=1.000 cmd=PITCH_BEND_CHANGE ch=0 d1=0x00 d2=0x70 (v=0.875)",
        },
        proxy
    );
})


TEST(distortions_are_applied_for_initial_value_on_reset, {
    Proxy proxy;

    turn_off_reset_for_all_rules(proxy);

    proxy.rules[0].in_cc.set_value(Proxy::ControllerId::PITCH_WHEEL);
    proxy.rules[0].out_cc.set_value(Proxy::ControllerId::PITCH_WHEEL);
    proxy.rules[0].target.set_value(Proxy::Target::TRG_GLOBAL);
    proxy.rules[0].reset.set_value(Proxy::Reset::RST_INIT);
    proxy.rules[0].init_value.set_ratio(0.80);
    proxy.rules[0].midpoint.set_ratio(0.75);
    proxy.rules[0].invert.set_value(Proxy::Toggle::ON);
    proxy.rules[0].distortion_type.set_value(
        Math::DistortionShape::DIST_SHAPE_SMOOTH_SHARP
    );
    proxy.rules[0].distortion_level.set_ratio(1.0);

    proxy.rules[1].in_cc.set_value(Proxy::ControllerId::CHANNEL_PRESSURE);
    proxy.rules[1].out_cc.set_value(Proxy::ControllerId::CHANNEL_PRESSURE);
    proxy.rules[1].target.set_value(Proxy::Target::TRG_NEWEST);
    proxy.rules[1].reset.set_value(Proxy::Reset::RST_INIT);
    proxy.rules[1].init_value.set_ratio(0.15);
    proxy.rules[1].distortion_type.set_value(
        Math::DistortionShape::DIST_SHAPE_SMOOTH_SHARP
    );
    proxy.rules[1].distortion_level.set_ratio(1.0);

    proxy.note_on(0.0, 0, 48, 127);     /* channel=1, oldest */
    proxy.channel_pressure(0.0, 0, 127);

    proxy.begin_processing();
    proxy.note_on(1.0, 0, 60, 127);     /* channel=2, newest */

    assert_out_events<4>(
        {
            "t=1.000 cmd=CHANNEL_PRESSURE ch=1 d1=0x00 d2=0x00 (v=0.000)",
            "t=1.000 cmd=CHANNEL_PRESSURE ch=2 d1=0x00 d2=0x00 (v=0.000) pre-NOTE_ON setup",
            "t=1.000 cmd=NOTE_ON ch=2 d1=0x3c d2=0x7f (v=1.000)",
            "t=1.000 cmd=CHANNEL_PRESSURE ch=2 d1=0x00 d2=0x00 (v=0.000)",
        },
        proxy
    );

    proxy.note_off(0.0, 0, 48, 64);
    proxy.note_off(0.0, 0, 60, 64);
    proxy.begin_processing();

    proxy.suspend();
    proxy.resume();

    assert_out_events<1>(
        {"t=0.000 cmd=PITCH_BEND_CHANGE ch=0 d1=0x00 d2=0x00 (v=0.000)"},
        proxy
    );
})


TEST(distortions_are_applied_for_last_value_on_reset, {
    Proxy proxy;

    turn_off_reset_for_all_rules(proxy);

    proxy.rules[0].in_cc.set_value(Proxy::ControllerId::PITCH_WHEEL);
    proxy.rules[0].out_cc.set_value(Proxy::ControllerId::PITCH_WHEEL);
    proxy.rules[0].target.set_value(Proxy::Target::TRG_GLOBAL);
    proxy.rules[0].reset.set_value(Proxy::Reset::RST_LAST);
    proxy.rules[0].init_value.set_ratio(0.80);
    proxy.rules[0].midpoint.set_ratio(0.75);
    proxy.rules[0].invert.set_value(Proxy::Toggle::ON);
    proxy.rules[0].distortion_type.set_value(
        Math::DistortionShape::DIST_SHAPE_SMOOTH_SHARP
    );
    proxy.rules[0].distortion_level.set_ratio(1.0);

    proxy.rules[1].in_cc.set_value(Proxy::ControllerId::CHANNEL_PRESSURE);
    proxy.rules[1].out_cc.set_value(Proxy::ControllerId::CHANNEL_PRESSURE);
    proxy.rules[1].target.set_value(Proxy::Target::TRG_NEWEST);
    proxy.rules[1].reset.set_value(Proxy::Reset::RST_LAST);
    proxy.rules[1].init_value.set_ratio(1.0);
    proxy.rules[1].distortion_type.set_value(
        Math::DistortionShape::DIST_SHAPE_SMOOTH_SHARP
    );
    proxy.rules[1].distortion_level.set_ratio(1.0);

    proxy.note_on(0.0, 0, 48, 127);     /* channel=1, oldest */
    proxy.channel_pressure(0.0, 0, 19);

    proxy.begin_processing();
    proxy.note_on(1.0, 0, 60, 127);     /* channel=2, newest */

    assert_out_events<4>(
        {
            "t=1.000 cmd=CHANNEL_PRESSURE ch=1 d1=0x00 d2=0x00 (v=0.000)",
            "t=1.000 cmd=CHANNEL_PRESSURE ch=2 d1=0x00 d2=0x00 (v=0.000) pre-NOTE_ON setup",
            "t=1.000 cmd=NOTE_ON ch=2 d1=0x3c d2=0x7f (v=1.000)",
            "t=1.000 cmd=CHANNEL_PRESSURE ch=2 d1=0x00 d2=0x00 (v=0.000)",
        },
        proxy
    );

    proxy.note_off(0.0, 0, 48, 64);
    proxy.note_off(0.0, 0, 60, 64);
    proxy.pitch_wheel_change(0.0, 0, 0);
    proxy.begin_processing();

    proxy.suspend();
    proxy.resume();

    assert_out_events<1>(
        {"t=0.000 cmd=PITCH_BEND_CHANGE ch=0 d1=0x00 d2=0x00 (v=0.000)"},
        proxy
    );
})
