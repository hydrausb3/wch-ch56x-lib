/********************************** (C) COPYRIGHT *******************************
Copyright (c) 2022 Benjamin VERNOUX
Copyright (c) 2023 Quarkslab

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*******************************************************************************/

#ifndef LOG_PRINTF_H
#define LOG_PRINTF_H

// Disable warnings in bsp arising from -pedantic -Wall -Wconversion
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic ignored "-Wvariadic-macros"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#include "CH56x_common.h"
#include "CH56xSFR.h"
#pragma GCC diagnostic pop
#pragma GCC diagnostic pop
#pragma GCC diagnostic pop

#include "wch-ch56x-lib/logging/logging_definitions.h"

#include <stdarg.h>
#include <stdint.h>

#ifdef LOG_TYPE_PRINTF
#define LOG_PRINTF(...) log_printf(__VA_ARGS__)
#else
#define LOG_PRINTF(...) \
	do {                \
	} while (0)
#endif

#ifdef __cplusplus
extern "C" {
#endif

void log_printf_init(debug_log_buf_t* buf);

/*
 * log_printf() write formatted text data with "timestamp" prefix format "%02us
 * %03ums %03uus " log_printf() output text data on both stdout and
 * debug_log_buf buffer Note: debug_log_buf buffer use debug_log_buf_idx with
 * max size DEBUG_LOG_BUF_SIZE Warning: log_printf() can output at maximum
 * LOG_PRINTF_BUFF_SIZE Can be used in main code & interrupts (use
 * bsp_disable_interrupts() / bsp_enable_interrupts())
 */
void _log_printf(const char* fmt, ...);

void _vlog_printf(const char* fmt, va_list args);

void _write_to_stdout(char* buffer, size_t size);

#ifdef __cplusplus
}
#endif

#endif
