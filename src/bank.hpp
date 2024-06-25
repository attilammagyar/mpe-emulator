/*
 * This file is part of MPE Emulator.
 * Copyright (C) 2023, 2024  Attila M. Magyar
 * Copyright (C) 2023  Patrik Ehringer
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

#ifndef MPE_EMULATOR__BANK_HPP
#define MPE_EMULATOR__BANK_HPP

#include <cstddef>
#include <string>

#include "common.hpp"
#include "serializer.hpp"


namespace MpeEmulator
{

class Bank
{
    public:
        class Program
        {
            public:
                static constexpr size_t NAME_MAX_LENGTH = 24;
                static constexpr size_t SHORT_NAME_MAX_LENGTH = 8;

                Program();

                Program(
                    std::string const& name,
                    std::string const& default_name,
                    std::string const& serialized
                );

                Program(Program const& program) = default;
                Program(Program&& program) = default;

                Program& operator=(Program const& program) = default;
                Program& operator=(Program&& program) = default;

                std::string const& get_name() const;
                std::string const& get_short_name() const;
                void set_name(std::string const& new_name);

                bool is_blank() const;

                std::string const& serialize() const;

                void import(std::string const& serialized);

                void import(
                    Serializer::Lines::const_iterator& it,
                    Serializer::Lines::const_iterator const& end
                );

            private:
                std::string sanitize_name(std::string const& name) const;

                std::string truncate(
                    std::string const& text,
                    std::string::size_type const max_length
                ) const;

                bool is_allowed_char(char const c) const;

                void set_name_without_update(std::string const& new_name);

                void import_without_update(std::string const& serialized);

                void import_without_update(
                    Serializer::Lines::const_iterator& it,
                    Serializer::Lines::const_iterator const& end
                );

                void update();

                std::string name;
                std::string short_name;
                std::string default_name;
                std::string serialized;
                std::string::size_type params_start;
        };

        static constexpr size_t NUMBER_OF_PROGRAMS = 128;

        static size_t normalized_parameter_value_to_program_index(
            double const parameter_value
        );
        static double program_index_to_normalized_parameter_value(
            size_t const index
        );

        Bank();

        Program& operator[](size_t const index);
        Program const& operator[](size_t const index) const;

        size_t get_current_program_index() const;
        void set_current_program_index(size_t const new_index);

        void import(std::string const& serialized_bank);
        void import_names(std::string const& serialized_bank);
        std::string serialize() const;

    private:
        static size_t const NUMBER_OF_BUILT_IN_PROGRAMS;
        static Program const BUILT_IN_PROGRAMS[];
        static constexpr double FLOAT_TO_PROGRAM_INDEX_SCALE = (
            (double)(NUMBER_OF_PROGRAMS - 1)
        );
        static constexpr double PROGRAM_INDEX_TO_FLOAT_SCALE = (
            1.0 / (double)(NUMBER_OF_PROGRAMS - 1)
        );

        void generate_empty_programs(size_t const start_index);

        Program programs[NUMBER_OF_PROGRAMS];
        size_t current_program_index;
        size_t non_blank_programs;
};

}

#endif
