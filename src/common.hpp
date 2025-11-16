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

#ifndef MPE_EMULATOR__CONSTS_HPP
#define MPE_EMULATOR__CONSTS_HPP

#ifdef MPE_EMULATOR_ASSERTIONS
#include <cassert>
#endif


#ifdef MPE_EMULATOR_ASSERTIONS
#define MPE_EMULATOR_ASSERT(condition) assert(condition)
#else
#define MPE_EMULATOR_ASSERT(condition)
#endif

#define MPE_EMULATOR_ASSERT_NOT_REACHED() MPE_EMULATOR_ASSERT(false)


#define MPE_EMULATOR_TO_STRING(x) _MPE_EMULATOR_EXPAND(x)
#define _MPE_EMULATOR_EXPAND(x) #x


#if defined(__GNUC__) || defined(__clang__)
  #define MPE_EMULATOR_LIKELY(condition) __builtin_expect((condition), 1)
  #define MPE_EMULATOR_UNLIKELY(condition) __builtin_expect((condition), 0)
#else
  #define MPE_EMULATOR_LIKELY(condition) (condition)
  #define MPE_EMULATOR_UNLIKELY(condition) (condition)
#endif


/*
Mark compile-time polymorphism. (E.g. Midi::EventHandler.)
*/
#define MPE_EMULATOR_OVERRIDE


namespace MpeEmulator
{

namespace Constants {
    constexpr char const* COMPANY_NAME = "Attila M. Magyar";
    constexpr char const* COMPANY_WEB = "https://github.com/attilammagyar/mpe-emulator";
    constexpr char const* COMPANY_EMAIL = "";

    constexpr char const* PLUGIN_NAME = "MPE Emulator";
    constexpr char const* PLUGIN_VERSION_STR = MPE_EMULATOR_TO_STRING(MPE_EMULATOR_VERSION_STR);
    constexpr int PLUGIN_VERSION_INT = MPE_EMULATOR_VERSION_INT;
}

}

#endif
