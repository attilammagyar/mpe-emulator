/*
 * This file is part of MPE Emulator
 * Copyright (C) 2026  Attila M. Magyar
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

#ifndef MPE_EMULATOR__GUI_MACOS_CPP
#define MPE_EMULATOR__GUI_MACOS_CPP

#include <ctime>
#include <map>
#include <string>

#include "gui/macos.hpp"


namespace MpeEmulator {

typedef std::map<std::string, char const*> ImageResourceNameMap;

ImageResourceNameMap const image_resource_names{
    {"ABOUT", "about"},
    {"DISTORTIONS", "distortions"},
    {"MIDPOINT", "midpoint_states"},
    {"KNOBSTATES", "knob_states"},
    {"ROCKERSWITCH", "rocker_switch"},
    {"VSTLOGO", "vst_logo"},
    {"ZONE1", "zone1"},
};


void GUI::initialize()
{
}


void GUI::destroy()
{
}


bool Widget::notify_paint(Widget* widget)
{
    return widget != NULL ? widget->paint() : false;
}


bool Widget::notify_double_click(Widget* widget)
{
    if (widget == NULL) {
        return false;
    }

    widget->is_clicking = false;

    return widget->double_click();
}


bool Widget::notify_mouse_down(Widget* widget, int const x, int const y)
{
    if (widget == NULL) {
        return false;
    }

    widget->is_clicking = true;

    return widget->mouse_down(x, y);
}


bool Widget::notify_mouse_up(Widget* widget, int const x, int const y)
{
    if (widget == NULL) {
        return false;
    }

    bool const is_handled = widget->mouse_up(x, y);

    if (widget->is_clicking) {
        widget->is_clicking = false;
        widget->click();
    }

    return is_handled;
}


bool Widget::notify_mouse_move(
        Widget* widget,
        int const x,
        int const y,
        bool const modifier
) {
    return widget != NULL ? widget->mouse_move(x, y, modifier) : false;
}


bool Widget::notify_mouse_leave(Widget* widget, int const x, int const y)
{
    return widget != NULL ? widget->mouse_leave(x, y) : false;
}


bool Widget::notify_mouse_wheel(
        Widget* widget,
        double const delta,
        bool const modifier
) {
    return widget != NULL ? widget->mouse_wheel(delta, modifier) : false;
}


bool Widget::notify_timer_tick(Widget* widget)
{
    return widget != NULL ? widget->timer_tick() : false;
}


Widget::Widget(char const* const text) : WidgetBase(text)
{
}


Widget::Widget(
        char const* const text,
        int const left,
        int const top,
        int const width,
        int const height,
        Type const type
) : WidgetBase(text, left, top, width, height, type)
{
}


Widget::Widget(
        GUI::PlatformData platform_data,
        GUI::PlatformWidget platform_widget,
        Type const type
) : WidgetBase(platform_data, platform_widget, type)
{
}


Widget::~Widget()
{
    destroy_children();

    if (platform_widget != NULL) {
        mpe_emulator_destroy_platform_widget(platform_widget);
        platform_widget = NULL;
    }
}


void Widget::set_up(GUI::PlatformData platform_data, WidgetBase* const parent)
{
    WidgetBase::set_up(platform_data, parent);

    platform_widget = mpe_emulator_create_platform_widget(
        this,
        left,
        top,
        width,
        height,
        parent != NULL ? parent->get_platform_widget() : NULL,
        type
    );
}


void Widget::fill_rectangle(
        int const left,
        int const top,
        int const width,
        int const height,
        GUI::Color const color
) {
    mpe_emulator_widget_fill_rectangle(left, top, width, height, color);
}


void Widget::draw_text(
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
    if (platform_widget == NULL) {
        return;
    }

    fill_rectangle(left, top, width, height, background);
    mpe_emulator_widget_draw_text(
        platform_widget,
        text,
        font_size_px,
        left + padding,
        top,
        width - 2 * padding,
        height,
        color,
        font_weight,
        alignment
    );
}


void Widget::draw_image(
        GUI::Image image,
        int const left,
        int const top,
        int const width,
        int const height
) {
    if (image == NULL) {
        return;
    }

    mpe_emulator_widget_draw_image(image, left, top, width, height);
}


void Widget::set_scale(double const new_scale)
{
    WidgetBase::set_scale(new_scale);

    if (platform_widget == NULL) {
        return;
    }

    mpe_emulator_widget_resize(
        platform_widget,
        scale_value(left),
        scale_value(top),
        scale_value(width),
        scale_value(height)
    );
}


GUI::Image Widget::load_image(
        GUI::PlatformData platform_data,
        char const* const name
) {
    ImageResourceNameMap::const_iterator it = image_resource_names.find(name);

    if (it == image_resource_names.end()) {
        MPE_EMULATOR_ASSERT_NOT_REACHED();

        return NULL;
    }

    return mpe_emulator_widget_load_image(it->second);
}


GUI::Image Widget::copy_image_region(
        GUI::Image source,
        int const left,
        int const top,
        int const width,
        int const height
) {
    if (source == NULL) {
        return NULL;
    }

    return mpe_emulator_widget_copy_image_region(
        source, left, top, width, height
    );
}


GUI::Image Widget::downscale_image(
        GUI::Image source,
        int const old_width,
        int const old_height,
        int const new_width,
        int const new_height
) {
    if (source == NULL) {
        return NULL;
    }

    return mpe_emulator_widget_downscale_image(
        source,
        old_width,
        old_height,
        new_width,
        new_height
    );
}


void Widget::delete_image(GUI::Image image)
{
    if (image != NULL) {
        mpe_emulator_widget_delete_image(image);
    }
}


uint64_t Widget::monotonic_clock_ms()
{
    struct timespec time;

    clock_gettime(CLOCK_MONOTONIC, &time);

    return time.tv_sec * 1000 + time.tv_nsec / 1000000;
}


void Widget::show()
{
    WidgetBase::show();

    if (platform_widget == NULL) {
        return;
    }

    mpe_emulator_widget_show(platform_widget);
}


void Widget::hide()
{
    WidgetBase::hide();

    if (platform_widget == NULL) {
        return;
    }

    mpe_emulator_widget_hide(platform_widget);
}


void Widget::focus()
{
    WidgetBase::focus();

    if (platform_widget == NULL) {
        return;
    }

    mpe_emulator_widget_focus(platform_widget);
}


void Widget::bring_to_top()
{
    WidgetBase::bring_to_top();

    if (platform_widget == NULL) {
        return;
    }

    mpe_emulator_widget_bring_to_top(platform_widget);
}


void Widget::redraw()
{
    WidgetBase::redraw();

    if (platform_widget == NULL) {
        return;
    }

    mpe_emulator_widget_redraw(platform_widget);
}


bool Widget::timer_tick()
{
    if (type != WidgetBase::Type::BACKGROUND || gui == NULL) {
        return false;
    }

    gui->idle();

    return true;
}

}


#include "gui/widgets.hpp"
#include "gui/gui.cpp"


namespace MpeEmulator {

void ImportSettingsButton::click()
{
    char* const buffer = new char[Serializer::MAX_SIZE];
    size_t length = 0;

    std::fill_n(buffer, Serializer::MAX_SIZE, '\x00');

    if (!mpe_emulator_import_settings(buffer, &length, Serializer::MAX_SIZE)) {
        return;
    }

    import_settings(buffer, (int)length);

    delete[] buffer;
}


void ExportSettingsButton::click()
{
    std::string const& settings = Serializer::serialize(proxy);

    mpe_emulator_export_settings(settings.c_str(), settings.length());
}


void GUI::idle()
{
    handle_scheduled_resize();

    if (background != NULL) {
        background->refresh();
    }
}

}


#include "gui/widgets.cpp"

#endif
