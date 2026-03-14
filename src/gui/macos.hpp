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

#ifndef MPE_EMULATOR__GUI_MACOS_HPP
#define MPE_EMULATOR__GUI_MACOS_HPP

#include <cstddef>

#include "common.hpp"

#include "gui/gui.hpp"


namespace MpeEmulator {

class Widget : public WidgetBase
{
    public:
        static bool notify_paint(Widget* widget);
        static bool notify_double_click(Widget* widget);
        static bool notify_mouse_down(Widget* widget, int const x, int const y);
        static bool notify_mouse_up(Widget* widget, int const x, int const y);
        static bool notify_mouse_move(Widget* widget, int const x, int const y, bool const modifier);
        static bool notify_mouse_leave(Widget* widget, int const x, int const y);
        static bool notify_mouse_wheel(Widget* widget, double const delta, bool const modifier);
        static bool notify_timer_tick(Widget* widget);

        explicit Widget(char const* const text);

        virtual ~Widget();

        virtual void set_scale(double const new_scale) override;

        virtual GUI::Image load_image(
            GUI::PlatformData platform_data,
            char const* const name
        ) override;

        GUI::Image copy_image_region(
            GUI::Image source,
            int const left,
            int const top,
            int const width,
            int const height
        ) override;

        GUI::Image downscale_image(
            GUI::Image source,
            int const old_width,
            int const old_height,
            int const new_width,
            int const new_height
        ) override;

        virtual void delete_image(GUI::Image image) override;

        virtual uint64_t monotonic_clock_ms() override;

        virtual void show() override;
        virtual void hide() override;

        virtual void focus() override;
        virtual void bring_to_top() override;
        virtual void redraw() override;

    protected:
        Widget(
            char const* const text,
            int const left,
            int const top,
            int const width,
            int const height,
            Type const type
        );

        Widget(
            GUI::PlatformData platform_data,
            GUI::PlatformWidget platform_widget,
            Type const type
        );

        virtual void set_up(
            GUI::PlatformData platform_data,
            WidgetBase* const parent
        ) override;

        virtual void fill_rectangle(
            int const left,
            int const top,
            int const width,
            int const height,
            GUI::Color const color
        ) override;

        virtual void draw_text(
            char const* const text,
            int const font_size_px,
            int const left,
            int const top,
            int const width,
            int const height,
            GUI::Color const color,
            GUI::Color const background,
            FontWeight const font_weight = FontWeight::NORMAL,
            int const padding = 0,
            TextAlignment const alignment = TextAlignment::CENTER
        ) override;

        virtual void draw_image(
            GUI::Image image,
            int const left,
            int const top,
            int const width,
            int const height
        ) override;

    private:
        bool timer_tick();
};

}


extern "C" {
    MpeEmulator::GUI::PlatformWidget mpe_emulator_create_platform_widget(
        MpeEmulator::Widget* cpp_widget,
        int const left,
        int const top,
        int const width,
        int const height,
        MpeEmulator::GUI::PlatformWidget parent,
        MpeEmulator::WidgetBase::Type const type
    );

    void mpe_emulator_destroy_platform_widget(MpeEmulator::GUI::PlatformWidget platform_widget);

    void mpe_emulator_widget_resize(
        MpeEmulator::GUI::PlatformWidget platform_widget,
        int const left,
        int const top,
        int const width,
        int const height
    );

    void mpe_emulator_widget_show(MpeEmulator::GUI::PlatformWidget platform_widget);
    void mpe_emulator_widget_hide(MpeEmulator::GUI::PlatformWidget platform_widget);
    void mpe_emulator_widget_focus(MpeEmulator::GUI::PlatformWidget platform_widget);
    void mpe_emulator_widget_bring_to_top(MpeEmulator::GUI::PlatformWidget platform_widget);
    void mpe_emulator_widget_redraw(MpeEmulator::GUI::PlatformWidget platform_widget);

    void mpe_emulator_widget_fill_rectangle(
        int const left,
        int const top,
        int const width,
        int const height,
        MpeEmulator::GUI::Color const color
    );

    void mpe_emulator_widget_draw_text(
        MpeEmulator::GUI::PlatformWidget platform_widget,
        char const* const text,
        int const font_size_px,
        int const left,
        int const top,
        int const width,
        int const height,
        MpeEmulator::GUI::Color const color,
        MpeEmulator::WidgetBase::FontWeight const font_weight,
        MpeEmulator::WidgetBase::TextAlignment const alignment
    );

    MpeEmulator::GUI::Image mpe_emulator_widget_load_image(char const* const name);

    MpeEmulator::GUI::Image mpe_emulator_widget_copy_image_region(
        MpeEmulator::GUI::Image source,
        int const left,
        int const top,
        int const width,
        int const height
    );

    MpeEmulator::GUI::Image mpe_emulator_widget_downscale_image(
        MpeEmulator::GUI::Image source,
        int const old_width,
        int const old_height,
        int const new_width,
        int const new_height
    );

    void mpe_emulator_widget_delete_image(MpeEmulator::GUI::Image image);

    void mpe_emulator_widget_draw_image(
        MpeEmulator::GUI::Image image,
        int const left,
        int const top,
        int const width,
        int const height
    );

    bool mpe_emulator_import_settings(
        char* const buffer,
        size_t* length,
        size_t const max_length
    );

    void mpe_emulator_export_settings(char const* const buffer, size_t const length);
}


#include "gui/widgets.hpp"

#endif
