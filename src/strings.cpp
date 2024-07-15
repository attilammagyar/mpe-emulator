/*
 * This file is part of MPE Emulator.
 * Copyright (C) 2024  Attila M. Magyar
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

#ifndef MPE_EMULATOR__STRINGS_CPP
#define MPE_EMULATOR__STRINGS_CPP

#include <cstring>

#include "strings.hpp"
#include "math.hpp"


namespace MpeEmulator
{

char const* const Strings::CONTROLLERS_SHORT[] = {
    [Proxy::ControllerId::BANK_SELECT] = "Bank",
    [Proxy::ControllerId::MODULATION_WHEEL] = "Mod",
    [Proxy::ControllerId::BREATH] = "Breath",
    [Proxy::ControllerId::UNDEFINED_1] = "CC3",
    [Proxy::ControllerId::FOOT_PEDAL] = "Foot",
    [Proxy::ControllerId::PORTAMENTO_TIME] = "CC5",
    [Proxy::ControllerId::DATA_ENTRY] = "Data",
    [Proxy::ControllerId::VOLUME] = "Vol",
    [Proxy::ControllerId::BALANCE] = "CC8",
    [Proxy::ControllerId::UNDEFINED_2] = "CC9",
    [Proxy::ControllerId::PAN] = "Pan",
    [Proxy::ControllerId::EXPRESSION_PEDAL] = "Expr",
    [Proxy::ControllerId::FX_CTL_1] = "Fx 1",
    [Proxy::ControllerId::FX_CTL_2] = "Fx 2",
    [Proxy::ControllerId::UNDEFINED_3] = "CC14",
    [Proxy::ControllerId::UNDEFINED_4] = "CC15",
    [Proxy::ControllerId::GENERAL_1] = "Gen 1",
    [Proxy::ControllerId::GENERAL_2] = "Gen 2",
    [Proxy::ControllerId::GENERAL_3] = "Gen 3",
    [Proxy::ControllerId::GENERAL_4] = "Gen 4",
    [Proxy::ControllerId::UNDEFINED_5] = "CC20",
    [Proxy::ControllerId::UNDEFINED_6] = "CC21",
    [Proxy::ControllerId::UNDEFINED_7] = "CC22",
    [Proxy::ControllerId::UNDEFINED_8] = "CC23",
    [Proxy::ControllerId::UNDEFINED_9] = "CC24",
    [Proxy::ControllerId::UNDEFINED_10] = "CC25",
    [Proxy::ControllerId::UNDEFINED_11] = "CC26",
    [Proxy::ControllerId::UNDEFINED_12] = "CC27",
    [Proxy::ControllerId::UNDEFINED_13] = "CC28",
    [Proxy::ControllerId::UNDEFINED_14] = "CC29",
    [Proxy::ControllerId::UNDEFINED_15] = "CC30",
    [Proxy::ControllerId::UNDEFINED_16] = "CC31",
    [Proxy::ControllerId::BANK_SELECT_LSB] = "CC32",
    [Proxy::ControllerId::MODULATION_WHEEL_LSB] = "CC33",
    [Proxy::ControllerId::BREATH_LSB] = "CC34",
    [Proxy::ControllerId::UNDEFINED_1_LSB] = "CC35",
    [Proxy::ControllerId::FOOT_PEDAL_LSB] = "CC36",
    [Proxy::ControllerId::PORTAMENTO_TIME_LSB] = "CC37",
    [Proxy::ControllerId::DATA_ENTRY_LSB] = "CC38",
    [Proxy::ControllerId::VOLUME_LSB] = "CC39",
    [Proxy::ControllerId::BALANCE_LSB] = "CC40",
    [Proxy::ControllerId::UNDEFINED_2_LSB] = "CC41",
    [Proxy::ControllerId::PAN_LSB] = "CC42",
    [Proxy::ControllerId::EXPRESSION_PEDAL_LSB] = "CC43",
    [Proxy::ControllerId::FX_CTL_1_LSB] = "CC44",
    [Proxy::ControllerId::FX_CTL_2_LSB] = "CC45",
    [Proxy::ControllerId::UNDEFINED_3_LSB] = "CC46",
    [Proxy::ControllerId::UNDEFINED_4_LSB] = "CC47",
    [Proxy::ControllerId::GENERAL_1_LSB] = "CC48",
    [Proxy::ControllerId::GENERAL_2_LSB] = "CC49",
    [Proxy::ControllerId::GENERAL_3_LSB] = "CC50",
    [Proxy::ControllerId::GENERAL_4_LSB] = "CC51",
    [Proxy::ControllerId::UNDEFINED_5_LSB] = "CC52",
    [Proxy::ControllerId::UNDEFINED_6_LSB] = "CC53",
    [Proxy::ControllerId::UNDEFINED_7_LSB] = "CC54",
    [Proxy::ControllerId::UNDEFINED_8_LSB] = "CC55",
    [Proxy::ControllerId::UNDEFINED_9_LSB] = "CC56",
    [Proxy::ControllerId::UNDEFINED_10_LSB] = "CC57",
    [Proxy::ControllerId::UNDEFINED_11_LSB] = "CC58",
    [Proxy::ControllerId::UNDEFINED_12_LSB] = "CC59",
    [Proxy::ControllerId::UNDEFINED_13_LSB] = "CC60",
    [Proxy::ControllerId::UNDEFINED_14_LSB] = "CC61",
    [Proxy::ControllerId::UNDEFINED_15_LSB] = "CC62",
    [Proxy::ControllerId::UNDEFINED_16_LSB] = "CC63",
    [Proxy::ControllerId::SUSTAIN_PEDAL] = "Sust",
    [Proxy::ControllerId::PORTAMENTO_ON_OFF] = "CC65",
    [Proxy::ControllerId::SOSTENUTO] = "CC66",
    [Proxy::ControllerId::SOFT_PEDAL] = "CC67",
    [Proxy::ControllerId::LEGATO_FOOTSWITCH] = "CC68",
    [Proxy::ControllerId::HOLD_2] = "CC69",
    [Proxy::ControllerId::SOUND_1] = "CC70",
    [Proxy::ControllerId::SOUND_2] = "CC71",
    [Proxy::ControllerId::SOUND_3] = "CC72",
    [Proxy::ControllerId::SOUND_4] = "CC73",
    [Proxy::ControllerId::SOUND_5] = "CC74",
    [Proxy::ControllerId::SOUND_6] = "CC75",
    [Proxy::ControllerId::SOUND_7] = "CC76",
    [Proxy::ControllerId::SOUND_8] = "CC77",
    [Proxy::ControllerId::SOUND_9] = "CC78",
    [Proxy::ControllerId::SOUND_10] = "CC79",
    [Proxy::ControllerId::GENERAL_ON_OFF_1] = "CC80",
    [Proxy::ControllerId::GENERAL_ON_OFF_2] = "CC81",
    [Proxy::ControllerId::GENERAL_ON_OFF_3] = "CC82",
    [Proxy::ControllerId::GENERAL_ON_OFF_4] = "CC83",
    [Proxy::ControllerId::PORTAMENTO_CONTROL] = "CC84",
    [Proxy::ControllerId::UNDEFINED_17] = "CC85",
    [Proxy::ControllerId::UNDEFINED_18] = "CC86",
    [Proxy::ControllerId::UNDEFINED_19] = "CC87",
    [Proxy::ControllerId::UNDEFINED_20] = "CC88",
    [Proxy::ControllerId::UNDEFINED_21] = "CC90",
    [Proxy::ControllerId::UNDEFINED_22] = "CC90",
    [Proxy::ControllerId::FX_1] = "CC91",
    [Proxy::ControllerId::FX_2] = "CC92",
    [Proxy::ControllerId::FX_3] = "CC93",
    [Proxy::ControllerId::FX_4] = "CC94",
    [Proxy::ControllerId::FX_5] = "CC95",
    [Proxy::ControllerId::DATA_INCREMENT] = "CC96",
    [Proxy::ControllerId::DATA_DECREMENT] = "CC97",
    [Proxy::ControllerId::NON_REG_PARAM_NUM_LSB] = "CC98",
    [Proxy::ControllerId::NON_REG_PARAM_NUM_MSB] = "CC99",
    [Proxy::ControllerId::REG_PARAM_NUM_LSB] = "CC100",
    [Proxy::ControllerId::REG_PARAM_NUM_MSB] = "CC101",
    [Proxy::ControllerId::UNDEFINED_23] = "CC102",
    [Proxy::ControllerId::UNDEFINED_24] = "CC103",
    [Proxy::ControllerId::UNDEFINED_25] = "CC104",
    [Proxy::ControllerId::UNDEFINED_26] = "CC105",
    [Proxy::ControllerId::UNDEFINED_27] = "CC106",
    [Proxy::ControllerId::UNDEFINED_28] = "CC107",
    [Proxy::ControllerId::UNDEFINED_29] = "CC108",
    [Proxy::ControllerId::UNDEFINED_30] = "CC109",
    [Proxy::ControllerId::UNDEFINED_31] = "CC110",
    [Proxy::ControllerId::UNDEFINED_32] = "CC111",
    [Proxy::ControllerId::UNDEFINED_33] = "CC112",
    [Proxy::ControllerId::UNDEFINED_34] = "CC113",
    [Proxy::ControllerId::UNDEFINED_35] = "CC114",
    [Proxy::ControllerId::UNDEFINED_36] = "CC115",
    [Proxy::ControllerId::UNDEFINED_37] = "CC116",
    [Proxy::ControllerId::UNDEFINED_38] = "CC117",
    [Proxy::ControllerId::UNDEFINED_39] = "CC118",
    [Proxy::ControllerId::UNDEFINED_40] = "CC119",
    [Proxy::ControllerId::PITCH_WHEEL] = "Pitch",
    [Proxy::ControllerId::CHANNEL_PRESSURE] = "Ch AT",
    [Proxy::ControllerId::MIDI_LEARN] = "Learn",
    [Proxy::ControllerId::NONE] = "none",
};

char const* const Strings::CONTROLLERS_LONG[] = {
    [Proxy::ControllerId::BANK_SELECT] = "CC 0 Bank Select",
    [Proxy::ControllerId::MODULATION_WHEEL] = "CC 1 Modulation Wheel",
    [Proxy::ControllerId::BREATH] = "CC 2 Breath",
    [Proxy::ControllerId::UNDEFINED_1] = "CC 3",
    [Proxy::ControllerId::FOOT_PEDAL] = "CC 4 Foot Pedal",
    [Proxy::ControllerId::PORTAMENTO_TIME] = "CC 5 Portamento Time",
    [Proxy::ControllerId::DATA_ENTRY] = "CC 6 Data Entry",
    [Proxy::ControllerId::VOLUME] = "CC 7 Volume",
    [Proxy::ControllerId::BALANCE] = "CC 8 Balance",
    [Proxy::ControllerId::UNDEFINED_2] = "CC 9",
    [Proxy::ControllerId::PAN] = "CC 10 Pan",
    [Proxy::ControllerId::EXPRESSION_PEDAL] = "CC 11 Expression Pedal",
    [Proxy::ControllerId::FX_CTL_1] = "CC 12 Effect Control 1",
    [Proxy::ControllerId::FX_CTL_2] = "CC 13 Effect Control 2",
    [Proxy::ControllerId::UNDEFINED_3] = "CC 14",
    [Proxy::ControllerId::UNDEFINED_4] = "CC 15",
    [Proxy::ControllerId::GENERAL_1] = "CC 16 General 1",
    [Proxy::ControllerId::GENERAL_2] = "CC 17 General 2",
    [Proxy::ControllerId::GENERAL_3] = "CC 18 General 3",
    [Proxy::ControllerId::GENERAL_4] = "CC 19 General 4",
    [Proxy::ControllerId::UNDEFINED_5] = "CC 20",
    [Proxy::ControllerId::UNDEFINED_6] = "CC 21",
    [Proxy::ControllerId::UNDEFINED_7] = "CC 22",
    [Proxy::ControllerId::UNDEFINED_8] = "CC 23",
    [Proxy::ControllerId::UNDEFINED_9] = "CC 24",
    [Proxy::ControllerId::UNDEFINED_10] = "CC 25",
    [Proxy::ControllerId::UNDEFINED_11] = "CC 26",
    [Proxy::ControllerId::UNDEFINED_12] = "CC 27",
    [Proxy::ControllerId::UNDEFINED_13] = "CC 28",
    [Proxy::ControllerId::UNDEFINED_14] = "CC 29",
    [Proxy::ControllerId::UNDEFINED_15] = "CC 30",
    [Proxy::ControllerId::UNDEFINED_16] = "CC 31",
    [Proxy::ControllerId::BANK_SELECT_LSB] = "CC 32 Bank Select LSB (14 bit)",
    [Proxy::ControllerId::MODULATION_WHEEL_LSB] = "CC 33 Mod Wheel LSB (14 bit)",
    [Proxy::ControllerId::BREATH_LSB] = "CC 34 Breath LSB (14 bit)",
    [Proxy::ControllerId::UNDEFINED_1_LSB] = "CC 35 LSB for CC 3 (14 bit)",
    [Proxy::ControllerId::FOOT_PEDAL_LSB] = "CC 36 Foot Pedal LSB (14 bit)",
    [Proxy::ControllerId::PORTAMENTO_TIME_LSB] = "CC 37 Portamento T LSB (14 bit)",
    [Proxy::ControllerId::DATA_ENTRY_LSB] = "CC 38 Data Entry LSB (14 bit)",
    [Proxy::ControllerId::VOLUME_LSB] = "CC 39 Volume LSB (14 bit)",
    [Proxy::ControllerId::BALANCE_LSB] = "CC 40 Balance LSB (14 bit)",
    [Proxy::ControllerId::UNDEFINED_2_LSB] = "CC 41 LSB for CC 9 (14 bit)",
    [Proxy::ControllerId::PAN_LSB] = "CC 42 Pan LSB (14 bit)",
    [Proxy::ControllerId::EXPRESSION_PEDAL_LSB] = "CC 43 Expr Pedal LSB (14 bit)",
    [Proxy::ControllerId::FX_CTL_1_LSB] = "CC 44 Effect Ctl 1 LSB (14 bit)",
    [Proxy::ControllerId::FX_CTL_2_LSB] = "CC 45 Effect Ctl 2 LSB (14 bit)",
    [Proxy::ControllerId::UNDEFINED_3_LSB] = "CC 46 LSB for CC 14 (14 bit)",
    [Proxy::ControllerId::UNDEFINED_4_LSB] = "CC 47 LSB for CC 15 (14 bit)",
    [Proxy::ControllerId::GENERAL_1_LSB] = "CC 48 General 1 LSB (14 bit)",
    [Proxy::ControllerId::GENERAL_2_LSB] = "CC 49 General 2 LSB (14 bit)",
    [Proxy::ControllerId::GENERAL_3_LSB] = "CC 50 General 3 LSB (14 bit)",
    [Proxy::ControllerId::GENERAL_4_LSB] = "CC 51 General 4 LSB (14 bit)",
    [Proxy::ControllerId::UNDEFINED_5_LSB] = "CC 52 LSB for CC 20 (14 bit)",
    [Proxy::ControllerId::UNDEFINED_6_LSB] = "CC 53 LSB for CC 21 (14 bit)",
    [Proxy::ControllerId::UNDEFINED_7_LSB] = "CC 54 LSB for CC 22 (14 bit)",
    [Proxy::ControllerId::UNDEFINED_8_LSB] = "CC 55 LSB for CC 23 (14 bit)",
    [Proxy::ControllerId::UNDEFINED_9_LSB] = "CC 56 LSB for CC 24 (14 bit)",
    [Proxy::ControllerId::UNDEFINED_10_LSB] = "CC 57 LSB for CC 25 (14 bit)",
    [Proxy::ControllerId::UNDEFINED_11_LSB] = "CC 58 LSB for CC 26 (14 bit)",
    [Proxy::ControllerId::UNDEFINED_12_LSB] = "CC 59 LSB for CC 27 (14 bit)",
    [Proxy::ControllerId::UNDEFINED_13_LSB] = "CC 60 LSB for CC 28 (14 bit)",
    [Proxy::ControllerId::UNDEFINED_14_LSB] = "CC 61 LSB for CC 29 (14 bit)",
    [Proxy::ControllerId::UNDEFINED_15_LSB] = "CC 62 LSB for CC 30 (14 bit)",
    [Proxy::ControllerId::UNDEFINED_16_LSB] = "CC 63 LSB for CC 31 (14 bit)",
    [Proxy::ControllerId::SUSTAIN_PEDAL] = "CC 64 Sustain Pedal",
    [Proxy::ControllerId::PORTAMENTO_ON_OFF] = "CC 65 Portamento On/Off",
    [Proxy::ControllerId::SOSTENUTO] = "CC 66 Sostenuto",
    [Proxy::ControllerId::SOFT_PEDAL] = "CC 67 Soft Pedal",
    [Proxy::ControllerId::LEGATO_FOOTSWITCH] = "CC 68 Legato Footswitch",
    [Proxy::ControllerId::HOLD_2] = "CC 69 Hold 2",
    [Proxy::ControllerId::SOUND_1] = "CC 70 Sound 1",
    [Proxy::ControllerId::SOUND_2] = "CC 71 Sound 2",
    [Proxy::ControllerId::SOUND_3] = "CC 72 Sound 3",
    [Proxy::ControllerId::SOUND_4] = "CC 73 Sound 4",
    [Proxy::ControllerId::SOUND_5] = "CC 74 Sound 5",
    [Proxy::ControllerId::SOUND_6] = "CC 75 Sound 6",
    [Proxy::ControllerId::SOUND_7] = "CC 76 Sound 7",
    [Proxy::ControllerId::SOUND_8] = "CC 77 Sound 8",
    [Proxy::ControllerId::SOUND_9] = "CC 78 Sound 9",
    [Proxy::ControllerId::SOUND_10] = "CC 79 Sound 10",
    [Proxy::ControllerId::GENERAL_ON_OFF_1] = "CC 80 General On/Off 1",
    [Proxy::ControllerId::GENERAL_ON_OFF_2] = "CC 81 General On/Off 2",
    [Proxy::ControllerId::GENERAL_ON_OFF_3] = "CC 82 General On/Off 3",
    [Proxy::ControllerId::GENERAL_ON_OFF_4] = "CC 83 General On/Off 4",
    [Proxy::ControllerId::PORTAMENTO_CONTROL] = "CC 84 Portamento Control",
    [Proxy::ControllerId::UNDEFINED_17] = "CC 85",
    [Proxy::ControllerId::UNDEFINED_18] = "CC 86",
    [Proxy::ControllerId::UNDEFINED_19] = "CC 87",
    [Proxy::ControllerId::UNDEFINED_20] = "CC 88",
    [Proxy::ControllerId::UNDEFINED_21] = "CC 90",
    [Proxy::ControllerId::UNDEFINED_22] = "CC 90",
    [Proxy::ControllerId::FX_1] = "CC 91 Effect 1",
    [Proxy::ControllerId::FX_2] = "CC 92 Effect 2",
    [Proxy::ControllerId::FX_3] = "CC 93 Effect 3",
    [Proxy::ControllerId::FX_4] = "CC 94 Effect 4",
    [Proxy::ControllerId::FX_5] = "CC 95 Effect 5",
    [Proxy::ControllerId::DATA_INCREMENT] = "CC 96 Data Increment",
    [Proxy::ControllerId::DATA_DECREMENT] = "CC 97 Data Decrement",
    [Proxy::ControllerId::NON_REG_PARAM_NUM_LSB] = "CC 98 Non-Reg Param Num LSB",
    [Proxy::ControllerId::NON_REG_PARAM_NUM_MSB] = "CC 99 Non-Reg Param Num MSB",
    [Proxy::ControllerId::REG_PARAM_NUM_LSB] = "CC 100 Reg Param Num LSB",
    [Proxy::ControllerId::REG_PARAM_NUM_MSB] = "CC 101 Reg Param Num MSB",
    [Proxy::ControllerId::UNDEFINED_23] = "CC 102",
    [Proxy::ControllerId::UNDEFINED_24] = "CC 103",
    [Proxy::ControllerId::UNDEFINED_25] = "CC 104",
    [Proxy::ControllerId::UNDEFINED_26] = "CC 105",
    [Proxy::ControllerId::UNDEFINED_27] = "CC 106",
    [Proxy::ControllerId::UNDEFINED_28] = "CC 107",
    [Proxy::ControllerId::UNDEFINED_29] = "CC 108",
    [Proxy::ControllerId::UNDEFINED_30] = "CC 109",
    [Proxy::ControllerId::UNDEFINED_31] = "CC 110",
    [Proxy::ControllerId::UNDEFINED_32] = "CC 111",
    [Proxy::ControllerId::UNDEFINED_33] = "CC 112",
    [Proxy::ControllerId::UNDEFINED_34] = "CC 113",
    [Proxy::ControllerId::UNDEFINED_35] = "CC 114",
    [Proxy::ControllerId::UNDEFINED_36] = "CC 115",
    [Proxy::ControllerId::UNDEFINED_37] = "CC 116",
    [Proxy::ControllerId::UNDEFINED_38] = "CC 117",
    [Proxy::ControllerId::UNDEFINED_39] = "CC 118",
    [Proxy::ControllerId::UNDEFINED_40] = "CC 119",
    [Proxy::ControllerId::PITCH_WHEEL] = "Pitch Wheel",
    [Proxy::ControllerId::CHANNEL_PRESSURE] = "Channel Pressure (Aftertouch)",
    [Proxy::ControllerId::MIDI_LEARN] = "MIDI Learn",
    [Proxy::ControllerId::NONE] = "none",
};

size_t const Strings::CONTROLLERS_COUNT = 124;


char const* const Strings::ZONE_TYPES[] = {
    [Proxy::ZoneType::ZT_LOWER] = "Lower",
    [Proxy::ZoneType::ZT_UPPER] = "Upper",
};

size_t const Strings::ZONE_TYPES_COUNT = 2;


char const* const Strings::CHANNELS[] = {
    "-",
    "1",
    "2",
    "3",
    "4",
    "5",
    "6",
    "7",
    "8",
    "9",
    "10",
    "11",
    "12",
    "13",
    "14",
    "15",
};

size_t const Strings::CHANNELS_COUNT = 16;


char const* const Strings::ANCHORS[] = {
    "C -1",         /*   0 */
    "C#/Db -1",     /*   1 */
    "D -1",         /*   2 */
    "D#/Eb -1",     /*   3 */
    "E -1",         /*   4 */
    "F -1",         /*   5 */
    "F#/Gb -1",     /*   6 */
    "G -1",         /*   7 */
    "G#/Ab -1",     /*   8 */
    "A -1",         /*   9 */
    "A#/Bb -1",     /*  10 */
    "B -1",         /*  11 */
    "C 0",          /*  12 */
    "C#/Db 0",      /*  13 */
    "D 0",          /*  14 */
    "D#/Eb 0",      /*  15 */
    "E 0",          /*  16 */
    "F 0",          /*  17 */
    "F#/Gb 0",      /*  18 */
    "G 0",          /*  19 */
    "G#/Ab 0",      /*  20 */
    "A 0",          /*  21 */
    "A#/Bb 0",      /*  22 */
    "B 0",          /*  23 */
    "C 1",          /*  24 */
    "C#/Db 1",      /*  25 */
    "D 1",          /*  26 */
    "D#/Eb 1",      /*  27 */
    "E 1",          /*  28 */
    "F 1",          /*  29 */
    "F#/Gb 1",      /*  30 */
    "G 1",          /*  31 */
    "G#/Ab 1",      /*  32 */
    "A 1",          /*  33 */
    "A#/Bb 1",      /*  34 */
    "B 1",          /*  35 */
    "C 2",          /*  36 */
    "C#/Db 2",      /*  37 */
    "D 2",          /*  38 */
    "D#/Eb 2",      /*  39 */
    "E 2",          /*  40 */
    "F 2",          /*  41 */
    "F#/Gb 2",      /*  42 */
    "G 2",          /*  43 */
    "G#/Ab 2",      /*  44 */
    "A 2",          /*  45 */
    "A#/Bb 2",      /*  46 */
    "B 2",          /*  47 */
    "C 3",          /*  48 */
    "C#/Db 3",      /*  49 */
    "D 3",          /*  50 */
    "D#/Eb 3",      /*  51 */
    "E 3",          /*  52 */
    "F 3",          /*  53 */
    "F#/Gb 3",      /*  54 */
    "G 3",          /*  55 */
    "G#/Ab 3",      /*  56 */
    "A 3",          /*  57 */
    "A#/Bb 3",      /*  58 */
    "B 3",          /*  59 */
    "C 4",          /*  60 */
    "C#/Db 4",      /*  61 */
    "D 4",          /*  62 */
    "D#/Eb 4",      /*  63 */
    "E 4",          /*  64 */
    "F 4",          /*  65 */
    "F#/Gb 4",      /*  66 */
    "G 4",          /*  67 */
    "G#/Ab 4",      /*  68 */
    "A 4",          /*  69 */
    "A#/Bb 4",      /*  70 */
    "B 4",          /*  71 */
    "C 5",          /*  72 */
    "C#/Db 5",      /*  73 */
    "D 5",          /*  74 */
    "D#/Eb 5",      /*  75 */
    "E 5",          /*  76 */
    "F 5",          /*  77 */
    "F#/Gb 5",      /*  78 */
    "G 5",          /*  79 */
    "G#/Ab 5",      /*  80 */
    "A 5",          /*  81 */
    "A#/Bb 5",      /*  82 */
    "B 5",          /*  83 */
    "C 6",          /*  84 */
    "C#/Db 6",      /*  85 */
    "D 6",          /*  86 */
    "D#/Eb 6",      /*  87 */
    "E 6",          /*  88 */
    "F 6",          /*  89 */
    "F#/Gb 6",      /*  90 */
    "G 6",          /*  91 */
    "G#/Ab 6",      /*  92 */
    "A 6",          /*  93 */
    "A#/Bb 6",      /*  94 */
    "B 6",          /*  95 */
    "C 7",          /*  96 */
    "C#/Db 7",      /*  97 */
    "D 7",          /*  98 */
    "D#/Eb 7",      /*  99 */
    "E 7",          /* 100 */
    "F 7",          /* 101 */
    "F#/Gb 7",      /* 102 */
    "G 7",          /* 103 */
    "G#/Ab 7",      /* 104 */
    "A 7",          /* 105 */
    "A#/Bb 7",      /* 106 */
    "B 7",          /* 107 */
    "C 8",          /* 108 */
    "C#/Db 8",      /* 109 */
    "D 8",          /* 110 */
    "D#/Eb 8",      /* 111 */
    "E 8",          /* 112 */
    "F 8",          /* 113 */
    "F#/Gb 8",      /* 114 */
    "G 8",          /* 115 */
    "G#/Ab 8",      /* 116 */
    "A 8",          /* 117 */
    "A#/Bb 8",      /* 118 */
    "B 8",          /* 119 */
    "C 9",          /* 120 */
    "C#/Db 9",      /* 121 */
    "D 9",          /* 122 */
    "D#/Eb 9",      /* 123 */
    "E 9",          /* 124 */
    "F 9",          /* 125 */
    "F#/Gb 9",      /* 126 */
    "G 9",          /* 127 */
};

size_t const Strings::ANCHORS_COUNT = 128;


char const* const Strings::EXCESS_NOTE_HANDLINGS[] = {
    [Proxy::ExcessNoteHandling::ENH_IGNORE] = "Never",
    [Proxy::ExcessNoteHandling::ENH_STEAL_LOWEST] = "Low",
    [Proxy::ExcessNoteHandling::ENH_STEAL_HIGHEST] = "High",
    [Proxy::ExcessNoteHandling::ENH_STEAL_OLDEST] = "Old",
    [Proxy::ExcessNoteHandling::ENH_STEAL_NEWEST] = "New",
};

size_t const Strings::EXCESS_NOTE_HANDLINGS_COUNT = 5;


char const* const Strings::TARGETS_SHORT[] = {
    [Proxy::Target::TRG_GLOBAL] = "Global",
    [Proxy::Target::TRG_ALL_BELOW_ANCHOR] = "All BA",
    [Proxy::Target::TRG_ALL_ABOVE_ANCHOR] = "All AA",
    [Proxy::Target::TRG_LOWEST] = "Low",
    [Proxy::Target::TRG_HIGHEST] = "High",
    [Proxy::Target::TRG_OLDEST] = "Old",
    [Proxy::Target::TRG_NEWEST] = "New",
    [Proxy::Target::TRG_LOWEST_BELOW_ANCHOR] = "Low BA",
    [Proxy::Target::TRG_HIGHEST_BELOW_ANCHOR] = "Hi BA",
    [Proxy::Target::TRG_OLDEST_BELOW_ANCHOR] = "Old BA",
    [Proxy::Target::TRG_NEWEST_BELOW_ANCHOR] = "New BA",
    [Proxy::Target::TRG_LOWEST_ABOVE_ANCHOR] = "Low AA",
    [Proxy::Target::TRG_HIGHEST_ABOVE_ANCHOR] = "Hi AA",
    [Proxy::Target::TRG_OLDEST_ABOVE_ANCHOR] = "Old AA",
    [Proxy::Target::TRG_NEWEST_ABOVE_ANCHOR] = "New AA",
};

char const* const Strings::TARGETS_LONG[] = {
    [Proxy::Target::TRG_GLOBAL] = "Global",
    [Proxy::Target::TRG_ALL_BELOW_ANCHOR] = "All below anchor",
    [Proxy::Target::TRG_ALL_ABOVE_ANCHOR] = "All above anchor",
    [Proxy::Target::TRG_LOWEST] = "Lowest",
    [Proxy::Target::TRG_HIGHEST] = "Highest",
    [Proxy::Target::TRG_OLDEST] = "Oldest",
    [Proxy::Target::TRG_NEWEST] = "Newest",
    [Proxy::Target::TRG_LOWEST_BELOW_ANCHOR] = "Lowest below anchor",
    [Proxy::Target::TRG_HIGHEST_BELOW_ANCHOR] = "Highest below anchor",
    [Proxy::Target::TRG_OLDEST_BELOW_ANCHOR] = "Oldest below anchor",
    [Proxy::Target::TRG_NEWEST_BELOW_ANCHOR] = "Newest below anchor",
    [Proxy::Target::TRG_LOWEST_ABOVE_ANCHOR] = "Lowest above anchor",
    [Proxy::Target::TRG_HIGHEST_ABOVE_ANCHOR] = "Highest above anchor",
    [Proxy::Target::TRG_OLDEST_ABOVE_ANCHOR] = "Oldest above anchor",
    [Proxy::Target::TRG_NEWEST_ABOVE_ANCHOR] = "Newest above anchor",
};

size_t const Strings::TARGETS_COUNT = 15;


char const* const Strings::RESETS[] = {
    [Proxy::Reset::RST_OFF] = "OFF",
    [Proxy::Reset::RST_LAST] = "LST",
    [Proxy::Reset::RST_INIT] = "INI",
};

size_t const Strings::RESETS_COUNT = 3;


char const* const Strings::TOGGLE_STATES[] = {
    [Proxy::Toggle::OFF] = "OFF",
    [Proxy::Toggle::ON] = "ON",
};

size_t const Strings::TOGGLE_STATES_COUNT = 2;


char const* const Strings::DISTORTIONS[] = {
    [Math::DistortionShape::DIST_SHAPE_SMOOTH_SMOOTH] = "SMT-SMT",
    [Math::DistortionShape::DIST_SHAPE_SMOOTH_SHARP] = "SMT-SHP",
    [Math::DistortionShape::DIST_SHAPE_SHARP_SMOOTH] = "SHP-SMT",
    [Math::DistortionShape::DIST_SHAPE_SHARP_SHARP] = "SHP-SHP",
};

size_t const Strings::DISTORTIONS_COUNT = 4;


char const* const Strings::TRANSPOSE_OPTIONS[] = {
    "-48", "-47", "-46", "-45", "-44", "-43", "-42", "-41", "-40", "-39", "-38", "-37",
    "-36", "-35", "-34", "-33", "-32", "-31", "-30", "-29", "-28", "-27", "-26", "-25",
    "-24", "-23", "-22", "-21", "-20", "-19", "-18", "-17", "-16", "-15", "-14", "-13",
    "-12", "-11", "-10",  "-9",  "-8",  "-7",  "-6",  "-5",  "-4",  "-3",  "-2",  "-1",
    "0",
     "+1",  "+2",  "+3",  "+4",  "+5",  "+6",  "+7",  "+8",  "+9", "+10", "+11", "+12",
    "+13", "+14", "+15", "+16", "+17", "+18", "+19", "+20", "+21", "+22", "+23", "+24",
    "+25", "+26", "+27", "+28", "+29", "+30", "+31", "+32", "+33", "+34", "+35", "+36",
    "+37", "+38", "+39", "+40", "+41", "+42", "+43", "+44", "+45", "+46", "+47", "+48",
};

size_t const Strings::TRANSPOSE_OPTIONS_COUNT = 97;


char const* const Strings::PARAMS[Proxy::ParamId::PARAM_ID_COUNT] = {
    [Proxy::ParamId::MCM] = "Emit MCM on reset",
    [Proxy::ParamId::Z1TYP] = "Zone type",
    [Proxy::ParamId::Z1CHN] = "Channels",
    [Proxy::ParamId::Z1ENH] = "Excess note handling",
    [Proxy::ParamId::Z1ANC] = "Anchor",
    [Proxy::ParamId::Z1ORV] = "Override release velocity with triggered velocity",
    [Proxy::ParamId::Z1R1IN] = "Rule 1 input",
    [Proxy::ParamId::Z1R1OU] = "Rule 1 output",
    [Proxy::ParamId::Z1R1IV] = "Rule 1 initial value (%)",
    [Proxy::ParamId::Z1R1TR] = "Rule 1 target",
    [Proxy::ParamId::Z1R1DT] = "Rule 1 distortion type",
    [Proxy::ParamId::Z1R1DL] = "Rule 1 distortion level (%)",
    [Proxy::ParamId::Z1R1MP] = "Rule 1 midpoint (%)",
    [Proxy::ParamId::Z1R1RS] = "Rule 1 reset on target change",
    [Proxy::ParamId::Z1R1NV] = "Rule 1 invert",
    [Proxy::ParamId::Z1R2IN] = "Rule 2 input",
    [Proxy::ParamId::Z1R2OU] = "Rule 2 output",
    [Proxy::ParamId::Z1R2IV] = "Rule 2 initial value (%)",
    [Proxy::ParamId::Z1R2TR] = "Rule 2 target",
    [Proxy::ParamId::Z1R2DT] = "Rule 2 distortion type",
    [Proxy::ParamId::Z1R2DL] = "Rule 2 distortion level (%)",
    [Proxy::ParamId::Z1R2MP] = "Rule 2 midpoint (%)",
    [Proxy::ParamId::Z1R2RS] = "Rule 2 reset on target change",
    [Proxy::ParamId::Z1R2NV] = "Rule 2 invert",
    [Proxy::ParamId::Z1R3IN] = "Rule 3 input",
    [Proxy::ParamId::Z1R3OU] = "Rule 3 output",
    [Proxy::ParamId::Z1R3IV] = "Rule 3 initial value (%)",
    [Proxy::ParamId::Z1R3TR] = "Rule 3 target",
    [Proxy::ParamId::Z1R3DT] = "Rule 3 distortion type",
    [Proxy::ParamId::Z1R3DL] = "Rule 3 distortion level (%)",
    [Proxy::ParamId::Z1R3MP] = "Rule 3 midpoint (%)",
    [Proxy::ParamId::Z1R3RS] = "Rule 3 reset on target change",
    [Proxy::ParamId::Z1R3NV] = "Rule 3 invert",
    [Proxy::ParamId::Z1R4IN] = "Rule 4 input",
    [Proxy::ParamId::Z1R4OU] = "Rule 4 output",
    [Proxy::ParamId::Z1R4IV] = "Rule 4 initial value (%)",
    [Proxy::ParamId::Z1R4TR] = "Rule 4 target",
    [Proxy::ParamId::Z1R4DT] = "Rule 4 distortion type",
    [Proxy::ParamId::Z1R4DL] = "Rule 4 distortion level (%)",
    [Proxy::ParamId::Z1R4MP] = "Rule 4 midpoint (%)",
    [Proxy::ParamId::Z1R4RS] = "Rule 4 reset on target change",
    [Proxy::ParamId::Z1R4NV] = "Rule 4 invert",
    [Proxy::ParamId::Z1R5IN] = "Rule 5 input",
    [Proxy::ParamId::Z1R5OU] = "Rule 5 output",
    [Proxy::ParamId::Z1R5IV] = "Rule 5 initial value (%)",
    [Proxy::ParamId::Z1R5TR] = "Rule 5 target",
    [Proxy::ParamId::Z1R5DT] = "Rule 5 distortion type",
    [Proxy::ParamId::Z1R5DL] = "Rule 5 distortion level (%)",
    [Proxy::ParamId::Z1R5MP] = "Rule 5 midpoint (%)",
    [Proxy::ParamId::Z1R5RS] = "Rule 5 reset on target change",
    [Proxy::ParamId::Z1R5NV] = "Rule 5 invert",
    [Proxy::ParamId::Z1R6IN] = "Rule 6 input",
    [Proxy::ParamId::Z1R6OU] = "Rule 6 output",
    [Proxy::ParamId::Z1R6IV] = "Rule 6 initial value (%)",
    [Proxy::ParamId::Z1R6TR] = "Rule 6 target",
    [Proxy::ParamId::Z1R6DT] = "Rule 6 distortion type",
    [Proxy::ParamId::Z1R6DL] = "Rule 6 distortion level (%)",
    [Proxy::ParamId::Z1R6MP] = "Rule 6 midpoint (%)",
    [Proxy::ParamId::Z1R6RS] = "Rule 6 reset on target change",
    [Proxy::ParamId::Z1R6NV] = "Rule 6 invert",
    [Proxy::ParamId::Z1R7IN] = "Rule 7 input",
    [Proxy::ParamId::Z1R7OU] = "Rule 7 output",
    [Proxy::ParamId::Z1R7IV] = "Rule 7 initial value (%)",
    [Proxy::ParamId::Z1R7TR] = "Rule 7 target",
    [Proxy::ParamId::Z1R7DT] = "Rule 7 distortion type",
    [Proxy::ParamId::Z1R7DL] = "Rule 7 distortion level (%)",
    [Proxy::ParamId::Z1R7MP] = "Rule 7 midpoint (%)",
    [Proxy::ParamId::Z1R7RS] = "Rule 7 reset on target change",
    [Proxy::ParamId::Z1R7NV] = "Rule 7 invert",
    [Proxy::ParamId::Z1R8IN] = "Rule 8 input",
    [Proxy::ParamId::Z1R8OU] = "Rule 8 output",
    [Proxy::ParamId::Z1R8IV] = "Rule 8 initial value (%)",
    [Proxy::ParamId::Z1R8TR] = "Rule 8 target",
    [Proxy::ParamId::Z1R8DT] = "Rule 8 distortion type",
    [Proxy::ParamId::Z1R8DL] = "Rule 8 distortion level (%)",
    [Proxy::ParamId::Z1R8MP] = "Rule 8 midpoint (%)",
    [Proxy::ParamId::Z1R8RS] = "Rule 8 reset on target change",
    [Proxy::ParamId::Z1R8NV] = "Rule 8 invert",
    [Proxy::ParamId::Z1R9IN] = "Rule 9 input",
    [Proxy::ParamId::Z1R9OU] = "Rule 9 output",
    [Proxy::ParamId::Z1R9IV] = "Rule 9 initial value (%)",
    [Proxy::ParamId::Z1R9TR] = "Rule 9 target",
    [Proxy::ParamId::Z1R9DT] = "Rule 9 distortion type",
    [Proxy::ParamId::Z1R9DL] = "Rule 9 distortion level (%)",
    [Proxy::ParamId::Z1R9MP] = "Rule 9 midpoint (%)",
    [Proxy::ParamId::Z1R9RS] = "Rule 9 reset on target change",
    [Proxy::ParamId::Z1R9NV] = "Rule 9 invert",
    [Proxy::ParamId::Z1TRB] = "Transpose below anchor",
    [Proxy::ParamId::Z1TRA] = "Transpose above anchor",
};


Strings::ParamFormat Strings::PARAM_FORMATS[Proxy::ParamId::PARAM_ID_COUNT] = {
    [Proxy::ParamId::MCM] = {Strings::TOGGLE_STATES, Strings::TOGGLE_STATES_COUNT},
    [Proxy::ParamId::Z1TYP] = {Strings::ZONE_TYPES, Strings::ZONE_TYPES_COUNT},
    [Proxy::ParamId::Z1CHN] = {Strings::CHANNELS, Strings::CHANNELS_COUNT},
    [Proxy::ParamId::Z1ENH] = {Strings::EXCESS_NOTE_HANDLINGS, Strings::EXCESS_NOTE_HANDLINGS_COUNT},
    [Proxy::ParamId::Z1ANC] = {Strings::ANCHORS, Strings::ANCHORS_COUNT},
    [Proxy::ParamId::Z1ORV] = {Strings::TOGGLE_STATES, Strings::TOGGLE_STATES_COUNT},

    [Proxy::ParamId::Z1R1IN] = {Strings::CONTROLLERS_SHORT, Strings::CONTROLLERS_COUNT},
    [Proxy::ParamId::Z1R1OU] = {Strings::CONTROLLERS_SHORT, Strings::CONTROLLERS_COUNT},
    [Proxy::ParamId::Z1R1IV] = {"%.2f%%", 100.0},
    [Proxy::ParamId::Z1R1TR] = {Strings::TARGETS_SHORT, Strings::TARGETS_COUNT},
    [Proxy::ParamId::Z1R1DT] = {Strings::DISTORTIONS, Strings::DISTORTIONS_COUNT},
    [Proxy::ParamId::Z1R1DL] = {"%.2f%%", 100.0},
    [Proxy::ParamId::Z1R1MP] = {"%.2f%%", 100.0},
    [Proxy::ParamId::Z1R1RS] = {Strings::RESETS, Strings::RESETS_COUNT},
    [Proxy::ParamId::Z1R1NV] = {Strings::TOGGLE_STATES, Strings::TOGGLE_STATES_COUNT},

    [Proxy::ParamId::Z1R2IN] = {Strings::CONTROLLERS_SHORT, Strings::CONTROLLERS_COUNT},
    [Proxy::ParamId::Z1R2OU] = {Strings::CONTROLLERS_SHORT, Strings::CONTROLLERS_COUNT},
    [Proxy::ParamId::Z1R2IV] = {"%.2f%%", 100.0},
    [Proxy::ParamId::Z1R2TR] = {Strings::TARGETS_SHORT, Strings::TARGETS_COUNT},
    [Proxy::ParamId::Z1R2DT] = {Strings::DISTORTIONS, Strings::DISTORTIONS_COUNT},
    [Proxy::ParamId::Z1R2DL] = {"%.2f%%", 100.0},
    [Proxy::ParamId::Z1R2MP] = {"%.2f%%", 100.0},
    [Proxy::ParamId::Z1R2RS] = {Strings::RESETS, Strings::RESETS_COUNT},
    [Proxy::ParamId::Z1R2NV] = {Strings::TOGGLE_STATES, Strings::TOGGLE_STATES_COUNT},

    [Proxy::ParamId::Z1R3IN] = {Strings::CONTROLLERS_SHORT, Strings::CONTROLLERS_COUNT},
    [Proxy::ParamId::Z1R3OU] = {Strings::CONTROLLERS_SHORT, Strings::CONTROLLERS_COUNT},
    [Proxy::ParamId::Z1R3IV] = {"%.2f%%", 100.0},
    [Proxy::ParamId::Z1R3TR] = {Strings::TARGETS_SHORT, Strings::TARGETS_COUNT},
    [Proxy::ParamId::Z1R3DT] = {Strings::DISTORTIONS, Strings::DISTORTIONS_COUNT},
    [Proxy::ParamId::Z1R3DL] = {"%.2f%%", 100.0},
    [Proxy::ParamId::Z1R3MP] = {"%.2f%%", 100.0},
    [Proxy::ParamId::Z1R3RS] = {Strings::RESETS, Strings::RESETS_COUNT},
    [Proxy::ParamId::Z1R3NV] = {Strings::TOGGLE_STATES, Strings::TOGGLE_STATES_COUNT},

    [Proxy::ParamId::Z1R4IN] = {Strings::CONTROLLERS_SHORT, Strings::CONTROLLERS_COUNT},
    [Proxy::ParamId::Z1R4OU] = {Strings::CONTROLLERS_SHORT, Strings::CONTROLLERS_COUNT},
    [Proxy::ParamId::Z1R4IV] = {"%.2f%%", 100.0},
    [Proxy::ParamId::Z1R4TR] = {Strings::TARGETS_SHORT, Strings::TARGETS_COUNT},
    [Proxy::ParamId::Z1R4DT] = {Strings::DISTORTIONS, Strings::DISTORTIONS_COUNT},
    [Proxy::ParamId::Z1R4DL] = {"%.2f%%", 100.0},
    [Proxy::ParamId::Z1R4MP] = {"%.2f%%", 100.0},
    [Proxy::ParamId::Z1R4RS] = {Strings::RESETS, Strings::RESETS_COUNT},
    [Proxy::ParamId::Z1R4NV] = {Strings::TOGGLE_STATES, Strings::TOGGLE_STATES_COUNT},

    [Proxy::ParamId::Z1R5IN] = {Strings::CONTROLLERS_SHORT, Strings::CONTROLLERS_COUNT},
    [Proxy::ParamId::Z1R5OU] = {Strings::CONTROLLERS_SHORT, Strings::CONTROLLERS_COUNT},
    [Proxy::ParamId::Z1R5IV] = {"%.2f%%", 100.0},
    [Proxy::ParamId::Z1R5TR] = {Strings::TARGETS_SHORT, Strings::TARGETS_COUNT},
    [Proxy::ParamId::Z1R5DT] = {Strings::DISTORTIONS, Strings::DISTORTIONS_COUNT},
    [Proxy::ParamId::Z1R5DL] = {"%.2f%%", 100.0},
    [Proxy::ParamId::Z1R5MP] = {"%.2f%%", 100.0},
    [Proxy::ParamId::Z1R5RS] = {Strings::RESETS, Strings::RESETS_COUNT},
    [Proxy::ParamId::Z1R5NV] = {Strings::TOGGLE_STATES, Strings::TOGGLE_STATES_COUNT},

    [Proxy::ParamId::Z1R6IN] = {Strings::CONTROLLERS_SHORT, Strings::CONTROLLERS_COUNT},
    [Proxy::ParamId::Z1R6OU] = {Strings::CONTROLLERS_SHORT, Strings::CONTROLLERS_COUNT},
    [Proxy::ParamId::Z1R6IV] = {"%.2f%%", 100.0},
    [Proxy::ParamId::Z1R6TR] = {Strings::TARGETS_SHORT, Strings::TARGETS_COUNT},
    [Proxy::ParamId::Z1R6DT] = {Strings::DISTORTIONS, Strings::DISTORTIONS_COUNT},
    [Proxy::ParamId::Z1R6DL] = {"%.2f%%", 100.0},
    [Proxy::ParamId::Z1R6MP] = {"%.2f%%", 100.0},
    [Proxy::ParamId::Z1R6RS] = {Strings::RESETS, Strings::RESETS_COUNT},
    [Proxy::ParamId::Z1R6NV] = {Strings::TOGGLE_STATES, Strings::TOGGLE_STATES_COUNT},

    [Proxy::ParamId::Z1R7IN] = {Strings::CONTROLLERS_SHORT, Strings::CONTROLLERS_COUNT},
    [Proxy::ParamId::Z1R7OU] = {Strings::CONTROLLERS_SHORT, Strings::CONTROLLERS_COUNT},
    [Proxy::ParamId::Z1R7IV] = {"%.2f%%", 100.0},
    [Proxy::ParamId::Z1R7TR] = {Strings::TARGETS_SHORT, Strings::TARGETS_COUNT},
    [Proxy::ParamId::Z1R7DT] = {Strings::DISTORTIONS, Strings::DISTORTIONS_COUNT},
    [Proxy::ParamId::Z1R7DL] = {"%.2f%%", 100.0},
    [Proxy::ParamId::Z1R7MP] = {"%.2f%%", 100.0},
    [Proxy::ParamId::Z1R7RS] = {Strings::RESETS, Strings::RESETS_COUNT},
    [Proxy::ParamId::Z1R7NV] = {Strings::TOGGLE_STATES, Strings::TOGGLE_STATES_COUNT},

    [Proxy::ParamId::Z1R8IN] = {Strings::CONTROLLERS_SHORT, Strings::CONTROLLERS_COUNT},
    [Proxy::ParamId::Z1R8OU] = {Strings::CONTROLLERS_SHORT, Strings::CONTROLLERS_COUNT},
    [Proxy::ParamId::Z1R8IV] = {"%.2f%%", 100.0},
    [Proxy::ParamId::Z1R8TR] = {Strings::TARGETS_SHORT, Strings::TARGETS_COUNT},
    [Proxy::ParamId::Z1R8DT] = {Strings::DISTORTIONS, Strings::DISTORTIONS_COUNT},
    [Proxy::ParamId::Z1R8DL] = {"%.2f%%", 100.0},
    [Proxy::ParamId::Z1R8MP] = {"%.2f%%", 100.0},
    [Proxy::ParamId::Z1R8RS] = {Strings::RESETS, Strings::RESETS_COUNT},
    [Proxy::ParamId::Z1R8NV] = {Strings::TOGGLE_STATES, Strings::TOGGLE_STATES_COUNT},

    [Proxy::ParamId::Z1R9IN] = {Strings::CONTROLLERS_SHORT, Strings::CONTROLLERS_COUNT},
    [Proxy::ParamId::Z1R9OU] = {Strings::CONTROLLERS_SHORT, Strings::CONTROLLERS_COUNT},
    [Proxy::ParamId::Z1R9IV] = {"%.2f%%", 100.0},
    [Proxy::ParamId::Z1R9TR] = {Strings::TARGETS_SHORT, Strings::TARGETS_COUNT},
    [Proxy::ParamId::Z1R9DT] = {Strings::DISTORTIONS, Strings::DISTORTIONS_COUNT},
    [Proxy::ParamId::Z1R9DL] = {"%.2f%%", 100.0},
    [Proxy::ParamId::Z1R9MP] = {"%.2f%%", 100.0},
    [Proxy::ParamId::Z1R9RS] = {Strings::RESETS, Strings::RESETS_COUNT},
    [Proxy::ParamId::Z1R9NV] = {Strings::TOGGLE_STATES, Strings::TOGGLE_STATES_COUNT},

    [Proxy::ParamId::Z1TRB] = {Strings::TRANSPOSE_OPTIONS, Strings::TRANSPOSE_OPTIONS_COUNT},
    [Proxy::ParamId::Z1TRA] = {Strings::TRANSPOSE_OPTIONS, Strings::TRANSPOSE_OPTIONS_COUNT},
};


char const* const* Strings::get_options(
        Proxy::ParamId const param_id,
        size_t& count
) noexcept {
    if (
            MPE_EMULATOR_UNLIKELY(
                param_id < Proxy::ParamId::MCM
                || param_id >= Proxy::ParamId::INVALID_PARAM_ID
            )
    ) {
        count = 0;

        return NULL;
    }

    ParamFormat const& param_format = PARAM_FORMATS[(size_t)param_id];

    count = param_format.number_of_options;

    return param_format.options;
}


void Strings::param_ratio_to_str(
        Proxy const& proxy,
        Proxy::ParamId const param_id,
        double const ratio,
        char* const buffer,
        size_t const buffer_size
) noexcept {
    if (
            MPE_EMULATOR_UNLIKELY(
                param_id < Proxy::ParamId::MCM
                || param_id >= Proxy::ParamId::INVALID_PARAM_ID
            )
    ) {
        buffer[0] = '\x00';

        return;
    }

    PARAM_FORMATS[(size_t)param_id].ratio_to_str(
        proxy, param_id, ratio, buffer, buffer_size
    );
}


constexpr Strings::ParamFormat::ParamFormat() noexcept
    : format(NULL),
    options(NULL),
    scale(1.0),
    number_of_options(0)
{
}


constexpr Strings::ParamFormat::ParamFormat(
        char const* const format,
        double const scale
) noexcept
    : format(format),
    options(NULL),
    scale(scale),
    number_of_options(0)
{
}


constexpr Strings::ParamFormat::ParamFormat(
        char const* const* const options,
        size_t const number_of_options
) noexcept
    : format(NULL),
    options(options),
    scale(1.0),
    number_of_options(number_of_options)
{
}


void Strings::ParamFormat::ratio_to_str(
        Proxy const& proxy,
        Proxy::ParamId const param_id,
        double const ratio,
        char* const buffer,
        size_t const buffer_size
) const noexcept {
    if (format != NULL) {
        ratio_to_str_float(ratio, buffer, buffer_size);
    } else if (options != NULL) {
        ratio_to_str_options(proxy, param_id, ratio, buffer, buffer_size);
    } else {
        ratio_to_str_int(proxy, param_id, ratio, buffer, buffer_size);
    }
}


void Strings::ParamFormat::ratio_to_str_float(
        double const ratio,
        char* const buffer,
        size_t const buffer_size
) const noexcept {
    double const value = ratio * scale;

    snprintf(buffer, buffer_size, format, value);

    bool minus_zero = buffer[0] == '-';

    for (size_t i = 1; minus_zero && i != buffer_size; ++i) {
        if (buffer[i] == '\x00') {
            break;
        }

        if (buffer[i] > '0' && buffer[i] <= '9') {
            minus_zero = false;
        }
    }

    if (minus_zero) {
        snprintf(buffer, buffer_size, format, 0.0);
    }

    buffer[buffer_size - 1] = '\x00';
}


void Strings::ParamFormat::ratio_to_str_options(
        Proxy const& proxy,
        Proxy::ParamId const param_id,
        double const ratio,
        char* const buffer,
        size_t const buffer_size
) const noexcept {
    size_t const value = (size_t)proxy.param_ratio_to_value(param_id, ratio);

    if (MPE_EMULATOR_UNLIKELY(value >= number_of_options)) {
        buffer[0] = '\x00';

        return;
    }

    strncpy(buffer, options[value], buffer_size);
    buffer[buffer_size - 1] = '\x00';
}


void Strings::ParamFormat::ratio_to_str_int(
        Proxy const& proxy,
        Proxy::ParamId const param_id,
        double const ratio,
        char* const buffer,
        size_t const buffer_size
) const noexcept {
    unsigned int const value = proxy.param_ratio_to_value(param_id, ratio);

    snprintf(buffer, buffer_size, "%u", value);
    buffer[buffer_size - 1] = '\x00';
}

}

#endif
