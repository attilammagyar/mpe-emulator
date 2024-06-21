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

#ifndef MPE_EMULATOR__MATH_HPP
#define MPE_EMULATOR__MATH_HPP

#include <cstddef>

#include "common.hpp"


namespace MpeEmulator
{

/**
 * \brief Faster versions of frequently used math.h functions, using table
 *        lookup with linear interpolation.
 */
class Math
{
    public:
        enum DistortionShape {
            DIST_SHAPE_SMOOTH_SMOOTH = 0,
            DIST_SHAPE_SMOOTH_SHARP = 1,
            DIST_SHAPE_SHARP_SMOOTH = 2,
            DIST_SHAPE_SHARP_SHARP = 3,
        };

        static constexpr size_t DISTORTIONS = 4;

        /**
         * \brief Compute a_weight * a + (1.0 - a_weight) * b
         */
        static double combine(
            double const a_weight,
            double const a,
            double const b
        ) noexcept;

        /**
         * \brief Apply the selected distortion to the given value between
         *        0.0 and 1.0.
         */
        static double distort(
            double const level,
            double const number,
            DistortionShape const shape = DistortionShape::DIST_SHAPE_SMOOTH_SMOOTH
        ) noexcept;

        /**
         * \brief Look up the given floating point, non-negative \c index in the
         *        given table, with linear interpolation. If \c index is greater
         *        than or equal to \c max_index, then the last element of the
         *        table is returned.
         */
        static double lookup(
            double const* const table,
            size_t const max_index,
            double const index
        ) noexcept;

    private:
        static constexpr size_t DISTORTION_TABLE_SIZE = 0x0800;
        static constexpr size_t DISTORTION_TABLE_MAX_INDEX = DISTORTION_TABLE_SIZE - 1;
        static constexpr double DISTORTION_SCALE = (double)DISTORTION_TABLE_MAX_INDEX;

        static Math const math;

        Math() noexcept;

        void init_distortions() noexcept;

        double dist_smooth_smooth(double const x) noexcept;
        double dist_smooth_sharp(double const x) noexcept;
        double dist_sharp_smooth(double const x) noexcept;
        double dist_sharp_sharp(double const x) noexcept;

        double distortions[DISTORTIONS][DISTORTION_TABLE_SIZE];
};

}

#endif
