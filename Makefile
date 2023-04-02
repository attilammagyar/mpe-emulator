OS ?= linux
PLATFORM ?= x86_64-w64-mingw32

BUILD_DIR_BASE ?= build
BUILD_DIR = $(BUILD_DIR_BASE)/$(PLATFORM)
DIST_DIR ?= dist
DOC_DIR ?= doc

TEST_MAX_ARRAY_PRINT ?= 20

JS80P_CXXINCS = \
	-I./src

FST_CXXINCS = \
	-I./lib/fst

JS80P_CXXFLAGS = \
	-Wall \
	-Werror \
	-msse2 \
	-ffast-math \
	-D FST_DONT_DEPRECATE_UNKNOWN \
	-O3

DEBUG_LOG ?= -D JS80P_DEBUG_LOG=$(DEBUG_LOG_FILE)

FST_MAIN_OS_DIR = $(DIST_DIR)/$(FST_MAIN_OS)$(SUFFIX)
FST_MAIN_DIR = $(FST_MAIN_OS_DIR)/fst

OBJ_GUI_PLAYGROUND = $(BUILD_DIR)/gui-playground$(SUFFIX).o

GUI_RES = $(BUILD_DIR)/js80p$(SUFFIX).res

GUI_IMAGES = \
	resources/about.bmp \
	resources/controllers.bmp \
	resources/effects.bmp \
	resources/envelopes.bmp \
	resources/knob_states.bmp \
	resources/lfos.bmp \
	resources/synth.bmp

include make/$(OS)-$(PLATFORM).mk

OBJ_FST_MAIN = $(BUILD_DIR)/fst-main$(SUFFIX).o
OBJ_FST_PLUGIN = $(BUILD_DIR)/fst-plugin$(SUFFIX).o
OBJ_SERIALIZER = $(BUILD_DIR)/serializer$(SUFFIX).o
OBJ_SYNTH = $(BUILD_DIR)/synth$(SUFFIX).o
OBJ_GUI = $(BUILD_DIR)/gui$(SUFFIX).o

FST_MAIN_OBJS = \
	$(GUI_RES) \
	$(OBJ_FST_MAIN) \
	$(OBJ_FST_PLUGIN) \
	$(OBJ_GUI) \
	$(OBJ_SERIALIZER) \
	$(OBJ_SYNTH)

GUI_PLAYGROUND_OBJS = \
	$(GUI_RES) \
	$(OBJ_GUI) \
	$(OBJ_GUI_PLAYGROUND) \
	$(OBJ_SERIALIZER) \
	$(OBJ_SYNTH)

PARAM_COMPONENTS = \
	synth/envelope \
	synth/flexible_controller \
	synth/lfo \
	synth/math \
	synth/midi_controller \
	synth/oscillator \
	synth/param \
	synth/queue \
	synth/signal_producer

SYNTH_COMPONENTS = \
	$(PARAM_COMPONENTS) \
	synth \
	synth/biquad_filter \
	synth/comb_filter \
	synth/delay \
	synth/distortion \
	synth/echo \
	synth/effect \
	synth/effects \
	synth/filter \
	synth/reverb \
	synth/voice \
	synth/wavefolder \
	synth/wavetable

TESTS = \
	test_example \
	test_biquad_filter \
	test_comb_filter \
	test_delay \
	test_distortion \
	test_envelope \
	test_flexible_controller \
	test_gui \
	test_lfo \
	test_math \
	test_midi_controller \
	test_oscillator \
	test_param \
	test_serializer \
	test_queue \
	test_signal_producer \
	test_synth \
	test_voice \
	test_wavefolder

PERF_TESTS = \
	perf_math

PARAM_HEADERS = \
	src/js80p.hpp \
	$(foreach COMPONENT,$(PARAM_COMPONENTS),src/$(COMPONENT).hpp)

PARAM_SOURCES = \
	$(foreach COMPONENT,$(PARAM_COMPONENTS),src/$(COMPONENT).cpp)

SYNTH_HEADERS = \
	src/debug.hpp \
	src/js80p.hpp \
	src/midi.hpp \
	$(foreach COMPONENT,$(SYNTH_COMPONENTS),src/$(COMPONENT).hpp)

SYNTH_SOURCES = \
	$(foreach COMPONENT,$(SYNTH_COMPONENTS),src/$(COMPONENT).cpp)

JS80P_HEADERS = \
	src/gui/gui.hpp \
	src/gui/widgets.hpp \
	src/serializer.hpp \
	$(SYNTH_HEADERS)

JS80P_SOURCES = \
	src/gui/gui.cpp \
	src/gui/widgets.cpp \
	src/serializer.cpp \
	$(SYNTH_SOURCES)

FST_HEADERS = \
	$(JS80P_HEADERS) \
	src/fst/plugin.hpp

FST_SOURCES = \
	src/fst/plugin.cpp \
	$(FST_MAIN_SOURCES) \
	$(JS80P_SOURCES)

GUI_HEADERS = \
	$(GUI_PLATFORM_HEADERS) \
	$(JS80P_HEADERS)

GUI_SOURCES = \
	$(GUI_PLATFORM_SOURCES) \
	src/gui/widgets.cpp \
	src/gui/gui.cpp

TEST_LIBS = \
	tests/test.cpp \
	tests/utils.cpp

TEST_CPPS = $(foreach TEST,$(TESTS),tests/$(TEST).cpp)
TEST_BINS = $(foreach TEST,$(TESTS),$(BUILD_DIR)/$(TEST)$(EXE))
PERF_TEST_BINS = $(foreach TEST,$(PERF_TESTS),$(BUILD_DIR)/$(TEST)$(EXE))

TEST_CXXFLAGS = \
	-D TEST_MAX_ARRAY_PRINT=$(TEST_MAX_ARRAY_PRINT) \
	-I./tests \
	-g

CXXINCS = $(PLATFORM_CXXINCS) $(JS80P_CXXINCS)

FST_CXXFLAGS = $(CXXINCS) $(FST_CXXINCS) $(JS80P_CXXFLAGS)

.PHONY: all check clean dirs docs perf guiplayground

all: dirs $(FST_MAIN)

dirs: $(BUILD_DIR) $(DIST_DIR) $(DOC_DIR)

$(BUILD_DIR): | $(BUILD_DIR_BASE)
	$(MKDIR) $@

$(BUILD_DIR_BASE):
	$(MKDIR) $@

$(DIST_DIR):
	$(MKDIR) $@

$(DOC_DIR):
	$(MKDIR) $@

clean:
	$(RM) \
		$(FST_MAIN) \
		$(FST_MAIN_OBJS) \
		$(TEST_BINS) \
		$(GUI_PLAYGROUND) \
		$(GUI_PLAYGROUND_OBJS) \
		$(PERF_TEST_BINS)
	$(RM) $(DOC_DIR)/html/*.* $(DOC_DIR)/html/search/*.*

check: $(BUILD_DIR) perf $(TEST_LIBS) $(TEST_BINS)
	$(BUILD_DIR)/test_math$(EXE)
	$(VALGRIND) $(BUILD_DIR)/test_queue$(EXE)
	$(VALGRIND) $(BUILD_DIR)/test_signal_producer$(EXE)
	$(VALGRIND) $(BUILD_DIR)/test_midi_controller$(EXE)
	$(VALGRIND) $(BUILD_DIR)/test_flexible_controller$(EXE)
	$(VALGRIND) $(BUILD_DIR)/test_param$(EXE)
	$(VALGRIND) $(BUILD_DIR)/test_envelope$(EXE)
	$(VALGRIND) $(BUILD_DIR)/test_oscillator$(EXE)
	$(VALGRIND) $(BUILD_DIR)/test_lfo$(EXE)
	$(VALGRIND) $(BUILD_DIR)/test_biquad_filter$(EXE)
	$(VALGRIND) $(BUILD_DIR)/test_delay$(EXE)
	$(VALGRIND) $(BUILD_DIR)/test_comb_filter$(EXE)
	$(VALGRIND) $(BUILD_DIR)/test_distortion$(EXE)
	$(VALGRIND) $(BUILD_DIR)/test_wavefolder$(EXE)
	$(VALGRIND) $(BUILD_DIR)/test_voice$(EXE)
	$(VALGRIND) $(BUILD_DIR)/test_synth$(EXE)
	$(VALGRIND) $(BUILD_DIR)/test_serializer$(EXE)
	$(VALGRIND) $(BUILD_DIR)/test_gui$(EXE)
# 	$(VALGRIND) $(BUILD_DIR)/test_example$(EXE)

perf: $(BUILD_DIR) $(PERF_TEST_BINS)

docs: $(DOC_DIR) $(DOC_DIR)/html/index.html

guiplayground: $(GUI_PLAYGROUND)

$(DOC_DIR)/html/index.html: \
		Doxyfile \
		$(JS80P_HEADERS) \
		$(JS80P_SOURCES) \
		$(TEST_LIBS) \
		$(GUI_HEADERS) \
		$(GUI_SOURCES) \
		$(FST_HEADERS) \
		$(FST_SOURCES) \
		| $(DOC_DIR)
	$(DOXYGEN)

$(FST_MAIN): $(FST_MAIN_OBJS) | $(FST_MAIN_DIR)
	$(LINK_FST_MAIN) $(FST_MAIN_OBJS) -o $@ $(PLATFORM_LFLAGS)

$(FST_MAIN_DIR): | $(FST_MAIN_OS_DIR) $(DIST_DIR)
	$(MKDIR) $@

$(FST_MAIN_OS_DIR): | $(DIST_DIR)
	$(MKDIR) $@

$(GUI_PLAYGROUND): $(GUI_PLAYGROUND_OBJS) | $(BUILD_DIR)
	$(LINK_GUI_PLAYGROUND) $(GUI_PLAYGROUND_OBJS) -o $@ $(PLATFORM_LFLAGS)

$(OBJ_GUI_PLAYGROUND): \
		$(GUI_PLAYGROUND_SOURCES) \
		$(GUI_SOURCES) $(GUI_HEADERS) \
		| $(BUILD_DIR)
	$(CPP_PLATFORM) $(CXXINCS) $(JS80P_CXXINCS) $(JS80P_CXXFLAGS) $(DEBUG_LOG) $(PLATFORM_CXXFLAGS) \
		-D OEMRESOURCE -c $< -o $@

$(OBJ_SYNTH): $(SYNTH_HEADERS) $(SYNTH_SOURCES) | $(BUILD_DIR)
	$(CPP_PLATFORM) $(CXXINCS) $(JS80P_CXXINCS) $(JS80P_CXXFLAGS) $(DEBUG_LOG) $(PLATFORM_CXXFLAGS) \
		-c src/synth.cpp -o $@

$(OBJ_SERIALIZER): \
		src/serializer.cpp src/serializer.hpp \
		$(SYNTH_HEADERS) \
		| $(BUILD_DIR)
	$(CPP_PLATFORM) $(CXXINCS) $(JS80P_CXXINCS) $(JS80P_CXXFLAGS) $(DEBUG_LOG) $(PLATFORM_CXXFLAGS) \
		-c $< -o $@

$(OBJ_FST_PLUGIN): \
		src/fst/plugin.cpp \
		$(FST_HEADERS) \
		| $(BUILD_DIR)
	$(CPP_PLATFORM) $(FST_CXXFLAGS) $(DEBUG_LOG) $(PLATFORM_CXXFLAGS) -c $< -o $@

$(OBJ_GUI) : \
		$(GUI_SOURCES) $(GUI_HEADERS) \
		| $(BUILD_DIR)
	$(CPP_PLATFORM) $(CXXINCS) $(JS80P_CXXFLAGS) $(DEBUG_LOG) $(PLATFORM_CXXFLAGS) -c $< -o $@

$(OBJ_FST_MAIN): \
		$(FST_MAIN_SOURCES) \
		$(FST_HEADERS) \
		| $(BUILD_DIR)
	$(CPP_PLATFORM) $(FST_CXXFLAGS) $(DEBUG_LOG) $(PLATFORM_CXXFLAGS) -c $< -o $@

$(GUI_RES): src/gui/gui.rc $(GUI_IMAGES) | $(BUILD_DIR)
	$(WINDRES) -i $< --input-format=rc -o $@ -O coff

$(BUILD_DIR)/test_example$(EXE): \
	tests/test_example.cpp \
	$(TEST_LIBS) \
	| $(BUILD_DIR)
	$(CPP) $(CXXINCS) $(TEST_CXXFLAGS) $(JS80P_CXXFLAGS) -o $@ $<

$(BUILD_DIR)/test_envelope$(EXE): \
		tests/test_envelope.cpp \
		$(PARAM_HEADERS) $(PARAM_SOURCES) \
		$(TEST_LIBS) \
		| $(BUILD_DIR)
	$(CPP) $(CXXINCS) $(TEST_CXXFLAGS) $(JS80P_CXXFLAGS) -o $@ $<

$(BUILD_DIR)/test_biquad_filter$(EXE): \
		tests/test_biquad_filter.cpp \
		src/synth/biquad_filter.cpp src/synth/biquad_filter.hpp \
		src/synth/filter.cpp src/synth/filter.hpp \
		$(PARAM_HEADERS) $(PARAM_SOURCES) \
		$(TEST_LIBS) \
		| $(BUILD_DIR)
	$(CPP) $(CXXINCS) $(TEST_CXXFLAGS) $(JS80P_CXXFLAGS) -o $@ $<

$(BUILD_DIR)/test_comb_filter$(EXE): \
		tests/test_comb_filter.cpp \
		src/synth/comb_filter.cpp src/synth/comb_filter.hpp \
		src/synth/delay.cpp src/synth/delay.hpp \
		src/synth/filter.cpp src/synth/filter.hpp \
		$(PARAM_HEADERS) $(PARAM_SOURCES) \
		$(TEST_LIBS) \
		| $(BUILD_DIR)
	$(CPP) $(CXXINCS) $(TEST_CXXFLAGS) $(JS80P_CXXFLAGS) -o $@ $<

$(BUILD_DIR)/test_delay$(EXE): \
		tests/test_delay.cpp \
		src/synth/delay.cpp src/synth/delay.hpp \
		src/synth/filter.cpp src/synth/filter.hpp \
		$(PARAM_HEADERS) $(PARAM_SOURCES) \
		$(TEST_LIBS) \
		| $(BUILD_DIR)
	$(CPP) $(CXXINCS) $(TEST_CXXFLAGS) $(JS80P_CXXFLAGS) -o $@ $<

$(BUILD_DIR)/test_distortion$(EXE): \
		tests/test_distortion.cpp \
		src/synth/distortion.cpp src/synth/distortion.hpp \
		src/synth/filter.cpp src/synth/filter.hpp \
		$(PARAM_HEADERS) $(PARAM_SOURCES) \
		$(TEST_LIBS) \
		| $(BUILD_DIR)
	$(CPP) $(CXXINCS) $(TEST_CXXFLAGS) $(JS80P_CXXFLAGS) -o $@ $<

$(BUILD_DIR)/test_flexible_controller$(EXE): \
		tests/test_flexible_controller.cpp \
		$(PARAM_HEADERS) $(PARAM_SOURCES) \
		$(TEST_LIBS) \
		| $(BUILD_DIR)
	$(CPP) $(CXXINCS) $(TEST_CXXFLAGS) $(JS80P_CXXFLAGS) -o $@ $<

$(BUILD_DIR)/test_gui$(EXE): \
		tests/test_gui.cpp \
		tests/gui_stubs.cpp \
		$(TEST_LIBS) \
		$(JS80P_HEADERS) \
		$(JS80P_SOURCES) \
		| $(BUILD_DIR)
	$(CPP) $(CXXINCS) $(TEST_CXXFLAGS) $(JS80P_CXXFLAGS) -o $@ $<

$(BUILD_DIR)/test_lfo$(EXE): \
		tests/test_lfo.cpp \
		src/synth/wavetable.cpp src/synth/wavetable.hpp \
		$(PARAM_HEADERS) $(PARAM_SOURCES) \
		$(TEST_LIBS) \
		| $(BUILD_DIR)
	$(CPP) $(CXXINCS) $(TEST_CXXFLAGS) $(JS80P_CXXFLAGS) -o $@ $<

$(BUILD_DIR)/test_math$(EXE): \
		tests/test_math.cpp \
		src/synth/math.cpp src/synth/math.hpp \
		src/js80p.hpp \
		$(TEST_LIBS) \
		| $(BUILD_DIR)
	$(CPP) $(CXXINCS) $(TEST_CXXFLAGS) $(JS80P_CXXFLAGS) -o $@ $<

$(BUILD_DIR)/test_midi_controller$(EXE): \
		tests/test_midi_controller.cpp \
		src/synth/midi_controller.cpp src/synth/midi_controller.hpp \
		src/synth/queue.cpp src/synth/queue.hpp \
		src/js80p.hpp \
		$(TEST_LIBS) \
		| $(BUILD_DIR)
	$(CPP) $(CXXINCS) $(TEST_CXXFLAGS) $(JS80P_CXXFLAGS) -o $@ $<

$(BUILD_DIR)/test_oscillator$(EXE): \
		tests/test_oscillator.cpp \
		src/synth/wavetable.cpp src/synth/wavetable.hpp \
		$(PARAM_HEADERS) $(PARAM_SOURCES) \
		$(TEST_LIBS) \
		| $(BUILD_DIR)
	$(CPP) $(CXXINCS) $(TEST_CXXFLAGS) $(JS80P_CXXFLAGS) -o $@ $<

$(BUILD_DIR)/test_param$(EXE): \
		tests/test_param.cpp \
		$(PARAM_HEADERS) $(PARAM_SOURCES) \
		$(TEST_LIBS) \
		| $(BUILD_DIR)
	$(CPP) $(CXXINCS) $(TEST_CXXFLAGS) $(JS80P_CXXFLAGS) -o $@ $<

$(BUILD_DIR)/test_queue$(EXE): \
		tests/test_queue.cpp \
		src/synth/queue.cpp src/synth/queue.hpp \
		$(TEST_LIBS) \
		| $(BUILD_DIR)
	$(CPP) $(CXXINCS) $(TEST_CXXFLAGS) $(JS80P_CXXFLAGS) -o $@ $<

$(BUILD_DIR)/test_serializer$(EXE): \
		tests/test_serializer.cpp \
		src/serializer.cpp src/serializer.hpp \
		$(TEST_LIBS) \
		$(SYNTH_HEADERS) \
		$(SYNTH_SOURCES) \
		| $(BUILD_DIR)
	$(CPP) $(CXXINCS) $(TEST_CXXFLAGS) $(JS80P_CXXFLAGS) -o $@ $<

$(BUILD_DIR)/test_signal_producer$(EXE): \
		tests/test_signal_producer.cpp \
		src/synth/queue.cpp src/synth/queue.hpp \
		src/synth/signal_producer.cpp src/synth/signal_producer.hpp \
		src/js80p.hpp \
		$(TEST_LIBS) \
		| $(BUILD_DIR)
	$(CPP) $(CXXINCS) $(TEST_CXXFLAGS) $(JS80P_CXXFLAGS) -o $@ $<

$(BUILD_DIR)/test_synth$(EXE): \
		tests/test_synth.cpp \
		$(TEST_LIBS) \
		$(SYNTH_HEADERS) \
		$(SYNTH_SOURCES) \
		| $(BUILD_DIR)
	$(CPP) $(CXXINCS) $(TEST_CXXFLAGS) $(JS80P_CXXFLAGS) -o $@ $<

$(BUILD_DIR)/test_voice$(EXE): \
		tests/test_voice.cpp \
		$(TEST_LIBS) \
		$(SYNTH_SOURCES) \
		$(SYNTH_HEADERS) \
		| $(BUILD_DIR)
	$(CPP) $(CXXINCS) $(TEST_CXXFLAGS) $(JS80P_CXXFLAGS) -o $@ $<

$(BUILD_DIR)/test_wavefolder$(EXE): \
		tests/test_wavefolder.cpp \
		src/synth/filter.cpp src/synth/filter.hpp \
		src/synth/wavefolder.cpp src/synth/wavefolder.hpp \
		$(PARAM_HEADERS) $(PARAM_SOURCES) \
		$(TEST_LIBS) \
		| $(BUILD_DIR)
	$(CPP) $(CXXINCS) $(TEST_CXXFLAGS) $(JS80P_CXXFLAGS) -o $@ $<

$(BUILD_DIR)/perf_math$(EXE): \
		tests/performance/perf_math.cpp \
		src/synth/math.hpp src/synth/math.cpp \
		src/js80p.hpp \
		| $(BUILD_DIR)
	$(CPP) $(CXXINCS) $(TEST_CXXFLAGS) $(JS80P_CXXFLAGS) -o $@ $<
