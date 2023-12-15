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

#ifndef LOG_TO_BUFFER_H
#define LOG_TO_BUFFER_H
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

#ifdef LOG_TYPE_BUFFER
#define LOG_TO_BUFFER(...) log_to_buffer(__VA_ARGS__)
#else
#define LOG_TO_BUFFER(...) \
	do {                   \
	} while (0)
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Init buffer state. Based on lwrb's ringbuffer.
 * @param
 */
void log_buffer_init(void);

/**
 * @brief Read num bytes to buffer if available, or less if num bytes are not
 * available
 * @param buffer pointer to buffer where num bytes will be written
 * @param num the number of bytes that we want to read
 * @return
 */
size_t log_buffer_read_to(void* buffer, size_t num);

/**
 * @brief Get the number of bytes currently stored in the buffer
 * @param
 * @return
 */
size_t log_buffer_get_num_ready(void);

/**
 * @brief Write a string to the buffer, syntax like printf
 * @param fmt format string
 * @param
 */
void log_to_buffer(const char* fmt, ...);

void vlog_to_buffer(const char* fmt, va_list args);

/**
 * Write all current data in the ringbuffer to UART1
 */
void log_to_buffer_dump_to_uart(void);

void _write_to_buffer(char* buffer, size_t size);

#ifdef __cplusplus
}
#endif

#endif
