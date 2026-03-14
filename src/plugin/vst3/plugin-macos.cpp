/*
 * This file is part of MPE Emulator
 * Copyright (C) 2026  Attila M. Magyar
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

#include "plugin/vst3/plugin.hpp"


using namespace Steinberg;


namespace MpeEmulator
{

void Vst3Plugin::GUI::initialize()
{
    gui = new MpeEmulator::GUI(
        kVstVersionString,
        NULL,
        (MpeEmulator::GUI::PlatformWidget)systemWindow,
        proxy,
        true,
        this
    );
    gui->show();

    this->plugFrame->resizeView(this, &gui_size);
}


void Vst3Plugin::GUI::removedFromParent()
{
    if (gui != NULL) {
        delete gui;

        gui = NULL;
    }
}

}
