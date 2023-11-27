/********************************** (C) COPYRIGHT
******************************* Copyright (c) 2023 Quarkslab

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

#ifndef LOGGING_DEFINITIONS_H
#define LOGGING_DEFINITIONS_H

// log levels. Custom log levels can be defined and must be > MAX_LOG_LEVEL.

#if !(defined LOG_LEVEL) && !(defined LOG_FILTER_IDS)
#if (defined LOG_TYPE_PRINTF) || (defined LOG_TYPE_PRINTF_NO_INTERRUPT) || \
	(defined LOG_TYPE_SERDES) || (defined LOG_TYPE_BUFFER)
#define LOG_LEVEL 1
#endif
#endif

#define LOG_LEVEL_CRITICAL 1
#define LOG_LEVEL_ERROR 2
#define LOG_LEVEL_WARNING 3
#define LOG_LEVEL_INFO 4
#define LOG_LEVEL_DEBUG 5
#define LOG_LEVEL_TRACE 6
#define MAX_LOG_LEVEL 6

#define LOG_ID_NONE 0

#define LOG_ID_USER 1
#define LOG_ID_USB2 2
#define LOG_ID_USB3 3
#define LOG_ID_HSPI 4
#define LOG_ID_SERDES 5
#define LOG_ID_INTERRUPT_QUEUE 6
#define LOG_ID_RAMX_ALLOC 7
#define LOG_ID_TRACE 8

#ifndef LOG_BAUDRATE
#define LOG_BAUDRATE 5000000
#endif

#ifndef LOG_BUFFER_SIZE
#define LOG_BUFFER_SIZE 4096 // in bytes
#endif

#define DEBUG_LOG_BUF_SIZE 512 // max size of the formated string

typedef struct
{
	char buf[DEBUG_LOG_BUF_SIZE + 1];
	volatile uint16_t idx; /* Position in the buffer */
} debug_log_buf_t;

#endif