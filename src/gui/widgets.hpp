/*
 * This file is part of MPE Emulator.
 * Copyright (C) 2023, 2024, 2025, 2026  Attila M. Magyar
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

#ifndef MPE_EMULATOR__GUI__WIDGETS_HPP
#define MPE_EMULATOR__GUI__WIDGETS_HPP

#include <cstddef>
#include <string>
#include <vector>

#include "common.hpp"
#include "proxy.hpp"
#include "serializer.hpp"
#include "strings.hpp"

#include "gui/gui.hpp"


namespace MpeEmulator
{

class ExternallyCreatedWindow : public Widget
{
    public:
        ExternallyCreatedWindow(
            GUI::PlatformData platform_data,
            GUI::PlatformWidget window
        );
        virtual ~ExternallyCreatedWindow();
};


class TransparentWidget : public Widget
{
    public:
        TransparentWidget(
            char const* const text,
            int const left,
            int const top,
            int const width,
            int const height,
            Type const type
        );

    protected:
        virtual bool paint() override;
};


class ImportSettingsButton : public TransparentWidget
{
    public:
        ImportSettingsButton(
            GUI& gui,
            int const left,
            int const top,
            int const width,
            int const height,
            Proxy& proxy,
            TabBody* const tab_body
        );

        void import_settings(char const* const buffer, int const size) const;

    protected:
        virtual void click() override;
        virtual bool mouse_move(int const x, int const y, bool const modifier) override;
        virtual bool mouse_leave(int const x, int const y) override;

    private:
        TabBody* const tab_body;

        Proxy& proxy;
};


class ExportSettingsButton : public TransparentWidget
{
    public:
        ExportSettingsButton(
            GUI& gui,
            int const left,
            int const top,
            int const width,
            int const height,
            Proxy& proxy
        );

    protected:
        virtual void click() override;
        virtual bool mouse_move(int const x, int const y, bool const modifier) override;
        virtual bool mouse_leave(int const x, int const y) override;

    private:
        Proxy& proxy;
};


class TabBody : public TransparentWidget
{
    public:
        static constexpr int LEFT = 0;
        static constexpr int TOP = 60;
        static constexpr int WIDTH = GUI::WIDTH;
        static constexpr int HEIGHT = GUI::HEIGHT - TOP;

        explicit TabBody(GUI& gui, char const* const text);

        using TransparentWidget::own;

        KnobParamEditor* own(KnobParamEditor* const knob_param_editor);
        ToggleSwitchParamEditor* own(ToggleSwitchParamEditor* const toggle_switch_param_editor);
        DiscreteParamEditor* own(DiscreteParamEditor* const discrete_param_editor);

        void stop_editing();

        void refresh_all_params();

        void set_tab_selector(TabSelector& tab_selector);

        virtual void hide() override;

        void hide_param_editors();
        void show_param_editors();

    private:
        GUI::KnobParamEditors knob_param_editors;
        GUI::ToggleSwitchParamEditors toggle_switch_param_editors;
        GUI::DiscreteParamEditors discrete_param_editors;

        TabSelector* tab_selector;
};


class Background : public Widget
{
    public:
        Background();
        ~Background();

        void replace_body(TabBody* const new_body);
        TabBody* get_body() const;
        void hide_body();
        void show_body();
        void refresh();

    private:
        static constexpr int FULL_REFRESH_TICKS = 3;

        TabBody* body;
        int next_full_refresh;
};


class TabSelector : public TransparentWidget
{
    public:
        static constexpr int LEFT = 6;
        static constexpr int TOP = 4;
        static constexpr int WIDTH = 144;
        static constexpr int HEIGHT = 56;

        TabSelector(
            Background* const background,
            char const* const tab_image_name,
            TabBody* const tab_body,
            char const* const text,
            int const left
        );

        virtual ~TabSelector();

        virtual void set_scale(double const new_scale) override;

        void tab_body_hidden();

    protected:
        virtual void click() override;

    private:
        void destroy_tab_image();
        void ensure_scaled_tab_image();

        char const* const tab_image_name;

        Background* background;
        TabBody* tab_body;

        GUI::Image tab_image_full_size;
        GUI::Image tab_image_scaled;

        bool needs_tab_image_update;
};


class OptionSelector : public Widget
{
    public:
        static constexpr int LEFT = 0;
        static constexpr int TOP = 0;
        static constexpr int WIDTH = GUI::WIDTH;
        static constexpr int HEIGHT = GUI::HEIGHT;
        static constexpr int TITLE_HEIGHT = 60;
        static constexpr int FONT_SIZE = 24;
        static constexpr int PADDING = 20;

        OptionSelector(
            Background& background,
            Proxy& proxy,
            char const* const title
        );

        /**
         * @warning Has no effect if called before passing the \c OptionSelector
         *          to its parent's \c WidgetBase::own() method.
         */
        void add_option(unsigned int value, char const* const name);

        void select_option(
            Proxy::ParamId const param_id,
            DiscreteParamEditor* const param_editor
        );

        virtual void hide() override;

        void handle_selection_change(unsigned int new_value);

    protected:
        virtual bool paint() override;

    private:
        static constexpr int TITLE_SIZE = 128;

        static constexpr size_t OPTIONS_SIZE = 128;

        class Option : public Widget
        {
            public:
                static constexpr int HEIGHT = 36;
                static constexpr int WIDTH = 484;
                static constexpr int PADDING = 6;

                Option(
                    OptionSelector& option_selector,
                    size_t const index,
                    char const* const text,
                    int const left,
                    int const top,
                    unsigned int value
                );

                void select();
                void unselect();

                size_t const index;

            protected:
                virtual bool paint() override;
                virtual bool mouse_up(int const x, int const y) override;
                virtual bool mouse_move(int const x, int const y, bool const modifier) override;
                virtual bool mouse_leave(int const x, int const y) override;

            private:
                unsigned int const value;

                OptionSelector& option_selector;
                bool is_selected;
                bool is_mouse_over;
        };

        char title[TITLE_SIZE];
        Background& background;
        Proxy& proxy;
        DiscreteParamEditor* param_editor;
        Option* options[OPTIONS_SIZE];
        Option* options_by_value[OPTIONS_SIZE];
        size_t options_count;
        size_t selected_option_index;
        int next_option_left;
        int next_option_top;
        Proxy::ParamId param_id;
};


class ParamStateImages
{
    public:
        ParamStateImages(
            WidgetBase* const widget,
            GUI::Image image,
            size_t const count,
            int const width,
            int const height
        );

        ~ParamStateImages();

        ParamStateImages(ParamStateImages const& param_state_images) = delete;
        ParamStateImages(ParamStateImages&& param_state_images) = delete;
        ParamStateImages& operator=(ParamStateImages const& param_state_images) = delete;
        ParamStateImages& operator=(ParamStateImages&& param_state_images) = delete;

        size_t ratio_to_index(double const ratio) const;

        void set_scale(double const new_scale);

        size_t const count;
        int const width;
        int const height;

        WidgetBase* widget;

        GUI::Image image_full;

        GUI::Image* images;

    private:
        void split_image(
            GUI::Image image,
            int const scaled_width,
            int const scaled_height,
            GUI::Image* images
        ) const;

        GUI::Image* free_images(GUI::Image* const images) const;

        size_t const last_index;
        double const last_index_float;
};


class KnobParamEditor : public TransparentWidget
{
    public:
        enum KnobType {
            DISCRETE = 0,
            CONTINUOUS = 1,
        };

        KnobParamEditor(
            GUI& gui,
            int const left,
            int const top,
            int const width,
            int const height,
            int const knob_top,
            Proxy& proxy,
            Proxy::ParamId const param_id,
            ParamStateImages const* const knob_states,
            KnobType const type
        );

        void adjust_ratio(double const ratio);
        void handle_ratio_change(double const new_ratio);

        void refresh();

        void update_editor(double const new_ratio);
        void update_editor();

        void reset_default();

        void stop_editing();

        Proxy::ParamId const param_id;
        bool const is_continuous;

    protected:
        virtual void set_up(
            GUI::PlatformData platform_data,
            WidgetBase* const parent
        ) override;

        virtual bool paint() override;
        virtual bool mouse_move(int const x, int const y, bool const modifier) override;
        virtual bool mouse_leave(int const x, int const y) override;

    private:
        static constexpr size_t TEXT_MAX_LENGTH = 16;
        static constexpr size_t TITLE_MAX_LENGTH = 64;

        static constexpr int VALUE_TEXT_HEIGHT = 40;

        class Knob : public Widget
        {
            public:
                static constexpr double MOUSE_WHEEL_COARSE_SCALE = 1.0 / 200.0;

                static constexpr double MOUSE_WHEEL_FINE_SCALE = (
                    MOUSE_WHEEL_COARSE_SCALE / 50.0
                );

                static constexpr double MOUSE_MOVE_COARSE_SCALE = 1.0 / 500.0;

                static constexpr double MOUSE_MOVE_FINE_SCALE = (
                    MOUSE_MOVE_COARSE_SCALE / 100.0
                );

                Knob(
                    KnobParamEditor& editor,
                    GUI& gui,
                    char const* const text,
                    int const left,
                    int const top,
                    double const steps,
                    ParamStateImages const* const knob_states
                );

                virtual ~Knob();

                virtual void set_scale(double const new_scale) override;

                void update(double const ratio);
                void update();

                bool is_editing() const;
                void start_editing();
                void stop_editing();

            protected:
                virtual void set_up(
                    GUI::PlatformData platform_data,
                    WidgetBase* const parent
                ) override;

                virtual bool double_click() override;
                virtual bool mouse_down(int const x, int const y) override;
                virtual bool mouse_up(int const x, int const y) override;
                virtual bool mouse_move(int const x, int const y, bool const modifier) override;
                virtual bool mouse_leave(int const x, int const y) override;
                virtual bool mouse_wheel(double const delta, bool const modifier) override;

            private:
                double const steps;

                ParamStateImages const* const knob_states;

                KnobParamEditor& editor;
                GUI::Image knob_state;
                double prev_x;
                double prev_y;
                double ratio;
                double mouse_move_delta;
                bool is_editing_;
                bool is_switch;
        };

        void update_value_str();

        double const discrete_step_size;

        ParamStateImages const* const knob_states;

        int const value_font_size;

        int const knob_top;
        bool const has_room_for_text;

        Proxy& proxy;
        double ratio;
        Knob* knob;
        char value_str[TEXT_MAX_LENGTH];
        char title[TITLE_MAX_LENGTH];
};


class AboutText : public Widget
{
    public:
        static constexpr int LEFT = 20;
        static constexpr int TOP = 20;
        static constexpr int WIDTH = 1920;
        static constexpr int HEIGHT = 1086;

        static constexpr int LOGO_WIDTH = 474;
        static constexpr int LOGO_HEIGHT = 443;

        static constexpr int FONT_SIZE = 28;
        static constexpr int TEXT_TOP = 20;
        static constexpr int LINE_HEIGHT = 50;
        static constexpr int EMPTY_LINE_HEIGHT = 24;
        static constexpr int PADDING = 20;

        static constexpr char const* NAME = "MPE Emulator";

        static constexpr char const* VERSION = (
            MPE_EMULATOR_TO_STRING(MPE_EMULATOR_VERSION_STR) ", "
            MPE_EMULATOR_TO_STRING(MPE_EMULATOR_TARGET_PLATFORM) ", "
            MPE_EMULATOR_TO_STRING(MPE_EMULATOR_INSTRUCTION_SET)
        );

        static constexpr char const* TEXT = (
            "\n"
            "MIDI Polyphonic Expression (MPE)\n"
            "for non-MPE-capable keyboards.\n"
            "\n"
            "Copyright (C) 2025 Attila M. Magyar and contributors\n"
            "https://attilammagyar.github.io/mpe-emulator\n"
            "\n"
            "License: GNU General Public License Version 3\n"
            "https://www.gnu.org/licenses/gpl-3.0.en.html\n"
            "\n"
            "\n"
            "Usage\n"
            "\n"
            "Move the cursor over a knob, and use the mouse wheel\n"
            "for adjusting its value, or start dragging it.\n"
            "\n"
            "Hold down the \"Control\" key while adjusting a knob\n"
            "for fine grained adjustments.\n"
            "\n"
            "Double click on a knob to reset it to its default value.\n"
        );

        AboutText(char const* const sdk_version, GUI::Image logo);

        virtual ~AboutText();

        virtual void set_scale(double const new_scale) override;

    protected:
        virtual bool paint() override;

    private:
        GUI::Image logo;
        GUI::Image logo_scaled;
        std::vector<std::string> lines;
        bool needs_logo_rescale;
};


class StatusLine : public TransparentWidget
{
    public:
        static constexpr int WIDTH = 650;
        static constexpr int HEIGHT = 48;
        static constexpr int LEFT = GUI::WIDTH - WIDTH - 2;
        static constexpr int TOP = 0;

        StatusLine();

        virtual void set_text(char const* const text) override;

    protected:
        virtual bool paint() override;
};


class ToggleSwitchParamEditor: public TransparentWidget
{
    public:
        ToggleSwitchParamEditor(
            GUI& gui,
            int const left,
            int const top,
            int const width,
            int const height,
            int const box_left,
            Proxy& proxy,
            Proxy::ParamId const param_id
        );

        void refresh();

        bool is_on() const;

        Proxy::ParamId const param_id;

    protected:
        virtual void set_up(
            GUI::PlatformData platform_data,
            WidgetBase* const parent
        ) override;

        virtual bool paint() override;
        virtual bool mouse_up(int const x, int const y) override;
        virtual bool mouse_move(int const x, int const y, bool const modifier) override;
        virtual bool mouse_leave(int const x, int const y) override;

    private:
        static constexpr size_t TEXT_MAX_LENGTH = 16;
        static constexpr size_t TITLE_MAX_LENGTH = 64;

        void update_title();
        bool is_editing() const;
        void start_editing();
        void stop_editing();

        int const box_left;

        Proxy& proxy;

        char value_str[TEXT_MAX_LENGTH];
        char title[TITLE_MAX_LENGTH];
        double default_ratio;
        double ratio;
        bool is_editing_;
};


class DiscreteParamEditor : public TransparentWidget
{
    public:
        DiscreteParamEditor(
            GUI& gui,
            int const left,
            int const top,
            int const width,
            int const height,
            int const value_left,
            int const value_width,
            Proxy& proxy,
            Proxy::ParamId const param_id,
            ParamStateImages const* const state_images = NULL
        );

        DiscreteParamEditor(
            GUI& gui,
            int const left,
            int const top,
            int const width,
            int const height,
            int const value_left,
            int const value_width,
            Proxy& proxy,
            Proxy::ParamId const param_id,
            OptionSelector* const option_selector
        );

        virtual void set_scale(double const new_scale) override;

        virtual void refresh();

        void set_ratio(double const new_ratio);

        Proxy::ParamId const param_id;

    protected:
        static constexpr size_t TEXT_MAX_LENGTH = 24;
        static constexpr size_t TITLE_MAX_LENGTH = 64;

        virtual void set_up(
            GUI::PlatformData platform_data,
            WidgetBase* const parent
        ) override;

        virtual bool paint() override;
        virtual bool mouse_up(int const x, int const y) override;
        virtual bool mouse_move(int const x, int const y, bool const modifier) override;
        virtual bool mouse_leave(int const x, int const y) override;
        virtual bool mouse_wheel(double const delta, bool const modifier) override;

        virtual void update();
        void update_value_str(unsigned char const value);

        void update_title();

        bool is_editing() const;

        Proxy& proxy;
        char value_str[TEXT_MAX_LENGTH];
        char title[TITLE_MAX_LENGTH];
        double ratio;

    private:
        void start_editing();
        void stop_editing();

        double const step_size;

        ParamStateImages const* const state_images;
        OptionSelector* const option_selector;

        int const value_left;
        int const value_width;

        bool is_editing_;
        bool is_selecting;
};


class ResizerHandle : public TransparentWidget
{
    public:
        static constexpr int RESIZE_MOVE_INTERVAL_MS = 150;
        static constexpr int WIDTH = 64;
        static constexpr int HEIGHT = 64;
        static constexpr int LEFT = TabBody::WIDTH - WIDTH;
        static constexpr int TOP = TabBody::HEIGHT - HEIGHT;

        ResizerHandle(GUI& gui, GUI::EventHandler& event_handler);

    protected:
        virtual bool mouse_down(int const x, int const y) override;
        virtual bool mouse_up(int const x, int const y) override;
        virtual bool mouse_move(int const x, int const y, bool const modifier) override;

    private:
        void init_movement(int const x, int const y);
        bool handle_movement(int const x, int const y);

        GUI::EventHandler& event_handler;
        uint64_t prev_resize_time_ms;
        int click_x;
        int click_y;
};

}

#endif
