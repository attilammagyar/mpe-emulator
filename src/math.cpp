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

#ifndef MPE_EMULATOR__MATH_CPP
#define MPE_EMULATOR__MATH_CPP

#include <cmath>

#include "math.hpp"


namespace MpeEmulator
{

Math const Math::math;


Math::Math() noexcept
{
    init_distortions();
}


void Math::init_distortions() noexcept
{
    constexpr double max_inv = 1.0 / (double)DISTORTION_TABLE_MAX_INDEX;

    double xs[DISTORTION_TABLE_SIZE];

    for (size_t i = 0; i != DISTORTION_TABLE_SIZE; ++i) {
        xs[i] = (double)i * max_inv;
    }

    for (size_t i = 0; i != DISTORTION_TABLE_SIZE; ++i) {
        distortions[DistortionCurve::DIST_CURVE_SMOOTH_SMOOTH][i] = dist_smooth_smooth(xs[i]);
    }

    for (size_t i = 0; i != DISTORTION_TABLE_SIZE; ++i) {
        distortions[DistortionCurve::DIST_CURVE_SMOOTH_SHARP][i] = dist_smooth_sharp(xs[i]);
    }

    for (size_t i = 0; i != DISTORTION_TABLE_SIZE; ++i) {
        distortions[DistortionCurve::DIST_CURVE_SHARP_SMOOTH][i] = dist_sharp_smooth(xs[i]);
    }

    for (size_t i = 0; i != DISTORTION_TABLE_SIZE; ++i) {
        distortions[DistortionCurve::DIST_CURVE_SHARP_SHARP][i] = dist_sharp_sharp(xs[i]);
    }
}


double Math::dist_smooth_smooth(double const x) noexcept
{
    return (std::tanh(8.0 * (2.0 * x - 1.0)) + 1.0) / 2.0;
}


double Math::dist_smooth_sharp(double const x) noexcept
{
    return std::pow(x, 5.0);
}


double Math::dist_sharp_smooth(double const x) noexcept
{
    return std::pow(x * (1.0 - std::log(x + 0.001)) / (1.0 - std::log(1.001)), 1.0 / 3.0);
}


double Math::dist_sharp_sharp(double const x) noexcept
{
    /*
    Antiderivative of ((2 * x - 1) ^ 2) ^ 5.

    Construction: the idea is to map [0, 1] to itself with a smooth function f
    for which all of the following properties hold:

     1. f(0) = 0.

     2. f(1) = 1.

     3. f'(0) = 1 and f'(1) = 1 (ie. connect sharply to the constant 0 and 1
        functions on the respective ends).

     4. f'(x) >= 0 for all x where 0 < x < 1.

     5. f'(x) = f'(1 - x) for all x where 0 < x < 1.

     6. f'(1 / 2) = 0.

     7. f''(1 / 2) = 0.

     7. f''(x) < 0 for all x where 0 <= x < 1 / 2.

     8. f''(x) > 0 for all x where 1 / 2 < x <= 1.

    The (2 * x - 1) ^ 2 function fits the bill nicely, and raising it to the 5th
    power exaggerates its properties.

    See also: https://en.wikipedia.org/wiki/Horner%27s_method
    */

    constexpr double a = 1024.0;
    constexpr double b = 5632.0;
    constexpr double c = 14080.0;
    constexpr double d = 21120.0;
    constexpr double e = 21120.0;
    constexpr double f = 14784.0;
    constexpr double g = 7392.0;
    constexpr double h = 2640.0;
    constexpr double i = 660.0;
    constexpr double j = 110.0;
    constexpr double k = 11.0;

    return ((((((((((a * x - b) * x + c) * x - d) * x + e) * x - f) * x + g) * x - h) * x + i) * x - j) * x + k) * x;
}


double Math::combine(
        double const a_weight,
        double const a,
        double const b
) noexcept {
    /*
    One of the multiplications can be eliminated from the following formula:

        a_weight * a + (1.0 - a_weight) * b
    */
    return a_weight * (a - b) + b;
}


double Math::distort(
        double const level,
        double const number,
        DistortionCurve const curve
) noexcept {
    if (level < 0.0001) {
        return number;
    }

    return combine(
        level,
        lookup(
            math.distortions[(size_t)curve],
            DISTORTION_TABLE_MAX_INDEX,
            number * DISTORTION_SCALE
        ),
        number
    );
}


double Math::lookup(
        double const* const table,
        size_t const max_index,
        double const index
) noexcept {
    size_t const before_index = (size_t)index;

    if (before_index >= max_index) {
        return table[max_index];
    }

    double const after_weight = index - std::floor(index);
    size_t const after_index = before_index + 1;

    return combine(after_weight, table[after_index], table[before_index]);
}

}

#endif
