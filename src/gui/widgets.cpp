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

#ifndef MPE_EMULATOR__GUI__WIDGETS_CPP
#define MPE_EMULATOR__GUI__WIDGETS_CPP

#include <algorithm>
#include <cmath>
#include <cstring>

#include "gui/widgets.hpp"


namespace MpeEmulator
{

ExternallyCreatedWindow::ExternallyCreatedWindow(
        GUI::PlatformData platform_data,
        GUI::PlatformWidget window
) : Widget(platform_data, window, Type::EXTERNALLY_CREATED_WINDOW)
{
}


ExternallyCreatedWindow::~ExternallyCreatedWindow()
{
    platform_widget = NULL;
}


TransparentWidget::TransparentWidget(
        char const* const text,
        int const left,
        int const top,
        int const width,
        int const height,
        Type const type
) : Widget(text, left, top, width, height, type)
{
}


bool TransparentWidget::paint()
{
    Widget::paint();

    return true;
}


ImportSettingsButton::ImportSettingsButton(
        GUI& gui,
        int const left,
        int const top,
        int const width,
        int const height,
        Proxy& proxy,
        TabBody* const tab_body
) : TransparentWidget("Import Settings", left, top, width, height, Type::IMPORT_SETTINGS_BUTTON),
    tab_body(tab_body),
    proxy(proxy)
{
    set_gui(gui);
}


void ImportSettingsButton::import_settings(char const* buffer, int const size) const
{
    std::string const settings(
        buffer,
        std::min(
            (std::string::size_type)size,
            (std::string::size_type)Serializer::MAX_SIZE
        )
    );

    Serializer::import_settings_in_gui_thread(proxy, settings);

    tab_body->stop_editing();
    tab_body->refresh_all_params();
}


bool ImportSettingsButton::mouse_move(int const x, int const y, bool const modifier)
{
    TransparentWidget::mouse_move(x, y, modifier);
    gui->set_status_line(text);

    return true;
}


bool ImportSettingsButton::mouse_leave(int const x, int const y)
{
    TransparentWidget::mouse_leave(x, y);
    gui->set_status_line("");

    return true;
}


ExportSettingsButton::ExportSettingsButton(
        GUI& gui,
        int const left,
        int const top,
        int const width,
        int const height,
        Proxy& proxy
) : TransparentWidget("Export Settings", left, top, width, height, Type::EXPORT_SETTINGS_BUTTON),
    proxy(proxy)
{
    set_gui(gui);
}


bool ExportSettingsButton::mouse_move(int const x, int const y, bool const modifier)
{
    TransparentWidget::mouse_move(x, y, modifier);
    gui->set_status_line(text);

    return true;
}


bool ExportSettingsButton::mouse_leave(int const x, int const y)
{
    TransparentWidget::mouse_leave(x, y);
    gui->set_status_line("");

    return true;
}


TabBody::TabBody(GUI& gui, char const* const text)
    : TransparentWidget(text, LEFT, TOP, WIDTH, HEIGHT, Type::TAB_BODY)
{
    set_gui(gui);
}


KnobParamEditor* TabBody::own(KnobParamEditor* knob_param_editor)
{
    Widget::own(knob_param_editor);

    knob_param_editors.push_back(knob_param_editor);

    return knob_param_editor;
}


ToggleSwitchParamEditor* TabBody::own(ToggleSwitchParamEditor* toggle_switch_param_editor)
{
    Widget::own(toggle_switch_param_editor);

    toggle_switch_param_editors.push_back(toggle_switch_param_editor);

    return toggle_switch_param_editor;
}


DiscreteParamEditor* TabBody::own(DiscreteParamEditor* discrete_param_editor)
{
    Widget::own(discrete_param_editor);

    discrete_param_editors.push_back(discrete_param_editor);

    return discrete_param_editor;
}


void TabBody::stop_editing()
{
    for (GUI::KnobParamEditors::iterator it = knob_param_editors.begin(); it != knob_param_editors.end(); ++it) {
        (*it)->stop_editing();
    }
}


void TabBody::refresh_all_params()
{
    for (GUI::KnobParamEditors::iterator it = knob_param_editors.begin(); it != knob_param_editors.end(); ++it) {
        (*it)->refresh();
    }

    for (GUI::ToggleSwitchParamEditors::iterator it = toggle_switch_param_editors.begin(); it != toggle_switch_param_editors.end(); ++it) {
        (*it)->refresh();
    }

    for (GUI::DiscreteParamEditors::iterator it = discrete_param_editors.begin(); it != discrete_param_editors.end(); ++it) {
        (*it)->refresh();
    }

    gui->update_active_voices_count();
}


Background::Background()
    : Widget("MPE Emulator", 0, 0, GUI::WIDTH, GUI::HEIGHT, Type::BACKGROUND),
    body(NULL),
    next_full_refresh(FULL_REFRESH_TICKS)
{
}


Background::~Background()
{
}


void Background::replace_body(TabBody* new_body)
{
    if (body != NULL) {
        body->hide();
    }

    body = new_body;
    body->show();
}


void Background::hide_body()
{
    if (body != NULL) {
        body->hide();
    }
}


void Background::show_body()
{
    if (body != NULL) {
        body->show();
    }
}


void Background::refresh()
{
    if (body == NULL) {
        return;
    }

    --next_full_refresh;

    if (next_full_refresh == 0) {
        next_full_refresh = FULL_REFRESH_TICKS;
        body->refresh_all_params();
    }
}


TabSelector::TabSelector(
        Background* background,
        GUI::Image tab_image,
        TabBody* tab_body,
        char const* const text,
        int const left
) : TransparentWidget(text, left, TOP, WIDTH, HEIGHT, Type::TAB_SELECTOR),
    background(background),
    tab_body(tab_body),
    tab_image(tab_image)
{
}


void TabSelector::click()
{
    TransparentWidget::click();

    background->set_image(tab_image);
    background->replace_body(tab_body);
}


OptionSelector::OptionSelector(
        Background& background,
        Proxy& proxy,
        char const* const title
) : Widget(title, LEFT, TOP, WIDTH, HEIGHT, Type::OPTION_SELECTOR),
    background(background),
    proxy(proxy),
    param_editor(NULL),
    options_count(0),
    selected_option_index(OPTIONS_SIZE),
    next_option_left(5),
    next_option_top(TITLE_HEIGHT),
    param_id(Proxy::ParamId::INVALID_PARAM_ID)
{
    std::fill_n(options, OPTIONS_SIZE, (Option*)NULL);
    std::fill_n(options_by_value, OPTIONS_SIZE, (Option*)NULL);
}


void OptionSelector::add_option(unsigned int value, char const* const name)
{
    constexpr int max_top = HEIGHT - Option::HEIGHT;

    if (
            MPE_EMULATOR_UNLIKELY(
                this->parent == NULL
                || options_count == OPTIONS_SIZE
                || value >= (unsigned int)OPTIONS_SIZE
            )
    ) {
        return;
    }

    Option* option = new Option(
        *this, options_count, name, next_option_left, next_option_top, value
    );

    this->own(option);

    options[options_count] = option;
    options_by_value[value] = option;

    ++options_count;

    next_option_top += Option::HEIGHT;

    if (next_option_top >= max_top) {
        next_option_top = TITLE_HEIGHT;
        next_option_left += Option::WIDTH;
    }
}


void OptionSelector::select_option(
        Proxy::ParamId const param_id,
        DiscreteParamEditor* param_editor
) {
    double const ratio = proxy.get_param_ratio_atomic(param_id);
    unsigned int value = proxy.param_ratio_to_value(param_id, ratio);

    if (MPE_EMULATOR_UNLIKELY(value >= (unsigned int)OPTIONS_SIZE)) {
        return;
    }

    Option* selected_option = options_by_value[value];

    if (MPE_EMULATOR_UNLIKELY(selected_option == NULL)) {
        return;
    }

    if (selected_option_index < OPTIONS_SIZE) {
        Option* previously_selected_option = options[selected_option_index];

        if (previously_selected_option != NULL) {
            previously_selected_option->unselect();
        }
    }

    selected_option->select();

    snprintf(
        title, TITLE_SIZE, "Select value for \"%s\"", Strings::PARAMS[param_id]
    );

    this->param_id = param_id;
    this->param_editor = param_editor;
    this->selected_option_index = selected_option->index;

    redraw();
    Widget::show();
    background.hide_body();
    bring_to_top();
}


void OptionSelector::hide()
{
    background.show_body();
    Widget::hide();
}


void OptionSelector::handle_selection_change(unsigned int const new_value)
{
    hide();

    if (
            MPE_EMULATOR_UNLIKELY(
                param_editor == NULL
                || param_id >= Proxy::Proxy::ParamId::INVALID_PARAM_ID
            )
    ) {
        return;
    }

    param_editor->set_ratio(proxy.param_value_to_ratio(param_id, new_value));
}


bool OptionSelector::paint()
{
    Widget::paint();

    fill_rectangle(0, 0, width, height, GUI::TEXT_BACKGROUND);
    draw_text(
        title,
        12,
        0,
        0,
        WIDTH,
        TITLE_HEIGHT,
        GUI::TEXT_COLOR,
        GUI::TEXT_BACKGROUND,
        FontWeight::BOLD,
        10,
        TextAlignment::LEFT
    );

    return true;
}


OptionSelector::Option::Option(
        OptionSelector& option_selector,
        size_t const index,
        char const* const text,
        int const left,
        int const top,
        unsigned int value
) : Widget(text, left, top, WIDTH, HEIGHT, Type::OPTION),
    index(index),
    value(value),
    option_selector(option_selector),
    is_selected(false),
    is_mouse_over(false)
{
}


void OptionSelector::Option::select()
{
    is_selected = true;
    redraw();
}


void OptionSelector::Option::unselect()
{
    is_selected = false;
    redraw();
}


bool OptionSelector::Option::paint()
{
    Widget::paint();

    GUI::Color background;
    GUI::Color color;

    if (is_mouse_over) {
        background = GUI::TEXT_HIGHLIGHT_BACKGROUND;
        color = GUI::TEXT_HIGHLIGHT_COLOR;
    } else if (is_selected) {
        background = GUI::TEXT_COLOR;
        color = GUI::TEXT_BACKGROUND;
    } else {
        background = GUI::TEXT_BACKGROUND;
        color = GUI::TEXT_COLOR;
    }

    draw_text(
        text,
        12,
        0,
        0,
        width,
        height,
        color,
        background,
        FontWeight::BOLD,
        3,
        TextAlignment::LEFT
    );

    return true;
}


bool OptionSelector::Option::mouse_up(int const x, int const y)
{
    Widget::mouse_up(x, y);

    option_selector.handle_selection_change(value);

    return true;
}


bool OptionSelector::Option::mouse_move(
        int const x,
        int const y,
        bool const modifier
) {
    Widget::mouse_move(x, y, modifier);

    if (!is_mouse_over) {
        is_mouse_over = true;
        redraw();
    }

    return true;
}


bool OptionSelector::Option::mouse_leave(int const x, int const y)
{
    Widget::mouse_leave(x, y);

    if (is_mouse_over) {
        is_mouse_over = false;
        redraw();
    }

    return 0;
}


ParamStateImages::ParamStateImages(
        WidgetBase* widget,
        GUI::Image image,
        size_t const count,
        int const width,
        int const height
) : count(count),
    width(width),
    height(height),
    widget(widget),
    image(image),
    last_index(count - 1),
    last_index_float((double)last_index)
{
    images = split_image(image);
}


GUI::Image* ParamStateImages::split_image(GUI::Image image) const
{
    if (image == NULL) {
        return NULL;
    }

    GUI::Image* images = new GUI::Image[count];

    for (size_t i = 0; i != count; ++i) {
        int const top = (int)i * height;

        images[i] = widget->copy_image_region(image, 0, top, width, height);
    }

    return images;
}


ParamStateImages::~ParamStateImages()
{
    if (image != NULL) {
        images = free_images(images);

        widget->delete_image(image);
        image = NULL;
    }
}


GUI::Image* ParamStateImages::free_images(GUI::Image* images) const
{
    if (images == NULL) {
        return NULL;
    }

    for (size_t i = 0; i != count; ++i) {
        widget->delete_image(images[i]);
        images[i] = NULL;
    }

    delete[] images;

    return NULL;
}


size_t ParamStateImages::ratio_to_index(double const ratio) const
{
    size_t const index = (size_t)std::round(last_index_float * ratio);

    if (index == 0 && last_index != 0 && ratio >= 0.000001) {
        /*
        Reserve the first image for 0.
        (E.g. screw_states images utilize this behaviour.)
        */
        return 1;
    }

    return index <= last_index ? index : last_index;
}


KnobParamEditor::KnobParamEditor(
        GUI& gui,
        int const left,
        int const top,
        int const width,
        int const height,
        int const knob_top,
        Proxy& proxy,
        Proxy::ParamId const param_id,
        ParamStateImages const* knob_states,
        KnobType const type
) : TransparentWidget(
        Strings::PARAMS[param_id],
        left,
        top,
        width,
        height,
        Type::KNOB_PARAM_EDITOR
    ),
    param_id(param_id),
    is_continuous(type == KnobType::CONTINUOUS),
    discrete_step_size(
        is_continuous ? 0.0 : 1.001 / (double)proxy.get_param_max_value(param_id)
    ),
    knob_states(knob_states),
    value_font_size(is_continuous ? 11 : 10),
    knob_top(knob_top),
    has_room_for_text(height >= knob_top + knob_states->height + VALUE_TEXT_HEIGHT),
    proxy(proxy),
    ratio(0.0),
    knob(NULL)
{
    set_gui(gui);
}


void KnobParamEditor::set_up(GUI::PlatformData platform_data, WidgetBase* parent)
{
    TransparentWidget::set_up(platform_data, parent);

    knob = new Knob(
        *this,
        *gui,
        text,
        (width - knob_states->width) / 2,
        knob_top,
        is_continuous ? 0.0 : (double)proxy.get_param_max_value(param_id),
        knob_states
    );

    own(knob);
    update_editor(proxy.get_param_ratio_atomic(param_id));
}


void KnobParamEditor::refresh()
{
    if (knob->is_editing()) {
        return;
    }

    double const new_ratio = proxy.get_param_ratio_atomic(param_id);

    if (new_ratio != ratio) {
        update_editor(new_ratio);
    } else {
        proxy.push_message(Proxy::MessageType::REFRESH_PARAM, param_id, 0.0);
    }
}


void KnobParamEditor::update_editor(double const new_ratio)
{
    ratio = GUI::clamp_ratio(new_ratio);
    update_editor();
}


void KnobParamEditor::update_editor()
{
    update_value_str();
    redraw();
    knob->update(ratio);
}


void KnobParamEditor::adjust_ratio(double const delta)
{
    if (is_continuous) {
        handle_ratio_change(ratio + delta);
    } else {
        handle_ratio_change(
            ratio + (delta < 0 ? - discrete_step_size : discrete_step_size)
        );
    }
}


void KnobParamEditor::handle_ratio_change(double const new_ratio)
{
    double const ratio = GUI::clamp_ratio(new_ratio);

    proxy.push_message(Proxy::MessageType::SET_PARAM, param_id, ratio);
    update_editor(ratio);
}


void KnobParamEditor::update_value_str()
{
    Strings::param_ratio_to_str(
        proxy,
        param_id,
        ratio,
        value_str,
        TEXT_MAX_LENGTH
    );

    snprintf(
        title, TITLE_MAX_LENGTH, "%s: %s", Strings::PARAMS[param_id], value_str
    );
    title[TITLE_MAX_LENGTH - 1] = '\x00';

    knob->set_text(title);

    if (knob->is_editing()) {
        gui->set_status_line(title);
        gui->redraw_status_line();
    }
}


void KnobParamEditor::reset_default()
{
    handle_ratio_change(proxy.get_param_default_ratio(param_id));
}


void KnobParamEditor::stop_editing()
{
    knob->stop_editing();
}


bool KnobParamEditor::paint()
{
    TransparentWidget::paint();

    if (has_room_for_text) {
        draw_text(
            value_str,
            value_font_size,
            1,
            height - VALUE_TEXT_HEIGHT,
            width - 2,
            VALUE_TEXT_HEIGHT,
            GUI::TEXT_COLOR,
            GUI::TEXT_BACKGROUND
        );
    }

    return true;
}


bool KnobParamEditor::mouse_move(int const x, int const y, bool const modifier)
{
    TransparentWidget::mouse_move(x, y, modifier);
    gui->set_status_line(text);

    return true;
}


bool KnobParamEditor::mouse_leave(int const x, int const y)
{
    TransparentWidget::mouse_leave(x, y);
    gui->set_status_line("");

    return true;
}


KnobParamEditor::Knob::Knob(
        KnobParamEditor& editor,
        GUI& gui,
        char const* const text,
        int const left,
        int const top,
        double const steps,
        ParamStateImages const* knob_states
) : Widget(text, left, top, knob_states->width, knob_states->height, Type::KNOB),
    steps(steps),
    knob_states(knob_states),
    editor(editor),
    knob_state(NULL),
    ratio(0.0),
    mouse_move_delta(0.0),
    is_editing_(false)
{
    set_gui(gui);
}


KnobParamEditor::Knob::~Knob()
{
}


void KnobParamEditor::Knob::set_up(
        GUI::PlatformData platform_data,
        WidgetBase* parent
) {
    Widget::set_up(platform_data, parent);

    update(0.0);
}


void KnobParamEditor::Knob::update(double const ratio)
{
    this->ratio = (
        steps > 0.0 ? std::round(ratio * steps) / steps : ratio
    );

    update();
}


void KnobParamEditor::Knob::update()
{
    size_t const index = knob_states->ratio_to_index(this->ratio);

    set_image(knob_states->images[index]);
}


bool KnobParamEditor::Knob::is_editing() const
{
    return is_editing_;
}


void KnobParamEditor::Knob::start_editing()
{
    is_editing_ = true;
}


void KnobParamEditor::Knob::stop_editing()
{
    is_editing_ = false;
}


bool KnobParamEditor::Knob::double_click()
{
    Widget::double_click();

    if (!is_clicking) {
        editor.reset_default();
    }

    return true;
}


bool KnobParamEditor::Knob::mouse_down(int const x, int const y)
{
    Widget::mouse_down(x, y);

    prev_x = (double)x;
    prev_y = (double)y;
    mouse_move_delta = 0.0;

    return true;
}


bool KnobParamEditor::Knob::mouse_up(int const x, int const y)
{
    Widget::mouse_up(x, y);

    focus();

    return true;
}


bool KnobParamEditor::Knob::mouse_move(
        int const x,
        int const y,
        bool const modifier
) {
    Widget::mouse_move(x, y, modifier);

    gui->set_status_line(text);

    start_editing();

    if (is_clicking) {
        double const scale = (
            modifier ? MOUSE_MOVE_FINE_SCALE : MOUSE_MOVE_COARSE_SCALE
        );
        double const float_x = (double)x;
        double const float_y = (double)y;
        double const dx = float_x - prev_x;
        double const dy = float_y - prev_y;
        double const delta = (
            scale * ((std::fabs(dx) > std::fabs(dy)) ? dx : -dy)
        );

        prev_x = float_x;
        prev_y = float_y;

        mouse_move_delta += delta;

        if (editor.is_continuous || std::fabs(mouse_move_delta) > 0.03) {
            editor.adjust_ratio(delta);
            mouse_move_delta = 0.0;
        }
    }

    focus();

    return is_clicking;
}


bool KnobParamEditor::Knob::mouse_leave(int const x, int const y)
{
    Widget::mouse_leave(x, y);
    stop_editing();
    gui->set_status_line("");

    return true;
}


bool KnobParamEditor::Knob::mouse_wheel(double const delta, bool const modifier)
{
    Widget::mouse_wheel(delta, modifier);

    double const scale = (
        modifier ? MOUSE_WHEEL_FINE_SCALE : MOUSE_WHEEL_COARSE_SCALE
    );

    if (steps > 0.0) {
        editor.adjust_ratio(delta * scale * 10.0);
    } else {
        editor.adjust_ratio(delta * scale);
    }

    return true;
}


AboutText::AboutText(char const* sdk_version, GUI::Image logo)
    : Widget(TEXT, LEFT, TOP, WIDTH, HEIGHT, Type::ABOUT_TEXT),
    logo(logo)
{
    std::string line("(Version: ");

    line += VERSION;

    if (sdk_version != NULL) {
        line += ", SDK: ";
        line += sdk_version;
    }

    line += ")";

    lines.push_back(NAME);
    lines.push_back(line);

    line = "";

    for (char const* c = TEXT; *c != '\x00'; ++c) {
        if (*c == '\n') {
            lines.push_back(line);
            line = "";
        } else {
            line += *c;
        }
    }

    lines.push_back(line);
}


bool AboutText::paint()
{
    Widget::paint();

    fill_rectangle(0, 0, width, height, GUI::TEXT_BACKGROUND);

    int const left = logo != NULL ? LOGO_WIDTH + 10 : 0;
    int const text_width = width - left;
    int top = TEXT_TOP;

    for (std::vector<std::string>::const_iterator it = lines.begin(); it != lines.end(); ++it) {
        draw_text(
            it->c_str(),
            FONT_SIZE,
            left,
            top,
            text_width,
            LINE_HEIGHT,
            GUI::TEXT_COLOR,
            GUI::TEXT_BACKGROUND,
            FontWeight::NORMAL,
            PADDING,
            TextAlignment::CENTER
        );

        top += it->length() == 0 ? EMPTY_LINE_HEIGHT : LINE_HEIGHT;
    }

    if (logo != NULL) {
        draw_image(logo, 5, (HEIGHT - LOGO_HEIGHT) / 2, LOGO_WIDTH, LOGO_HEIGHT);
    }

    return true;
}


StatusLine::StatusLine()
    : TransparentWidget("", LEFT, TOP, WIDTH, HEIGHT, Type::STATUS_LINE)
{
}


void StatusLine::set_text(char const* text)
{
    TransparentWidget::set_text(text);

    if (parent == NULL) {
        return;
    }

    if (text[0] == '\x00') {
        hide();
    } else {
        show();
    }
}


bool StatusLine::paint()
{
    TransparentWidget::paint();

    if (text[0] != '\x00') {
        fill_rectangle(0, 0, WIDTH, HEIGHT, GUI::STATUS_LINE_BACKGROUND);
        draw_text(
            text,
            9,
            0,
            3,
            WIDTH,
            20,
            GUI::TEXT_COLOR,
            GUI::STATUS_LINE_BACKGROUND,
            FontWeight::NORMAL,
            5,
            TextAlignment::RIGHT
        );
    }

    return true;
}


ToggleSwitchParamEditor::ToggleSwitchParamEditor(
        GUI& gui,
        int const left,
        int const top,
        int const width,
        int const height,
        int const box_left,
        Proxy& proxy,
        Proxy::ParamId const param_id
) : TransparentWidget(
        Strings::PARAMS[param_id],
        left,
        top,
        width,
        height,
        Type::TOGGLE_SWITCH
    ),
    param_id(param_id),
    box_left(box_left),
    proxy(proxy),
    is_editing_(false)
{
    set_gui(gui);
}


void ToggleSwitchParamEditor::set_up(GUI::PlatformData platform_data, WidgetBase* parent)
{
    TransparentWidget::set_up(platform_data, parent);

    default_ratio = proxy.get_param_default_ratio(param_id);
    ratio = default_ratio;
    update_title();
    refresh();
    redraw();
}


void ToggleSwitchParamEditor::refresh()
{
    if (is_editing()) {
        return;
    }

    double const new_ratio = proxy.get_param_ratio_atomic(param_id);

    if (new_ratio != ratio) {
        ratio = GUI::clamp_ratio(new_ratio);
        redraw();
    } else {
        proxy.push_message(Proxy::MessageType::REFRESH_PARAM, param_id, 0.0);
    }

    update_title();
}


bool ToggleSwitchParamEditor::paint()
{
    TransparentWidget::paint();

    unsigned int const toggle = proxy.param_ratio_to_value(param_id, ratio);
    GUI::Color const color = (
        toggle == Proxy::Toggle::ON ? GUI::TOGGLE_ON_COLOR : GUI::TOGGLE_OFF_COLOR
    );

    fill_rectangle(box_left + 5, 8, 11, 8, color);

    return true;
}


bool ToggleSwitchParamEditor::mouse_up(int const x, int const y)
{
    ratio = is_on() ? 0.0 : 1.0;
    proxy.push_message(Proxy::MessageType::SET_PARAM, param_id, ratio);
    update_title();
    redraw();

    return true;
}


bool ToggleSwitchParamEditor::is_on() const
{
    return ratio >= 0.5;
}


bool ToggleSwitchParamEditor::mouse_move(int const x, int const y, bool const modifier)
{
    TransparentWidget::mouse_move(x, y, modifier);

    gui->set_status_line(title);
    start_editing();

    return true;
}


bool ToggleSwitchParamEditor::mouse_leave(int const x, int const y)
{
    TransparentWidget::mouse_leave(x, y);

    gui->set_status_line("");
    stop_editing();

    return true;
}


void ToggleSwitchParamEditor::update_title()
{
    Strings::param_ratio_to_str(
        proxy,
        param_id,
        ratio,
        value_str,
        TEXT_MAX_LENGTH
    );
    snprintf(
        title, TITLE_MAX_LENGTH, "%s: %s", Strings::PARAMS[param_id], value_str
    );
    title[TITLE_MAX_LENGTH - 1] = '\x00';

    if (is_editing()) {
        gui->set_status_line(title);
        gui->redraw_status_line();
    }
}


bool ToggleSwitchParamEditor::is_editing() const
{
    return is_editing_;
}


void ToggleSwitchParamEditor::start_editing()
{
    is_editing_ = true;
}


void ToggleSwitchParamEditor::stop_editing()
{
    is_editing_ = false;
}


DiscreteParamEditor::DiscreteParamEditor(
        GUI& gui,
        int const left,
        int const top,
        int const width,
        int const height,
        int const value_left,
        int const value_width,
        Proxy& proxy,
        Proxy::ParamId const param_id,
        ParamStateImages const* state_images
) : TransparentWidget(
        Strings::PARAMS[param_id],
        left,
        top,
        width,
        height,
        Type::DISCRETE_PARAM_EDITOR
    ),
    param_id(param_id),
    proxy(proxy),
    ratio(0.0),
    step_size(1.001 / (double)proxy.get_param_max_value(param_id)),
    state_images(state_images),
    option_selector(NULL),
    value_left(value_left),
    value_width(value_width),
    is_editing_(false)
{
    set_gui(gui);
}


DiscreteParamEditor::DiscreteParamEditor(
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
) : TransparentWidget(
        Strings::PARAMS[param_id],
        left,
        top,
        width,
        height,
        Type::DISCRETE_PARAM_EDITOR
    ),
    param_id(param_id),
    proxy(proxy),
    ratio(0.0),
    step_size(1.001 / (double)proxy.get_param_max_value(param_id)),
    state_images(NULL),
    option_selector(option_selector),
    value_left(value_left),
    value_width(value_width),
    is_editing_(false),
    is_selecting(false)
{
    set_gui(gui);
}



void DiscreteParamEditor::set_up(
        GUI::PlatformData platform_data,
        WidgetBase* parent
) {
    Widget::set_up(platform_data, parent);

    update();
}


void DiscreteParamEditor::refresh()
{
    if (is_editing()) {
        return;
    }

    double const new_ratio = proxy.get_param_ratio_atomic(param_id);
    bool const is_changed = std::fabs(new_ratio - ratio) > 0.000001;

    if (is_changed) {
        ratio = GUI::clamp_ratio(new_ratio);
        update();
        redraw();
    } else {
        proxy.push_message(Proxy::MessageType::REFRESH_PARAM, param_id, 0.0);
    }
}


void DiscreteParamEditor::update()
{
    update_value_str(proxy.param_ratio_to_value(param_id, ratio));

    if (state_images != NULL) {
        set_image(state_images->images[state_images->ratio_to_index(ratio)]);
    }
}


void DiscreteParamEditor::update_value_str(unsigned char const value)
{
    Strings::param_ratio_to_str(
        proxy,
        param_id,
        ratio,
        value_str,
        TEXT_MAX_LENGTH
    );
    update_title();
}


void DiscreteParamEditor::update_title()
{
    snprintf(
        title, TITLE_MAX_LENGTH, "%s: %s", Strings::PARAMS[param_id], value_str
    );
    title[TITLE_MAX_LENGTH - 1] = '\x00';

    if (is_editing()) {
        gui->set_status_line(title);
        gui->redraw_status_line();
    }
}


bool DiscreteParamEditor::is_editing() const
{
    return is_editing_;
}


void DiscreteParamEditor::start_editing()
{
    is_editing_ = true;
}


void DiscreteParamEditor::stop_editing()
{
    is_editing_ = false;
}


bool DiscreteParamEditor::paint()
{
    TransparentWidget::paint();

    if (state_images == NULL) {
        draw_text(
            value_str, 10, value_left, 0, value_width, height, GUI::TEXT_COLOR, GUI::TEXT_BACKGROUND
        );
    }

    return true;
}


bool DiscreteParamEditor::mouse_up(int const x, int const y)
{
    TransparentWidget::mouse_up(x, y);

    if (option_selector == NULL) {
        set_ratio(ratio + step_size);
    } else if (!is_selecting) {
        is_selecting = true;
        option_selector->select_option(param_id, this);
    }

    return false;
}


void DiscreteParamEditor::set_ratio(double const new_ratio)
{
    double const old_ratio = ratio;

    is_selecting = false;

    if (option_selector != NULL) {
        is_editing_ = false;
    }

    if (
            (new_ratio > 1.0 && old_ratio > 0.999999)
            || std::fabs(new_ratio) < 0.000001
    ) {
        ratio = 0.0;
    } else if (
            (new_ratio < 0.0 && old_ratio < 0.000001)
            || std::fabs(new_ratio - 1.0) < 0.000001
    ) {
        ratio = 1.0;
    } else {
        ratio = GUI::clamp_ratio(new_ratio);
    }

    proxy.push_message(Proxy::MessageType::SET_PARAM, param_id, ratio);
    update();
    redraw();
}



bool DiscreteParamEditor::mouse_move(int const x, int const y, bool const modifier)
{
    TransparentWidget::mouse_move(x, y, modifier);

    gui->set_status_line(title);
    start_editing();

    return true;
}


bool DiscreteParamEditor::mouse_leave(int const x, int const y)
{
    TransparentWidget::mouse_leave(x, y);

    if (!is_selecting) {
        gui->set_status_line("");
        stop_editing();
    }

    return true;
}


bool DiscreteParamEditor::mouse_wheel(double const delta, bool const modifier)
{
    TransparentWidget::mouse_wheel(delta, modifier);

    if (option_selector == NULL) {
        set_ratio(ratio + (delta < 0 ? - step_size : step_size));
    }

    return false;
}

}

#endif
