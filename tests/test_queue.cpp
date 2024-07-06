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

#include <cstddef>

#include "test.cpp"

#include "queue.cpp"


using namespace MpeEmulator;


constexpr size_t CAPACITY = 10;


class TestObj
{
    public:
        explicit TestObj(int const v = 0) : value(v)
        {
        }

        TestObj(TestObj const& o) = default;
        TestObj(TestObj&& o) = default;
        TestObj& operator=(TestObj const& o) = default;
        TestObj& operator=(TestObj&& o) = default;

        int value;
};


using TestObjQueue = Queue<TestObj, CAPACITY>;


TEST(newly_created_queue_is_empty, {
    TestObjQueue q;

    assert_true(q.is_empty());
    assert_eq(0, q.length());
})


TEST(pushed_item_can_be_popped, {
    TestObjQueue q;
    TestObj item(123);

    q.push(item);
    assert_false(q.is_empty());

    assert_eq(1, q.length());
    assert_eq(123, q.pop().value);
})


TEST(fifo, {
    TestObjQueue q;
    TestObj a(1);
    TestObj b(2);
    TestObj c(3);
    TestObj d(4);
    TestObj e(5);

    q.push(a);
    q.push(b);
    q.push(c);
    q.push(d);
    q.push(e);

    assert_eq(5, q.length());
    assert_eq(1, q.pop().value);

    assert_eq(4, q.length());
    assert_eq(2, q.pop().value);

    assert_eq(3, q.length());
    assert_eq(3, q.pop().value);

    assert_eq(2, q.length());
    assert_eq(4, q.pop().value);

    assert_eq(1, q.length());
    assert_eq(5, q.pop().value);

    assert_eq(0, q.length());
    assert_true(q.is_empty());
})


TEST(can_be_filled_and_emptied_multiple_times, {
    TestObjQueue q;

    for (int i = 0; i != 3; ++i) {
        for (size_t j = 0; j != CAPACITY; ++j) {
            assert_true(q.push(TestObj((int)j)), "i=%d, j=%d", i, (int)j);
        }

        assert_false(q.is_empty(), "i=%d", i);

        for (size_t j = 0; j != CAPACITY; ++j) {
            assert_eq((int)j, q.pop().value, "i=%d", i);
        }

        assert_true(q.is_empty(), "i=%d", i);
    }
})


TEST(when_queue_is_full_then_push_fails, {
    TestObjQueue q;

    for (size_t i = 0; i != CAPACITY; ++i) {
        assert_true(q.push(TestObj((int)i)), "i=%d", (int)i);
    }

    assert_false(q.push(TestObj(123)));
})


TEST(queue_can_be_emptied, {
    TestObjQueue q;

    q.push(TestObj(10));
    q.push(TestObj(20));
    q.push(TestObj(30));

    q.clear();

    assert_eq(0, q.length());
    assert_true(q.is_empty());

    assert_true(q.push(TestObj(123)));
})
