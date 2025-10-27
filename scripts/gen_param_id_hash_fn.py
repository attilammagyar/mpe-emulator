###############################################################################
# This file is part of MPE Emulator.
# Copyright (C) 2023, 2024, 2025  Attila M. Magyar
#
# MPE Emulator is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# MPE Emulator is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
###############################################################################

import random
import sys
import time


# Inspiration from https://orlp.net/blog/worlds-smallest-hash-table/


LETTER_OFFSET = ord("A") - 10   # 0x41 - 10
DIGIT_OFFSET = ord("0")         # 0x30

params = [
    "MCM",
    "Z1TYP",
    "Z1CHN",
    "Z1ENH",
    "Z1ANC",
    "Z1ORV",
    "Z1R1IN",
    "Z1R1OU",
    "Z1R1IV",
    "Z1R1TR",
    "Z1R1DT",
    "Z1R1DL",
    "Z1R1MP",
    "Z1R1RS",
    "Z1R1NV",
    "Z1R2IN",
    "Z1R2OU",
    "Z1R2IV",
    "Z1R2TR",
    "Z1R2DT",
    "Z1R2DL",
    "Z1R2MP",
    "Z1R2RS",
    "Z1R2NV",
    "Z1R3IN",
    "Z1R3OU",
    "Z1R3IV",
    "Z1R3TR",
    "Z1R3DT",
    "Z1R3DL",
    "Z1R3MP",
    "Z1R3RS",
    "Z1R3NV",
    "Z1R4IN",
    "Z1R4OU",
    "Z1R4IV",
    "Z1R4TR",
    "Z1R4DT",
    "Z1R4DL",
    "Z1R4MP",
    "Z1R4RS",
    "Z1R4NV",
    "Z1R5IN",
    "Z1R5OU",
    "Z1R5IV",
    "Z1R5TR",
    "Z1R5DT",
    "Z1R5DL",
    "Z1R5MP",
    "Z1R5RS",
    "Z1R5NV",
    "Z1R6IN",
    "Z1R6OU",
    "Z1R6IV",
    "Z1R6TR",
    "Z1R6DT",
    "Z1R6DL",
    "Z1R6MP",
    "Z1R6RS",
    "Z1R6NV",
    "Z1R7IN",
    "Z1R7OU",
    "Z1R7IV",
    "Z1R7TR",
    "Z1R7DT",
    "Z1R7DL",
    "Z1R7MP",
    "Z1R7RS",
    "Z1R7NV",
    "Z1R8IN",
    "Z1R8OU",
    "Z1R8IV",
    "Z1R8TR",
    "Z1R8DT",
    "Z1R8DL",
    "Z1R8MP",
    "Z1R8RS",
    "Z1R8NV",
    "Z1R9IN",
    "Z1R9OU",
    "Z1R9IV",
    "Z1R9TR",
    "Z1R9DT",
    "Z1R9DL",
    "Z1R9MP",
    "Z1R9RS",
    "Z1R9NV",
    "Z1TRA",
    "Z1TRB",
    "Z1SUS",
]


def main(argv):
    # for name in params:
        # h = compute_hash(name, 23781, 9, 128)
        # print(
            # f"assert_eq(Synth::ParamId::{name}, synth.find_param_id(\"{name}\"));"
        # )

    # return 0

    best_utilized = 0
    best_mod = 2 ** 31
    best_max_coll = 2 ** 31
    best_avg_len = 2 ** 31
    start = time.time()
    delta = 0

    for i in range(1, 100000):
        # multiplier = random.randrange(2 ** 15)
        multiplier = i
        multiplier = multiplier * 2 + 1

        for shift in range(23):
            # for mod in range(220, 290):
            for mod in [128]:
                hashes = {}

                for name in params:
                    h = compute_hash(name, multiplier, shift, mod)
                    hashes.setdefault(h, []).append(name)

                max_coll = max(len(n) for n in hashes.values())
                avg_len = [len(n) for n in hashes.values()]
                avg_len = sum(avg_len) / len(avg_len)
                utilized = len(hashes.keys())

                if (
                    (max_coll < best_max_coll)
                    or (max_coll == best_max_coll and avg_len < best_avg_len)
                ):
                    delta_t = time.time() - start
                    best_utilized = utilized
                    best_max_coll = max_coll
                    best_avg_len = avg_len
                    best_mod = mod
                    print(
                        "\t".join([
                            f"time={delta_t}",
                            f"multiplier={multiplier}",
                            f"shift={shift}",
                            f"max_coll={max_coll}",
                            f"avg_len={avg_len}",
                            f"mod={mod}",
                            f"utilized={len(hashes)}"
                        ])
                    )


def compute_hash(text: str, multiplier: int, shift: int, mod: int) -> int:
    h = 0

    for i, c in enumerate(text[1:]):
        c = ord(c)

        if c >= LETTER_OFFSET:
            c -= LETTER_OFFSET
        else:
            c -= DIGIT_OFFSET

        h *= 36
        h += c

        if i == 4:
            break

    h <<= 3
    h += i
    h = (h * multiplier) >> shift
    h = h % mod

    return h


if __name__ == "__main__":
    sys.exit(main(sys.argv))
