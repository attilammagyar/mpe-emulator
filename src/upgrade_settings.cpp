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

#include <cerrno>
#include <cstring>
#include <istream>
#include <iostream>
#include <fstream>
#include <ostream>

#include "common.hpp"
#include "serializer.hpp"
#include "proxy.hpp"


bool read_settings(char const* const file_path, std::string& result)
{
    std::ifstream settings_file(file_path, std::ios::in | std::ios::binary);

    if (!settings_file.is_open()) {
        return false;
    }

    char* const buffer = new char[MpeEmulator::Serializer::MAX_SIZE];

    std::fill_n(buffer, MpeEmulator::Serializer::MAX_SIZE, '\x00');
    settings_file.read(buffer, MpeEmulator::Serializer::MAX_SIZE);

    result = buffer;

    delete[] buffer;

    return true;
}


bool is_whole_line_comment_or_white_space(std::string const& line)
{
    std::string::const_iterator it = line.begin();

    return MpeEmulator::Serializer::skipping_remaining_whitespace_or_comment_reaches_the_end(
        it, line.end()
    );
}


void collect_comments(std::string const& settings, MpeEmulator::Serializer::Lines& comments)
{
    MpeEmulator::Serializer::Lines* const lines = MpeEmulator::Serializer::parse_lines(settings);

    for (MpeEmulator::Serializer::Lines::const_iterator it = lines->begin(); it != lines->end(); ++it) {
        std::string const& line = *it;

        if (is_whole_line_comment_or_white_space(line)) {
            comments.push_back(line);
        }
    }
}


bool write_settings(
        char const* const file_path,
        std::string const& settings,
        MpeEmulator::Serializer::Lines const& comments
) {
    std::ofstream settings_file(file_path, std::ios::out | std::ios::binary);

    if (!settings_file.is_open()) {
        return false;
    }

    std::string const line_end = MpeEmulator::Serializer::LINE_END;

    for (MpeEmulator::Serializer::Lines::const_iterator it = comments.begin(); it != comments.end(); ++it) {
        std::string const& comment = *it;
        std::cout << "c: " << comment << std::endl;

        settings_file.write(comment.c_str(), comment.length());
        settings_file.write(line_end.c_str(), line_end.length());
    }

    settings_file.write(settings.c_str(), settings.length());

    return true;
}


int error(char const* const message, char const* const file_path)
{
    std::cerr
        << "ERROR: "
        << message
        << std::endl
        << "  File: " << file_path << std::endl
        << "  Errno: " << errno << std::endl
        << "  Message: " << std::strerror(errno) << std::endl;

    return 1;
}


int upgrade_settings(char const* const settings_file)
{
    std::cout << "Upgrading " << settings_file << std::endl;

    std::string settings;

    if (!read_settings(settings_file, settings)) {
        return error("Error reading settings file", settings_file);
    }

    MpeEmulator::Proxy proxy;
    MpeEmulator::Serializer::Lines comments;

    MpeEmulator::Serializer::import_settings_in_audio_thread(proxy, settings);

    collect_comments(settings, comments);

    if (!write_settings(settings_file, MpeEmulator::Serializer::serialize(proxy), comments)) {
        return error("Error writing settings file", settings_file);
    }

    return 0;
}


int main(int argc, char const* argv[])
{
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " settings_file.mp" << std::endl;

        return 1;
    }

    return upgrade_settings(argv[1]);
}
