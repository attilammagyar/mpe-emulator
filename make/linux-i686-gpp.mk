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

LIB_PATH ?= $(BUILD_DIR)/lib32
SYS_LIB_PATH ?= /usr/lib/i386-linux-gnu
SUFFIX = x86
OBJ_GUI_BFDNAME = elf32-i386
OBJ_GUI_BFDARCH = i386
ARCH_CXXFLAGS = -m32
ARCH_LFLAGS = -m32 -L$(LIB_PATH)

include make/linux-gpp.mk
