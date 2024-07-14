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

#ifndef MPE_EMULATOR__PROXY_HPP
#define MPE_EMULATOR__PROXY_HPP

#include <atomic>
#include <cstddef>
#include <string>
#include <vector>

#include "common.hpp"
#include "midi.hpp"
#include "note_stack.hpp"
#include "queue.hpp"
#include "spscqueue.hpp"


namespace MpeEmulator
{

/**
 * \warning Calling any method of a \c MpeEmulator object or its members
 *          outside the audio thread is not safe, unless indicated otherwise.
 */
class Proxy : public Midi::EventHandler
{
    public:
        enum MessageType {
            SET_PARAM = 1,          ///< Set the given parameter's ratio to
                                    ///< \c double_param.

            REFRESH_PARAM = 2,      ///< Make sure that \c get_param_ratio_atomic()
                                    ///< will return the most recent value of
                                    ///< the given parameter.

            CLEAR = 3,              ///< Clear all buffers and reset all
                                    ///< parameters to their default values.

            CLEAR_DIRTY_FLAG = 4,   ///< Clear the dirty flag.

            INVALID_MESSAGE_TYPE,
        };

        enum ParamId {
            MCM     = 0,            ///< Emit MCM on reset

            Z1TYP   = 1,            ///< Zone 1 type
            Z1CHN   = 2,            ///< Zone 1 channels
            Z1ENH   = 3,            ///< Zone 1 excess note handling
            Z1ANC   = 4,            ///< Zone 1 anchor
            Z1ORV   = 5,            ///< Zone 1 override release velocity with triggered velocity

            Z1R1IN  = 6,            ///< Zone 1 Rule 1 input
            Z1R1OU  = 7,            ///< Zone 1 Rule 1 output
            Z1R1IV  = 8,            ///< Zone 1 Rule 1 initial value
            Z1R1TR  = 9,            ///< Zone 1 Rule 1 target
            Z1R1DT  = 10,           ///< Zone 1 Rule 1 distortion type
            Z1R1DL  = 11,           ///< Zone 1 Rule 1 distortion level
            Z1R1MP  = 12,           ///< Zone 1 Rule 1 midpoint
            Z1R1RS  = 13,           ///< Zone 1 Rule 1 reset on target change
            Z1R1NV  = 14,           ///< Zone 1 Rule 1 invert

            Z1R2IN  = 15,           ///< Zone 1 Rule 2 input
            Z1R2OU  = 16,           ///< Zone 1 Rule 2 output
            Z1R2IV  = 17,           ///< Zone 1 Rule 2 initial value
            Z1R2TR  = 18,           ///< Zone 1 Rule 2 target
            Z1R2DT  = 19,           ///< Zone 1 Rule 2 distortion type
            Z1R2DL  = 20,           ///< Zone 1 Rule 2 distortion level
            Z1R2MP  = 21,           ///< Zone 1 Rule 2 midpoint
            Z1R2RS  = 22,           ///< Zone 1 Rule 2 reset on target change
            Z1R2NV  = 23,           ///< Zone 1 Rule 2 invert

            Z1R3IN  = 24,           ///< Zone 1 Rule 3 input
            Z1R3OU  = 25,           ///< Zone 1 Rule 3 output
            Z1R3IV  = 26,           ///< Zone 1 Rule 3 initial value
            Z1R3TR  = 27,           ///< Zone 1 Rule 3 target
            Z1R3DT  = 28,           ///< Zone 1 Rule 3 distortion type
            Z1R3DL  = 29,           ///< Zone 1 Rule 3 distortion level
            Z1R3MP  = 30,           ///< Zone 1 Rule 3 midpoint
            Z1R3RS  = 31,           ///< Zone 1 Rule 3 reset on target change
            Z1R3NV  = 32,           ///< Zone 1 Rule 3 invert

            Z1R4IN  = 33,           ///< Zone 1 Rule 4 input
            Z1R4OU  = 34,           ///< Zone 1 Rule 4 output
            Z1R4IV  = 35,           ///< Zone 1 Rule 4 initial value
            Z1R4TR  = 36,           ///< Zone 1 Rule 4 target
            Z1R4DT  = 37,           ///< Zone 1 Rule 4 distortion type
            Z1R4DL  = 38,           ///< Zone 1 Rule 4 distortion level
            Z1R4MP  = 39,           ///< Zone 1 Rule 4 midpoint
            Z1R4RS  = 40,           ///< Zone 1 Rule 4 reset on target change
            Z1R4NV  = 41,           ///< Zone 1 Rule 4 invert

            Z1R5IN  = 42,           ///< Zone 1 Rule 5 input
            Z1R5OU  = 43,           ///< Zone 1 Rule 5 output
            Z1R5IV  = 44,           ///< Zone 1 Rule 5 initial value
            Z1R5TR  = 45,           ///< Zone 1 Rule 5 target
            Z1R5DT  = 46,           ///< Zone 1 Rule 5 distortion type
            Z1R5DL  = 47,           ///< Zone 1 Rule 5 distortion level
            Z1R5MP  = 48,           ///< Zone 1 Rule 5 midpoint
            Z1R5RS  = 49,           ///< Zone 1 Rule 5 reset on target change
            Z1R5NV  = 50,           ///< Zone 1 Rule 5 invert

            Z1R6IN  = 51,           ///< Zone 1 Rule 6 input
            Z1R6OU  = 52,           ///< Zone 1 Rule 6 output
            Z1R6IV  = 53,           ///< Zone 1 Rule 6 initial value
            Z1R6TR  = 54,           ///< Zone 1 Rule 6 target
            Z1R6DT  = 55,           ///< Zone 1 Rule 6 distortion type
            Z1R6DL  = 56,           ///< Zone 1 Rule 6 distortion level
            Z1R6MP  = 57,           ///< Zone 1 Rule 6 midpoint
            Z1R6RS  = 58,           ///< Zone 1 Rule 6 reset on target change
            Z1R6NV  = 59,           ///< Zone 1 Rule 6 invert

            Z1R7IN  = 60,           ///< Zone 1 Rule 7 input
            Z1R7OU  = 61,           ///< Zone 1 Rule 7 output
            Z1R7IV  = 62,           ///< Zone 1 Rule 7 initial value
            Z1R7TR  = 63,           ///< Zone 1 Rule 7 target
            Z1R7DT  = 64,           ///< Zone 1 Rule 7 distortion type
            Z1R7DL  = 65,           ///< Zone 1 Rule 7 distortion level
            Z1R7MP  = 66,           ///< Zone 1 Rule 7 midpoint
            Z1R7RS  = 67,           ///< Zone 1 Rule 7 reset on target change
            Z1R7NV  = 68,           ///< Zone 1 Rule 7 invert

            Z1R8IN  = 69,           ///< Zone 1 Rule 8 input
            Z1R8OU  = 70,           ///< Zone 1 Rule 8 output
            Z1R8IV  = 71,           ///< Zone 1 Rule 8 initial value
            Z1R8TR  = 72,           ///< Zone 1 Rule 8 target
            Z1R8DT  = 73,           ///< Zone 1 Rule 8 distortion type
            Z1R8DL  = 74,           ///< Zone 1 Rule 8 distortion level
            Z1R8MP  = 75,           ///< Zone 1 Rule 8 midpoint
            Z1R8RS  = 76,           ///< Zone 1 Rule 8 reset on target change
            Z1R8NV  = 77,           ///< Zone 1 Rule 8 invert

            Z1R9IN  = 78,           ///< Zone 1 Rule 9 input
            Z1R9OU  = 79,           ///< Zone 1 Rule 9 output
            Z1R9IV  = 80,           ///< Zone 1 Rule 9 initial value
            Z1R9TR  = 81,           ///< Zone 1 Rule 9 target
            Z1R9DT  = 82,           ///< Zone 1 Rule 9 distortion type
            Z1R9DL  = 83,           ///< Zone 1 Rule 9 distortion level
            Z1R9MP  = 84,           ///< Zone 1 Rule 9 midpoint
            Z1R9RS  = 85,           ///< Zone 1 Rule 9 reset on target change
            Z1R9NV  = 86,           ///< Zone 1 Rule 9 invert

            Z1TRB   = 87,           ///< Zone 1 transpose below anchor
            Z1TRA   = 88,           ///< Zone 1 transpose above anchor

            PARAM_ID_COUNT = 89,
            INVALID_PARAM_ID = PARAM_ID_COUNT,
        };

        enum ControllerId {
            BANK_SELECT =               0,          ///< Bank Select (CC 0)
            MODULATION_WHEEL =          1,          ///< Modulation Wheel (CC 1)
            BREATH =                    2,          ///< Breath (CC 2)
            UNDEFINED_1 =               3,          ///< Undefined 1 (CC 3)
            FOOT_PEDAL =                4,          ///< Foot Pedal (CC 4)
            PORTAMENTO_TIME =           5,          ///< Portamento Time (CC 5)
            DATA_ENTRY =                6,          ///< Data Entry (CC 6)
            VOLUME =                    7,          ///< Volume (CC 7)
            BALANCE =                   8,          ///< Balance (CC 8)
            UNDEFINED_2 =               9,          ///< Undefined 2 (CC 9)
            PAN =                       10,         ///< Pan (CC 10)
            EXPRESSION_PEDAL =          11,         ///< Expression Pedal (CC 11)
            FX_CTL_1 =                  12,         ///< Effect Control 1 (CC 12)
            FX_CTL_2 =                  13,         ///< Effect Control 2 (CC 13)
            UNDEFINED_3 =               14,         ///< Undefined 3 (CC 14)
            UNDEFINED_4 =               15,         ///< Undefined 4 (CC 15)
            GENERAL_1 =                 16,         ///< General 1 (CC 16)
            GENERAL_2 =                 17,         ///< General 2 (CC 17)
            GENERAL_3 =                 18,         ///< General 3 (CC 18)
            GENERAL_4 =                 19,         ///< General 4 (CC 19)
            UNDEFINED_5 =               20,         ///< Undefined 5 (CC 20)
            UNDEFINED_6 =               21,         ///< Undefined 6 (CC 21)
            UNDEFINED_7 =               22,         ///< Undefined 7 (CC 22)
            UNDEFINED_8 =               23,         ///< Undefined 8 (CC 23)
            UNDEFINED_9 =               24,         ///< Undefined 9 (CC 24)
            UNDEFINED_10 =              25,         ///< Undefined 10 (CC 25)
            UNDEFINED_11 =              26,         ///< Undefined 11 (CC 26)
            UNDEFINED_12 =              27,         ///< Undefined 12 (CC 27)
            UNDEFINED_13 =              28,         ///< Undefined 13 (CC 28)
            UNDEFINED_14 =              29,         ///< Undefined 14 (CC 29)
            UNDEFINED_15 =              30,         ///< Undefined 15 (CC 30)
            UNDEFINED_16 =              31,         ///< Undefined 16 (CC 31)
            BANK_SELECT_LSB =           32,         ///< Bank Select LSB (14 bit) (CC 32)
            MODULATION_WHEEL_LSB =      33,         ///< Modulation Wheel LSB (14 bit) (CC 33)
            BREATH_LSB =                34,         ///< Breath LSB (14 bit) (CC 34)
            UNDEFINED_1_LSB =           35,         ///< Undefined 1 LSB (14 bit) (CC 35)
            FOOT_PEDAL_LSB =            36,         ///< Foot Pedal LSB (14 bit) (CC 36)
            PORTAMENTO_TIME_LSB =       37,         ///< Portamento Time LSB (14 bit) (CC 37)
            DATA_ENTRY_LSB =            38,         ///< Data Entry LSB (14 bit) (CC 38)
            VOLUME_LSB =                39,         ///< Volume LSB (14 bit) (CC 39)
            BALANCE_LSB =               40,         ///< Balance LSB (14 bit) (CC 40)
            UNDEFINED_2_LSB =           41,         ///< Undefined 2 LSB (14 bit) (CC 41)
            PAN_LSB =                   42,         ///< Pan LSB (14 bit) (CC 42)
            EXPRESSION_PEDAL_LSB =      43,         ///< Expression Pedal LSB (14 bit) (CC 43)
            FX_CTL_1_LSB =              44,         ///< Effect Control 1 LSB (14 bit) (CC 44)
            FX_CTL_2_LSB =              45,         ///< Effect Control 2 LSB (14 bit) (CC 45)
            UNDEFINED_3_LSB =           46,         ///< Undefined 3 LSB (14 bit) (CC 46)
            UNDEFINED_4_LSB =           47,         ///< Undefined 4 LSB (14 bit) (CC 47)
            GENERAL_1_LSB =             48,         ///< General 1 LSB (14 bit) (CC 48)
            GENERAL_2_LSB =             49,         ///< General 2 LSB (14 bit) (CC 49)
            GENERAL_3_LSB =             50,         ///< General 3 LSB (14 bit) (CC 50)
            GENERAL_4_LSB =             51,         ///< General 4 LSB (14 bit) (CC 51)
            UNDEFINED_5_LSB =           52,         ///< Undefined 5 LSB (14 bit) (CC 52)
            UNDEFINED_6_LSB =           53,         ///< Undefined 6 LSB (14 bit) (CC 53)
            UNDEFINED_7_LSB =           54,         ///< Undefined 7 LSB (14 bit) (CC 54)
            UNDEFINED_8_LSB =           55,         ///< Undefined 8 LSB (14 bit) (CC 55)
            UNDEFINED_9_LSB =           56,         ///< Undefined 9 LSB (14 bit) (CC 56)
            UNDEFINED_10_LSB =          57,         ///< Undefined 10 LSB (14 bit) (CC 57)
            UNDEFINED_11_LSB =          58,         ///< Undefined 11 LSB (14 bit) (CC 58)
            UNDEFINED_12_LSB =          59,         ///< Undefined 12 LSB (14 bit) (CC 59)
            UNDEFINED_13_LSB =          60,         ///< Undefined 13 LSB (14 bit) (CC 60)
            UNDEFINED_14_LSB =          61,         ///< Undefined 14 LSB (14 bit) (CC 61)
            UNDEFINED_15_LSB =          62,         ///< Undefined 15 LSB (14 bit) (CC 62)
            UNDEFINED_16_LSB =          63,         ///< Undefined 16 LSB (14 bit) (CC 63)
            SUSTAIN_PEDAL =             64,         ///< Sustain Pedal (CC 64)
            PORTAMENTO_ON_OFF =         65,         ///< Portamento On/Off (CC 65)
            SOSTENUTO =                 66,         ///< Sostenuto (CC 66)
            SOFT_PEDAL =                67,         ///< Soft Pedal (CC 67)
            LEGATO_FOOTSWITCH =         68,         ///< Legato Footswitch (CC 68)
            HOLD_2 =                    69,         ///< Hold 2 (CC 69)
            SOUND_1 =                   70,         ///< Sound 1 (CC 70)
            SOUND_2 =                   71,         ///< Sound 2 (CC 71)
            SOUND_3 =                   72,         ///< Sound 3 (CC 72)
            SOUND_4 =                   73,         ///< Sound 4 (CC 73)
            SOUND_5 =                   74,         ///< Sound 5 (CC 74)
            SOUND_6 =                   75,         ///< Sound 6 (CC 75)
            SOUND_7 =                   76,         ///< Sound 7 (CC 76)
            SOUND_8 =                   77,         ///< Sound 8 (CC 77)
            SOUND_9 =                   78,         ///< Sound 9 (CC 78)
            SOUND_10 =                  79,         ///< Sound 10 (CC 79)
            GENERAL_ON_OFF_1 =          80,         ///< General On/Off 1 (CC 80)
            GENERAL_ON_OFF_2 =          81,         ///< General On/Off 2 (CC 81)
            GENERAL_ON_OFF_3 =          82,         ///< General On/Off 3 (CC 82)
            GENERAL_ON_OFF_4 =          83,         ///< General On/Off 4 (CC 83)
            PORTAMENTO_CONTROL =        84,         ///< Portamento Control (CC 84)
            UNDEFINED_17 =              85,         ///< Undefined 17 (CC 85)
            UNDEFINED_18 =              86,         ///< Undefined 18 (CC 86)
            UNDEFINED_19 =              87,         ///< Undefined 19 (CC 87)
            UNDEFINED_20 =              88,         ///< Undefined 20 (CC 88)
            UNDEFINED_21 =              89,         ///< Undefined 21 (CC 90)
            UNDEFINED_22 =              90,         ///< Undefined 22 (CC 90)
            FX_1 =                      91,         ///< Effect 1 (CC 91)
            FX_2 =                      92,         ///< Effect 2 (CC 92)
            FX_3 =                      93,         ///< Effect 3 (CC 93)
            FX_4 =                      94,         ///< Effect 4 (CC 94)
            FX_5 =                      95,         ///< Effect 5 (CC 95)
            DATA_INCREMENT =            96,         ///< Data Increment (CC 96)
            DATA_DECREMENT =            97,         ///< Data Decrement (CC 97)
            NON_REG_PARAM_NUM_LSB =     98,         ///< Non-Registered Parameter Number LSB (CC 98)
            NON_REG_PARAM_NUM_MSB =     99,         ///< Non-Registered Parameter Number MSB (CC 99)
            REG_PARAM_NUM_LSB =         100,        ///< Registered Parameter Number LSB (CC 100)
            REG_PARAM_NUM_MSB =         101,        ///< Registered Parameter Number MSB (CC 101)
            UNDEFINED_23 =              102,        ///< Undefined 23 (CC 102)
            UNDEFINED_24 =              103,        ///< Undefined 24 (CC 103)
            UNDEFINED_25 =              104,        ///< Undefined 25 (CC 104)
            UNDEFINED_26 =              105,        ///< Undefined 26 (CC 105)
            UNDEFINED_27 =              106,        ///< Undefined 27 (CC 106)
            UNDEFINED_28 =              107,        ///< Undefined 28 (CC 107)
            UNDEFINED_29 =              108,        ///< Undefined 29 (CC 108)
            UNDEFINED_30 =              109,        ///< Undefined 30 (CC 109)
            UNDEFINED_31 =              110,        ///< Undefined 31 (CC 110)
            UNDEFINED_32 =              111,        ///< Undefined 32 (CC 111)
            UNDEFINED_33 =              112,        ///< Undefined 33 (CC 112)
            UNDEFINED_34 =              113,        ///< Undefined 34 (CC 113)
            UNDEFINED_35 =              114,        ///< Undefined 35 (CC 114)
            UNDEFINED_36 =              115,        ///< Undefined 36 (CC 115)
            UNDEFINED_37 =              116,        ///< Undefined 37 (CC 116)
            UNDEFINED_38 =              117,        ///< Undefined 38 (CC 117)
            UNDEFINED_39 =              118,        ///< Undefined 39 (CC 118)
            UNDEFINED_40 =              119,        ///< Undefined 40 (CC 119)

            MAX_MIDI_CC =               119,

            PITCH_WHEEL =               120,        ///< Pitch Wheel
            CHANNEL_PRESSURE =          121,        ///< Channel Pressure
            MIDI_LEARN =                122,        ///< MIDI Learn
            NONE =                      123,        ///< None

            CONTROLLER_ID_COUNT =       124,
            INVALID_CONTROLLER_ID =     CONTROLLER_ID_COUNT,
        };

        enum Toggle {
            OFF = 0,
            ON = 1,
        };

        enum ZoneType {
            ZT_LOWER = 0,
            ZT_UPPER = 1,
        };

        enum ExcessNoteHandling {
            ENH_IGNORE = 0,
            ENH_STEAL_LOWEST = 1,
            ENH_STEAL_HIGHEST = 2,
            ENH_STEAL_OLDEST = 3,
            ENH_STEAL_NEWEST = 4,
        };

        enum Target {
            TRG_GLOBAL = 0,
            TRG_ALL_BELOW_ANCHOR = 1,
            TRG_ALL_ABOVE_ANCHOR = 2,
            TRG_LOWEST = 3,
            TRG_HIGHEST = 4,
            TRG_OLDEST = 5,
            TRG_NEWEST = 6,
            TRG_LOWEST_BELOW_ANCHOR = 7,
            TRG_HIGHEST_BELOW_ANCHOR = 8,
            TRG_OLDEST_BELOW_ANCHOR = 9,
            TRG_NEWEST_BELOW_ANCHOR = 10,
            TRG_LOWEST_ABOVE_ANCHOR = 11,
            TRG_HIGHEST_ABOVE_ANCHOR = 12,
            TRG_OLDEST_ABOVE_ANCHOR = 13,
            TRG_NEWEST_ABOVE_ANCHOR = 14,
        };

        enum Reset {
            RST_OFF = 0,
            RST_LAST = 1,
            RST_INIT = 2,
        };

        class Param
        {
            public:
                Param(
                    std::string const& name,
                    unsigned int const min_value,
                    unsigned int const max_value,
                    unsigned int const default_value
                ) noexcept;

                std::string const& get_name() const noexcept;

                unsigned int get_min_value() const noexcept;
                unsigned int get_max_value() const noexcept;
                unsigned int get_value() const noexcept;
                double get_ratio() const noexcept;
                double get_default_ratio() const noexcept;

                double value_to_ratio(unsigned int const value) const noexcept;
                double ratio_to_value(double const ratio) const noexcept;

                void set_value(unsigned int const new_value) noexcept;

                void set_ratio(double const new_ratio) noexcept;

            private:
                unsigned int clamp_value(unsigned int const value) const noexcept;
                double clamp_ratio(double const ratio) const noexcept;

                std::string const name;

                double const min_value_dbl;
                double const range_dbl;
                double const range_inv;

                unsigned int const min_value;
                unsigned int const max_value;
                unsigned int const default_value;

                double ratio;
                unsigned int value;
        };

        class Rule
        {
            public:
                explicit Rule(
                    std::string const& name,
                    ControllerId const in_cc = ControllerId::NONE,
                    ControllerId const out_cc = ControllerId::NONE,
                    Target const target = Target::TRG_NEWEST,
                    unsigned int const init_value_ = 0,
                    Reset const reset = Reset::RST_INIT
                ) noexcept;

                double distort(double const value) const noexcept;

                bool needs_reset_for_note_event(
                    bool const is_above_anchor
                ) const noexcept;

                double get_reset_value() const noexcept;

                Param in_cc;
                Param out_cc;
                Param init_value;
                Param target;
                Param distortion_type;
                Param distortion_level;
                Param midpoint;
                Param reset;
                Param invert;

                double last_input_value;
        };

        class Message
        {
            public:
                Message() noexcept;
                Message(Message const& message) noexcept = default;
                Message(Message&& message) noexcept = default;

                Message(
                    MessageType const type,
                    ParamId const param_id,
                    double const double_param
                ) noexcept;

                Message& operator=(Message const& message) noexcept = default;
                Message& operator=(Message&& message) noexcept = default;

                MessageType type;
                ParamId param_id;
                double double_param;
        };

        typedef std::vector<Midi::Event> OutEvents;

        static constexpr size_t RULES = 9;

        Proxy() noexcept;
        ~Proxy();

        bool is_dirty() const noexcept;
        void clear_dirty_flag() noexcept;

        void suspend() noexcept;
        void resume() noexcept;

        /**
         * \brief Thread-safe way to change the state of the object outside
         *        the audio thread.
         */
        void push_message(
            MessageType const type,
            ParamId const param_id,
            double const double_param
        ) noexcept;

        /**
         * \brief Prepare to process MIDI events for the next block.
         */
        void begin_processing() noexcept;

        /**
         * \brief Thread-safe way to change the state of the object outside
         *        the audio thread.
         */
        void push_message(Message const& message) noexcept;

        /**
         * \brief Process all previously queued state changing messages inside
         *        the audio thread.
         */
        void process_messages() noexcept;

        /**
         * \brief Process a state changing message inside the audio thread.
         */
        void process_message(
            MessageType const type,
            ParamId const param_id,
            double const double_param
        ) noexcept;

        /**
         * \brief Process a state changing message inside the audio thread.
         */
        void process_message(Message const& message) noexcept;

        std::string const& get_param_name(ParamId const param_id) const noexcept;
        ParamId get_param_id(std::string const& name) const noexcept;

        unsigned int get_active_voices_count() const noexcept;
        unsigned int get_channel_count() const noexcept;

#ifdef MPE_EMULATOR_ASSERTIONS
        bool is_lock_free() const noexcept;

        void get_param_id_hash_table_statistics(
            unsigned int& max_collisions,
            double& avg_collisions,
            double& avg_bucket_size
        ) const noexcept;

        unsigned int get_param_value(ParamId const param_id) const noexcept;
#endif

        unsigned int param_ratio_to_value(
            ParamId const param_id,
            double const ratio
        ) const noexcept;

        double param_value_to_ratio(
            ParamId const param_id,
            unsigned int value
        ) const noexcept;

        unsigned int get_param_max_value(ParamId const param_id) const noexcept;
        double get_param_ratio_atomic(ParamId const param_id) const noexcept;
        double get_param_default_ratio(ParamId const param_id) const noexcept;

        void note_off(
            double const time_offset,
            Midi::Channel const channel,
            Midi::Note const note,
            Midi::Byte const velocity
        ) noexcept;

        void note_on(
            double const time_offset,
            Midi::Channel const channel,
            Midi::Note const note,
            Midi::Byte const velocity
        ) noexcept;

        void aftertouch(
            double const time_offset,
            Midi::Channel const channel,
            Midi::Note const note,
            Midi::Byte const pressure
        ) noexcept;

        void control_change(
            double const time_offset,
            Midi::Channel const channel,
            Midi::Controller const controller,
            Midi::Byte const new_value
        ) noexcept;

        void channel_pressure(
            double const time_offset,
            Midi::Channel const channel,
            Midi::Byte const pressure
        ) noexcept;

        void pitch_wheel_change(
            double const time_offset,
            Midi::Channel const channel,
            Midi::Word const new_value
        ) noexcept;

        void channel_mode(
            double const time_offset,
            Midi::Channel const channel,
            Midi::Byte const message,
            Midi::Byte const data
        ) noexcept;

        Param send_mcm;

        Param zone_type;
        Param channels;
        Param excess_note_handling;
        Param anchor;
        Param override_release_velocity;
        Param transpose_below_anchor;
        Param transpose_above_anchor;

        Rule rules[RULES];

        OutEvents const& out_events;

    private:
        class ParamIdHashTable
        {
            public:
                ParamIdHashTable() noexcept;
                ~ParamIdHashTable();

                void add(std::string const& name, ParamId const param_id) noexcept;
                ParamId lookup(std::string const& name) noexcept;

#ifdef MPE_EMULATOR_ASSERTIONS
                void get_statistics(
                    unsigned int& max_collisions,
                    double& avg_collisions,
                    double& avg_bucket_size
                ) const noexcept;
#endif

            private:
                class Entry
                {
                    public:
                        static constexpr size_t NAME_SIZE = 8;
                        static constexpr size_t NAME_MAX_INDEX = NAME_SIZE - 1;

                        Entry() noexcept;
                        Entry(const char* name, ParamId const param_id) noexcept;
                        ~Entry();

                        void set(const char* name, ParamId const param_id) noexcept;

                        Entry *next;
                        char name[NAME_SIZE];
                        ParamId param_id;
                };

                static constexpr size_t ENTRIES = 0x80;
                static constexpr size_t MASK = ENTRIES - 1;
                static constexpr int MULTIPLIER = 111;
                static constexpr int SHIFT = 5;

                static int hash(std::string const& name) noexcept;

                void lookup(
                    std::string const& name,
                    Entry** root,
                    Entry** parent,
                    Entry** entry
                ) noexcept;

                Entry entries[ENTRIES];
        };

        class MidiControllerMessage
        {
            public:
                MidiControllerMessage();
                MidiControllerMessage(MidiControllerMessage const& message) = default;
                MidiControllerMessage(MidiControllerMessage&& message) = default;

                MidiControllerMessage(double const time_offset, Midi::Word const value);

                bool operator==(MidiControllerMessage const& message) const noexcept;
                MidiControllerMessage& operator=(MidiControllerMessage const& message) noexcept = default;
                MidiControllerMessage& operator=(MidiControllerMessage&& message) noexcept = default;

            private:
                double time_offset;
                Midi::Word value;
        };

        struct ZoneTypeDescriptor
        {
            Midi::Channel manager_channel;
            Midi::Byte channel_increment;
        };

        static constexpr ZoneTypeDescriptor ZONE_TYPES[2] = {
            [ZoneType::ZT_LOWER] = {0, 1},
            [ZoneType::ZT_UPPER] = {Midi::CHANNEL_MAX, (Midi::Byte)-1},
        };

        static constexpr SPSCQueue<Message>::SizeType MESSAGE_QUEUE_SIZE = 8192;

        static ParamIdHashTable param_id_hash_table;
        static std::string param_names_by_id[ParamId::PARAM_ID_COUNT];

        static constexpr size_t MPE_MEMBER_CHANNELS_MAX = Midi::CHANNELS - 1;

        void register_param(ParamId const param_id, Param& param) noexcept;

        bool is_repeated_midi_controller_message(
            ControllerId const controller_id,
            double const time_offset,
            Midi::Channel const channel,
            Midi::Word const value
        ) noexcept;

        /**
         * \brief Reset the proxy's state: turn off the sustain pedal and all
         *        active notes, send new MCM (if configured), and reset all
         *        controllers, pitch bend, channel pressure, etc.
         */
        void reset() noexcept;

        void reset_available_channels() noexcept;

        bool handle_set_param(
            ParamId const param_id,
            double const ratio
        ) noexcept;

        void handle_refresh_param(ParamId const param_id) noexcept;
        bool handle_clear() noexcept;
        double get_param_ratio(ParamId const param_id) const noexcept;
        bool update_zone_config() noexcept;
        void stop_all_notes() noexcept;

        void push_mcms() noexcept;

        void push_mcm(
            Midi::Channel const channel,
            Midi::Channel const channel_count
        ) noexcept;

        void reset_rules_and_global_controllers() noexcept;

        void push_controller_event(
            double const time_offset,
            Midi::Channel const channel,
            ControllerId const controller_id,
            double const value,
            bool const is_note_on_setup = false
        ) noexcept;

        void push_note_on(
            double const time_offset,
            Midi::Channel const channel,
            Midi::Note const note,
            Midi::Byte velocity
        ) noexcept;

        template<bool is_note_on_setup>
        void push_resets_for_new_note(
                double const time_offset,
                Midi::Channel const new_note_channel,
                bool const is_above_anchor,
                NoteStack::ChannelStats const& old_channel_stats,
                NoteStack::ChannelStats const& old_channel_stats_below,
                NoteStack::ChannelStats const& old_channel_stats_above
        ) noexcept;

        void push_resets_for_note_off(
            double const time_offset,
            bool const was_above_anchor,
            NoteStack::ChannelStats const& old_channel_stats,
            NoteStack::ChannelStats const& old_channel_stats_below,
            NoteStack::ChannelStats const& old_channel_stats_above
        ) noexcept;

        void reset_outdated_targets_if_changed(
            Rule const& rule,
            double const time_offset,
            Midi::Channel const new_note_channel,
            NoteStack::ChannelStats const& a_channel_stats,
            NoteStack::ChannelStats const& a_channel_stats_below,
            NoteStack::ChannelStats const& a_channel_stats_above,
            NoteStack::ChannelStats const& b_channel_stats,
            NoteStack::ChannelStats const& b_channel_stats_below,
            NoteStack::ChannelStats const& b_channel_stats_above,
            double const reset_value,
            ControllerId const out_cc
        ) noexcept;

        void push_note_off(
            double const time_offset,
            Midi::Channel const channel,
            Midi::Note const note,
            Midi::Byte const velocity
        ) noexcept;

        template<Midi::Command midi_command>
        void process_controller_event(
            double const time_offset,
            ControllerId const controller_id,
            double const value
        ) noexcept;

        template<Midi::Command midi_command>
        void push_controller_event(
            double const time_offset,
            Midi::Channel const channel,
            ControllerId const controller_id,
            double const value,
            bool const is_note_on_setup = false
        ) noexcept;

        void push_out_event(Midi::Event const& event) noexcept;

        Midi::Note transpose(
            Midi::Note const note,
            bool const is_above_anchor
        ) const noexcept;

        OutEvents out_events_rw;
        MidiControllerMessage previous_controller_message[ControllerId::CONTROLLER_ID_COUNT];
        Param* params[ParamId::PARAM_ID_COUNT];
        Queue<Midi::Channel, MPE_MEMBER_CHANNELS_MAX> available_channels;
        NoteStack::ChannelsByNotes channels_by_notes;
        Midi::Byte velocities_by_notes[Midi::NOTES];
        std::atomic<double> param_ratios_atomic[ParamId::PARAM_ID_COUNT];
        SPSCQueue<Message> messages;
        std::atomic<unsigned int> active_voices_count_atomic;
        std::atomic<unsigned int> channel_count_atomic;
        NoteStack note_stack;
        NoteStack note_stack_below;
        NoteStack note_stack_above;
        NoteStack::ChannelStats channel_stats;
        NoteStack::ChannelStats channel_stats_below;
        NoteStack::ChannelStats channel_stats_above;
        int offset_below_anchor;
        int offset_above_anchor;
        Midi::Note anchor_;
        Midi::Channel channel_count;
        Midi::Channel manager_channel;
        Midi::Channel channel_increment;
        Midi::Channel first_channel;
        Midi::Channel last_channel;

        bool is_suspended;
        bool is_dirty_;
        bool had_reset;
};

}

#endif
