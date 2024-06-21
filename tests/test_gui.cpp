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

#include "test.cpp"

#include <cstddef>

#include "common.hpp"

#include "serializer.hpp"
#include "proxy.hpp"

#include "gui/gui.hpp"


using namespace MpeEmulator;


TEST(clamp_ratio, {
    assert_eq(0.0, GUI::clamp_ratio(-0.1), 0.000001);
    assert_eq(0.0, GUI::clamp_ratio(-0.0), 0.000001);
    assert_eq(0.0, GUI::clamp_ratio(0.0), 0.000001);
    assert_eq(0.1, GUI::clamp_ratio(0.1), 0.000001);
    assert_eq(1.0, GUI::clamp_ratio(1.0), 0.000001);
    assert_eq(1.0, GUI::clamp_ratio(1.1), 0.000001);
})


TEST(gui_initialization, {
    Proxy proxy;

    GUI gui(NULL, NULL, NULL, proxy, false);
    gui.show();
})
