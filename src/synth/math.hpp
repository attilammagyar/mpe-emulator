/*
 * This file is part of JS80P, a synthesizer plugin.
 * Copyright (C) 2022  Attila M. Magyar
 *
 * JS80P is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * JS80P is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef JS80P__SYNTH__MATH_HPP
#define JS80P__SYNTH__MATH_HPP

#include <cmath>
#include <algorithm>
#include <vector>

#include "js80p.hpp"


namespace JS80P
{

/**
 * \brief Faster versions of frequently used math.h functions, using table
 *        lookup with linear interpolation.
 */
class Math
{
    public:
        static constexpr Number PI = 3.14159265358979323846264338327950288419716939937510;
        static constexpr Number PI_DOUBLE = 2.0 * PI;
        static constexpr Number PI_HALF = PI / 2.0;
        static constexpr Number PI_SQR = PI * PI;

        static constexpr Number SQRT_OF_2 = std::sqrt(2.0);

        static constexpr Number LN_OF_2 = std::log(2.0);
        static constexpr Number LN_OF_10 = std::log(10.0);

        static constexpr Number POW_10_MIN = (
            Constants::BIQUAD_FILTER_GAIN_MIN * Constants::BIQUAD_FILTER_GAIN_SCALE
        ); /// \warning This limit is not enforced. Values outside the limit may be imprecise.

        static constexpr Number POW_10_MAX = (
            Constants::BIQUAD_FILTER_GAIN_MAX * Constants::BIQUAD_FILTER_GAIN_SCALE
        ); /// \warning This limit is not enforced. Values outside the limit may be imprecise.

        static constexpr Number POW_10_INV_MIN = (
            Constants::BIQUAD_FILTER_Q_MIN * Constants::BIQUAD_FILTER_Q_SCALE
        ); /// \warning This limit is not enforced. Values outside the limit may be imprecise.

        static constexpr Number POW_10_INV_MAX = (
            Constants::BIQUAD_FILTER_Q_MAX * Constants::BIQUAD_FILTER_Q_SCALE
        ); /// \warning This limit is not enforced. Values outside the limit may be imprecise.

        static constexpr Number EXP_MIN = (
            std::min(LN_OF_10 * POW_10_MIN, -1.0 * LN_OF_10 * POW_10_INV_MAX)
        ); /// \warning This limit is not enforced. Values outside the limit may be imprecise.

        static constexpr Number EXP_MAX = (
            std::max(LN_OF_10 * POW_10_MAX, -1.0 * LN_OF_10 * POW_10_INV_MIN)
        ); /// \warning This limit is not enforced. Values outside the limit may be imprecise.

        /**
         * \warning Negative numbers close to multiples of PI are not handled
         *          very well with regards to precision.
         */
        static Number sin(Number const x);

        /**
         * \warning Negative numbers close to multiples of PI are not handled
         *          very well with regards to precision.
         */
        static Number cos(Number const x);

        static Number exp(Number const x);
        static Number pow_10(Number const x);
        static Number pow_10_inv(Number const x);

        static Frequency detune(
            Frequency const frequency,
            Number const cents
        );

        /**
         * \brief Return a pseudo random number between 0.0 and 1.0, based on
         *        the given number between 0.0 and 1.0. The return value is
         *        deterministic, the same input number will always generate the
         *        same result.
         */
        static Number randomize(Number const number);

        static Number lookup(
            Number const* const table,
            int const max_index,
            Number const index
        );

        class Statistics;

        static void compute_statistics(
            std::vector<Number> const numbers,
            Statistics& statistics
        );

        class Statistics {
            public:
                Number min;
                Number max;
                Number median;
                Number mean;
                Number standard_deviation;
                bool is_valid;
        };

    private:
        static constexpr int RANDOMS = 0x0200;
        static constexpr Number RANDOM_SCALE = (Number)RANDOMS;

        static constexpr int TABLE_SIZE = 0x0800;
        static constexpr int TABLE_MASK = 0x07ff;

        static constexpr Number SINE_SCALE = (Number)TABLE_SIZE / PI_DOUBLE;

        static constexpr int EXP_ITERATIONS = 8;

        static constexpr Number EXP_SCALE = 1.0 / (Number)(1 << EXP_ITERATIONS);
        static constexpr Number POW_10_SCALE = LN_OF_10 * EXP_SCALE;
        static constexpr Number POW_10_INV_SCALE = -1.0 * POW_10_SCALE;

        static Math const math;

        static Number iterate_exp(Number const x, Number const scale);

        Math();

        void init_sines();
        void init_randoms();

        Number sin_impl(Number const x) const;

        /*
        With a little bit of templating, this might seem to be reusable for
        oscillator wavetables, but actually that would mean calculating weights
        and indices twice for a single sample lookup in the two tables for
        fewer and more partials. Alternatively, bringing the multiple-table
        logic here from there would be overkill.
        */
        Number lookup_periodic(Number const* table, Number const index) const;

        Number sines[TABLE_SIZE];
        Number randoms[RANDOMS];
};

}

#endif
