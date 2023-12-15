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

#ifndef LOGGING_H
#define LOGGING_H

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

/** Logging HOWTO
 * Choose a logging method : LOG_TYPE_x=1
 * Choose a log level : LOG_LEVEL=x
 * Or choose to log only logs with a certain ID : LOG_FILTER_IDS=x,y,z, ...
 *
 * If you choose both LOG_LEVEL and LOG_FILTER_IDS, the selected ids will have
 * to be of the right LOG_LEVEL to be shown If you don't choose either LOG_LEVEL
 * or LOG_FILTER_IDS, LOG_LEVEL=1 (critical) if you activated a logging method
 *
 * Then, call LOG_INIT(freq_sys); and depending on the method, LOG_DUMP() if
 * logs are stored somewhere and need to be flushed at some point. By default,
 * the macros are equivalent to empty while loops and will be optimized out
 */

#if (defined LOG_TYPE_PRINTF)
#include "wch-ch56x-lib/logging/log_printf.h"
#endif

#if (defined LOG_TYPE_SERDES)
#include "wch-ch56x-lib/logging/log_serdes.h"
#endif

#if (defined LOG_TYPE_BUFFER)
#include "wch-ch56x-lib/logging/log_to_buffer.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the logging method defined by LOG_TYPE_x
 * @param sys_freq
 */
void _log_init(int sys_freq);

/**
 * @brief (Re)-initialize timebase
 */
void log_time_init(void);

/**
 * @brief Log to the LOG_TYPE_x output
 * @param fmt printf style format string
 * @param
 */
void _log(const char* fmt, ...);

/**
 * @brief Log if log_prio <= LOG_LEVEL, or log_id in LOG_FILTER_IDS or both
 * @param log_prio
 * @param log_id
 * @param fmt
 * @param
 */
void log_if(uint8_t log_prio, uint8_t log_id, const char* fmt, ...);

#if (defined LOG_TYPE_PRINTF) || (defined LOG_TYPE_SERDES) || \
	(defined LOG_TYPE_BUFFER)
#define LOG_INIT(sys_freq) _log_init(sys_freq)
#define LOG_IF(log_prio, log_id, ...) log_if(log_prio, log_id, __VA_ARGS__)
#define LOG_IF_LEVEL(log_prio, ...) log_if(log_prio, LOG_ID_NONE, __VA_ARGS__)
#define LOG(...) _log(__VA_ARGS__)
#else

#define LOG_INIT(sys_freq) \
	do {                   \
	} while (0)
#define LOG_IF(log_prio, log_id, ...) \
	do {                              \
	} while (0)
#define LOG_IF_LEVEL(log_prio, ...) \
	do {                            \
	} while (0)
#define LOG(...) \
	do {         \
	} while (0)
#endif

#ifdef LOG_TYPE_BUFFER
#define LOG_DUMP() log_to_buffer_dump_to_uart()
#else
#define LOG_DUMP() \
	do {           \
	} while (0)
#endif

#ifdef __cplusplus
}
#endif

#endif
