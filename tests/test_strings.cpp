/*
 * This file is part of MPE Emulator.
 * Copyright (C) 2024  Attila M. Magyar
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

#include "test.cpp"

#include <cstddef>

#include "common.hpp"

#include "proxy.hpp"
#include "strings.hpp"


using namespace MpeEmulator;


void assert_ratio_as_str(
        char const* const expected,
        Proxy::ParamId param_id,
        double const ratio,
        char const* const expected_long = NULL
) {
    constexpr size_t buffer_size = 32;

    Proxy proxy;
    char buffer[buffer_size];

    Strings::param_ratio_to_str(
        proxy, param_id, ratio, buffer, buffer_size, Strings::Length::SHORT
    );
    assert_eq(
        expected,
        buffer,
        "ratio=%f, param_id=\"%d\", length=short",
        ratio,
        (int)param_id
    );

    Strings::param_ratio_to_str(
        proxy, param_id, ratio, buffer, buffer_size, Strings::Length::LONG
    );
    assert_eq(
        expected_long != NULL ? expected_long : expected,
        buffer,
        "ratio=%f, param_id=\"%d\", length=long",
        ratio,
        (int)param_id
    );
}


TEST(param_ratio_to_str, {
    assert_ratio_as_str("0.00%", Proxy::ParamId::Z1R1DL, 0.0);
    assert_ratio_as_str("100.00%", Proxy::ParamId::Z1R1DL, 1.0);
    assert_ratio_as_str("0.00%", Proxy::ParamId::Z1R1DL, -0.0);
    assert_ratio_as_str("50.00%", Proxy::ParamId::Z1R1DL, 0.5);
    assert_ratio_as_str("50.00%", Proxy::ParamId::Z1R1DL, 0.4999999);

    assert_ratio_as_str("SMT-SMT", Proxy::ParamId::Z1R1DT, 0.0);
    assert_ratio_as_str("SHP-SHP", Proxy::ParamId::Z1R1DT, 1.0);
    assert_ratio_as_str("SMT-SMT", Proxy::ParamId::Z1R1DT, -0.0);
    assert_ratio_as_str("SMT-SHP", Proxy::ParamId::Z1R1DT, 1.0 / 3.0);
    assert_ratio_as_str("SHP-SMT", Proxy::ParamId::Z1R1DT, 2.0 / 3.0);

    assert_ratio_as_str("Never", Proxy::ParamId::Z1ENH, 0.0);
    assert_ratio_as_str("New", Proxy::ParamId::Z1ENH, 1.0);
    assert_ratio_as_str("Never", Proxy::ParamId::Z1ENH, -0.0);
    assert_ratio_as_str("Low", Proxy::ParamId::Z1ENH, 0.25);
    assert_ratio_as_str("High", Proxy::ParamId::Z1ENH, 0.5);
    assert_ratio_as_str("New", Proxy::ParamId::Z1ENH, 2.0);

    assert_ratio_as_str("New AA", Proxy::ParamId::Z1R1TR, 1.0, "Newest above anchor");
})


TEST(options, {
    size_t count;

    assert_eq(NULL, Strings::get_options(Proxy::ParamId::Z1R1DL, count));
    assert_eq(0, count);

    assert_neq(NULL, Strings::get_options(Proxy::ParamId::Z1ENH, count));
    assert_lt(0, count);
})
