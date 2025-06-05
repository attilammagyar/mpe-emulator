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

#ifndef MPE_EMULATOR__GUI_STUB_CPP
#define MPE_EMULATOR__GUI_STUB_CPP

#include <cstddef>
#include <string>

#include "common.hpp"
#include "proxy.hpp"

#include "gui/gui.hpp"


namespace MpeEmulator {

void GUI::idle()
{
}


void GUI::initialize()
{
}


void GUI::destroy()
{
}


class Widget : public WidgetBase
{
    public:
        explicit Widget(char const* const text) : WidgetBase(text)
        {
        }

        virtual ~Widget()
        {
            destroy_children();
        }

        virtual GUI::Image load_image(
                GUI::PlatformData platform_data,
                char const* const name
        ) override {
            return (GUI::Image)new DummyObject();
        }

        virtual void delete_image(GUI::Image image) override
        {
            delete (DummyObject*)image;
        }

    protected:
        Widget(
                char const* const text,
                int const left,
                int const top,
                int const width,
                int const height,
                Type const type
        ) : WidgetBase(text, left, top, width, height, type)
        {
        }

        Widget(
                GUI::PlatformData platform_data,
                GUI::PlatformWidget platform_widget,
                Type const type
        ) : WidgetBase(platform_data, platform_widget, type)
        {
        }

        GUI::Image copy_image_region(
                GUI::Image source,
                int const left,
                int const top,
                int const width,
                int const height
        ) {
            return (GUI::Image)new DummyObject();
        }

    private:
        class DummyObject
        {
            public:
                DummyObject() {
                    constexpr size_t size = 1024 * 1024;
                    constexpr size_t size_half = size / 2;

                    data.reserve(size);
                    data = "Dummy GUI object";

                    while (data.length() < size_half) {
                        data += data;
                    }
                }

                std::string data;
        };
};

}


#include "gui/widgets.hpp"
#include "gui/gui.cpp"


namespace MpeEmulator {

void ImportSettingsButton::click()
{
}


void ExportSettingsButton::click()
{
}

}


#include "gui/widgets.cpp"

#endif
