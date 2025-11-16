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

#include <algorithm>
#include <cstddef>
#include <cmath>
#include <cstring>
#include <limits>
#include <sstream>
#include <type_traits>

#include "proxy.hpp"

#include "midi.hpp"

#include "math.cpp"
#include "note_stack.cpp"
#include "queue.cpp"
#include "spscqueue.cpp"


namespace MpeEmulator
{

Proxy::ParamIdHashTable Proxy::param_id_hash_table;

std::string Proxy::param_names_by_id[ParamId::PARAM_ID_COUNT];


Proxy::Param::Param(
        std::string const& name,
        unsigned int const min_value,
        unsigned int const max_value,
        unsigned int const default_value
) noexcept
    : name(name),
    min_value_dbl((double)min_value),
    range_dbl((double)(max_value - min_value)),
    range_inv(1.0 / range_dbl),
    min_value(min_value),
    max_value(max_value),
    default_value(default_value)
{
    set_value(default_value);
}


std::string const& Proxy::Param::get_name() const noexcept
{
    return name;
}


unsigned int Proxy::Param::get_min_value() const noexcept
{
    return min_value;
}


unsigned int Proxy::Param::get_max_value() const noexcept
{
    return max_value;
}


unsigned int Proxy::Param::get_value() const noexcept
{
    return value;
}


double Proxy::Param::get_ratio() const noexcept
{
    return ratio;
}


double Proxy::Param::value_to_ratio(unsigned int const value) const noexcept
{
    return clamp_ratio(((double)value - (double)min_value) * range_inv);
}


double Proxy::Param::ratio_to_value(double const ratio) const noexcept
{
    return clamp_value(min_value + (unsigned int)std::round(range_dbl * ratio));
}


unsigned int Proxy::Param::clamp_value(unsigned int const value) const noexcept
{
    return std::min(max_value, std::max(min_value, value));
}


double Proxy::Param::clamp_ratio(double const ratio) const noexcept
{
    return std::min(1.0, std::max(0.0, ratio));
}


double Proxy::Param::get_default_ratio() const noexcept
{
    return value_to_ratio(default_value);
}


void Proxy::Param::set_value(unsigned int const new_value) noexcept
{
    value = clamp_value(new_value);
    ratio = clamp_ratio(value_to_ratio(value));
}


void Proxy::Param::set_ratio(double const new_ratio) noexcept
{
    ratio = clamp_ratio(new_ratio);
    value = ratio_to_value(ratio);
}


Proxy::Rule::Rule(
        std::string const& name,
        ControllerId const in_cc,
        ControllerId const out_cc,
        Target const target,
        unsigned int const init_value_,
        Reset const reset
) noexcept
    : in_cc(name + "IN", ControllerId::BANK_SELECT, ControllerId::NONE, in_cc),
    out_cc(name + "OU", ControllerId::BANK_SELECT, ControllerId::NONE, out_cc),
    init_value(name + "IV", 0, 16383, init_value_),
    target(
        name + "TR",
        Target::TRG_GLOBAL,
        Target::TRG_NEWEST_ABOVE_ANCHOR,
        target
    ),
    distortion_type(
        name + "DT",
        Math::DistortionCurve::DIST_CURVE_SMOOTH_SMOOTH,
        Math::DistortionCurve::DIST_CURVE_SHARP_SHARP,
        Math::DistortionCurve::DIST_CURVE_SMOOTH_SMOOTH
    ),
    distortion_level(name + "DL", 0, 16383, 0),
    midpoint(name + "MP", 0, 20000, 10000),
    reset(name + "RS", Reset::RST_OFF, Reset::RST_INIT, reset),
    invert(name + "NV", Toggle::OFF, Toggle::ON, Toggle::OFF),
    fallback(name + "FB", Toggle::OFF, Toggle::ON, Toggle::OFF),
    last_input_value(init_value.get_ratio())
{
}


double Proxy::Rule::distort(double const value) const noexcept
{
    double const m = midpoint.get_ratio();
    double const shifted = (
        value < 0.5 ? 2.0 * value * m : (m + (2.0 * value - 1.0) * (1.0 - m))
    );

    return Math::distort(
        distortion_level.get_ratio(),
        (Toggle)invert.get_value() == Toggle::ON ? 1.0 - shifted : shifted,
        (Math::DistortionCurve)distortion_type.get_value()
    );
}


bool Proxy::Rule::needs_reset_for_note_event(
        bool const is_above_anchor
) const noexcept {
    if ((Proxy::Reset)reset.get_value() == Proxy::Reset::RST_OFF) {
        return false;
    }

    Proxy::Target const target = (Proxy::Target)this->target.get_value();

    return (
        target != Proxy::Target::TRG_GLOBAL
        && (target != Proxy::Target::TRG_ALL_BELOW_ANCHOR || !is_above_anchor)
        && (target != Proxy::Target::TRG_ALL_ABOVE_ANCHOR || is_above_anchor)
    );
}


double Proxy::Rule::get_reset_value() const noexcept
{
    Proxy::Target const target = (Proxy::Target)this->target.get_value();

    return distort(
        (
            (Proxy::Reset)reset.get_value() == Proxy::Reset::RST_LAST
            || target == Proxy::Target::TRG_ALL_ABOVE_ANCHOR
            || target == Proxy::Target::TRG_ALL_BELOW_ANCHOR
        )
            ? last_input_value
            : init_value.get_ratio()
    );
}


Proxy::Proxy() noexcept
    : send_mcm("MCM", Toggle::OFF, Toggle::ON, Toggle::OFF),
    zone_type(
        "Z1TYP", ZoneType::ZT_LOWER, ZoneType::ZT_UPPER, ZoneType::ZT_LOWER
    ),
    channels("Z1CHN", 1, 15, 15),
    excess_note_handling(
        "Z1ENH",
        ExcessNoteHandling::ENH_IGNORE,
        ExcessNoteHandling::ENH_STEAL_NEWEST,
        ExcessNoteHandling::ENH_STEAL_OLDEST
    ),
    anchor("Z1ANC", 0, 127, 60),
    override_release_velocity("Z1ORV", Toggle::OFF, Toggle::ON, Toggle::OFF),
    transpose_below_anchor("Z1TRB", 0, 96, 48),
    transpose_above_anchor("Z1TRA", 0, 96, 48),
    sustain_pedal_handling("Z1SUS", Toggle::OFF, Toggle::ON, Toggle::OFF),
    rules{
        Rule("Z1R1", ControllerId::PITCH_WHEEL, ControllerId::PITCH_WHEEL, Target::TRG_NEWEST, 8192),
        Rule("Z1R2", ControllerId::CHANNEL_PRESSURE, ControllerId::CHANNEL_PRESSURE, Target::TRG_NEWEST, 0),
        Rule("Z1R3", ControllerId::SOUND_5, ControllerId::SOUND_5, Target::TRG_NEWEST, 8192),
        Rule("Z1R4"),
        Rule("Z1R5"),
        Rule("Z1R6"),
        Rule("Z1R7"),
        Rule("Z1R8"),
        Rule("Z1R9"),
    },
    out_events(out_events_rw),
    messages(MESSAGE_QUEUE_SIZE),
    is_suspended(false),
    is_dirty_(false),
    had_reset(false),
    is_sustain_pedal_on(false)
{
    std::fill_n(channels_by_notes, Midi::NOTES, Midi::INVALID_CHANNEL);
    std::fill_n(deferred_note_off_velocities, Midi::NOTES, 64);
    std::fill_n(velocities_by_notes, Midi::NOTES, 0);

    register_param(ParamId::MCM, send_mcm);
    register_param(ParamId::Z1TYP, zone_type);
    register_param(ParamId::Z1CHN, channels);
    register_param(ParamId::Z1ENH, excess_note_handling);
    register_param(ParamId::Z1ANC, anchor);
    register_param(ParamId::Z1ORV, override_release_velocity);

    int param_id = (int)ParamId::Z1R1IN;

    for (size_t i = 0; i != RULES; ++i) {
        register_param((ParamId)(param_id++), rules[i].in_cc);
        register_param((ParamId)(param_id++), rules[i].out_cc);
        register_param((ParamId)(param_id++), rules[i].init_value);
        register_param((ParamId)(param_id++), rules[i].target);
        register_param((ParamId)(param_id++), rules[i].distortion_type);
        register_param((ParamId)(param_id++), rules[i].distortion_level);
        register_param((ParamId)(param_id++), rules[i].midpoint);
        register_param((ParamId)(param_id++), rules[i].reset);
        register_param((ParamId)(param_id++), rules[i].invert);
    }

    MPE_EMULATOR_ASSERT((ParamId)param_id == ParamId::Z1TRB);

    register_param((ParamId)(param_id++), transpose_below_anchor);
    register_param((ParamId)(param_id++), transpose_above_anchor);
    register_param((ParamId)(param_id++), sustain_pedal_handling);

    MPE_EMULATOR_ASSERT((ParamId)param_id == ParamId::Z1R1FB);

    for (size_t i = 0; i != RULES; ++i) {
        register_param((ParamId)(param_id++), rules[i].fallback);
    }

    for (size_t i = 0; i != (size_t)ParamId::PARAM_ID_COUNT; ++i) {
        param_ratios_atomic[i].store(params[i]->get_ratio());
    }

    ZoneTypeDescriptor const& ztd = ZONE_TYPES[zone_type.get_value()];

    offset_below_anchor = 0;
    offset_above_anchor = 0;
    anchor_ = (Midi::Note)anchor.get_value();
    channel_count = channels.get_value();
    manager_channel = ztd.manager_channel;
    channel_increment = ztd.channel_increment;
    first_channel = manager_channel + channel_increment;
    last_channel = manager_channel + channel_increment * channel_count;

    reset_available_channels();

    active_voices_count_atomic.store(0);
    channel_count_atomic.store(channel_count);

    out_events_rw.reserve(32768);
}


void Proxy::register_param(ParamId const param_id, Param& param) noexcept
{
    std::string const& name = param.get_name();

    param_id_hash_table.add(name, param_id);

    if (param_names_by_id[param_id].length() == 0) {
        param_names_by_id[param_id] = name;
    }

    params[(size_t)param_id] = &param;
}


void Proxy::reset_available_channels() noexcept
{
    Midi::Channel channel = first_channel;

    available_channels.clear();

    for (Midi::Channel i = 0; i != channel_count; ++i) {
        available_channels.push(channel);
        channel += channel_increment;
    }
}


Proxy::~Proxy()
{
}


#ifdef MPE_EMULATOR_ASSERTIONS
bool Proxy::is_lock_free() const noexcept
{
    bool is_lock_free = true;

    for (size_t i = 0; is_lock_free && i != (size_t)ParamId::PARAM_ID_COUNT; ++i) {
        is_lock_free = param_ratios_atomic[i].is_lock_free();
    }

    return (
        is_lock_free
        && messages.is_lock_free()
        && active_voices_count_atomic.is_lock_free()
        && channel_count_atomic.is_lock_free()
    );
}
#endif


bool Proxy::is_dirty() const noexcept
{
    return is_dirty_;
}


void Proxy::clear_dirty_flag() noexcept
{
    is_dirty_ = false;
}


void Proxy::suspend() noexcept
{
    is_suspended = true;
}


void Proxy::resume() noexcept
{
    is_suspended = false;
    reset();
}


unsigned int Proxy::get_active_voices_count() const noexcept
{
    return active_voices_count_atomic.load();
}


unsigned int Proxy::get_channel_count() const noexcept
{
    return channel_count_atomic.load();
}


void Proxy::note_on(
        double const time_offset,
        Midi::Channel const channel,
        Midi::Note const note,
        Midi::Byte const velocity
) noexcept {
    if (is_suspended) {
        return;
    }

    bool const already_on = note_stack.find(note);

    if (MPE_EMULATOR_UNLIKELY(already_on)) {
        if ((ExcessNoteHandling)excess_note_handling.get_value() == ExcessNoteHandling::ENH_IGNORE) {
            return;
        }

        Midi::Channel const steal_channel = channels_by_notes[note];

        push_note_off(time_offset, steal_channel, note, 64);
        push_note_on(time_offset, steal_channel, note, velocity);

        return;
    }

    if (MPE_EMULATOR_UNLIKELY(available_channels.is_empty())) {
        if (MPE_EMULATOR_UNLIKELY(note_stack.is_empty())) {
            MPE_EMULATOR_ASSERT_NOT_REACHED();

            return;
        }

        Midi::Note steal_note;

        switch ((ExcessNoteHandling)excess_note_handling.get_value()) {
            case ExcessNoteHandling::ENH_STEAL_LOWEST:
                steal_note = note_stack.lowest();
                break;

            case ExcessNoteHandling::ENH_STEAL_HIGHEST:
                steal_note = note_stack.highest();
                break;

            case ExcessNoteHandling::ENH_STEAL_OLDEST:
                steal_note = note_stack.oldest();
                break;

            case ExcessNoteHandling::ENH_STEAL_NEWEST:
                steal_note = note_stack.top();
                break;

            default:
                return;
        }

        Midi::Channel const steal_channel = channels_by_notes[steal_note];

        push_note_off(time_offset, steal_channel, steal_note, 64);
        push_note_on(time_offset, steal_channel, note, velocity);
    } else {
        Midi::Channel allocated_channel = available_channels.pop();
        push_note_on(time_offset, allocated_channel, note, velocity);
    }
}


void Proxy::push_note_on(
        double const time_offset,
        Midi::Channel const channel,
        Midi::Note const note,
        Midi::Byte velocity
) noexcept {
    NoteStack::ChannelStats old_channel_stats(channel_stats);
    NoteStack::ChannelStats old_channel_stats_below(channel_stats_below);
    NoteStack::ChannelStats old_channel_stats_above(channel_stats_above);

    bool const is_first_note = note_stack.is_empty();

    note_stack.push(note);
    channels_by_notes[note] = channel;
    velocities_by_notes[note] = velocity;

    note_stack.make_stats(channels_by_notes, channel_stats);

    bool const is_above_anchor = note >= anchor_;

    if (is_above_anchor) {
        note_stack_above.push(note);
        note_stack_above.make_stats(channels_by_notes, channel_stats_above);
    } else {
        note_stack_below.push(note);
        note_stack_below.make_stats(channels_by_notes, channel_stats_below);
    }

    push_resets_for_new_note<true>(
        time_offset,
        channel,
        is_first_note,
        is_above_anchor,
        old_channel_stats,
        old_channel_stats_below,
        old_channel_stats_above
    );

    push_out_event(
        Midi::Event(
            time_offset,
            Midi::NOTE_ON,
            channel,
            transpose(note, is_above_anchor),
            velocity,
            Midi::byte_to_float<double>(velocity)
        )
    );

    /*
    MPE specs recommend Note On setup events to come before the Note On event
    itself, but just to be on the safe side (e.g. in case a synth ignores events
    that come on an MPE channel while there are no active notes on it), we send
    a setup sequence both before and after the Note On.
    */
    push_resets_for_new_note<false>(
        time_offset,
        channel,
        is_first_note,
        is_above_anchor,
        old_channel_stats,
        old_channel_stats_below,
        old_channel_stats_above
    );
}


Midi::Note Proxy::transpose(
        Midi::Note const note,
        bool const is_above_anchor
) const noexcept {
    int const offset = (
        is_above_anchor ? offset_above_anchor : offset_below_anchor
    );

    return (Midi::Note)std::max(0, std::min(127, (int)note + offset));
}


void Proxy::push_out_event(Midi::Event const& event) noexcept
{
    if (MPE_EMULATOR_UNLIKELY(event.channel > Midi::CHANNEL_MAX)) {
        return;
    }

    out_events_rw.push_back(event);
}


template<bool is_pre_note_on_setup>
void Proxy::push_resets_for_new_note(
        double const time_offset,
        Midi::Channel const new_note_channel,
        bool const is_first_note,
        bool const is_above_anchor,
        NoteStack::ChannelStats const& old_channel_stats,
        NoteStack::ChannelStats const& old_channel_stats_below,
        NoteStack::ChannelStats const& old_channel_stats_above
) noexcept {
    for (size_t i = 0; i != RULES; ++i) {
        Rule const& rule = rules[i];

        if (!rule.needs_reset_for_note_event(is_above_anchor)) {
            continue;
        }

        double const reset_value = rule.get_reset_value();
        ControllerId const out_cc = (ControllerId)rule.out_cc.get_value();

        if constexpr (is_pre_note_on_setup) {
            reset_outdated_targets_if_changed(
                rule,
                time_offset,
                new_note_channel,
                old_channel_stats,
                old_channel_stats_below,
                old_channel_stats_above,
                channel_stats,
                channel_stats_below,
                channel_stats_above,
                reset_value,
                out_cc
            );
        }

        if (is_first_note && (Toggle)rule.fallback.get_value() == Toggle::ON) {
            push_controller_event(
                time_offset,
                manager_channel,
                out_cc,
                reset_value,
                is_pre_note_on_setup
            );
        }

        push_controller_event(
            time_offset,
            new_note_channel,
            out_cc,
            reset_value,
            is_pre_note_on_setup
        );
    }
}


void Proxy::reset_outdated_targets_if_changed(
        Rule const& rule,
        double const time_offset,
        Midi::Channel const new_note_channel,
        NoteStack::ChannelStats const& a_channel_stats,
        NoteStack::ChannelStats const& a_channel_stats_below,
        NoteStack::ChannelStats const& a_channel_stats_above,
        NoteStack::ChannelStats const& b_channel_stats,
        NoteStack::ChannelStats const& b_channel_stats_below,
        NoteStack::ChannelStats const& b_channel_stats_above,
        double const reset_value,
        ControllerId const out_cc
) noexcept {
    Target const target = (Target)rule.target.get_value();
    Midi::Channel channel = Midi::INVALID_CHANNEL;

    switch (target) {
        case Target::TRG_LOWEST:
            if (a_channel_stats.lowest != b_channel_stats.lowest) {
                channel = a_channel_stats.lowest;
            }

            break;

        case Target::TRG_HIGHEST:
            if (a_channel_stats.highest != b_channel_stats.highest) {
                channel = a_channel_stats.highest;
            }

            break;

        case Target::TRG_OLDEST:
            if (a_channel_stats.oldest != b_channel_stats.oldest) {
                channel = a_channel_stats.oldest;
            }

            break;

        case Target::TRG_NEWEST:
            if (a_channel_stats.newest != b_channel_stats.newest) {
                channel = a_channel_stats.newest;
            }

            break;

        case Target::TRG_LOWEST_BELOW_ANCHOR:
            if (a_channel_stats_below.lowest != b_channel_stats_below.lowest) {
                channel = a_channel_stats_below.lowest;
            }

            break;

        case Target::TRG_HIGHEST_BELOW_ANCHOR:
            if (a_channel_stats_below.highest != b_channel_stats_below.highest) {
                channel = a_channel_stats_below.highest;
            }

            break;

        case Target::TRG_OLDEST_BELOW_ANCHOR:
            if (a_channel_stats_below.oldest != b_channel_stats_below.oldest) {
                channel = a_channel_stats_below.oldest;
            }

            break;

        case Target::TRG_NEWEST_BELOW_ANCHOR:
            if (a_channel_stats_below.newest != b_channel_stats_below.newest) {
                channel = a_channel_stats_below.newest;
            }

            break;

        case Target::TRG_LOWEST_ABOVE_ANCHOR:
            if (a_channel_stats_above.lowest != b_channel_stats_above.lowest) {
                channel = a_channel_stats_above.lowest;
            }

            break;

        case Target::TRG_HIGHEST_ABOVE_ANCHOR:
            if (a_channel_stats_above.highest != b_channel_stats_above.highest) {
                channel = a_channel_stats_above.highest;
            }

            break;

        case Target::TRG_OLDEST_ABOVE_ANCHOR:
            if (a_channel_stats_above.oldest != b_channel_stats_above.oldest) {
                channel = a_channel_stats_above.oldest;
            }

            break;

        case Target::TRG_NEWEST_ABOVE_ANCHOR:
            if (a_channel_stats_above.newest != b_channel_stats_above.newest) {
                channel = a_channel_stats_above.newest;
            }

            break;

        default:
            /*
            Global, all-below-anchor, and all-above-anchor targets are not to
            be reset for changes in polyphonic channels, so there's nothing to
            do here.
            */
            return;
    }

    if (channel != Midi::INVALID_CHANNEL && channel != new_note_channel) {
        push_controller_event(time_offset, channel, out_cc, reset_value);
    }
}


void Proxy::push_controller_event(
        double const time_offset,
        Midi::Channel const channel,
        ControllerId const controller_id,
        double const value,
        bool const is_pre_note_on_setup
) noexcept {
    if (controller_id == ControllerId::PITCH_WHEEL) {
        push_controller_event<Midi::PITCH_BEND_CHANGE>(
            time_offset, channel, controller_id, value, is_pre_note_on_setup
        );
    } else if (controller_id == ControllerId::CHANNEL_PRESSURE) {
        push_controller_event<Midi::CHANNEL_PRESSURE>(
            time_offset, channel, controller_id, value, is_pre_note_on_setup
        );
    } else if (controller_id <= ControllerId::MAX_MIDI_CC) {
        push_controller_event<Midi::CONTROL_CHANGE>(
            time_offset, channel, controller_id, value, is_pre_note_on_setup
        );
    }
}


template<Midi::Command midi_command>
void Proxy::push_controller_event(
        double const time_offset,
        Midi::Channel const channel,
        ControllerId const controller_id,
        double const value,
        bool const is_pre_note_on_setup
) noexcept {
    if constexpr (midi_command == Midi::CHANNEL_PRESSURE) {
        push_out_event(
            Midi::Event(
                time_offset,
                Midi::CHANNEL_PRESSURE,
                channel,
                Midi::float_to_byte<double>(value),
                0x00,
                value,
                is_pre_note_on_setup
            )
        );
    } else if constexpr (midi_command == Midi::PITCH_BEND_CHANGE) {
        Midi::Word const value_as_word = Midi::float_to_word<double>(value);
        Midi::Byte const lsb = (Midi::Byte)(value_as_word & 0x7f);
        Midi::Byte const msb = (Midi::Byte)(value_as_word >> 7);

        push_out_event(
            Midi::Event(
                time_offset,
                Midi::PITCH_BEND_CHANGE,
                channel,
                lsb,
                msb,
                value,
                is_pre_note_on_setup
            )
        );
    } else {
        push_out_event(
            Midi::Event(
                time_offset,
                Midi::CONTROL_CHANGE,
                channel,
                (Midi::Controller)controller_id,
                Midi::float_to_byte<double>(value),
                value,
                is_pre_note_on_setup
            )
        );
    }
}


void Proxy::push_note_off(
        double const time_offset,
        Midi::Channel const channel,
        Midi::Note const note,
        Midi::Byte const velocity
) noexcept {
    Midi::Byte const note_off_velocity = (
        (Toggle)override_release_velocity.get_value() == Toggle::ON
            ? velocities_by_notes[note]
            : velocity
    );
    bool const was_above_anchor = note >= anchor_;

    push_out_event(
        Midi::Event(
            time_offset,
            Midi::NOTE_OFF,
            channel,
            transpose(note, was_above_anchor),
            note_off_velocity,
            Midi::byte_to_float<double>(note_off_velocity)
        )
    );

    NoteStack::ChannelStats old_channel_stats(channel_stats);
    NoteStack::ChannelStats old_channel_stats_below(channel_stats_below);
    NoteStack::ChannelStats old_channel_stats_above(channel_stats_above);

    note_stack.remove(note);
    note_stack_above.remove(note);
    note_stack_below.remove(note);

    note_stack.make_stats(channels_by_notes, channel_stats);
    note_stack_above.make_stats(channels_by_notes, channel_stats_above);
    note_stack_below.make_stats(channels_by_notes, channel_stats_below);

    push_resets_for_note_off(
        time_offset,
        was_above_anchor,
        old_channel_stats,
        old_channel_stats_below,
        old_channel_stats_above
    );

    deferred_note_offs.remove(note);
}


void Proxy::push_resets_for_note_off(
        double const time_offset,
        bool const was_above_anchor,
        NoteStack::ChannelStats const& old_channel_stats,
        NoteStack::ChannelStats const& old_channel_stats_below,
        NoteStack::ChannelStats const& old_channel_stats_above
) noexcept {
    for (size_t i = 0; i != RULES; ++i) {
        Rule const& rule = rules[i];

        if (!rule.needs_reset_for_note_event(was_above_anchor)) {
            continue;
        }

        double const reset_value = rule.get_reset_value();
        ControllerId const out_cc = (ControllerId)rule.out_cc.get_value();

        reset_outdated_targets_if_changed(
            rule,
            time_offset,
            Midi::INVALID_CHANNEL,
            channel_stats,
            channel_stats_below,
            channel_stats_above,
            old_channel_stats,
            old_channel_stats_below,
            old_channel_stats_above,
            reset_value,
            out_cc
        );
    }
}


void Proxy::aftertouch(
        double const time_offset,
        Midi::Channel const channel,
        Midi::Note const note,
        Midi::Byte const pressure
) noexcept {
    // if (is_suspended) {
        // return;
    // }
}


void Proxy::channel_pressure(
        double const time_offset,
        Midi::Channel const channel,
        Midi::Byte const pressure
) noexcept {
    if (
            is_suspended
            || is_repeated_midi_controller_message(
                ControllerId::CHANNEL_PRESSURE, time_offset, channel, pressure
            )
    ) {
        return;
    }

    process_controller_event<Midi::CHANNEL_PRESSURE>(
        time_offset,
        ControllerId::CHANNEL_PRESSURE,
        Midi::byte_to_float<double>(pressure)
    );
}


template<Midi::Command midi_command>
void Proxy::process_controller_event(
        double const time_offset,
        ControllerId const controller_id,
        double const value
) noexcept {
    Midi::Channel target_channels[Midi::CHANNELS];
    size_t target_channels_count;
    bool matched = false;

    bool const is_note_stack_empty = note_stack.is_empty();

    for (size_t i = 0; i != RULES; ++i) {
        Rule& rule = rules[i];
        ControllerId const rule_ctl_id = (ControllerId)rule.in_cc.get_value();

        if (rule_ctl_id == ControllerId::MIDI_LEARN) {
            rule.in_cc.set_value(controller_id);
            is_dirty_ = true;
        } else if (rule_ctl_id != controller_id) {
            continue;
        }

        matched = true;
        target_channels_count = 0;

        rule.last_input_value = value;

        ControllerId const out_controller_id = (
            (ControllerId)rule.out_cc.get_value()
        );

        if (is_note_stack_empty && (Toggle)rule.fallback.get_value() == Toggle::ON) {
            target_channels[target_channels_count++] = manager_channel;
        } else {
            switch ((Target)rule.target.get_value()) {
                case Target::TRG_ALL_BELOW_ANCHOR:
                    note_stack_below.collect_active_channels(
                        channels_by_notes, target_channels, target_channels_count
                    );
                    break;

                case Target::TRG_ALL_ABOVE_ANCHOR:
                    note_stack_above.collect_active_channels(
                        channels_by_notes, target_channels, target_channels_count
                    );
                    break;

                case Target::TRG_LOWEST:
                    if (!note_stack.is_empty()) {
                        Midi::Note note = note_stack.lowest();
                        target_channels[target_channels_count++] = channels_by_notes[note];
                    }
                    break;

                case Target::TRG_HIGHEST:
                    if (!note_stack.is_empty()) {
                        Midi::Note note = note_stack.highest();
                        target_channels[target_channels_count++] = channels_by_notes[note];
                    }
                    break;

                case Target::TRG_OLDEST:
                    if (!note_stack.is_empty()) {
                        Midi::Note note = note_stack.oldest();
                        target_channels[target_channels_count++] = channels_by_notes[note];
                    }
                    break;

                case Target::TRG_NEWEST:
                    if (!note_stack.is_empty()) {
                        Midi::Note note = note_stack.top();
                        target_channels[target_channels_count++] = channels_by_notes[note];
                    }
                    break;

                case Target::TRG_LOWEST_BELOW_ANCHOR:
                    if (!note_stack_below.is_empty()) {
                        Midi::Note note = note_stack_below.lowest();
                        target_channels[target_channels_count++] = channels_by_notes[note];
                    }
                    break;

                case Target::TRG_HIGHEST_BELOW_ANCHOR:
                    if (!note_stack_below.is_empty()) {
                        Midi::Note note = note_stack_below.highest();
                        target_channels[target_channels_count++] = channels_by_notes[note];
                    }
                    break;

                case Target::TRG_OLDEST_BELOW_ANCHOR:
                    if (!note_stack_below.is_empty()) {
                        Midi::Note note = note_stack_below.oldest();
                        target_channels[target_channels_count++] = channels_by_notes[note];
                    }
                    break;

                case Target::TRG_NEWEST_BELOW_ANCHOR:
                    if (!note_stack_below.is_empty()) {
                        Midi::Note note = note_stack_below.top();
                        target_channels[target_channels_count++] = channels_by_notes[note];
                    }
                    break;

                case Target::TRG_LOWEST_ABOVE_ANCHOR:
                    if (!note_stack_above.is_empty()) {
                        Midi::Note note = note_stack_above.lowest();
                        target_channels[target_channels_count++] = channels_by_notes[note];
                    }
                    break;

                case Target::TRG_HIGHEST_ABOVE_ANCHOR:
                    if (!note_stack_above.is_empty()) {
                        Midi::Note note = note_stack_above.highest();
                        target_channels[target_channels_count++] = channels_by_notes[note];
                    }
                    break;

                case Target::TRG_OLDEST_ABOVE_ANCHOR:
                    if (!note_stack_above.is_empty()) {
                        Midi::Note note = note_stack_above.oldest();
                        target_channels[target_channels_count++] = channels_by_notes[note];
                    }
                    break;

                case Target::TRG_NEWEST_ABOVE_ANCHOR:
                    if (!note_stack_above.is_empty()) {
                        Midi::Note note = note_stack_above.top();
                        target_channels[target_channels_count++] = channels_by_notes[note];
                    }
                    break;

                case Target::TRG_GLOBAL:
                default:
                    target_channels[target_channels_count++] = manager_channel;
                    break;
            }
        }

        if (target_channels_count != 0) {
            double const out_value = rule.distort(value);

            for (size_t c = 0; c != target_channels_count; ++c) {
                push_controller_event(
                    time_offset, target_channels[c], out_controller_id, out_value
                );
            }
        }
    }

    if (!matched) {
        push_controller_event<midi_command>(
            time_offset, manager_channel, controller_id, value
        );
    }

    if (
            (Toggle)sustain_pedal_handling.get_value() != Toggle::OFF
            && controller_id == ControllerId::SUSTAIN_PEDAL
    ) {
        is_sustain_pedal_on = value >= 0.5;

        if (!is_sustain_pedal_on) {
            process_deferred_note_offs(time_offset);
        }
    }
}


void Proxy::process_deferred_note_offs(double const time_offset) noexcept
{
    while (!deferred_note_offs.is_empty()) {
        Midi::Note const note = deferred_note_offs.pop();
        Midi::Byte const velocity = deferred_note_off_velocities[note];
        handle_note_off(time_offset, note, velocity);
    }
}


bool Proxy::is_repeated_midi_controller_message(
        ControllerId const controller_id,
        double const time_offset,
        Midi::Channel const channel,
        Midi::Word const value
) noexcept {
    /*
    By default, FL Studio 21 sends multiple clones of the same pitch bend event
    separately on all channels, but it's enough for us to handle only one of
    those.
    */
    MidiControllerMessage const message(time_offset, value);

    if (previous_controller_message[controller_id] == message) {
        return true;
    }

    previous_controller_message[controller_id] = message;

    return false;
}


void Proxy::note_off(
        double const time_offset,
        Midi::Channel const channel,
        Midi::Note const note,
        Midi::Byte const velocity
) noexcept {
    if (is_suspended) {
        return;
    }

    if (!note_stack.find(note)) {
        return;
    }

    if (
            (Toggle)sustain_pedal_handling.get_value() != Toggle::OFF
            && is_sustain_pedal_on
    ) {
        deferred_note_offs.push(note);
        deferred_note_off_velocities[note] = velocity;
    } else {
        handle_note_off(time_offset, note, velocity);
    }
}


void Proxy::handle_note_off(
        double const time_offset,
        Midi::Note const note,
        Midi::Byte const velocity
) noexcept {
    Midi::Channel const assigned_channel = channels_by_notes[note];

    push_note_off(time_offset, assigned_channel, note, velocity);

    available_channels.push(assigned_channel);
}


void Proxy::control_change(
        double const time_offset,
        Midi::Channel const channel,
        Midi::Controller const controller,
        Midi::Byte const new_value
) noexcept {
    ControllerId const controller_id = (ControllerId)controller;

    if (
            is_suspended
            || controller_id > ControllerId::MAX_MIDI_CC
            || is_repeated_midi_controller_message(
                (ControllerId)controller, time_offset, channel, new_value
            )
    ) {
        return;
    }

    process_controller_event<Midi::CONTROL_CHANGE>(
        time_offset,
        controller_id,
        Midi::byte_to_float<double>(new_value)
    );
}


void Proxy::pitch_wheel_change(
        double const time_offset,
        Midi::Channel const channel,
        Midi::Word const new_value
) noexcept {
    if (
            is_suspended
            || is_repeated_midi_controller_message(
                ControllerId::PITCH_WHEEL, time_offset, channel, new_value
            )
    ) {
        return;
    }

    process_controller_event<Midi::PITCH_BEND_CHANGE>(
        time_offset,
        ControllerId::PITCH_WHEEL,
        Midi::word_to_float<double>(new_value)
    );
}


void Proxy::channel_mode(
        double const time_offset,
        Midi::Channel const channel,
        Midi::Byte const message,
        Midi::Byte const data
) noexcept {
    // if (is_suspended) {
        // return;
    // }
}


void Proxy::push_message(
        MessageType const type,
        ParamId const param_id,
        double const double_param
) noexcept {
    Message message(type, param_id, double_param);

    push_message(message);
}


void Proxy::push_message(Message const& message) noexcept
{
    messages.push(message);
}


std::string const& Proxy::get_param_name(ParamId const param_id) const noexcept
{
    return param_names_by_id[param_id];
}


Proxy::ParamId Proxy::get_param_id(std::string const& name) const noexcept
{
    return param_id_hash_table.lookup(name);
}


#ifdef MPE_EMULATOR_ASSERTIONS
void Proxy::get_param_id_hash_table_statistics(
        unsigned int& max_collisions,
        double& avg_collisions,
        double& avg_bucket_size
) const noexcept {
    param_id_hash_table.get_statistics(
        max_collisions, avg_collisions, avg_bucket_size
    );
}
#endif


double Proxy::get_param_ratio_atomic(ParamId const param_id) const noexcept
{
    return param_ratios_atomic[param_id].load();
}


double Proxy::get_param_default_ratio(ParamId const param_id) const noexcept
{
    return params[(size_t)param_id]->get_default_ratio();
}


unsigned int Proxy::param_ratio_to_value(
        ParamId const param_id,
        double const ratio
) const noexcept {
    return params[(size_t)param_id]->ratio_to_value(ratio);
}


double Proxy::param_value_to_ratio(
        ParamId const param_id,
        unsigned int const value
) const noexcept {
    return params[(size_t)param_id]->value_to_ratio(value);
}


unsigned int Proxy::get_param_max_value(ParamId const param_id) const noexcept
{
    return params[(size_t)param_id]->get_max_value();
}


#ifdef MPE_EMULATOR_ASSERTIONS
unsigned int Proxy::get_param_value(ParamId const param_id) const noexcept
{
    return params[(size_t)param_id]->get_value();
}
#endif


void Proxy::process_messages() noexcept
{
    SPSCQueue<Message>::SizeType const message_count = messages.length();

    for (SPSCQueue<Message>::SizeType i = 0; i != message_count; ++i) {
        Message message;

        if (messages.pop(message)) {
            process_message(message);
        }
    }
}


void Proxy::process_message(
        MessageType const type,
        ParamId const param_id,
        double const double_param
) noexcept {
    Message message(type, param_id, double_param);

    process_message(message);
}


void Proxy::process_message(Message const& message) noexcept
{
    switch (message.type) {
        case MessageType::SET_PARAM:
            is_dirty_ = handle_set_param(message.param_id, message.double_param);
            break;

        case MessageType::REFRESH_PARAM:
            handle_refresh_param(message.param_id);
            break;

        case MessageType::CLEAR:
            is_dirty_ = handle_clear();
            break;

        case MessageType::CLEAR_DIRTY_FLAG:
            is_dirty_ = false;
            break;

        default:
            break;
    }
}


bool Proxy::handle_set_param(ParamId const param_id, double const ratio) noexcept
{
    Param& param = *(params[(size_t)param_id]);

    unsigned int const old_value = param.get_value();

    param.set_ratio(ratio);

    handle_refresh_param(param_id);

    return old_value != param.get_value();
}


void Proxy::handle_refresh_param(ParamId const param_id) noexcept
{
    param_ratios_atomic[(size_t)param_id].store(get_param_ratio(param_id));
}


bool Proxy::handle_clear() noexcept
{
    bool has_changed = false;

    for (int i = 0; i != ParamId::PARAM_ID_COUNT; ++i) {
        ParamId const param_id = (ParamId)i;

        has_changed = (
            handle_set_param(param_id, get_param_default_ratio(param_id))
            || has_changed
        );
    }

    return has_changed;
}


double Proxy::get_param_ratio(ParamId const param_id) const noexcept
{
    return params[(size_t)param_id]->get_ratio();
}


void Proxy::reset() noexcept
{
    if (MPE_EMULATOR_LIKELY(!update_zone_config())) {
        out_events_rw.clear();
        stop_all_notes();
        push_mcms();
        reset_rules_and_global_controllers();
        reset_available_channels();
        had_reset = true;
    }
}


bool Proxy::update_zone_config() noexcept
{
    ZoneTypeDescriptor const& ztd = ZONE_TYPES[zone_type.get_value()];
    int const new_offset_below_anchor = (
        (int)transpose_below_anchor.get_value() - 48
    );
    int const new_offset_above_anchor = (
        (int)transpose_above_anchor.get_value() - 48
    );
    Midi::Note const new_anchor = (Midi::Note)anchor.get_value();
    Midi::Channel const new_manager_channel = ztd.manager_channel;
    Midi::Channel const new_channel_count = channels.get_value();

    if (
            new_channel_count == channel_count
            && new_manager_channel == manager_channel
            && new_offset_below_anchor == offset_below_anchor
            && new_offset_above_anchor == offset_above_anchor
            && new_anchor == anchor_
    ) {
        return false;
    }

    out_events_rw.clear();
    stop_all_notes();

    channel_count_atomic.store(new_channel_count);

    offset_below_anchor = new_offset_below_anchor;
    offset_above_anchor = new_offset_above_anchor;
    anchor_ = new_anchor;
    channel_count = new_channel_count;
    manager_channel = new_manager_channel;
    channel_increment = ztd.channel_increment;
    first_channel = manager_channel + channel_increment;
    last_channel = manager_channel + channel_increment * channel_count;

    reset_available_channels();

    push_mcms();
    reset_rules_and_global_controllers();

    had_reset = true;

    return true;
}


void Proxy::stop_all_notes() noexcept
{
    if (!note_stack.is_empty()) {
        push_controller_event<Midi::CONTROL_CHANGE>(
            0.0, manager_channel, ControllerId::SUSTAIN_PEDAL, 0.0
        );

        for (Midi::Note i = 0; !note_stack.is_empty() && i != Midi::NOTE_MAX; ++i) {
            Midi::Note const note = note_stack.pop();
            Midi::Channel const channel = channels_by_notes[note];

            push_controller_event<Midi::CONTROL_CHANGE>(
                0.0, channel, ControllerId::SUSTAIN_PEDAL, 0.0
            );

            push_note_off(0.0, channel, note, 64);
        }
    }

    deferred_note_offs.clear();

    note_stack.clear();
    note_stack_below.clear();
    note_stack_above.clear();

    is_sustain_pedal_on = false;
}


void Proxy::push_mcms() noexcept
{
    if (send_mcm.get_value() == Toggle::ON) {
        push_mcm(manager_channel, channel_count);
        push_mcm(manager_channel ^ 0x0f, 0);
    }
}


void Proxy::push_mcm(
        Midi::Channel const channel,
        Midi::Channel const channel_count
) noexcept {
    push_out_event(
        Midi::Event(0.0, Midi::CONTROL_CHANGE, channel, Midi::RPN_MSB, 0x00)
    );
    push_out_event(
        Midi::Event(0.0, Midi::CONTROL_CHANGE, channel, Midi::RPN_LSB, 0x06)
    );
    push_out_event(
        Midi::Event(
            0.0, Midi::CONTROL_CHANGE, channel, Midi::DATA_ENTRY_MSB, channel_count
        )
    );
}


void Proxy::reset_rules_and_global_controllers() noexcept
{
    for (size_t i = 0; i != RULES; ++i) {
        Rule& rule = rules[i];
        double const init_value = rule.init_value.get_ratio();

        rule.last_input_value = init_value;

        if (
                (Reset)rule.reset.get_value() != Reset::RST_OFF
                && (Target)rule.target.get_value() == Target::TRG_GLOBAL
        ) {
            push_controller_event(
                0.0,
                manager_channel,
                (ControllerId)rule.out_cc.get_value(),
                rule.distort(init_value)
            );
        }
    }
}


void Proxy::begin_processing() noexcept
{
    process_messages();

    if (is_suspended) {
        return;
    }

    update_zone_config();

    if (MPE_EMULATOR_UNLIKELY(had_reset)) {
        had_reset = false;
    } else {
        out_events_rw.clear();
    }
}


Proxy::Message::Message() noexcept
    : type(MessageType::INVALID_MESSAGE_TYPE),
    param_id(ParamId::INVALID_PARAM_ID),
    double_param(0.0)
{
}


Proxy::Message::Message(
        MessageType const type,
        ParamId const param_id,
        double const double_param
) noexcept
    : type(type),
    param_id(param_id),
    double_param(double_param)
{
}


Proxy::ParamIdHashTable::ParamIdHashTable() noexcept
{
}


Proxy::ParamIdHashTable::~ParamIdHashTable() noexcept
{
}


void Proxy::ParamIdHashTable::add(std::string const& name, ParamId const param_id) noexcept
{
    Entry* root;
    Entry* parent;
    Entry* entry;

    lookup(name, &root, &parent, &entry);

    if (entry != NULL) {
        return;
    }

    if (parent != NULL) {
        parent->next = new Entry(name.c_str(), param_id);

        return;
    }

    root->set(name.c_str(), param_id);
}


void Proxy::ParamIdHashTable::lookup(
        std::string const& name,
        Entry** root,
        Entry** parent,
        Entry** entry
) noexcept {
    char const* const name_ptr = name.c_str();
    int const hash = this->hash(name);
    *root = &entries[hash];

    *parent = NULL;

    if ((*root)->param_id == ParamId::INVALID_PARAM_ID) {
        *entry = NULL;

        return;
    }

    *entry = *root;

    while (strncmp((*entry)->name, name_ptr, Entry::NAME_SIZE) != 0) {
        *parent = *entry;
        *entry = (*entry)->next;

        if (*entry == NULL) {
            break;
        }
    }
}


Proxy::ParamId Proxy::ParamIdHashTable::lookup(std::string const& name) noexcept
{
    Entry* root;
    Entry* parent;
    Entry* entry;

    lookup(name, &root, &parent, &entry);

    return entry == NULL ? ParamId::INVALID_PARAM_ID : entry->param_id;
}


#ifdef MPE_EMULATOR_ASSERTIONS
void Proxy::ParamIdHashTable::get_statistics(
        unsigned int& max_collisions,
        double& avg_collisions,
        double& avg_bucket_size
) const noexcept {
    unsigned int collisions_sum = 0;
    unsigned int collisions_count = 0;
    unsigned int bucket_size_sum = 0;
    unsigned int bucket_count = 0;

    max_collisions = 0;

    for (unsigned int i = 0; i != ENTRIES; ++i) {
        Entry const* entry = &entries[i];

        if (entry->param_id == ParamId::INVALID_PARAM_ID) {
            continue;
        }

        unsigned int collisions = 1;
        ++bucket_count;
        ++bucket_size_sum;
        entry = entry->next;

        while (entry != NULL) {
            ++collisions;
            ++bucket_size_sum;
            entry = entry->next;
        }

        if (collisions > 1) {
            collisions_sum += collisions;
            ++collisions_count;

            if (collisions > max_collisions) {
                max_collisions = collisions;
            }
        }
    }

    avg_collisions = (double)collisions_sum / (double)collisions_count;
    avg_bucket_size = (double)bucket_size_sum / (double)bucket_count;
}
#endif


/*
Inspiration from https://orlp.net/blog/worlds-smallest-hash-table/
*/
int Proxy::ParamIdHashTable::hash(std::string const& name) noexcept
{
    /*
    We only care about the 36 characters which are used in param names: capital
    letters and numbers.
    */
    constexpr int alphabet_size = 36;
    constexpr char letter_offset = 'A' - 10;
    constexpr char number_offset = '0';

    char const* name_ptr = name.c_str();

    if (*name_ptr == '\x00') {
        return 0;
    }

    int i;
    int hash = 0;

    /* The first letter is the same for almost all parameters, let's skip it. */
    ++name_ptr;

    for (i = -1; *name_ptr != '\x00'; ++name_ptr) {
        char c = *name_ptr;

        if (c >= letter_offset) {
            c -= letter_offset;
        } else {
            c -= number_offset;
        }

        hash = hash * alphabet_size + c;

        if (++i == 4) {
            break;
        }
    }

    hash = (hash << 3) + i;

    if (hash < 0) {
        hash = -hash;
    }

    hash = (hash * MULTIPLIER >> SHIFT) & MASK;

    return hash;
}


Proxy::ParamIdHashTable::Entry::Entry() noexcept : next(NULL)
{
    set("", ParamId::INVALID_PARAM_ID);
}


Proxy::ParamIdHashTable::Entry::Entry(
        const char* const name,
        ParamId const param_id
) noexcept
    : next(NULL)
{
    set(name, param_id);
}


Proxy::ParamIdHashTable::Entry::~Entry()
{
    if (next != NULL) {
        delete next;

        next = NULL;
    }
}


void Proxy::ParamIdHashTable::Entry::set(
        const char* const name,
        ParamId const param_id
) noexcept {
    std::fill_n(this->name, NAME_SIZE, '\x00');
    strncpy(this->name, name, NAME_MAX_INDEX);
    this->param_id = param_id;
}


Proxy::MidiControllerMessage::MidiControllerMessage() : time_offset(-INFINITY), value(0)
{
}


Proxy::MidiControllerMessage::MidiControllerMessage(
        double const time_offset,
        Midi::Word const value
) : time_offset(time_offset),
    value(value)
{
}


bool Proxy::MidiControllerMessage::operator==(
        MidiControllerMessage const& message
) const noexcept {
    return value == message.value && time_offset == message.time_offset;
}

}
