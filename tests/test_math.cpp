/*
 * This file is part of MPE Emulator.
 * Copyright (C) 2022, 2023, 2024  Attila M. Magyar
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

#include "common.hpp"
#include "math.cpp"


using namespace MpeEmulator;


TEST(combine, {
    assert_eq(42.0, Math::combine(1.0, 42.0, 123.0), 0.000001);
    assert_eq(123.0, Math::combine(0.0, 42.0, 123.0), 0.000001);
    assert_eq(
        0.3 * 42.0 + 0.7 * 123.0, Math::combine(0.3, 42.0, 123.0), 0.000001
    );
})


TEST(lookup, {
    constexpr size_t max_index = 6;
    double const table[] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0};

    assert_eq(1.0, Math::lookup(table, max_index, 0.0), 0.000001);
    assert_eq(1.5, Math::lookup(table, max_index, 0.5), 0.000001);
    assert_eq(1.7, Math::lookup(table, max_index, 0.7), 0.000001);
    assert_eq(6.0, Math::lookup(table, max_index, 5.0), 0.000001);
    assert_eq(6.3, Math::lookup(table, max_index, 5.3), 0.000001);
    assert_eq(6.999, Math::lookup(table, max_index, 5.999), 0.000001);
    assert_eq(7.0, Math::lookup(table, max_index, 6.0), 0.000001);
    assert_eq(7.0, Math::lookup(table, max_index, 6.1), 0.000001);
    assert_eq(7.0, Math::lookup(table, max_index, 7.0), 0.000001);
})


void assert_distorted(
        double const expected,
        double const level,
        double const number,
        double const tolerance
) {
    assert_eq(
        expected,
        Math::distort(level, number),
        tolerance,
        "level=%f, number=%f",
        level,
        number
    );
}


TEST(distort, {
    assert_distorted(0.0, 1.0, 0.0, 0.000001);
    assert_distorted(1.0, 1.0, 1.0, 0.01);
    assert_distorted(0.0, 1.0, 0.1, 0.01);
    assert_distorted(0.0, 1.0, 0.2, 0.01);
    assert_distorted(0.5, 1.0, 0.5, 0.01);
    assert_distorted(1.0, 1.0, 0.8, 0.01);
    assert_distorted(1.0, 1.0, 0.9, 0.01);

    assert_distorted(0.0, 0.5, 0.0, 0.000001);
    assert_gt(0.1, Math::distort(0.5, 0.1));
    assert_gt(0.2, Math::distort(0.5, 0.2));
    assert_distorted(0.5, 0.5, 0.5, 0.01);
    assert_lt(0.8, Math::distort(0.5, 0.8));
    assert_lt(0.9, Math::distort(0.5, 0.9));
    assert_distorted(1.0, 0.5, 1.0, 0.000001);

    assert_distorted(0.0, 0.0, 0.0, 0.000001);
    assert_distorted(0.1, 0.0, 0.1, 0.000001);
    assert_distorted(0.2, 0.0, 0.2, 0.000001);
    assert_distorted(0.5, 0.0, 0.5, 0.000001);
    assert_distorted(0.8, 0.0, 0.8, 0.000001);
    assert_distorted(0.9, 0.0, 0.9, 0.000001);
    assert_distorted(1.0, 0.0, 1.0, 0.000001);
})
