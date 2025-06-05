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

#ifndef MPE_EMULATOR__DEBUG_HPP
#define MPE_EMULATOR__DEBUG_HPP

#ifndef MPE_EMULATOR_DEBUG_LOG

#define MPE_EMULATOR_DEBUG(message_template, ...)
#define MPE_EMULATOR_DEBUG_ARRAY(message, arr, len, format)

#endif

#ifdef MPE_EMULATOR_DEBUG_LOG

#include <cstdio>
#include <cstring>

#include "common.hpp"


#ifdef _WIN32
#include <windows.h>
#define _MPE_EMULATOR_GET_TID() ((unsigned int)GetCurrentThreadId())
#define _MPE_EMULATOR_TID_FMT "\tTID=%#x"
#elif __linux__ || __gnu_linux__
#include <unistd.h>
#include <sys/syscall.h>
#define _MPE_EMULATOR_GET_TID() ((unsigned int)syscall(SYS_gettid))
#define _MPE_EMULATOR_TID_FMT "\tTID=%#x"
#else
#define _MPE_EMULATOR_GET_TID() ("")
#define _MPE_EMULATOR_TID_FMT "%s"
#endif


#define _MPE_EMULATOR_DEBUG_CTX(debug_action) do {                          \
    char const* const _mpe_last_slash = strrchr(__FILE__, '/');             \
    char const* const _mpe_basename = (                                     \
        _mpe_last_slash != NULL ? _mpe_last_slash + 1 : __FILE__            \
    );                                                                      \
    bool const _mpe_use_stderr = (                                          \
        strncmp(                                                            \
            "STDERR",                                                       \
            MPE_EMULATOR_TO_STRING(MPE_EMULATOR_DEBUG_LOG),                 \
            7                                                               \
        ) == 0                                                              \
    );                                                                      \
    FILE* const _mpe_f = (                                                  \
        _mpe_use_stderr                                                     \
            ? stderr                                                        \
            : fopen(MPE_EMULATOR_TO_STRING(MPE_EMULATOR_DEBUG_LOG), "a+")   \
    );                                                                      \
                                                                            \
    if (_mpe_f) {                                                           \
        fprintf(                                                            \
            _mpe_f,                                                         \
            "%s:%d/%s():" _MPE_EMULATOR_TID_FMT "\t",                       \
            _mpe_basename,                                                  \
            __LINE__,                                                       \
            __FUNCTION__,                                                   \
            _MPE_EMULATOR_GET_TID()                                         \
        );                                                                  \
                                                                            \
        debug_action;                                                       \
                                                                            \
        fprintf(_mpe_f, "\n");                                              \
                                                                            \
        if (!_mpe_use_stderr) {                                             \
            fclose(_mpe_f);                                                 \
        }                                                                   \
    }                                                                       \
} while (false)


#define MPE_EMULATOR_DEBUG(message_template, ...) do {                      \
    _MPE_EMULATOR_DEBUG_CTX(                                                \
        do {                                                                \
            fprintf(_mpe_f, (message_template), ## __VA_ARGS__);            \
        } while (false)                                                     \
    );                                                                      \
} while (false)


#define MPE_EMULATOR_DEBUG_ARRAY(message, array, length, format_string)     \
    _MPE_EMULATOR_DEBUG_CTX(                                                \
        if ((array) == NULL) {                                              \
            fprintf(_mpe_f, "%s: <NULL>", (message));                       \
        } else {                                                            \
            int _mpe_i;                                                     \
                                                                            \
            fprintf(_mpe_f, "%s: [ ", (message));                           \
                                                                            \
            for (_mpe_i = 0; _mpe_i < (length); ++_mpe_i) {                 \
                if (_mpe_i > 0) {                                           \
                    fprintf(_mpe_f, ", ");                                  \
                }                                                           \
                                                                            \
                fprintf(_mpe_f, (format_string), (array)[_mpe_i]);          \
            }                                                               \
                                                                            \
            fprintf(_mpe_f, " ]");                                          \
        }                                                                   \
    )

#endif

#endif
