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

#ifndef MPE_EMULATOR__GUI__GUI_HPP
#define MPE_EMULATOR__GUI__GUI_HPP

#include <cstddef>
#include <vector>

#include "common.hpp"
#include "proxy.hpp"


namespace MpeEmulator
{

class Background;
class ExportSettingsButton;
class ExternallyCreatedWindow;
class ImportSettingsButton;
class ParamStateImages;
class KnobParamEditor;
class StatusLine;
class TabBody;
class TabSelector;
class OptionSelector;
class DiscreteParamEditor;
class ToggleSwitchParamEditor;
class TuningSelector;
class Widget;
class WidgetBase;


class GUI
{
    public:
        typedef void* PlatformWidget; ///< \brief GUI platform dependent widget type.
        typedef void* PlatformData; ///< \brief GUI platform dependent data (e.g. HINSTANCE on Windows).
        typedef void* Image; ///< \brief GUI platform dependent image handle.

        typedef std::vector<WidgetBase*> Widgets;

        typedef std::vector<KnobParamEditor*> KnobParamEditors;
        typedef std::vector<ToggleSwitchParamEditor*> ToggleSwitchParamEditors;
        typedef std::vector<DiscreteParamEditor*> DiscreteParamEditors;

        typedef unsigned int Color;
        typedef unsigned char ColorComponent;

        static constexpr long int WIDTH = 980;
        static constexpr long int HEIGHT = 600;

        static constexpr double REFRESH_RATE = 18.0;
        static constexpr double REFRESH_RATE_SECONDS = 1.0 / REFRESH_RATE;

        static char const* const CONTROLLERS_SHORT[];
        static char const* const CONTROLLERS_LONG[];
        static size_t const CONTROLLERS_COUNT;

        static char const* const ZONE_TYPES[];
        static size_t const ZONE_TYPES_COUNT;

        static char const* const CHANNELS[];
        static size_t const CHANNELS_COUNT;

        static char const* const ANCHORS[];
        static size_t const ANCHORS_COUNT;

        static char const* const EXCESS_NOTE_HANDLINGS[];
        static size_t const EXCESS_NOTE_HANDLINGS_COUNT;

        static char const* const TARGETS[];
        static size_t const TARGETS_COUNT;

        static char const* const PARAMS[Proxy::ParamId::PARAM_ID_COUNT];

        static constexpr Color rgb(
            ColorComponent const red,
            ColorComponent const green,
            ColorComponent const blue
        );
        static constexpr ColorComponent red(Color const color);
        static constexpr ColorComponent green(Color const color);
        static constexpr ColorComponent blue(Color const color);

        static Color const TEXT_COLOR;
        static Color const TEXT_BACKGROUND;
        static Color const TEXT_HIGHLIGHT_COLOR;
        static Color const TEXT_HIGHLIGHT_BACKGROUND;
        static Color const STATUS_LINE_BACKGROUND;
        static Color const TOGGLE_OFF_COLOR;
        static Color const TOGGLE_ON_COLOR;

        static void param_ratio_to_str(
            Proxy const& synth,
            Proxy::ParamId const param_id,
            double const ratio,
            double const scale,
            char const* const format,
            char const* const* const options,
            size_t const number_of_options,
            char* const buffer,
            size_t const buffer_size
        );

        static double clamp_ratio(double const ratio);

        GUI(
            char const* const sdk_version,
            PlatformData platform_data,
            PlatformWidget parent_window,
            Proxy& synth,
            bool const show_vst_logo
        );

        GUI(GUI const& gui) = delete;
        GUI& operator=(GUI const& gui) = delete;

        ~GUI();

        void show();
        void idle();

        void update_active_voices_count();

        void set_status_line(char const* const text);
        void redraw_status_line();

        PlatformData get_platform_data() const;

    private:
        static constexpr size_t DEFAULT_STATUS_LINE_MAX_LENGTH = 32;

        static void param_ratio_to_str_float(
            Proxy const& synth,
            Proxy::ParamId const param_id,
            double const ratio,
            double const scale,
            char const* const format,
            char* const buffer,
            size_t const buffer_size
        );

        static void param_ratio_to_str_options(
            Proxy const& synth,
            Proxy::ParamId const param_id,
            double const ratio,
            char const* const* const options,
            size_t const number_of_options,
            char* const buffer,
            size_t const buffer_size
        );

        static void param_ratio_to_str_int(
            Proxy const& synth,
            Proxy::ParamId const param_id,
            double const ratio,
            char* const buffer,
            size_t const buffer_size
        );

        void initialize();
        void destroy();

        void build_about_body(char const* const sdk_version);

        void build_zone_1_body(
            ParamStateImages const* const knob_states,
            ParamStateImages const* const rocker_switch,
            ParamStateImages const* const distortions,
            ParamStateImages const* const midpoint_states,
            OptionSelector* const controller_selector
        );

        bool const show_vst_logo;

        char default_status_line[DEFAULT_STATUS_LINE_MAX_LENGTH];

        Widget* dummy_widget;

        Image about_image;
        Image zone_1_image;
        Image vst_logo_image;

        ParamStateImages const* knob_states;
        ParamStateImages const* rocker_switch;
        ParamStateImages const* distortions;
        ParamStateImages const* midpoint_states;
        Background* background;
        OptionSelector* controller_selector;
        OptionSelector* target_selector;
        TabBody* about_body;
        TabBody* zone_1_body;
        StatusLine* status_line;

        Proxy& proxy;
        MpeEmulator::GUI::PlatformData platform_data;
        ExternallyCreatedWindow* parent_window;

        unsigned int active_voices_count;
        unsigned int polyphony;
};


/**
 * \brief Base class for the platform-dependent \c Widget class.
 */
class WidgetBase
{
    public:
        enum Type {
            BACKGROUND = 1 << 0,
            EXPORT_SETTINGS_BUTTON = 1 << 1,
            EXTERNALLY_CREATED_WINDOW = 1 << 2,
            IMPORT_SETTINGS_BUTTON = 1 << 3,
            KNOB = 1 << 4,
            KNOB_PARAM_EDITOR = 1 << 5,
            TAB_BODY = 1 << 6,
            TAB_SELECTOR = 1 << 7,
            ABOUT_TEXT = 1 << 8,
            STATUS_LINE = 1 << 9,
            TOGGLE_SWITCH = 1 << 10,
            DISCRETE_PARAM_EDITOR = 1 << 11,
            OPTION_SELECTOR = 1 << 12,
            OPTION = 1 << 13,
        };

        enum TextAlignment {
            LEFT = 0,
            CENTER = 1,
            RIGHT = 2,
        };

        enum FontWeight {
            NORMAL = 0,
            BOLD = 1,
        };

        explicit WidgetBase(char const* const text);
        virtual ~WidgetBase();

        virtual int get_left() const;
        virtual int get_top() const;
        virtual int get_width() const;
        virtual int get_height() const;
        virtual WidgetBase* get_parent() const;

        virtual void set_text(char const* const text);
        virtual char const* get_text() const;

        virtual GUI::Image load_image(
            GUI::PlatformData platform_data,
            char const* const name
        );

        virtual GUI::Image copy_image_region(
            GUI::Image source,
            int const left,
            int const top,
            int const width,
            int const height
        );

        virtual void delete_image(GUI::Image image);

        virtual void show();
        virtual void hide();
        virtual void focus();
        virtual void bring_to_top();
        virtual void redraw();
        virtual WidgetBase* own(WidgetBase* const widget);

        virtual GUI::Image set_image(GUI::Image image);
        virtual GUI::Image get_image() const;

        virtual GUI::PlatformWidget get_platform_widget();

        virtual void click();

    protected:
        WidgetBase(
            char const* const text,
            int const left,
            int const top,
            int const width,
            int const height,
            Type const type
        );

        WidgetBase(
            GUI::PlatformData platform_data,
            GUI::PlatformWidget platform_widget,
            Type const type
        );

        virtual void destroy_children();

        virtual void set_up(
            GUI::PlatformData platform_data,
            WidgetBase* const parent
        );

        virtual void set_gui(GUI& gui);

        /**
         * \brief Event handler for painting the widget on the screen.
         *
         * \return              Return \c true if the event was handled,
         *                      \c false if the platform's default event handler
         *                      needs to be run.
         */
        virtual bool paint();

        /**
         * \brief Handle the double click event.
         *
         * \return              Return \c true if the event was handled,
         *                      \c false if the platform's default event handler
         *                      needs to be run.
         */
        virtual bool double_click();

        /**
         * \brief Event handler to run when the (left) mouse button is
         *        pressed down.
         *
         * \param   x           Horizontal coordinate of the cursor relative to
         *                      the top left corner of the widget.
         *
         * \param   y           Horizontal coordinate of the cursor relative to
         *                      the top left corner of the widget.
         *
         * \return              Return \c true if the event was handled,
         *                      \c false if the platform's default event handler
         *                      needs to be run.
         */
        virtual bool mouse_down(int const x, int const y);

        /**
         * \brief Event handler to run when the (left) mouse button is released.
         *
         * \param   x           Horizontal coordinate of the cursor relative to
         *                      the top left corner of the widget.
         *
         * \param   y           Horizontal coordinate of the cursor relative to
         *                      the top left corner of the widget.
         *
         * \return              Return \c true if the event was handled,
         *                      \c false if the platform's default event handler
         *                      needs to be run.
         */
        virtual bool mouse_up(int const x, int const y);

        /**
         * \brief Event handler to run when the mouse cursor moves over the widget.
         *
         * \param   x           Horizontal coordinate of the cursor relative to
         *                      the top left corner of the widget.
         *
         * \param   y           Horizontal coordinate of the cursor relative to
         *                      the top left corner of the widget.
         *
         * \param   modifier    Tells if the modifier key (e.g. "Control" on PC)
         *                      is held when the event occurred.
         *
         * \return              Return \c true if the event was handled,
         *                      \c false if the platform's default event handler
         *                      needs to be run.
         */
        virtual bool mouse_move(int const x, int const y, bool const modifier);

        /**
         * \brief Event handler to run when the mouse cursor leaves the widget's area.
         *
         * \param   x           Horizontal coordinate of the cursor relative to
         *                      the top left corner of the widget.
         *
         * \param   y           Horizontal coordinate of the cursor relative to
         *                      the top left corner of the widget.
         *
         * \return              Return \c true if the event was handled,
         *                      \c false if the platform's default event handler
         *                      needs to be run.
         */
        virtual bool mouse_leave(int const x, int const y);

        /**
         * \brief Event handler to run when the mouse wheel is scrolled while
         *        the cursor is over the widget.
         *
         * \param   delta       A floating point number between roughly -1.0 and
         *                      1.0, indicating the direction and relative speed
         *                      of the scrolling.
         *
         * \param   modifier    Tells if the modifier key (e.g. "Control" on PC)
         *                      is held when the event occurred.
         *
         * \return              Return \c true if the event was handled,
         *                      \c false if the platform's default event handler
         *                      needs to be run.
         */
        virtual bool mouse_wheel(double const delta, bool const modifier);

        virtual void fill_rectangle(
            int const left,
            int const top,
            int const width,
            int const height,
            GUI::Color const color
        );

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
        );

        virtual void draw_image(
            GUI::Image image,
            int const left,
            int const top,
            int const width,
            int const height
        );

        Type const type;

        GUI::Widgets children;
        GUI::PlatformWidget platform_widget;
        GUI::PlatformData platform_data;
        GUI::Image image;
        GUI* gui;
        WidgetBase* parent;
        char const* text;

        int left;
        int top;
        int width;
        int height;

        bool is_clicking;
};

}

#endif
