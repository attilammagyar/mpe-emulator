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
#include <cstdio>
#include <string>

#include "test.cpp"

#include "common.hpp"
#include "midi.hpp"
#include "proxy.hpp"
#include "serializer.hpp"


using namespace MpeEmulator;


TEST(can_convert_configuration_to_string_and_import_it, {
    Proxy proxy_1;
    Proxy proxy_2;
    std::string serialized;
    double const five_channels_as_ratio = proxy_1.channels.value_to_ratio(5);
    double const ten_channels_as_ratio = proxy_1.channels.value_to_ratio(10);
    double const c4_as_ratio = proxy_1.anchor.value_to_ratio(60);
    double const a0_as_ratio = proxy_1.anchor.value_to_ratio(21);

    proxy_1.push_message(
        Proxy::MessageType::SET_PARAM, Proxy::ParamId::Z1CHN, five_channels_as_ratio
    );
    proxy_1.push_message(
        Proxy::MessageType::SET_PARAM, Proxy::ParamId::Z1ANC, c4_as_ratio
    );
    proxy_1.process_messages();

    proxy_2.push_message(
        Proxy::MessageType::SET_PARAM, Proxy::ParamId::Z1CHN, ten_channels_as_ratio
    );
    proxy_2.push_message(
        Proxy::MessageType::SET_PARAM, Proxy::ParamId::Z1ANC, a0_as_ratio
    );
    proxy_2.process_messages();

    serialized = Serializer::serialize(proxy_1);
    Serializer::import_settings_in_audio_thread(proxy_2, serialized);

    assert_eq(
        five_channels_as_ratio,
        proxy_2.get_param_ratio_atomic(Proxy::ParamId::Z1CHN),
        0.000001
    );
    assert_eq(
        c4_as_ratio,
        proxy_2.get_param_ratio_atomic(Proxy::ParamId::Z1ANC),
        0.000001
    );
})


TEST(importing_settings_ignores_comments_and_whitespace_and_unknown_sections, {
    Proxy proxy;
    std::string const settings = (
        "  [  \t   mpeemulator   \t   ]    ; comment\n"
        "; Z1ANC = 0.99\n"
        "  ; Z1ANC = 0.98\n"
        "   \t   Z1ANC    = \t    0.42        \n"
        "Z1CHN = 0.2 ; some comment\n"
        "\n"
        "[unknown]\n"
        "Z1ANC = 0.123\n"
    );

    Serializer::import_settings_in_audio_thread(proxy, settings);

    assert_eq(
        0.42, proxy.get_param_ratio_atomic(Proxy::ParamId::Z1ANC), 0.000001
    );
    assert_eq(
        0.2, proxy.get_param_ratio_atomic(Proxy::ParamId::Z1CHN), 0.000001
    );
})


TEST(importing_settings_ignores_invalid_lines_and_unknown_sections, {
    Proxy proxy;
    std::string const settings = (
        "Z1R3IN = 0.99\n"
        "[mpeemulator]\n"
        "Z1ANC = 0.42\n"
        "Z1CHN = 1\n"
        "Z1ENH = .6\n"
        "= 0.98\n"
        "Z1CHN 0.97\n"
        "Z1CHN =\n"
        "Z1CHN = a\n"
        "Z1CHN = 0.96  a   \n"
        "Z1CHNx = 0.95\n"
        "Z1CHN = 0.94a   \n"
        "Z1CHN = 0.93  a   \n"
        "Z1CHN = -0.92\n"
        "Z1CHN = 0..91\n"
        "Z1CHN = ..90\n"
        "\n"
        "[mpeemulator]]\n"
        "[mpeemulator]x\n"
        "\n"
        "MCM = 0.\n"
        "Z1R1IN = 0.123\n"
        "[mpeemulatorx]\n"
        "Z1ANC = 0.89\n"
        "Z1CHN = 0.88\n"
        "MCM = 0.87\n"
        "Z1R1IN = 0.86\n"
        "Z1R2IN = 0.85\n"
        "[mpeemulatoraaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa]\n"
        "Z1ANC = 0.84\n"
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA = 0.83\n"
    );

    Serializer::import_settings_in_audio_thread(proxy, settings);

    assert_eq(
        0.42, proxy.get_param_ratio_atomic(Proxy::ParamId::Z1ANC), 0.000001
    );
    assert_eq(
        1.00, proxy.get_param_ratio_atomic(Proxy::ParamId::Z1CHN), 0.000001
    );
    assert_eq(
        0.60, proxy.get_param_ratio_atomic(Proxy::ParamId::Z1ENH), 0.000001
    );
    assert_eq(
        0.0, proxy.get_param_ratio_atomic(Proxy::ParamId::MCM), 0.000001
    );
    assert_eq(
        0.123, proxy.get_param_ratio_atomic(Proxy::ParamId::Z1R1IN), 0.000001
    );
    assert_eq(
        proxy.get_param_default_ratio(Proxy::ParamId::Z1R2IN),
        proxy.get_param_ratio_atomic(Proxy::ParamId::Z1R2IN),
        0.000001
    );
    assert_neq(
        0.99,
        proxy.get_param_ratio_atomic(Proxy::ParamId::Z1R3IN),
        0.000001
    );
})


TEST(importing_settings_does_not_require_terminating_new_line, {
    Proxy proxy;
    std::string const settings = (
        "[mpeemulator]\n"
        "Z1ANC = 0.42"
    );

    Serializer::import_settings_in_audio_thread(proxy, settings);

    assert_eq(
        0.42, proxy.get_param_ratio_atomic(Proxy::ParamId::Z1ANC), 0.000001
    );
})


TEST(imported_values_are_clamped, {
    Proxy proxy;
    std::string const settings = (
        "[mpeemulator]\n"
        "Z1ANC = 2.1\n"
    );

    Serializer::import_settings_in_audio_thread(proxy, settings);

    assert_eq(
        1.0, proxy.get_param_ratio_atomic(Proxy::ParamId::Z1ANC), 0.000001
    );
})


TEST(extremely_long_lines_may_be_truncated, {
    constexpr size_t spaces_count = Serializer::MAX_SIZE * 2 + 123;
    Proxy proxy;
    std::string settings("[mpeemulator]\n");
    std::string spaces(" ");
    std::string long_line("");

    for (size_t i = spaces_count; i != 0;)
    {
        if ((i & 1) != 0) {
            long_line += spaces;
        }

        spaces += spaces;
        i >>= 1;
    }

    assert_eq((int)spaces_count, (int)long_line.size());

    settings += long_line;
    settings += (
        "Z1ANC = 0.42\n"
        "Z1CHN = 0.2\n"
    );
    Serializer::import_settings_in_audio_thread(proxy, settings);

    assert_eq(
        proxy.get_param_default_ratio(Proxy::ParamId::Z1ENH),
        proxy.get_param_ratio_atomic(Proxy::ParamId::Z1ENH),
        0.000001
    );
    assert_eq(
        proxy.get_param_default_ratio(Proxy::ParamId::Z1ANC),
        proxy.get_param_ratio_atomic(Proxy::ParamId::Z1ANC),
        0.000001
    );
    assert_eq(
        0.2, proxy.get_param_ratio_atomic(Proxy::ParamId::Z1CHN), 0.000001
    );
})


TEST(param_names_are_parsed_case_insensitively_and_converted_to_upper_case, {
    Proxy proxy;
    std::string const settings = (
        "[mpeemulator]\n"
        "z1anc = 0.5\n"
    );
    std::string const line = "z1enH = 0.1";
    Serializer::ParamName param_name;
    std::string::const_iterator line_it = line.begin();

    Serializer::import_settings_in_audio_thread(proxy, settings);

    assert_eq(0.5, proxy.get_param_ratio_atomic(Proxy::ParamId::Z1ANC), 0.000001);

    param_name[0] = 'x';

    Serializer::parse_line_until_value(line_it, line.end(), param_name);
    assert_eq("Z1ENH", param_name);
})


TEST(params_which_are_missing_from_the_exported_settings_are_cleared_and_reset_to_default, {
    Proxy proxy;
    std::string const settings = (
        "[mpeemulator]\n"
        "Z1CHN = 0.42\n"
    );

    proxy.push_message(Proxy::MessageType::SET_PARAM, Proxy::ParamId::Z1CHN, 0.123);
    proxy.push_message(Proxy::MessageType::SET_PARAM, Proxy::ParamId::Z1ANC, 0.123);
    proxy.process_messages();

    Serializer::import_settings_in_audio_thread(proxy, settings);

    assert_eq(0.42, proxy.get_param_ratio_atomic(Proxy::ParamId::Z1CHN), 0.000001);
    assert_eq(
        proxy.get_param_default_ratio(Proxy::ParamId::Z1ANC),
        proxy.get_param_ratio_atomic(Proxy::ParamId::Z1ANC),
        0.000001
    );
})


TEST(message_queue_is_cleared_before_importing_settings_inside_audio_thread, {
    Proxy proxy;
    std::string const settings = (
        "[mpeemulator]\n"
        "Z1ANC = 0.42\n"
    );

    proxy.push_message(Proxy::MessageType::SET_PARAM, Proxy::ParamId::Z1ANC, 0.123);
    Serializer::import_settings_in_audio_thread(proxy, settings);
    proxy.process_messages();

    assert_eq(0.42, proxy.get_param_ratio_atomic(Proxy::ParamId::Z1ANC), 0.000001);
})


TEST(can_import_settings_inside_the_gui_thread, {
    Proxy proxy;
    std::string const settings = (
        "[mpeemulator]\n"
        "Z1ANC = 0.42\n"
    );

    proxy.push_message(Proxy::MessageType::SET_PARAM, Proxy::ParamId::Z1CHN, 0.123);
    Serializer::import_settings_in_gui_thread(proxy, settings);
    proxy.process_messages();

    assert_eq(0.42, proxy.get_param_ratio_atomic(Proxy::ParamId::Z1ANC), 0.000001);
    assert_eq(
        proxy.get_param_default_ratio(Proxy::ParamId::Z1CHN),
        proxy.get_param_ratio_atomic(Proxy::ParamId::Z1CHN),
        0.000001
    );
})


void assert_trimmed(char const* const expected, char const* const raw_number)
{
    constexpr size_t buffer_size = 16;

    char buffer[buffer_size];
    int const length = snprintf(buffer, buffer_size, "%s", raw_number);

    Serializer::trim_excess_zeros_from_end_after_snprintf(buffer, length, buffer_size);
    assert_eq(expected, buffer);

    if (strncmp(expected, raw_number, buffer_size) != 0) {
        std::fill_n(buffer, buffer_size, '0');
        int const terminating_zero = snprintf(buffer, buffer_size, "%s", expected);
        buffer[terminating_zero] = '0';
        buffer[buffer_size - 1] = '\x00';
        Serializer::trim_excess_zeros_from_end_after_snprintf(buffer, 12345, buffer_size);
        assert_eq(expected, buffer);
    }

    snprintf(buffer, buffer_size, "000");
    Serializer::trim_excess_zeros_from_end_after_snprintf(buffer, -1, buffer_size);
    assert_eq("000", buffer);
}


TEST(trimming_zeros_from_end_of_numbers, {
    assert_trimmed("", "");
    assert_trimmed("0", "0");
    assert_trimmed("1", "1");
    assert_trimmed("10", "10");
    assert_trimmed("100", "100");
    assert_trimmed("1000", "1000");
    assert_trimmed("0.0", "0.0");
    assert_trimmed("0.1", "0.1");
    assert_trimmed("0.10", "0.10");
    assert_trimmed("0.12", "0.12");
    assert_trimmed("0.120", "0.120");
    assert_trimmed("0.0", "0.00");
    assert_trimmed("0.0", "0.00000");
    assert_trimmed("0.120", "0.1200");
    assert_trimmed("0.120", "0.120000");
    assert_trimmed("0.120", "0.1200000000000");
    assert_trimmed("0.1234567890123", "0.1234567890123");
})


TEST(trailing_zeros_and_params_with_default_values_are_omitted_from_serialized_settings, {
    Proxy proxy;
    std::string settings = "";

    settings += "[mpeemulator]";
    settings += Serializer::LINE_END;
    settings += "Z1ANC = 0.50";
    settings += Serializer::LINE_END;

    proxy.push_message(Proxy::MessageType::CLEAR, Proxy::ParamId::INVALID_PARAM_ID, 0.0);
    proxy.push_message(Proxy::MessageType::SET_PARAM, Proxy::ParamId::Z1ANC, 0.5);
    proxy.process_messages();

    assert_eq(settings, Serializer::serialize(proxy));
})
