#!/bin/bash

###############################################################################
# This file is part of MPE Emulator.
# Copyright (C) 2025  Attila M. Magyar
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

main()
{
    local mod_year

    find Makefile gui jsfx make scripts src tests -type f \
      | grep -Eiv '\.(bmp|def|flac|png|rc|rpp|tpl|xcf)$' \
      | while read
        do
            mod_year="$(git log -1 --format="%ci" -- "$REPLY" | cut -d"-" -f1)"
            head -n30 -- "$REPLY" \
              | grep "Copyright.*$mod_year" >/dev/null || printf "%s\n" "$REPLY"
        done
}


main "$@"
exit $?
