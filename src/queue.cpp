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

#ifndef MPE_EMULATOR__QUEUE_CPP
#define MPE_EMULATOR__QUEUE_CPP

#include "queue.hpp"


namespace MpeEmulator
{

template<class Item, size_t capacity>
Queue<Item, capacity>::Queue() noexcept
    : next_push(0),
    next_pop(0)
{
}


template<class Item, size_t capacity>
bool Queue<Item, capacity>::is_empty() const noexcept
{
    return next_push == next_pop;
}


template<class Item, size_t capacity>
bool Queue<Item, capacity>::push(Item const& item) noexcept
{
    size_t const old_next_push = next_push;
    size_t const new_next_push = advance(old_next_push);

    if (next_pop == new_next_push) {
        return false;
    }

    items[old_next_push] = item;
    next_push = new_next_push;

    return true;
}


template<class Item, size_t capacity>
size_t Queue<Item, capacity>::advance(size_t const index) const noexcept
{
    size_t const new_index = index + 1;

    return new_index >= ITEMS_SIZE ? new_index - ITEMS_SIZE : new_index;
}


template<class Item, size_t capacity>
Item& Queue<Item, capacity>::pop() noexcept
{
    Item& item = items[next_pop];

    next_pop = advance(next_pop);

    return item;
}


template<class Item, size_t capacity>
size_t Queue<Item, capacity>::length() const noexcept
{
    return next_push - next_pop;
}


template<class Item, size_t capacity>
void Queue<Item, capacity>::clear() noexcept
{
    next_push = 0;
    next_pop = 0;
}

}

#endif
