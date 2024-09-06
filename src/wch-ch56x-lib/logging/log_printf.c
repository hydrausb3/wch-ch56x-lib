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

#include "wch-ch56x-lib/logging/log_printf.h"
#include "wch-ch56x-lib/logging/logging_definitions.h"
#include "wch-ch56x-lib/logging/nanoprintf_impl.h"
#include "wch-ch56x-lib/utils/critical_section.h"

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

#define LOG_PRINTF_BUFF_SIZE (511)

static uint64_t startCNT64;

static debug_log_buf_t* debug_log_buf;

#ifdef CH56x_DEBUG_LOG_LIBDIVIDE_SYSLCLK
static struct libdivide_u64_t fast_u64_divSysClock;
static struct libdivide_u64_t fast_u64_divSysClock_nbtick_1ms;
static struct libdivide_u64_t fast_u64_divSysClock_nbtick_1us;
#endif

void log_printf_init(debug_log_buf_t* buf)
{
	debug_log_buf = buf;
	debug_log_buf->idx = 0;
	startCNT64 = bsp_get_SysTickCNT();
#ifdef CH56x_DEBUG_LOG_LIBDIVIDE_SYSLCLK
	fast_u64_divSysClock = libdivide_u64_gen(CH56x_DEBUG_LOG_LIBDIVIDE_SYSLCLK);
	fast_u64_divSysClock_nbtick_1ms =
		libdivide_u64_gen((CH56x_DEBUG_LOG_LIBDIVIDE_SYSLCLK / 1000ULL));
	fast_u64_divSysClock_nbtick_1us =
		libdivide_u64_gen((CH56x_DEBUG_LOG_LIBDIVIDE_SYSLCLK / 1000000ULL));
#endif
}

void _vlog_printf(const char* fmt, va_list args)
{
	int print_size1;
	int print_size2;
	uint64_t delta;
	uint64_t deltaCNT64;
	uint32_t sec;
	uint32_t msec;
	uint32_t usec;
	uint32_t tick_freq;
	char log_printf_buff[LOG_PRINTF_BUFF_SIZE + 1];

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

	BSP_ENTER_CRITICAL(); // Enter Critical Section
#ifdef CH56x_DEBUG_LOG_BASIC_TIMESTAMP
	print_size1 = npf_snprintf(log_printf_buff, sizeof(log_printf_buff),
							   "0x%08X ", (uint32_t)(delta));
#else
	print_size1 = npf_snprintf(log_printf_buff, sizeof(log_printf_buff),
							   "%02lus %03lums %03luus ", sec, msec, usec);
#endif

	print_size2 = npf_vsnprintf(&log_printf_buff[print_size1],
								LOG_PRINTF_BUFF_SIZE, fmt, args);
	print_size2 += print_size1;

	if (print_size2 > 0)
	{
		/* Save all log_printf data in a big buffer */
		int idx = debug_log_buf->idx;
		if ((idx + print_size2) < DEBUG_LOG_BUF_SIZE)
		{
			memcpy(&debug_log_buf->buf[idx], log_printf_buff, print_size2);
			debug_log_buf->idx += print_size2;
		}
		UART1_SendString((uint8_t*)log_printf_buff, print_size2);
	}
	BSP_EXIT_CRITICAL(); // Exit Critical Section
	bsp_uled_off();
}

void _log_printf(const char* fmt, ...)
{
	va_list va_args;
	va_start(va_args, fmt);
	_vlog_printf(fmt, va_args);
	va_end(va_args);
}

void _write_to_stdout(char* buffer, size_t size)
{
	UART1_SendString((uint8_t*)buffer, size);
}
