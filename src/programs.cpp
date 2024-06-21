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

#ifndef MPE_EMULATOR__PROGRAMS_CPP
#define MPE_EMULATOR__PROGRAMS_CPP

#include "bank.hpp"


namespace MpeEmulator
{

size_t const Bank::NUMBER_OF_BUILT_IN_PROGRAMS = 1;

Bank::Program const Bank::BUILT_IN_PROGRAMS[] = {
    Program(
        "Default",
        "Prog001",
        (
            "[mpeemulator]\n"
        )
    ),
};

}

#endif

