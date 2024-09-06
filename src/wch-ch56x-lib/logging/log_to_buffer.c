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

#include "wch-ch56x-lib/logging/log_to_buffer.h"
#include "wch-ch56x-lib/logging/nanoprintf_impl.h"
#include "wch-ch56x-lib/utils/critical_section.h"
#include <stdarg.h>
#include "lwrb/lwrb.h"

// Add option in pre-processor compiler option
// CH56x_DEBUG_LOG_BASIC_TIMESTAMP=1
// or
// CH56x_DEBUG_LOG_LIBDIVIDE_SYSLCLK=120000000

#ifdef CH56x_DEBUG_LOG_BASIC_TIMESTAMP
#ifdef CH56x_DEBUG_LOG_LIBDIVIDE_SYSLCLK
#error \
	"CH56x_DEBUG_LOG_BASIC_TIMESTAMP and CH56x_DEBUG_LOG_LIBDIVIDE_SYSLCLK shall not be set together"
#endif
#endif

#ifdef CH56x_DEBUG_LOG_BASIC_TIMESTAMP
#undef CH56x_DEBUG_LOG_LIBDIVIDE_SYSLCLK
#endif

#ifdef CH56x_DEBUG_LOG_LIBDIVIDE_SYSLCLK
#include "libdivide.h"
#endif

#include "wch-ch56x-lib/logging/logging_definitions.h"

#define LOG_PRINTF_BUFF_SIZE (511)

// Must be defined by user
// #ifndef LOG_BUFFER_SIZE
// #define LOG_BUFFER_SIZE (6000)
// #endif

static uint64_t startCNT64;

#ifdef CH56x_DEBUG_LOG_LIBDIVIDE_SYSLCLK
static struct libdivide_u64_t fast_u64_divSysClock;
static struct libdivide_u64_t fast_u64_divSysClock_nbtick_1ms;
static struct libdivide_u64_t fast_u64_divSysClock_nbtick_1us;
#endif

__attribute__((aligned(16))) static uint8_t log_buffer_uart[512]
	__attribute__((section(".DMADATA")));
__attribute__((aligned(16))) char log_buffer[LOG_BUFFER_SIZE + 1]
	__attribute__((section(".DMADATA")));

lwrb_t lwrb_buffer;

void log_buffer_init(void)
{
	startCNT64 = bsp_get_SysTickCNT();
#ifdef CH56x_DEBUG_LOG_LIBDIVIDE_SYSLCLK
	fast_u64_divSysClock = libdivide_u64_gen(CH56x_DEBUG_LOG_LIBDIVIDE_SYSLCLK);
	fast_u64_divSysClock_nbtick_1ms =
		libdivide_u64_gen((CH56x_DEBUG_LOG_LIBDIVIDE_SYSLCLK / 1000ULL));
	fast_u64_divSysClock_nbtick_1us =
		libdivide_u64_gen((CH56x_DEBUG_LOG_LIBDIVIDE_SYSLCLK / 1000000ULL));
#endif
	lwrb_init(&lwrb_buffer, log_buffer, sizeof(log_buffer));
}

size_t log_buffer_read_to(void* buffer, size_t num)
{
	return lwrb_read(&lwrb_buffer, buffer, num);
}

size_t log_buffer_get_num_ready(void) { return lwrb_get_full(&lwrb_buffer); }

void vlog_to_buffer(const char* fmt, va_list args)
{
	int print_size1;
	int print_size2;
	uint64_t delta;
	uint64_t deltaCNT64;
	uint32_t sec;
	uint32_t msec;
	uint32_t usec;
	uint32_t tick_freq;
	char temp_buffer[512];

	delta = startCNT64 -
			bsp_get_SysTickCNT(); // CNT is decremented so comparison is inverted
	deltaCNT64 = delta;

	tick_freq = bsp_get_tick_frequency();
#ifndef CH56x_DEBUG_LOG_BASIC_TIMESTAMP
#ifdef CH56x_DEBUG_LOG_LIBDIVIDE_SYSLCLK
	{
		// Fast, computes division using libdivide
		sec = (uint32_t)libdivide_u64_do(deltaCNT64, &fast_u64_divSysClock);
		deltaCNT64 -= ((uint64_t)(sec) * (uint64_t)(tick_freq));

		msec = (uint32_t)libdivide_u64_do(deltaCNT64,
										  &fast_u64_divSysClock_nbtick_1ms);
		deltaCNT64 -= ((uint64_t)(msec) *
					   (uint64_t)((CH56x_DEBUG_LOG_LIBDIVIDE_SYSLCLK / 1000ULL)));

		usec = (uint32_t)libdivide_u64_do(deltaCNT64,
										  &fast_u64_divSysClock_nbtick_1us);
	}
#else
	{
		uint32_t nbtick_1ms = bsp_get_nbtick_1ms();

		sec = (uint32_t)(deltaCNT64 / (uint64_t)(tick_freq));
		deltaCNT64 -= ((uint64_t)(sec) * (uint64_t)(tick_freq));

		msec = (uint32_t)(deltaCNT64 / (uint64_t)(nbtick_1ms));
		deltaCNT64 -= ((uint64_t)(msec) * (uint64_t)(nbtick_1ms));

		usec = (uint32_t)(deltaCNT64 / (uint64_t)(bsp_get_nbtick_1us()));
	}
#endif
#endif // ifndef CH56x_DEBUG_LOG_BASIC_TIMESTAMP

#ifdef CH56x_DEBUG_LOG_BASIC_TIMESTAMP
	print_size1 = npf_snprintf(temp_buffer, sizeof(temp_buffer), "0x%08X ",
							   (uint32_t)(delta));
#else
	print_size1 = npf_snprintf(temp_buffer, sizeof(temp_buffer),
							   "%02lus %03lums %03lus ", sec, msec, usec);
#endif

	print_size2 = npf_vsnprintf(&temp_buffer[print_size1],
								LOG_PRINTF_BUFF_SIZE - print_size1, fmt, args);
	print_size2 += print_size1;
	if (print_size2 > 0)
	{
		BSP_ENTER_CRITICAL();
		lwrb_write(&lwrb_buffer, temp_buffer, print_size2);
		BSP_EXIT_CRITICAL();
	}
}

void log_to_buffer(const char* fmt, ...)
{
	va_list va_args;
	va_start(va_args, fmt);
	vlog_to_buffer(fmt, va_args);
	va_end(va_args);
}

void log_to_buffer_dump_to_uart(void)
{
	size_t num_log_bytes_ready = log_buffer_get_num_ready();
	do {
		size_t num_to_read = num_log_bytes_ready > sizeof(log_buffer_uart)
								 ? sizeof(log_buffer_uart)
								 : num_log_bytes_ready;
		if (num_to_read > 0)
		{
			size_t num_read = log_buffer_read_to(
				log_buffer_uart,
				num_to_read > sizeof(uint16_t) ? sizeof(uint16_t) : num_to_read);
			UART1_SendString(log_buffer_uart, num_read);
		}
		num_log_bytes_ready = log_buffer_get_num_ready();
	} while (num_log_bytes_ready > 0);
}

void _write_to_buffer(char* buffer, size_t size)
{
	BSP_ENTER_CRITICAL();
	lwrb_write(&lwrb_buffer, buffer, size);
	BSP_EXIT_CRITICAL();
}
