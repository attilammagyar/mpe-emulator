/*
 * This file is part of MPE Emulator.
 * Copyright (C) 2023, 2024, 2026  Attila M. Magyar
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


void assert_applied_gui_size_constraints(
        GUI& gui,
        int const expected_width,
        int const expected_height,
        int const new_width,
        int const new_height
) {
    double scale;
    int width = new_width;
    int height = new_height;

    gui.apply_size_constraints(width, height, scale);

    assert_eq((double)expected_width, (double)width, 1.0);
    assert_eq((double)expected_height, (double)height, 1.0);

    gui.resize(new_width, new_height);
    assert_eq((double)expected_width, (double)gui.get_width(), 1.0);
    assert_eq((double)expected_height, (double)gui.get_height(), 1.0);
}


TEST(gui_can_apply_size_constraints, {
    constexpr int max_w = GUI::MAX_WIDTH;
    constexpr int max_h = GUI::MAX_HEIGHT;
    constexpr int min_w = GUI::MIN_WIDTH;
    constexpr int min_h = GUI::MIN_HEIGHT;
    constexpr int half_w = GUI::WIDTH / 2;
    constexpr int half_h = GUI::HEIGHT / 2;
    Proxy proxy;
    GUI gui(NULL, NULL, NULL, proxy, false);

    assert_applied_gui_size_constraints(gui, max_w, max_h, 999999, 999999);

    assert_applied_gui_size_constraints(gui, min_w, min_h, 1, 1);
    assert_applied_gui_size_constraints(gui, min_w, min_h, 999999, 1);
    assert_applied_gui_size_constraints(gui, min_w, min_h, 1, 999999);

    assert_applied_gui_size_constraints(gui, half_w, half_h, half_w, half_h);

    assert_applied_gui_size_constraints(
        gui, half_w, half_h, half_w + 100, half_h
    );
    assert_applied_gui_size_constraints(
        gui, half_w, half_h, half_w, half_h + 100
    );

    assert_applied_gui_size_constraints(gui, half_w, half_h, half_w, 999999);
    assert_applied_gui_size_constraints(gui, half_w, half_h, 999999, half_h);
})
