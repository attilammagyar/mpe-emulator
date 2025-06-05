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

TARGET_OS = linux
DIR_SEP = /

FST = $(FST_DIR)/mpe-emulator.so
FST_MAIN_SOURCES = src/plugin/fst/so.cpp
FST_EXTRA =

VST3 = $(VST3_DIR)/mpe-emulator.vst3
VST3_MAIN_SOURCES = src/plugin/vst3/so.cpp
VST3_GUI_PLATFORM = kPlatformTypeX11EmbedWindowID
VST3_EXTRA =
VST3_PLUGIN_SOURCES = \
	src/plugin/vst3/plugin.cpp \
	src/plugin/vst3/plugin-xcb.cpp

VST3_MODULE_INFO_TOOL = $(BUILD_DIR)$(DIR_SEP)vst3_module_info_tool
VST3_MODULE_INFO_LFLAGS = -pthread -Wl,--no-as-needed -ldl

DEV_PLATFORM_CLEAN = $(TEX_ARTIFACTS) $(VST3_MODULE_INFO_TOOL)

.PHONY: vst3moduleinfo

vst3moduleinfo: $(VST3_MODULE_INFO_TOOL)

$(VST3_MODULE_INFO_TOOL): src/plugin/vst3/moduleinfo.cpp | $(BUILD_DIR)
	$(CPP_TARGET_PLATFORM) \
		$(MPE_EMULATOR_CXXINCS) $(VST3_CXXINCS) $(VST3_CXXFLAGS) $(MPE_EMULATOR_CXXFLAGS) \
		$(VST3_MODULE_INFO_LFLAGS) \
		$< -o $@

GUI_PLAYGROUND = $(BUILD_DIR)/gui-playground-$(SUFFIX)
GUI_PLAYGROUND_SOURCES = src/gui/xcb-playground.cpp
GUI_TARGET_PLATFORM_HEADERS = src/gui/xcb.hpp
GUI_TARGET_PLATFORM_SOURCES = src/gui/xcb.cpp

OBJ_TARGET_GUI_EXTRA = \
	$(LIB_PATH)/libcairo.so \
	$(LIB_PATH)/libxcb.so \
	$(LIB_PATH)/libxcb-render.so \
	$(BUILD_DIR)/img_about.o \
	$(BUILD_DIR)/img_distortions.o \
	$(BUILD_DIR)/img_midpoint_states.o \
	$(BUILD_DIR)/img_knob_states.o \
	$(BUILD_DIR)/img_rocker_switch.o \
	$(BUILD_DIR)/img_vst_logo.o \
	$(BUILD_DIR)/img_zone1.o

$(BUILD_DIR)/img_%.o: gui/img/%.png | $(BUILD_DIR)
	/usr/bin/objcopy \
		--input binary \
		--output $(OBJ_GUI_BFDNAME) \
		--binary-architecture $(OBJ_GUI_BFDARCH) \
		$< $@

$(LIB_PATH): | $(BUILD_DIR)
	$(MKDIR) $@

$(LIB_PATH)/libcairo.so: $(SYS_LIB_PATH)/libcairo.so.2 | $(LIB_PATH)
	ln -vs $< $@

$(LIB_PATH)/libxcb.so: $(SYS_LIB_PATH)/libxcb.so.1 | $(LIB_PATH)
	ln -vs $< $@

$(LIB_PATH)/libxcb-render.so: $(SYS_LIB_PATH)/libxcb-render.so.0 | $(LIB_PATH)
	ln -vs $< $@

# DEBUG_LOG ?= STDERR
# DEBUG_LOG ?= /tmp/debug.txt
DEBUG_LOG ?=

EXE =

CPP_TARGET_PLATFORM ?= /usr/bin/g++

LINK_SO = $(CPP_TARGET_PLATFORM) -Wall -shared
LINK_TARGET_EXE = $(CPP_TARGET_PLATFORM) -Wall

TARGET_PLATFORM_LFLAGS = \
	$(ARCH_LFLAGS) \
	-lcairo \
	-lxcb \
	-lxcb-render

LINK_FST = $(LINK_SO)
LINK_VST3 = $(LINK_SO)

TARGET_PLATFORM_CXXFLAGS = \
	$(ARCH_CXXFLAGS) \
	-Wno-cpp \
	-fPIC \
	-fvisibility=hidden

TARGET_PLATFORM_CXXINCS = -I/usr/include

include make/linux.mk

show_versions:
	@echo ___ Target platform compiler:
	$(CPP_TARGET_PLATFORM) --version
	@echo ___ Dev platform compiler:
	$(CPP_DEV_PLATFORM) --version
	@echo ___ Cppcheck:
	$(CPPCHECK) --version
