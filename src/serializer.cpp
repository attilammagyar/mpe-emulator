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

#ifndef MPE_EMULATOR__SERIALIZER_CPP
#define MPE_EMULATOR__SERIALIZER_CPP

#include <algorithm>
#include <cmath>
#include <cctype>
#include <cstring>
#include <sstream>

#include "serializer.hpp"


namespace MpeEmulator
{

std::string const Serializer::LINE_END = "\r\n";


std::string Serializer::serialize(Proxy const& proxy) noexcept
{
    constexpr size_t line_size = 128;
    char line[line_size];
    std::string serialized("");

    serialized.reserve(MAX_SIZE);
    serialized += "[";
    serialized += MPE_EMULATOR_SECTION_NAME;
    serialized += "]";
    serialized += LINE_END;

    for (int i = 0; i != Proxy::ParamId::PARAM_ID_COUNT; ++i) {
        Proxy::ParamId const param_id = (Proxy::ParamId)i;
        std::string const param_name = proxy.get_param_name(param_id);

        if (param_name.length() > 0) {
            double const set_ratio = proxy.get_param_ratio_atomic(param_id);
            double const default_ratio = proxy.get_param_default_ratio(param_id);

            if (std::fabs(default_ratio - set_ratio) > 0.000001) {
                int const length = snprintf(
                    line,
                    line_size,
                    "%s = %.15f",
                    param_name.c_str(),
                    set_ratio
                );
                trim_excess_zeros_from_end_after_snprintf(line, length, line_size);
                serialized += line;
                serialized += LINE_END;
            }
        }
    }

    return serialized;
}


void Serializer::trim_excess_zeros_from_end_after_snprintf(
        char* number,
        int const length,
        size_t const max_length
) noexcept {
    if (MPE_EMULATOR_UNLIKELY(length < 1 || max_length < 1)) {
        return;
    }

    size_t dot_index = max_length;

    for (size_t i = 0; number[i] != '\x00' && i != max_length; ++i) {
        if (number[i] == '.') {
            dot_index = i;

            break;
        }
    }

    if (MPE_EMULATOR_UNLIKELY(dot_index == max_length)) {
        return;
    }

    size_t last_zero_index = max_length;

    for (size_t i = dot_index; number[i] != '\x00' && i != max_length; ++i) {
        if (number[i] != '0') {
            last_zero_index = max_length;
        } else if (last_zero_index == max_length) {
            last_zero_index = i;
        }
    }

    if (last_zero_index == max_length) {
        return;
    }

    ++last_zero_index;

    if (last_zero_index < max_length) {
        number[last_zero_index] = '\x00';
    }
}


void Serializer::import_settings_in_gui_thread(
        Proxy& proxy,
        std::string const& serialized
) noexcept {
    import_settings<Thread::GUI>(proxy, serialized);
}


void Serializer::import_settings_in_audio_thread(
        Proxy& proxy,
        std::string const& serialized
) noexcept {
    proxy.process_messages();
    import_settings<Thread::AUDIO>(proxy, serialized);
}


template<Serializer::Thread thread>
void Serializer::import_settings(Proxy& proxy, std::string const& serialized) noexcept
{
    Lines* lines = parse_lines(serialized);
    process_lines<thread>(proxy, lines);

    delete lines;
}


Serializer::Lines* Serializer::parse_lines(std::string const& serialized) noexcept
{
    constexpr size_t max_line_pos = MAX_SIZE - 1;
    Lines* lines = new Lines();
    char* line = new char[MAX_SIZE];
    std::string::const_iterator const end = serialized.end();
    size_t line_pos = 0;
    bool truncating = false;

    for (std::string::const_iterator it = serialized.begin(); it != end; ++it) {
        char const c = *it;

        if (is_line_break(c)) {
            if (line_pos != 0) {
                lines->push_back(std::string(line, line_pos));
            }

            line_pos = 0;
            truncating = false;

            continue;
        }

        if (truncating) {
            continue;
        }

        line[line_pos++] = c;

        if (line_pos == max_line_pos) {
            lines->push_back(std::string(line, line_pos));
            line_pos = 0;
            truncating = true;
        }
    }

    if (line_pos != 0) {
        lines->push_back(std::string(line, line_pos));
    }

    delete[] line;

    return lines;
}


bool Serializer::is_digit(char const c) noexcept
{
    return '0' <= c && c <= '9';
}


bool Serializer::is_capital_letter(char const c) noexcept
{
    return 'A' <= c && c <= 'Z';
}


bool Serializer::is_lowercase_letter(char const c) noexcept
{
    return 'a' <= c && c <= 'z';
}


bool Serializer::is_line_break(char const c) noexcept
{
    return c == '\n' || c == '\r';
}


bool Serializer::is_inline_whitespace(char const c) noexcept
{
    return c == ' ' || c == '\t';
}


bool Serializer::is_comment_leader(char const c) noexcept
{
    return c == ';';
}


bool Serializer::is_section_name_char(char const c) noexcept
{
    return is_digit(c) || is_capital_letter(c) || is_lowercase_letter(c);
}


template<Serializer::Thread thread>
void Serializer::process_lines(Proxy& proxy, Lines* lines) noexcept
{
    typedef std::vector<Proxy::Message> Messages;

    Messages messages;
    SectionName section_name;
    bool inside_mpe_emulator_section = false;

    messages.reserve(800);

    for (Lines::const_iterator it = lines->begin(); it != lines->end(); ++it) {
        std::string line = *it;

        if (parse_section_name(line, section_name)) {
            inside_mpe_emulator_section = false;

            if (is_mpe_emulator_section_start(section_name)) {
                inside_mpe_emulator_section = true;
                continue;
            }
        } else if (inside_mpe_emulator_section) {
            process_line(messages, proxy, line);
        }
    }

    send_message<thread>(
        proxy,
        Proxy::Message(
            Proxy::MessageType::CLEAR, Proxy::ParamId::INVALID_PARAM_ID, 0.0
        )
    );

    for (Messages::const_iterator it = messages.begin(); it != messages.end(); ++it) {
        send_message<thread>(proxy, *it);
    }
}


template<Serializer::Thread thread>
void Serializer::send_message(Proxy& proxy, Proxy::Message const& message) noexcept
{
    if constexpr (thread == Thread::AUDIO) {
        proxy.process_message(message);
    } else {
        proxy.push_message(message);
    }
}


bool Serializer::is_mpe_emulator_section_start(SectionName const& section_name) noexcept
{
    return strncmp(section_name, MPE_EMULATOR_SECTION_NAME, SECTION_NAME_MAX_LENGTH) == 0;
}


bool Serializer::parse_section_name(
        std::string const& line,
        SectionName& section_name
) noexcept {
    constexpr size_t section_name_pos_limit = strlen(MPE_EMULATOR_SECTION_NAME) + 1;

    std::string::const_iterator it = line.begin();
    std::string::const_iterator const end = line.end();
    size_t pos = 0;

    std::fill_n(section_name, SECTION_NAME_MAX_LENGTH, '\x00');

    if (skipping_remaining_whitespace_or_comment_reaches_the_end(it, end)) {
        return false;
    }

    if (*it != '[') {
        return false;
    }

    ++it;

    while (it != end && is_inline_whitespace(*it)) {
        ++it;
    }

    while (it != end && is_section_name_char(*it)) {
        if (pos != section_name_pos_limit) {
            section_name[pos++] = *it;
        }

        ++it;
    }

    while (it != end && is_inline_whitespace(*it)) {
        ++it;
    }

    if (it == end || *it != ']') {
        return false;
    }

    ++it;

    return skipping_remaining_whitespace_or_comment_reaches_the_end(it, end);
}


bool Serializer::parse_line_until_value(
        std::string::const_iterator& it,
        std::string::const_iterator const& end,
        ParamName& param_name
) noexcept {
    return (
        !skipping_remaining_whitespace_or_comment_reaches_the_end(it, end)
        && parse_param_name(it, end, param_name)
        && !skipping_remaining_whitespace_or_comment_reaches_the_end(it, end)
        && parse_equal_sign(it, end)
    );
}


void Serializer::process_line(
        std::vector<Proxy::Message>& messages,
        Proxy const& proxy,
        std::string const& line
) noexcept {
    std::string::const_iterator it = line.begin();
    std::string::const_iterator const end = line.end();
    Proxy::ParamId param_id;
    double number;
    ParamName param_name;

    if (
            !parse_line_until_value(it, end, param_name)
            || skipping_remaining_whitespace_or_comment_reaches_the_end(it, end)
            || !parse_number(it, end, number)
            || !skipping_remaining_whitespace_or_comment_reaches_the_end(it, end)
    ) {
        return;
    }

    param_id = proxy.get_param_id(param_name);

    if (param_id == Proxy::ParamId::INVALID_PARAM_ID) {
        return;
    }

    messages.push_back(
        Proxy::Message(Proxy::MessageType::SET_PARAM, param_id, number)
    );
}


bool Serializer::skipping_remaining_whitespace_or_comment_reaches_the_end(
        std::string::const_iterator& it,
        std::string::const_iterator const& end
) noexcept {
    if (it == end) {
        return true;
    }

    while (is_inline_whitespace(*it)) {
        ++it;

        if (it == end) {
            return true;
        }
    }

    if (is_comment_leader(*it)) {
        while (it != end) {
            ++it;
        }

        return true;
    }

    return false;
}


bool Serializer::parse_param_name(
        std::string::const_iterator& it,
        std::string::const_iterator const& end,
        ParamName& param_name
) noexcept {
    constexpr size_t param_name_pos_max = PARAM_NAME_MAX_LENGTH - 1;
    size_t param_name_pos = 0;

    std::fill_n(param_name, PARAM_NAME_MAX_LENGTH, '\x00');

    while (is_capital_letter(*it) || is_digit(*it) || is_lowercase_letter(*it)) {
        param_name[param_name_pos++] = toupper(*(it++));

        if (param_name_pos == param_name_pos_max || it == end) {
            return false;
        }
    }

    return param_name_pos > 0;
}


bool Serializer::parse_equal_sign(
        std::string::const_iterator& it,
        std::string::const_iterator const& end
) noexcept {
    if (*it != '=') {
        return false;
    }

    ++it;

    return true;
}


bool Serializer::parse_number(
        std::string::const_iterator& it,
        std::string::const_iterator const& end,
        double& number
) noexcept {
    std::string number_text;
    bool has_dot = false;

    while (it != end) {
        if (*it == '.') {
            if (has_dot) {
                return false;
            }

            has_dot = true;
        } else if (!is_digit(*it)) {
            break;
        }

        number_text += *(it++);
    }

    if (number_text.length() == 0) {
        return false;
    }

    double const parsed_number = std::min(
        1.0, std::max(0.0, to_number(number_text))
    );

    number = parsed_number;

    return true;
}


double Serializer::to_number(std::string const& text) noexcept
{
    std::istringstream s(text);
    double n;

    s >> n;

    return n;
}

}

#endif
