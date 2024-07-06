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

#ifndef MPE_EMULATOR__QUEUE_HPP
#define MPE_EMULATOR__QUEUE_HPP

#include <cstddef>


namespace MpeEmulator
{

/**
 * \brief A fixed size FIFO container where operations run in constant time.
 */
template<class Item, size_t capacity>
class Queue
{
    public:
        Queue() noexcept;

        bool is_empty() const noexcept;
        bool push(Item const& item) noexcept;
        Item& pop() noexcept;
        size_t length() const noexcept;
        void clear() noexcept;

    private:
        static constexpr size_t ITEMS_SIZE = capacity + 1;

        size_t advance(size_t const index) const noexcept;

        Item items[ITEMS_SIZE];

        size_t next_push;
        size_t next_pop;
};

}

#endif
