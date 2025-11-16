###############################################################################
# This file is part of MPE Emulator.
# Copyright (C) 2023, 2024, 2025  Attila M. Magyar
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

DEV_OS ?= linux
TARGET_PLATFORM ?= x86_64-gpp
VERSION_STR ?= dev
VERSION_INT ?= 999000
VERSION_AS_FILE_NAME ?= dev

BUILD_DIR_BASE ?= build
BUILD_DIR = $(BUILD_DIR_BASE)$(DIR_SEP)$(TARGET_PLATFORM)-$(SUFFIX)-$(INSTRUCTION_SET)
DEV_DIR = $(BUILD_DIR_BASE)$(DIR_SEP)dev-$(DEV_OS)-$(SUFFIX)-$(INSTRUCTION_SET)
DIST_DIR_BASE ?= dist
DIST_DIR_PREFIX ?= $(DIST_DIR_BASE)$(DIR_SEP)mpe-emulator-$(VERSION_AS_FILE_NAME)-$(TARGET_OS)-$(SUFFIX)-$(INSTRUCTION_SET)
API_DOC_DIR ?= doc

TEST_MAX_ARRAY_PRINT ?= 20

MPE_EMULATOR_CXXINCS = \
	-I./lib \
	-I./src

FST_CXXINCS = $(MPE_EMULATOR_CXXINCS)

VST3_CXXINCS = \
	-I./lib/vst3sdk \
	$(MPE_EMULATOR_CXXINCS)

MPE_EMULATOR_CXXFLAGS = \
	-D MPE_EMULATOR_VERSION_STR=$(VERSION_STR) \
	-D MPE_EMULATOR_VERSION_INT=$(VERSION_INT) \
	-D MPE_EMULATOR_TARGET_PLATFORM=$(TARGET_PLATFORM) \
	-D MPE_EMULATOR_INSTRUCTION_SET=$(INSTRUCTION_SET) \
	-Wall \
	-Werror \
	-ffast-math \
	-O3 \
	-std=c++17

ifneq ($(INSTRUCTION_SET),none)
MPE_EMULATOR_CXXFLAGS += -m$(INSTRUCTION_SET)
endif

ifeq ($(DEBUG_LOG),)
DEBUG_LOG_CXXFLAGS =
else
DEBUG_LOG_CXXFLAGS = -D MPE_EMULATOR_DEBUG_LOG=$(DEBUG_LOG)
endif

FST_DIR = $(DIST_DIR_PREFIX)-fst
VST3_DIR = $(DIST_DIR_PREFIX)-vst3_single_file

VSTXML = $(DIST_DIR_BASE)/mpe-emulator.vstxml
VSTXMLGEN = $(DEV_DIR)/vstxmlgen$(DEV_EXE)

UPGRADE_SETTINGS = $(DEV_DIR)/upgrade-settings$(DEV_EXE)

.PHONY: \
	all \
	check \
	check_proxy \
	clean \
	dirs \
	docs \
	fst \
	gui_playground \
	show_fst_dir \
	show_versions \
	show_vst3_dir \
	static_analysis \
	test_example \
	tests \
	upgrade_settings \
	vst3

all: dirs fst vst3

include make/$(DEV_OS)-$(TARGET_PLATFORM).mk

OBJ_TARGET_FST_MAIN = $(BUILD_DIR)/fst-main.o
OBJ_TARGET_FST_PLUGIN = $(BUILD_DIR)/fst-plugin.o
OBJ_TARGET_VST3_MAIN = $(BUILD_DIR)/vst3-main.o
OBJ_TARGET_VST3_PLUGIN = $(BUILD_DIR)/vst3-plugin.o
OBJ_TARGET_BANK = $(BUILD_DIR)/bank.o
OBJ_TARGET_SERIALIZER = $(BUILD_DIR)/serializer.o
OBJ_TARGET_PROXY = $(BUILD_DIR)/proxy.o
OBJ_TARGET_GUI = $(BUILD_DIR)/gui.o
OBJ_TARGET_STRINGS = $(BUILD_DIR)/strings.o

OBJ_TARGET_GUI_PLAYGROUND = $(BUILD_DIR)/gui-playground.o

OBJ_DEV_FST_PLUGIN = $(DEV_DIR)/fst-plugin.o
OBJ_DEV_BANK = $(DEV_DIR)/bank.o
OBJ_DEV_GUI_STUB = $(DEV_DIR)/gui-stub.o
OBJ_DEV_SERIALIZER = $(DEV_DIR)/serializer.o
OBJ_DEV_STRINGS = $(DEV_DIR)/strings.o
OBJ_DEV_PROXY = $(DEV_DIR)/proxy.o
OBJ_DEV_UPGRADE_SETTINGS = $(DEV_DIR)/upgrade-settings.o
OBJ_DEV_VSTXMLGEN = $(DEV_DIR)/vstxmlgen.o

OBJ_DEV_TEST_BANK = $(DEV_DIR)/test_bank.o
OBJ_DEV_TEST_GUI = $(DEV_DIR)/test_gui.o
OBJ_DEV_TEST_SERIALIZER = $(DEV_DIR)/test_serializer.o
OBJ_DEV_TEST_STRINGS = $(DEV_DIR)/test_strings.o

TEST_OBJS = \
	$(OBJ_DEV_BANK) \
	$(OBJ_DEV_GUI_STUB) \
	$(OBJ_DEV_SERIALIZER) \
	$(OBJ_DEV_STRINGS) \
	$(OBJ_DEV_PROXY) \
	$(OBJ_DEV_TEST_BANK) \
	$(OBJ_DEV_TEST_GUI) \
	$(OBJ_DEV_TEST_SERIALIZER) \
	$(OBJ_DEV_TEST_STRINGS)

FST_OBJS = \
	$(OBJ_TARGET_GUI_EXTRA) \
	$(OBJ_TARGET_FST_MAIN) \
	$(OBJ_TARGET_FST_PLUGIN) \
	$(OBJ_TARGET_GUI) \
	$(OBJ_TARGET_BANK) \
	$(OBJ_TARGET_SERIALIZER) \
	$(OBJ_TARGET_STRINGS) \
	$(OBJ_TARGET_PROXY)

VST3_OBJS = \
	$(OBJ_TARGET_GUI_EXTRA) \
	$(OBJ_TARGET_VST3_MAIN) \
	$(OBJ_TARGET_VST3_PLUGIN) \
	$(OBJ_TARGET_GUI) \
	$(OBJ_TARGET_SERIALIZER) \
	$(OBJ_TARGET_STRINGS) \
	$(OBJ_TARGET_PROXY)

GUI_PLAYGROUND_OBJS = \
	$(OBJ_TARGET_GUI_EXTRA) \
	$(OBJ_TARGET_GUI) \
	$(OBJ_TARGET_GUI_PLAYGROUND) \
	$(OBJ_TARGET_SERIALIZER) \
	$(OBJ_TARGET_STRINGS) \
	$(OBJ_TARGET_PROXY)

UPGRADE_SETTINGS_OBJS = \
	$(OBJ_DEV_UPGRADE_SETTINGS) \
	$(OBJ_DEV_PROXY) \
	$(OBJ_DEV_SERIALIZER)

VSTXMLGEN_OBJS = \
	$(OBJ_DEV_PROXY) \
	$(OBJ_DEV_SERIALIZER) \
	$(OBJ_DEV_STRINGS) \
	$(OBJ_DEV_BANK) \
	$(OBJ_DEV_GUI_STUB) \
	$(OBJ_DEV_FST_PLUGIN) \
	$(OBJ_DEV_VSTXMLGEN)

PROXY_COMPONENTS = \
	proxy \
	note_stack \
	queue \
	spscqueue

TESTS_PROXY = \
	test_math \
	test_note_stack \
	test_queue \
	test_spscqueue \
	test_proxy

TESTS = \
	$(TESTS_PROXY) \
	test_gui \
	test_bank \
	test_midi \
	test_serializer \
	test_strings

PROXY_HEADERS = \
	src/debug.hpp \
	src/common.hpp \
	src/midi.hpp \
	$(foreach COMPONENT,$(PROXY_COMPONENTS),src/$(COMPONENT).hpp)

PROXY_SOURCES = \
	$(foreach COMPONENT,$(PROXY_COMPONENTS),src/$(COMPONENT).cpp)

BANK_HEADERS = \
	src/bank.hpp \
	src/serializer.hpp

BANK_SOURCES = \
	src/bank.cpp \
	src/programs.cpp

MAIN_HEADERS = \
	src/gui/gui.hpp \
	src/gui/widgets.hpp \
	src/strings.hpp \
	$(BANK_HEADERS) \
	$(PROXY_HEADERS)

MAIN_SOURCES = \
	src/gui/gui.cpp \
	src/gui/widgets.cpp \
	src/bank.cpp \
	src/programs.cpp \
	src/serializer.cpp \
	src/strings.cpp \
	$(PROXY_SOURCES)

FST_HEADERS = \
	$(MAIN_HEADERS) \
	src/plugin/fst/plugin.hpp

FST_SOURCES = \
	src/plugin/fst/plugin.cpp \
	$(FST_MAIN_SOURCES) \
	$(MAIN_SOURCES)

VSTXMLGEN_SOURCES = src/plugin/fst/vstxmlgen.cpp

VST3_HEADERS = \
	$(MAIN_HEADERS) \
	src/plugin/vst3/plugin.hpp

VST3_SOURCES = \
	src/plugin/vst3/plugin.cpp \
	$(VST3_MAIN_SOURCES) \
	$(MAIN_SOURCES)

GUI_COMMON_HEADERS = $(MAIN_HEADERS)

GUI_COMMON_SOURCES = \
	src/gui/widgets.cpp \
	src/gui/gui.cpp

GUI_HEADERS = \
	$(GUI_TARGET_PLATFORM_HEADERS) \
	$(MAIN_HEADERS)

GUI_SOURCES = \
	$(GUI_TARGET_PLATFORM_SOURCES) \
	$(GUI_COMMON_SOURCES)

GUI_STUB_HEADERS = \
	$(GUI_COMMON_HEADERS)

GUI_STUB_SOURCES = \
	src/gui/stub.cpp \
	$(GUI_COMMON_SOURCES)

UPGRADE_SETTINGS_SOURCES = src/upgrade_settings.cpp

CPPCHECK_DONE = $(BUILD_DIR)/cppcheck-done.txt

TEST_LIBS = \
	tests/test.cpp

TEST_CPPS = $(foreach TEST,$(TESTS),tests/$(TEST).cpp)
TEST_BINS = $(foreach TEST,$(TESTS),$(DEV_DIR)/$(TEST)$(DEV_EXE))
TEST_PROXY_BINS = $(foreach TEST,$(TESTS_PROXY),$(DEV_DIR)/$(TEST)$(DEV_EXE))

PERF_TEST_BINS = $(foreach TEST,$(PERF_TESTS),$(DEV_DIR)/$(TEST)$(DEV_EXE))

TEST_CXXFLAGS = \
	-D TEST_MAX_ARRAY_PRINT=$(TEST_MAX_ARRAY_PRINT) \
	-D MPE_EMULATOR_ASSERTIONS=1 \
	-I./tests \
	-g

FST_CXXFLAGS = \
	-DFST_DONT_DEPRECATE_UNKNOWN \
	$(MPE_EMULATOR_CXXFLAGS)

VST3_CXXFLAGS = \
	-DRELEASE \
	-DMPE_EMULATOR_VST3_GUI_PLATFORM=$(VST3_GUI_PLATFORM) \
	$(MPE_EMULATOR_CXXFLAGS) \
	-Wno-class-memaccess \
	-Wno-error=deprecated-declarations \
	-Wno-format \
	-Wno-multichar \
	-Wno-parentheses \
	-Wno-pragmas \
	-Wno-unknown-pragmas \
	-Wno-unused-value

COMPILE_TARGET = \
	$(CPP_TARGET_PLATFORM) \
		$(TARGET_PLATFORM_CXXINCS) \
		$(MPE_EMULATOR_CXXINCS) $(MPE_EMULATOR_CXXFLAGS) \
		$(TARGET_PLATFORM_CXXFLAGS) \
		$(DEBUG_LOG_CXXFLAGS)

COMPILE_FST = \
	$(CPP_TARGET_PLATFORM) \
		$(TARGET_PLATFORM_CXXINCS) \
		$(FST_CXXINCS) $(FST_CXXFLAGS) \
		$(TARGET_PLATFORM_CXXFLAGS) \
		$(DEBUG_LOG_CXXFLAGS)

COMPILE_VST3 = \
	$(CPP_TARGET_PLATFORM) \
		$(TARGET_PLATFORM_CXXINCS) \
		$(VST3_CXXINCS) $(VST3_CXXFLAGS) \
		$(TARGET_PLATFORM_CXXFLAGS) \
		$(DEBUG_LOG_CXXFLAGS)

COMPILE_DEV = \
	$(CPP_DEV_PLATFORM) \
		$(MPE_EMULATOR_CXXINCS) $(MPE_EMULATOR_CXXFLAGS) \
		$(TEST_CXXFLAGS) \
		$(DEBUG_LOG_CXXFLAGS)

RUN_WITH_VALGRIND = $(VALGRIND) $(VALGRIND_FLAGS)

show_fst_dir:
	@echo $(FST_DIR)

show_vst3_dir:
	@echo $(VST3_DIR)

fst: $(FST) $(VSTXML)

vst3: $(VST3)

vstxml: $(VSTXML)

dirs: $(BUILD_DIR) $(DEV_DIR) $(API_DOC_DIR) $(FST_DIR) $(VST3_DIR)

$(BUILD_DIR) $(DEV_DIR): | $(BUILD_DIR_BASE)
	$(MKDIR) $@

$(BUILD_DIR_BASE) $(DIST_DIR_BASE) $(API_DOC_DIR):
	$(MKDIR) $@

clean:
	$(RM) \
		$(CPPCHECK_DONE) \
		$(DEV_PLATFORM_CLEAN) \
		$(FST) \
		$(FST_OBJS) \
		$(GUI_PLAYGROUND) \
		$(GUI_PLAYGROUND_OBJS) \
		$(TEST_BINS) \
		$(TEST_OBJS) \
		$(UPGRADE_SETTINGS) \
		$(UPGRADE_SETTINGS_OBJS) \
		$(VST3) \
		$(VST3_OBJS) \
		$(VSTXML) \
		$(VSTXMLGEN) \
		$(VSTXMLGEN_OBJS)
	$(RM) $(API_DOC_DIR)/html/*.* $(API_DOC_DIR)/html/search/*.*

check: $(CPPCHECK_DONE) upgrade_settings tests | $(DEV_DIR)
check_proxy: $(TEST_LIBS) $(TEST_PROXY_BINS) | $(DEV_DIR)

tests: $(TEST_LIBS) $(TEST_BINS) | $(DEV_DIR)

test_example: $(DEV_DIR)/test_example$(DEV_EXE) | $(DEV_DIR)

docs: Doxyfile $(API_DOC_DIR) $(API_DOC_DIR)/html/index.html

gui_playground: $(GUI_PLAYGROUND)

static_analysis: $(CPPCHECK_DONE)

$(CPPCHECK_DONE): \
		$(FST_HEADERS) \
		$(FST_SOURCES) \
		$(GUI_HEADERS) \
		$(GUI_PLAYGROUND_SOURCES) \
		$(GUI_SOURCES) \
		$(MAIN_HEADERS) \
		$(MAIN_SOURCES) \
		$(UPGRADE_SETTINGS_SOURCES) \
		$(VST3_HEADERS) \
		$(VST3_SOURCES) \
		$(VSTXMLGEN_SOURCES) \
		$(TEST_CPPS) \
		$(TEST_LIBS) \
		| $(BUILD_DIR) show_versions
	$(CPPCHECK) $(CPPCHECK_FLAGS) src/ tests/
	echo > $@

upgrade_settings: $(UPGRADE_SETTINGS)

$(API_DOC_DIR)/html/index.html: \
		Doxyfile \
		$(MAIN_HEADERS) \
		$(MAIN_SOURCES) \
		$(TEST_LIBS) \
		$(GUI_HEADERS) \
		$(GUI_SOURCES) \
		$(FST_HEADERS) \
		$(FST_SOURCES) \
		$(VST3_HEADERS) \
		$(VST3_SOURCES) \
		| $(API_DOC_DIR)
	$(DOXYGEN)

$(FST): $(FST_EXTRA) $(FST_OBJS) | $(FST_DIR) show_versions
	$(LINK_FST) $^ -o $@ $(TARGET_PLATFORM_LFLAGS)

$(VST3): $(VST3_EXTRA) $(VST3_OBJS) | $(VST3_DIR) show_versions
	$(LINK_VST3) $^ -o $@ $(TARGET_PLATFORM_LFLAGS)

$(FST_DIR) $(VST3_DIR): | $(DIST_DIR_BASE)
	$(MKDIR) $@

$(GUI_PLAYGROUND): $(GUI_PLAYGROUND_OBJS) | $(BUILD_DIR) show_versions
	$(LINK_TARGET_EXE) $^ -o $@ $(TARGET_PLATFORM_LFLAGS)

$(OBJ_TARGET_GUI_PLAYGROUND): \
		$(GUI_PLAYGROUND_SOURCES) \
		$(GUI_SOURCES) $(GUI_HEADERS) \
		| $(BUILD_DIR)
	$(COMPILE_TARGET) -c -o $@ $<

$(UPGRADE_SETTINGS): $(UPGRADE_SETTINGS_OBJS) | $(DEV_DIR) show_versions
	$(LINK_DEV_EXE) $^ -o $@

$(OBJ_DEV_UPGRADE_SETTINGS): $(UPGRADE_SETTINGS_SOURCES) | $(DEV_DIR)
	$(COMPILE_DEV) -c -o $@ $<

$(OBJ_TARGET_PROXY): $(PROXY_SOURCES) $(PROXY_HEADERS) | $(BUILD_DIR)
	$(COMPILE_TARGET) -c -o $@ $<

$(OBJ_DEV_PROXY): $(PROXY_SOURCES) $(PROXY_HEADERS) | $(DEV_DIR)
	$(COMPILE_DEV) -c -o $@ $<

$(OBJ_TARGET_BANK): \
		$(BANK_SOURCES) $(BANK_HEADERS) $(PROXY_HEADERS) | $(BUILD_DIR)
	$(COMPILE_TARGET) -c -o $@ $<

$(OBJ_DEV_BANK): $(BANK_SOURCES) $(BANK_HEADERS) $(PROXY_HEADERS) | $(DEV_DIR)
	$(COMPILE_DEV) -c -o $@ $<

$(OBJ_TARGET_SERIALIZER): \
		src/serializer.cpp src/serializer.hpp $(PROXY_HEADERS) \
		| $(BUILD_DIR)
	$(COMPILE_TARGET) -c -o $@ $<

$(OBJ_DEV_SERIALIZER): \
		src/serializer.cpp src/serializer.hpp $(PROXY_HEADERS) \
		| $(DEV_DIR)
	$(COMPILE_DEV) -c -o $@ $<

$(OBJ_DEV_STRINGS): \
		src/strings.cpp src/strings.hpp $(PROXY_HEADERS) \
		| $(DEV_DIR)
	$(COMPILE_DEV) -c -o $@ $<

$(OBJ_TARGET_GUI): $(GUI_SOURCES) $(GUI_HEADERS) | $(BUILD_DIR)
	$(COMPILE_TARGET) -c -o $@ $<

$(OBJ_DEV_GUI_STUB): $(GUI_STUB_SOURCES) $(GUI_STUB_HEADERS) | $(DEV_DIR)
	$(COMPILE_DEV) -c -o $@ $<

$(OBJ_TARGET_STRINGS): \
		src/strings.cpp src/strings.hpp \
		$(PROXY_HEADERS) \
		| $(BUILD_DIR)
	$(COMPILE_TARGET) -c -o $@ $<

$(OBJ_TARGET_FST_PLUGIN): src/plugin/fst/plugin.cpp $(FST_HEADERS) | $(BUILD_DIR)
	$(COMPILE_FST) -c -o $@ $<

$(OBJ_DEV_FST_PLUGIN): src/plugin/fst/plugin.cpp $(FST_HEADERS) | $(DEV_DIR)
	$(CPP_DEV_PLATFORM) \
		$(FST_CXXINCS) $(FST_CXXFLAGS) $(DEBUG_LOG_CXXFLAGS) -c -o $@ $<

$(OBJ_TARGET_FST_MAIN): $(FST_MAIN_SOURCES) $(FST_HEADERS) | $(BUILD_DIR)
	$(COMPILE_FST) -c -o $@ $<

$(OBJ_TARGET_VST3_PLUGIN): $(VST3_PLUGIN_SOURCES) $(VST3_HEADERS) | $(BUILD_DIR)
	$(COMPILE_VST3) -c -o $@ $<

$(OBJ_TARGET_VST3_MAIN): \
		$(VST3_MAIN_SOURCES) \
		src/plugin/vst3/vst3.cpp \
		$(VST3_HEADERS) \
		| $(BUILD_DIR)
	$(COMPILE_VST3) -c -o $@ $<

$(VSTXML): $(VSTXMLGEN) | $(DIST_DIR_BASE)
	$(VSTXMLGEN) $(VSTXML)

$(VSTXMLGEN): $(VSTXMLGEN_OBJS) | $(DEV_DIR) show_versions
	$(LINK_DEV_EXE) $^ -o $@

$(OBJ_DEV_VSTXMLGEN): $(VSTXMLGEN_SOURCES) $(FST_HEADERS) | $(DEV_DIR)
	$(COMPILE_DEV) \
		$(FST_CXXINCS) $(FST_CXXFLAGS) \
		-c -o $@ $<

$(DEV_DIR)/test_example$(DEV_EXE): \
		tests/test_example.cpp $(TEST_LIBS) | $(DEV_DIR)
	$(COMPILE_DEV) -o $@ $<
	$(RUN_WITH_VALGRIND) $@

$(DEV_DIR)/test_bank$(DEV_EXE): \
		$(OBJ_DEV_BANK) \
		$(OBJ_DEV_SERIALIZER) \
		$(OBJ_DEV_PROXY) \
		$(OBJ_DEV_TEST_BANK) \
		| $(DEV_DIR) show_versions $(TEST_PROXY_BINS)
	$(LINK_DEV_EXE) $^ -o $@
	$(RUN_WITH_VALGRIND) $@

$(OBJ_DEV_TEST_BANK): \
		tests/test_bank.cpp \
		$(BANK_HEADERS) $(PROXY_HEADERS) \
		$(TEST_LIBS) \
		| $(DEV_DIR) show_versions
	$(COMPILE_DEV) -c -o $@ $<

$(DEV_DIR)/test_gui$(DEV_EXE): \
		$(OBJ_DEV_GUI_STUB) \
		$(OBJ_DEV_SERIALIZER) \
		$(OBJ_DEV_STRINGS) \
		$(OBJ_DEV_PROXY) \
		$(OBJ_DEV_TEST_GUI) \
		| $(DEV_DIR) show_versions $(TEST_PROXY_BINS)
	$(LINK_DEV_EXE) $^ -o $@
	$(RUN_WITH_VALGRIND) $@

$(OBJ_DEV_TEST_GUI): \
		tests/test_gui.cpp $(GUI_COMMON_HEADERS) $(TEST_LIBS) \
		| $(DEV_DIR) show_versions
	$(COMPILE_DEV) -c -o $@ $<

$(DEV_DIR)/test_math$(DEV_EXE): \
		tests/test_math.cpp \
		src/math.cpp src/math.hpp \
		src/common.hpp \
		$(TEST_LIBS) \
		| $(DEV_DIR) show_versions
	$(COMPILE_DEV) -o $@ $<
	$@

$(DEV_DIR)/test_midi$(DEV_EXE): \
		tests/test_midi.cpp src/midi.hpp src/common.hpp \
		$(TEST_LIBS) \
		| $(DEV_DIR) show_versions
	$(COMPILE_DEV) -o $@ $<
	$(RUN_WITH_VALGRIND) $@

$(DEV_DIR)/test_note_stack$(DEV_EXE): \
		tests/test_note_stack.cpp \
		src/note_stack.hpp src/note_stack.cpp \
		src/common.hpp \
		src/midi.hpp \
		$(TEST_LIBS) \
		| $(DEV_DIR) show_versions
	$(COMPILE_DEV) -o $@ $<
	$(RUN_WITH_VALGRIND) $@

$(DEV_DIR)/test_proxy$(DEV_EXE): \
		tests/test_proxy.cpp \
		$(TEST_LIBS) \
		$(PROXY_HEADERS) \
		$(PROXY_SOURCES) \
		| $(DEV_DIR) show_versions
	$(COMPILE_DEV) -o $@ $<
	$(RUN_WITH_VALGRIND) $@

$(DEV_DIR)/test_queue$(DEV_EXE): \
		tests/test_queue.cpp \
		src/queue.hpp src/queue.cpp \
		src/common.hpp \
		$(TEST_LIBS) \
		| $(DEV_DIR) show_versions
	$(COMPILE_DEV) -o $@ $<
	$(RUN_WITH_VALGRIND) $@

$(DEV_DIR)/test_serializer$(DEV_EXE): \
		$(OBJ_DEV_SERIALIZER) \
		$(OBJ_DEV_PROXY) \
		$(OBJ_DEV_TEST_SERIALIZER) \
		| $(DEV_DIR) show_versions $(TEST_PROXY_BINS)
	$(LINK_DEV_EXE) $^ -o $@
	$(RUN_WITH_VALGRIND) $@

$(OBJ_DEV_TEST_SERIALIZER): \
		tests/test_serializer.cpp \
		src/serializer.hpp $(PROXY_HEADERS) \
		$(TEST_LIBS) \
		| $(DEV_DIR) show_versions
	$(COMPILE_DEV) -c -o $@ $<

$(DEV_DIR)/test_spscqueue$(DEV_EXE): \
		tests/test_spscqueue.cpp \
		src/spscqueue.hpp src/spscqueue.cpp \
		src/common.hpp \
		$(TEST_LIBS) \
		| $(DEV_DIR) show_versions
	$(COMPILE_DEV) -o $@ $<
	$(RUN_WITH_VALGRIND) $@

$(DEV_DIR)/test_strings$(DEV_EXE): \
		$(OBJ_DEV_STRINGS) \
		$(OBJ_DEV_PROXY) \
		$(OBJ_DEV_TEST_STRINGS) \
		| $(DEV_DIR) show_versions $(TEST_PROXY_BINS)
	$(LINK_DEV_EXE) $^ -o $@
	$(RUN_WITH_VALGRIND) $@

$(OBJ_DEV_TEST_STRINGS): \
		tests/test_strings.cpp \
		src/strings.hpp \
		$(PROXY_HEADERS) \
		$(TEST_LIBS) \
		| $(DEV_DIR) show_versions
	$(COMPILE_DEV) -c -o $@ $<
