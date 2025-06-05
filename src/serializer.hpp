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

#ifndef MPE_EMULATOR__SERIALIZER_HPP
#define MPE_EMULATOR__SERIALIZER_HPP

#include <cstddef>
#include <string>
#include <vector>

#include "common.hpp"
#include "proxy.hpp"


namespace MpeEmulator
{

class Serializer
{
    public:
        static constexpr size_t PARAM_NAME_MAX_LENGTH = 8;
        static constexpr size_t SECTION_NAME_MAX_LENGTH = 16;

        typedef char ParamName[PARAM_NAME_MAX_LENGTH];
        typedef char SectionName[SECTION_NAME_MAX_LENGTH];

        static constexpr size_t MAX_SIZE = 256 * 1024;

        static std::string const LINE_END;

        typedef std::vector<std::string> Lines;

        static Lines* parse_lines(std::string const& serialized) noexcept;

        static bool parse_section_name(
            std::string const& line,
            SectionName& section_name
        ) noexcept;

        static bool parse_line_until_value(
            std::string::const_iterator& it,
            std::string::const_iterator const& end,
            ParamName& param_name
        ) noexcept;

        static bool skipping_remaining_whitespace_or_comment_reaches_the_end(
            std::string::const_iterator& it,
            std::string::const_iterator const& end
        ) noexcept;

        static bool is_mpe_emulator_section_start(
            SectionName const& section_name
        ) noexcept;

        static std::string serialize(Proxy const& proxy) noexcept;

        static void import_settings_in_gui_thread(
            Proxy& proxy,
            std::string const& serialized
        ) noexcept;

        static void import_settings_in_audio_thread(
            Proxy& proxy,
            std::string const& serialized
        ) noexcept;

        static void trim_excess_zeros_from_end_after_snprintf(
            char* const number,
            int const length,
            size_t const max_length
        ) noexcept;

    private:
        enum Thread {
            AUDIO = 0,
            GUI = 1,
        };

        static constexpr char const* MPE_EMULATOR_SECTION_NAME = "mpeemulator";

        template<Thread thread>
        static void import_settings(
            Proxy& proxy,
            std::string const& serialized
        ) noexcept;

        template<Thread thread>
        static void process_lines(Proxy& proxy, Lines* const lines) noexcept;

        template<Thread thread>
        static void send_message(
            Proxy& proxy,
            Proxy::Message const& message
        ) noexcept;

        static bool is_section_name_char(char const c) noexcept;
        static bool is_digit(char const c) noexcept;
        static bool is_capital_letter(char const c) noexcept;
        static bool is_lowercase_letter(char const c) noexcept;
        static bool is_line_break(char const c) noexcept;
        static bool is_inline_whitespace(char const c) noexcept;
        static bool is_comment_leader(char const c) noexcept;

        static void process_line(
            std::vector<Proxy::Message>& messages,
            Proxy const& proxy,
            std::string const& line
        ) noexcept;

        static bool parse_param_name(
            std::string::const_iterator& it,
            std::string::const_iterator const& end,
            ParamName& param_name
        ) noexcept;

        static bool parse_equal_sign(
            std::string::const_iterator& it,
            std::string::const_iterator const& end
        ) noexcept;

        static bool parse_number(
            std::string::const_iterator& it,
            std::string::const_iterator const& end,
            double& number
        ) noexcept;

        static double to_number(std::string const& text) noexcept;
};

}

#endif
