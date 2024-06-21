###############################################################################
# This file is part of MPE Emulator.
# Copyright (C) 2023, 2024  Attila M. Magyar
# Copyright (C) 2023  @aimixsaka (https://github.com/aimixsaka/)
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

INSTRUCTION_SET ?= sse2

SUFFIX = x86

include make/linux-mingw.mk
