#!/bin/bash

###############################################################################
# This file is part of MPE Emulator.
# Copyright (C) 2023, 2024  Attila M. Magyar
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

DOWNLOAD_URL="https://github.com/attilammagyar/mpe-emulator/releases/download"

main()
{
    local instruction_set
    local os
    local arch
    local plugin_type

    for instruction_set in "sse2"
    do
        find dist -name "mpe-emulator-*-$instruction_set-vst3_bundle.zip" \
          | while read
            do
                print_link "$REPLY" "VST 3 Bundle"
            done

        find dist -name "mpe-emulator-*-$instruction_set-*.zip" \
          | grep -v "mpe-emulator-.*-src\\.zip" \
          | grep -v "mpe-emulator-.*-vst3_bundle\\.zip" \
          | sort -r \
          | while read
            do
                arch="$(get_arch "$REPLY")"
                plugin_type="$(get_plugin_type "$REPLY")"
                print_link "$REPLY" "$arch-bit, $plugin_type"
            done
    done

    find dist -name "mpe-emulator-*-src.zip" \
      | while read
        do
            print_link "$REPLY" "Source"
        done
}

get_os()
{
    local file_name="$1"

    printf "%s\n" "$file_name" \
        | cut -d"-" -f4 \
        | uppercase_first
}

uppercase_first()
{
    local first
    local rest

    read

    first="$(printf "%s\n" "${REPLY:0:1}" | tr [[:lower:]] [[:upper:]])"
    rest="${REPLY:1}"

    printf "%s%s\n" "$first" "$rest"
}

get_arch()
{
    local file_name="$1"

    if [[ "$file_name" =~ x86_64 ]]
    then
        echo "64"
        return
    fi

    echo "32"
}

get_plugin_type()
{
    local file_name="$1"

    printf "%s\n" "$file_name" \
        | cut -d"-" -f7 \
        | cut -d"." -f1 \
        | sed "s/fst/FST (VST 2.4)/ ; s/vst3_single_file/VST 3 Single File/"
}

print_link()
{
    local file_name="$1"
    local description="$2"
    local size="$(get_size "$file_name")"
    local zip="$(basename "$file_name")"
    local version="$(get_version "$file_name")"
    local uri="$DOWNLOAD_URL/v$version/$zip"

    cat <<HTML
            <li>
              $description: <a href="$uri">Download ($size)</a>
            </li>
HTML
}

get_version()
{
    local file_name="$1"

    printf "v%s\n" "$file_name" \
        | cut -d "-" -f 3 \
        | sed "s/_/./g"
}

get_size()
{
    local file_name="$1"

    du -hs "$file_name" \
        | cut -f 1
}

main "$@"
