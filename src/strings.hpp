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

#ifndef MPE_EMULATOR__STRINGS_HPP
#define MPE_EMULATOR__STRINGS_HPP

#include <cstddef>
#include <vector>

#include "common.hpp"
#include "proxy.hpp"


namespace MpeEmulator
{

class Strings
{
    public:
        static char const* const CONTROLLERS_SHORT[];
        static char const* const CONTROLLERS_LONG[];
        static size_t const CONTROLLERS_COUNT;

        static char const* const ZONE_TYPES[];
        static size_t const ZONE_TYPES_COUNT;

        static char const* const CHANNELS[];
        static size_t const CHANNELS_COUNT;

        static char const* const ANCHORS[];
        static size_t const ANCHORS_COUNT;

        static char const* const EXCESS_NOTE_HANDLINGS[];
        static size_t const EXCESS_NOTE_HANDLINGS_COUNT;

        static char const* const TARGETS_SHORT[];
        static char const* const TARGETS_LONG[];
        static size_t const TARGETS_COUNT;

        static char const* const RESETS[];
        static size_t const RESETS_COUNT;

        static char const* const TOGGLE_STATES[];
        static size_t const TOGGLE_STATES_COUNT;

        static char const* const DISTORTIONS[];
        static size_t const DISTORTIONS_COUNT;

        static char const* const TRANSPOSE_OPTIONS[];
        static size_t const TRANSPOSE_OPTIONS_COUNT;

        static char const* const PARAMS[Proxy::ParamId::PARAM_ID_COUNT];

        static char const* const* get_options(
            Proxy::ParamId const param_id,
            size_t& count
        ) noexcept;

        static void param_ratio_to_str(
            Proxy const& proxy,
            Proxy::ParamId const param_id,
            double const ratio,
            char* const buffer,
            size_t const buffer_size
        ) noexcept;

    private:
        class ParamFormat
        {
            public:
                constexpr ParamFormat() noexcept;

                constexpr ParamFormat(
                    char const* const format,
                    double const scale
                ) noexcept;

                constexpr ParamFormat(
                    char const* const* const options,
                    size_t const number_of_options
                ) noexcept;

                void ratio_to_str(
                    Proxy const& proxy,
                    Proxy::ParamId const param_id,
                    double const ratio,
                    char* const buffer,
                    size_t const buffer_size
                ) const noexcept;

                char const* const format;
                char const* const* const options;

                double const scale;
                size_t const number_of_options;

            private:
                void ratio_to_str_float(
                    double const ratio,
                    char* const buffer,
                    size_t const buffer_size
                ) const noexcept;

                void ratio_to_str_options(
                    Proxy const& proxy,
                    Proxy::ParamId const param_id,
                    double const ratio,
                    char* const buffer,
                    size_t const buffer_size
                ) const noexcept;

                void ratio_to_str_int(
                    Proxy const& proxy,
                    Proxy::ParamId const param_id,
                    double const ratio,
                    char* const buffer,
                    size_t const buffer_size
                ) const noexcept;
        };

        static ParamFormat PARAM_FORMATS[Proxy::ParamId::PARAM_ID_COUNT];
};

}

#endif
