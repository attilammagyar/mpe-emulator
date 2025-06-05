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

#ifndef MPE_EMULATOR__GUI__GUI_CPP
#define MPE_EMULATOR__GUI__GUI_CPP

#include <algorithm>
#include <cstdio>
#include <cstring>

#include "strings.hpp"

#include "gui/gui.hpp"


namespace MpeEmulator
{

constexpr GUI::Color GUI::rgb(
        ColorComponent const red,
        ColorComponent const green,
        ColorComponent const blue
) {
    return (Color)(
        (unsigned int)red << 16 | (unsigned int)green << 8 | (unsigned int)blue
    );
}


const GUI::Color GUI::TEXT_COLOR = GUI::rgb(181, 181, 189);
const GUI::Color GUI::TEXT_BACKGROUND = GUI::rgb(0, 0, 0);
const GUI::Color GUI::TEXT_HIGHLIGHT_COLOR = GUI::rgb(230, 230, 235);
const GUI::Color GUI::TEXT_HIGHLIGHT_BACKGROUND = GUI::rgb(82, 82, 86);
const GUI::Color GUI::STATUS_LINE_BACKGROUND = GUI::rgb(21, 21, 32);
const GUI::Color GUI::TOGGLE_OFF_COLOR = GUI::rgb(0, 0, 0);
const GUI::Color GUI::TOGGLE_ON_COLOR = GUI::rgb(150, 200, 230);


constexpr GUI::ColorComponent GUI::red(Color const color)
{
    return color >> 16;
}


constexpr GUI::ColorComponent GUI::green(Color const color)
{
    return color >> 8;
}


constexpr GUI::ColorComponent GUI::blue(Color const color)
{
    return color;
}


double GUI::clamp_ratio(double const ratio)
{
    return std::min(1.0, std::max(0.0, ratio));
}


#define KNOB_W 58
#define KNOB_H 85
#define KNOB_TOP 16


constexpr int pos_rel_offset_left = 0;
constexpr int pos_rel_offset_top = 0;

#define POSITION_RELATIVE_BEGIN(left, top)                          \
    do {                                                            \
        constexpr int pos_rel_offset_left = left;                   \
        constexpr int pos_rel_offset_top = top;                     \

#define POSITION_RELATIVE_END()                                     \
    } while (false)


#define KNOB(owner, left, top, param_id, ks, cnt)                   \
    owner->own(                                                     \
        new KnobParamEditor(                                        \
            *this,                                                  \
            pos_rel_offset_left + left,                             \
            pos_rel_offset_top + top,                               \
            KNOB_W,                                                 \
            KNOB_H,                                                 \
            KNOB_TOP,                                               \
            proxy,                                                  \
            param_id,                                               \
            ks,                                                     \
            cnt                                                     \
        )                                                           \
    )

#define KNOBC(owner, left, top, param_id, ks)                       \
    KNOB(owner, left, top, param_id, ks, KnobParamEditor::KnobType::CONTINUOUS)

#define KNOBD(owner, left, top, param_id, ks)                       \
    KNOB(owner, left, top, param_id, ks, KnobParamEditor::KnobType::DISCRETE)

#define TOGG(owner, left, top, width, height, box_left, param_id)   \
    owner->own(                                                     \
        new ToggleSwitchParamEditor(                                \
            *this,                                                  \
            pos_rel_offset_left + left,                             \
            pos_rel_offset_top + top,                               \
            width,                                                  \
            height,                                                 \
            box_left,                                               \
            proxy,                                                  \
            param_id                                                \
        )                                                           \
    )

#define DPEI(owner, left, top, w, h, vleft, vwidth, param_id, imgs) \
    owner->own(                                                     \
        new DiscreteParamEditor(                                    \
            *this,                                                  \
            pos_rel_offset_left + left,                             \
            pos_rel_offset_top + top,                               \
            w,                                                      \
            h,                                                      \
            vleft,                                                  \
            vwidth,                                                 \
            proxy,                                                  \
            param_id,                                               \
            imgs                                                    \
        )                                                           \
    )

#define DPET(owner, l, t, w, h, vl, vw, param_id)                   \
    DPETO(owner, l, t, w, h, vl, vw, param_id, (OptionSelector*)NULL)

#define DPETO(owner, l, t, w, h, vl, vw, param_id, option_selector) \
    (DiscreteParamEditor*)owner->own(                               \
        new DiscreteParamEditor(                                    \
            *this,                                                  \
            pos_rel_offset_left + l,                                \
            pos_rel_offset_top + t,                                 \
            w,                                                      \
            h,                                                      \
            vl,                                                     \
            vw,                                                     \
            proxy,                                                  \
            param_id,                                               \
            option_selector                                         \
        )                                                           \
    )

#define MIDP(owner, left, top, param_id, ks)                        \
    owner->own(                                                     \
        new KnobParamEditor(                                        \
            *this,                                                  \
            pos_rel_offset_left + left,                             \
            pos_rel_offset_top + top,                               \
            21,                                                     \
            21,                                                     \
            0,                                                      \
            proxy,                                                  \
            param_id,                                               \
            ks,                                                     \
            KnobParamEditor::KnobType::CONTINUOUS                   \
        )                                                           \
    )


GUI::GUI(
        char const* const sdk_version,
        PlatformData platform_data,
        PlatformWidget parent_window,
        Proxy& proxy,
        bool const show_vst_logo
)
    : show_vst_logo(show_vst_logo),
    dummy_widget(NULL),
    background(NULL),
    about_body(NULL),
    zone_1_body(NULL),
    status_line(NULL),
    proxy(proxy),
    platform_data(platform_data),
    active_voices_count(0),
    polyphony(0)
{
    default_status_line[0] = '\x00';
    update_active_voices_count();

    initialize();

    dummy_widget = new Widget("");

    knob_states = new ParamStateImages(
        dummy_widget,
        dummy_widget->load_image(this->platform_data, "KNOBSTATES"),
        128,
        48,
        48
    );

    rocker_switch = new ParamStateImages(
        dummy_widget,
        dummy_widget->load_image(this->platform_data, "ROCKERSWITCH"),
        2,
        48,
        48
    );

    distortions = new ParamStateImages(
        dummy_widget,
        dummy_widget->load_image(this->platform_data, "DISTORTIONS"),
        4,
        21,
        21
    );

    midpoint_states = new ParamStateImages(
        dummy_widget,
        dummy_widget->load_image(this->platform_data, "MIDPOINT"),
        128,
        21,
        21
    );

    about_image = dummy_widget->load_image(this->platform_data, "ABOUT");
    zone_1_image = dummy_widget->load_image(this->platform_data, "ZONE1");
    vst_logo_image = dummy_widget->load_image(this->platform_data, "VSTLOGO");

    background = new Background();

    this->parent_window = new ExternallyCreatedWindow(this->platform_data, parent_window);
    this->parent_window->own(background);

    background->set_image(zone_1_image);

    status_line = new StatusLine();
    status_line->set_text("");

    controller_selector = new OptionSelector(*background, proxy, "Select controller");

    build_about_body(sdk_version);
    build_zone_1_body(
        knob_states,
        rocker_switch,
        distortions,
        midpoint_states,
        controller_selector
    );

    background->own(
        new TabSelector(
            background,
            zone_1_image,
            zone_1_body,
            "Settings",
            TabSelector::LEFT + TabSelector::WIDTH * 0
        )
    );
    background->own(
        new TabSelector(
            background,
            about_image,
            about_body,
            "About",
            TabSelector::LEFT + TabSelector::WIDTH * 1
        )
    );

    background->replace_body(zone_1_body);
    background->own(status_line);

    background->own(controller_selector);
    controller_selector->hide();

    controller_selector->add_option(
        (unsigned int)Proxy::ControllerId::NONE,
        Strings::CONTROLLERS_LONG[Proxy::ControllerId::NONE]
    );
    controller_selector->add_option(
        (unsigned int)Proxy::ControllerId::MIDI_LEARN,
        Strings::CONTROLLERS_LONG[Proxy::ControllerId::MIDI_LEARN]
    );
    controller_selector->add_option(
        (unsigned int)Proxy::ControllerId::PITCH_WHEEL,
        Strings::CONTROLLERS_LONG[Proxy::ControllerId::PITCH_WHEEL]
    );
    controller_selector->add_option(
        (unsigned int)Proxy::ControllerId::CHANNEL_PRESSURE,
        Strings::CONTROLLERS_LONG[Proxy::ControllerId::CHANNEL_PRESSURE]
    );

    constexpr int limit = Proxy::ControllerId::MAX_MIDI_CC + 1;

    for (int i = 0; i != limit; ++i) {
        controller_selector->add_option(
            (unsigned int)i, Strings::CONTROLLERS_LONG[i]
        );
    }
}


void GUI::build_about_body(char const* const sdk_version)
{
    about_body = new TabBody(*this, "About");

    background->own(about_body);

    ((Widget*)about_body)->own(
        new AboutText(sdk_version, show_vst_logo ? vst_logo_image : NULL)
    );

    about_body->hide();
}


void GUI::build_zone_1_body(
        ParamStateImages const* const knob_states,
        ParamStateImages const* const rocker_switch,
        ParamStateImages const* const distortions,
        ParamStateImages const* const midpoint_states,
        OptionSelector* const controller_selector
) {
    zone_1_body = new TabBody(*this, "Settings");

    background->own(zone_1_body);

    POSITION_RELATIVE_BEGIN(295, 11);

    ((Widget*)zone_1_body)->own(
        new ImportSettingsButton(
            *this, pos_rel_offset_left + 4, pos_rel_offset_top + 30, 30, 30, proxy, zone_1_body
        )
    );
    ((Widget*)zone_1_body)->own(
        new ExportSettingsButton(
            *this, pos_rel_offset_left + 4, pos_rel_offset_top + 60, 30, 30, proxy
        )
    );

    KNOBD(zone_1_body, 38 + KNOB_W * 0, 30, Proxy::ParamId::Z1TYP, rocker_switch);
    KNOBD(zone_1_body, 38 + KNOB_W * 1, 30, Proxy::ParamId::Z1CHN, knob_states);
    KNOBD(zone_1_body, 38 + KNOB_W * 2, 30, Proxy::ParamId::Z1ENH, knob_states);
    KNOBD(zone_1_body, 38 + KNOB_W * 3, 30, Proxy::ParamId::Z1ANC, knob_states);
    KNOBD(zone_1_body, 38 + KNOB_W * 4, 30, Proxy::ParamId::Z1TRB, knob_states);
    KNOBD(zone_1_body, 38 + KNOB_W * 5, 30, Proxy::ParamId::Z1TRA, knob_states);

    TOGG(zone_1_body, 195, 4, 110, 24, 89, Proxy::ParamId::Z1ORV);
    TOGG(zone_1_body, 329, 4, 53, 24, 32, Proxy::ParamId::MCM);

    POSITION_RELATIVE_END();


    POSITION_RELATIVE_BEGIN(19, 149);

    DPETO(zone_1_body, 42, 44, 70, 23, 0, 70, Proxy::ParamId::Z1R1IN, controller_selector);
    DPETO(zone_1_body, 42, 79, 70, 23, 0, 70, Proxy::ParamId::Z1R1OU, controller_selector);
    KNOBC(zone_1_body, 6 + KNOB_W * 2, 30, Proxy::ParamId::Z1R1IV, knob_states);
    KNOBD(zone_1_body, 6 + KNOB_W * 3, 30, Proxy::ParamId::Z1R1TR, knob_states);
    KNOBC(zone_1_body, 6 + KNOB_W * 4, 30, Proxy::ParamId::Z1R1DL, knob_states);

    TOGG(zone_1_body,  66, 4, 62, 24, 41, Proxy::ParamId::Z1R1NV);
    DPET(zone_1_body, 138, 5, 73, 21, 38, 34, Proxy::ParamId::Z1R1RS);
    MIDP(zone_1_body, 251, 5, Proxy::ParamId::Z1R1MP, midpoint_states);
    DPEI(zone_1_body, 272, 5, 21, 21, 0, 21, Proxy::ParamId::Z1R1DT, distortions);

    POSITION_RELATIVE_END();


    POSITION_RELATIVE_BEGIN(339, 149);

    DPETO(zone_1_body, 42, 44, 70, 23, 0, 70, Proxy::ParamId::Z1R2IN, controller_selector);
    DPETO(zone_1_body, 42, 79, 70, 23, 0, 70, Proxy::ParamId::Z1R2OU, controller_selector);
    KNOBC(zone_1_body, 6 + KNOB_W * 2, 30, Proxy::ParamId::Z1R2IV, knob_states);
    KNOBD(zone_1_body, 6 + KNOB_W * 3, 30, Proxy::ParamId::Z1R2TR, knob_states);
    KNOBC(zone_1_body, 6 + KNOB_W * 4, 30, Proxy::ParamId::Z1R2DL, knob_states);

    TOGG(zone_1_body,  66, 4, 62, 24, 41, Proxy::ParamId::Z1R2NV);
    DPET(zone_1_body, 138, 5, 73, 21, 38, 34, Proxy::ParamId::Z1R2RS);
    MIDP(zone_1_body, 251, 5, Proxy::ParamId::Z1R2MP, midpoint_states);
    DPEI(zone_1_body, 272, 5, 21, 21, 0, 21, Proxy::ParamId::Z1R2DT, distortions);

    POSITION_RELATIVE_END();


    POSITION_RELATIVE_BEGIN(659, 149);

    DPETO(zone_1_body, 42, 44, 70, 23, 0, 70, Proxy::ParamId::Z1R3IN, controller_selector);
    DPETO(zone_1_body, 42, 79, 70, 23, 0, 70, Proxy::ParamId::Z1R3OU, controller_selector);
    KNOBC(zone_1_body, 6 + KNOB_W * 2, 30, Proxy::ParamId::Z1R3IV, knob_states);
    KNOBD(zone_1_body, 6 + KNOB_W * 3, 30, Proxy::ParamId::Z1R3TR, knob_states);
    KNOBC(zone_1_body, 6 + KNOB_W * 4, 30, Proxy::ParamId::Z1R3DL, knob_states);

    TOGG(zone_1_body,  66, 4, 62, 24, 41, Proxy::ParamId::Z1R3NV);
    DPET(zone_1_body, 138, 5, 73, 21, 38, 34, Proxy::ParamId::Z1R3RS);
    MIDP(zone_1_body, 251, 5, Proxy::ParamId::Z1R3MP, midpoint_states);
    DPEI(zone_1_body, 272, 5, 21, 21, 0, 21, Proxy::ParamId::Z1R3DT, distortions);

    POSITION_RELATIVE_END();


    POSITION_RELATIVE_BEGIN(19, 287);

    DPETO(zone_1_body, 42, 44, 70, 23, 0, 70, Proxy::ParamId::Z1R4IN, controller_selector);
    DPETO(zone_1_body, 42, 79, 70, 23, 0, 70, Proxy::ParamId::Z1R4OU, controller_selector);
    KNOBC(zone_1_body, 6 + KNOB_W * 2, 30, Proxy::ParamId::Z1R4IV, knob_states);
    KNOBD(zone_1_body, 6 + KNOB_W * 3, 30, Proxy::ParamId::Z1R4TR, knob_states);
    KNOBC(zone_1_body, 6 + KNOB_W * 4, 30, Proxy::ParamId::Z1R4DL, knob_states);

    TOGG(zone_1_body,  66, 4, 62, 24, 41, Proxy::ParamId::Z1R4NV);
    DPET(zone_1_body, 138, 5, 73, 21, 38, 34, Proxy::ParamId::Z1R4RS);
    MIDP(zone_1_body, 251, 5, Proxy::ParamId::Z1R4MP, midpoint_states);
    DPEI(zone_1_body, 272, 5, 21, 21, 0, 21, Proxy::ParamId::Z1R4DT, distortions);

    POSITION_RELATIVE_END();


    POSITION_RELATIVE_BEGIN(339, 287);

    DPETO(zone_1_body, 42, 44, 70, 23, 0, 70, Proxy::ParamId::Z1R5IN, controller_selector);
    DPETO(zone_1_body, 42, 79, 70, 23, 0, 70, Proxy::ParamId::Z1R5OU, controller_selector);
    KNOBC(zone_1_body, 6 + KNOB_W * 2, 30, Proxy::ParamId::Z1R5IV, knob_states);
    KNOBD(zone_1_body, 6 + KNOB_W * 3, 30, Proxy::ParamId::Z1R5TR, knob_states);
    KNOBC(zone_1_body, 6 + KNOB_W * 4, 30, Proxy::ParamId::Z1R5DL, knob_states);

    TOGG(zone_1_body,  66, 4, 62, 24, 41, Proxy::ParamId::Z1R5NV);
    DPET(zone_1_body, 138, 5, 73, 21, 38, 34, Proxy::ParamId::Z1R5RS);
    MIDP(zone_1_body, 251, 5, Proxy::ParamId::Z1R5MP, midpoint_states);
    DPEI(zone_1_body, 272, 5, 21, 21, 0, 21, Proxy::ParamId::Z1R5DT, distortions);

    POSITION_RELATIVE_END();


    POSITION_RELATIVE_BEGIN(659, 287);

    DPETO(zone_1_body, 42, 44, 70, 23, 0, 70, Proxy::ParamId::Z1R6IN, controller_selector);
    DPETO(zone_1_body, 42, 79, 70, 23, 0, 70, Proxy::ParamId::Z1R6OU, controller_selector);
    KNOBC(zone_1_body, 6 + KNOB_W * 2, 30, Proxy::ParamId::Z1R6IV, knob_states);
    KNOBD(zone_1_body, 6 + KNOB_W * 3, 30, Proxy::ParamId::Z1R6TR, knob_states);
    KNOBC(zone_1_body, 6 + KNOB_W * 4, 30, Proxy::ParamId::Z1R6DL, knob_states);

    TOGG(zone_1_body,  66, 4, 62, 24, 41, Proxy::ParamId::Z1R6NV);
    DPET(zone_1_body, 138, 5, 73, 21, 38, 34, Proxy::ParamId::Z1R6RS);
    MIDP(zone_1_body, 251, 5, Proxy::ParamId::Z1R6MP, midpoint_states);
    DPEI(zone_1_body, 272, 5, 21, 21, 0, 21, Proxy::ParamId::Z1R6DT, distortions);

    POSITION_RELATIVE_END();


    POSITION_RELATIVE_BEGIN(19, 425);

    DPETO(zone_1_body, 42, 44, 70, 23, 0, 70, Proxy::ParamId::Z1R7IN, controller_selector);
    DPETO(zone_1_body, 42, 79, 70, 23, 0, 70, Proxy::ParamId::Z1R7OU, controller_selector);
    KNOBC(zone_1_body, 6 + KNOB_W * 2, 30, Proxy::ParamId::Z1R7IV, knob_states);
    KNOBD(zone_1_body, 6 + KNOB_W * 3, 30, Proxy::ParamId::Z1R7TR, knob_states);
    KNOBC(zone_1_body, 6 + KNOB_W * 4, 30, Proxy::ParamId::Z1R7DL, knob_states);

    TOGG(zone_1_body,  66, 4, 62, 24, 41, Proxy::ParamId::Z1R7NV);
    DPET(zone_1_body, 138, 5, 73, 21, 38, 34, Proxy::ParamId::Z1R7RS);
    MIDP(zone_1_body, 251, 5, Proxy::ParamId::Z1R7MP, midpoint_states);
    DPEI(zone_1_body, 272, 5, 21, 21, 0, 21, Proxy::ParamId::Z1R7DT, distortions);

    POSITION_RELATIVE_END();


    POSITION_RELATIVE_BEGIN(339, 425);

    DPETO(zone_1_body, 42, 44, 70, 23, 0, 70, Proxy::ParamId::Z1R8IN, controller_selector);
    DPETO(zone_1_body, 42, 79, 70, 23, 0, 70, Proxy::ParamId::Z1R8OU, controller_selector);
    KNOBC(zone_1_body, 6 + KNOB_W * 2, 30, Proxy::ParamId::Z1R8IV, knob_states);
    KNOBD(zone_1_body, 6 + KNOB_W * 3, 30, Proxy::ParamId::Z1R8TR, knob_states);
    KNOBC(zone_1_body, 6 + KNOB_W * 4, 30, Proxy::ParamId::Z1R8DL, knob_states);

    TOGG(zone_1_body,  66, 4, 62, 24, 41, Proxy::ParamId::Z1R8NV);
    DPET(zone_1_body, 138, 5, 73, 21, 38, 34, Proxy::ParamId::Z1R8RS);
    MIDP(zone_1_body, 251, 5, Proxy::ParamId::Z1R8MP, midpoint_states);
    DPEI(zone_1_body, 272, 5, 21, 21, 0, 21, Proxy::ParamId::Z1R8DT, distortions);

    POSITION_RELATIVE_END();


    POSITION_RELATIVE_BEGIN(659, 425);

    DPETO(zone_1_body, 42, 44, 70, 23, 0, 70, Proxy::ParamId::Z1R9IN, controller_selector);
    DPETO(zone_1_body, 42, 79, 70, 23, 0, 70, Proxy::ParamId::Z1R9OU, controller_selector);
    KNOBC(zone_1_body, 6 + KNOB_W * 2, 30, Proxy::ParamId::Z1R9IV, knob_states);
    KNOBD(zone_1_body, 6 + KNOB_W * 3, 30, Proxy::ParamId::Z1R9TR, knob_states);
    KNOBC(zone_1_body, 6 + KNOB_W * 4, 30, Proxy::ParamId::Z1R9DL, knob_states);

    TOGG(zone_1_body,  66, 4, 62, 24, 41, Proxy::ParamId::Z1R9NV);
    DPET(zone_1_body, 138, 5, 73, 21, 38, 34, Proxy::ParamId::Z1R9RS);
    MIDP(zone_1_body, 251, 5, Proxy::ParamId::Z1R9MP, midpoint_states);
    DPEI(zone_1_body, 272, 5, 21, 21, 0, 21, Proxy::ParamId::Z1R9DT, distortions);

    POSITION_RELATIVE_END();


    zone_1_body->hide();
}


GUI::~GUI()
{
    delete parent_window;

    delete knob_states;
    delete rocker_switch;
    delete distortions;
    delete midpoint_states;

    dummy_widget->delete_image(about_image);
    dummy_widget->delete_image(zone_1_image);
    dummy_widget->delete_image(vst_logo_image);

    delete dummy_widget;

    dummy_widget = NULL;

    destroy();
}


void GUI::show()
{
    background->show();
}


void GUI::update_active_voices_count()
{
    unsigned int const old_active_voices_count = active_voices_count;
    unsigned int const old_polyphony = polyphony;

    active_voices_count = proxy.get_active_voices_count();
    polyphony = proxy.get_channel_count();

    if (active_voices_count == old_active_voices_count && polyphony == old_polyphony) {
        return;
    }

    if (active_voices_count < 1) {
        default_status_line[0] = '\x00';
    } else {
        snprintf(
            default_status_line,
            DEFAULT_STATUS_LINE_MAX_LENGTH,
            "Voices: %u / %u",
            active_voices_count,
            polyphony
        );
        default_status_line[DEFAULT_STATUS_LINE_MAX_LENGTH - 1] = '\x00';
    }

    if (status_line != NULL) {
        set_status_line(default_status_line);
        redraw_status_line();
    }
}


void GUI::set_status_line(char const* const text)
{
    if (text[0] == '\x00') {
        status_line->set_text(default_status_line);
    } else {
        status_line->set_text(text);
    }
}


void GUI::redraw_status_line()
{
    status_line->redraw();
}


GUI::PlatformData GUI::get_platform_data() const
{
    return platform_data;
}


WidgetBase::WidgetBase(char const* const text)
    : type(Type::BACKGROUND),
    platform_widget(NULL),
    platform_data(NULL),
    image(NULL),
    gui(NULL),
    parent(NULL),
    text(text),
    left(0),
    top(0),
    width(0),
    height(0),
    is_clicking(false)
{
}


WidgetBase::WidgetBase(
        char const* const text,
        int const left,
        int const top,
        int const width,
        int const height,
        Type const type
) : type(type),
    platform_widget(NULL),
    platform_data(NULL),
    image(NULL),
    gui(NULL),
    parent(NULL),
    text(text),
    left(left),
    top(top),
    width(width),
    height(height),
    is_clicking(false)
{
}


WidgetBase::WidgetBase(
        GUI::PlatformData platform_data,
        GUI::PlatformWidget platform_widget,
        Type const type
) : type(type),
    platform_widget(platform_widget),
    platform_data(platform_data),
    image(NULL),
    gui(NULL),
    parent(NULL),
    text(""),
    left(0),
    top(0),
    width(0),
    height(0),
    is_clicking(false)
{
}


WidgetBase::~WidgetBase()
{
}


void WidgetBase::destroy_children()
{
    for (GUI::Widgets::iterator it = children.begin(); it != children.end(); ++it) {
        delete *it;
    }
}


int WidgetBase::get_left() const
{
    return left;
}


int WidgetBase::get_top() const
{
    return top;
}


int WidgetBase::get_width() const
{
    return width;
}


int WidgetBase::get_height() const
{
    return height;
}


void WidgetBase::set_text(char const* const text)
{
    this->text = text;
}


char const* WidgetBase::get_text() const
{
    return text;
}


WidgetBase* WidgetBase::get_parent() const
{
    return parent;
}


GUI::Image WidgetBase::load_image(
    GUI::PlatformData platform_data,
    char const* const name
) {
    return NULL;
}


void WidgetBase::delete_image(GUI::Image image)
{
}


void WidgetBase::show()
{
}


void WidgetBase::hide()
{
}



void WidgetBase::focus()
{
}


void WidgetBase::bring_to_top()
{
}


void WidgetBase::redraw()
{
}


WidgetBase* WidgetBase::own(WidgetBase* const widget)
{
    children.push_back(widget);
    widget->set_up(platform_data, (WidgetBase*)this);

    return widget;
}


GUI::Image WidgetBase::set_image(GUI::Image image)
{
    GUI::Image old = this->image;
    this->image = image;
    redraw();

    return old;
}


GUI::Image WidgetBase::get_image() const
{
    return image;
}


GUI::PlatformWidget WidgetBase::get_platform_widget()
{
    return platform_widget;
}


void WidgetBase::click()
{
}


void WidgetBase::set_up(
        GUI::PlatformData platform_data,
        WidgetBase* const parent
) {
    this->platform_data = platform_data;
    this->parent = parent;
}


void WidgetBase::set_gui(GUI& gui)
{
    this->gui = &gui;
}


bool WidgetBase::paint()
{
    if (image == NULL) {
        return false;
    }

    draw_image(image, 0, 0, width, height);

    return true;
}


bool WidgetBase::double_click()
{
    return false;
}


bool WidgetBase::mouse_down(int const x, int const y)
{
    return false;
}


bool WidgetBase::mouse_up(int const x, int const y)
{
    return false;
}


bool WidgetBase::mouse_move(int const x, int const y, bool const modifier)
{
    return false;
}


bool WidgetBase::mouse_leave(int const x, int const y)
{
    return false;
}


bool WidgetBase::mouse_wheel(double const delta, bool const modifier)
{
    return false;
}


void WidgetBase::fill_rectangle(
    int const left,
    int const top,
    int const width,
    int const height,
    GUI::Color const color
) {
}


void WidgetBase::draw_text(
    char const* const text,
    int const font_size_px,
    int const left,
    int const top,
    int const width,
    int const height,
    GUI::Color const color,
    GUI::Color const background,
    FontWeight const font_weight,
    int const padding,
    TextAlignment const alignment
) {
}


void WidgetBase::draw_image(
        GUI::Image image,
        int const left,
        int const top,
        int const width,
        int const height
) {
}


GUI::Image WidgetBase::copy_image_region(
    GUI::Image source,
    int const left,
    int const top,
    int const width,
    int const height
) {
    return NULL;
}

}

#endif
