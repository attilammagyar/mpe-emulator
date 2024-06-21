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

#ifndef MPE_EMULATOR__SPSCQUEUE_HPP
#define MPE_EMULATOR__SPSCQUEUE_HPP

#include <atomic>
#include <vector>


namespace MpeEmulator
{

/*
See Timur Doumler [ACCU 2017]: Lock-free programming with modern C++
  https://www.youtube.com/watch?v=qdrp6k4rcP4
*/

/**
 * \brief A lockless, waitless FIFO container for a single producer thread and a
 *        single consumer thread.
 */
template<class ItemClass>
class SPSCQueue
{
    public:
        typedef typename std::vector<ItemClass>::size_type SizeType;

        explicit SPSCQueue(SizeType const capacity) noexcept;

        SPSCQueue(SPSCQueue<ItemClass> const& queue) = delete;

        bool is_empty() const noexcept;
        SizeType length() const noexcept;
        bool is_lock_free() const noexcept;
        bool push(ItemClass const& item) noexcept;
        bool pop(ItemClass& item) noexcept;

    private:
        SizeType advance(SizeType const index) const noexcept;

        SizeType const capacity;

        std::vector<ItemClass> items;

        std::atomic<SizeType> next_push;
        std::atomic<SizeType> next_pop;
};

}

#endif
