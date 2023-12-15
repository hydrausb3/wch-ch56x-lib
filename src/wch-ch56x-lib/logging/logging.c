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

#include "wch-ch56x-lib/logging/logging.h"

#include "wch-ch56x-lib/logging/nanoprintf_impl.h"

#define LOG_PRINTF_BUFF_SIZE (511)
static uint64_t startCNT64;

#if (defined LOG_FILTER_IDS)
static uint8_t log_ids[] = { LOG_FILTER_IDS };
static size_t log_filter_size = sizeof(log_ids) / sizeof(log_ids[0]);
#endif

void _log_init(int sys_freq)
{
	startCNT64 = bsp_get_SysTickCNT();
#if (defined LOG_TYPE_PRINTF) || (defined LOG_TYPE_PRINTF_NO_INTERRUPT) || \
	(defined LOG_TYPE_BUFFER)
#if (defined LOG_TYPE_PRINTF || defined LOG_TYPE_PRINTF_NO_INTERRUPT)
	static debug_log_buf_t log_buf;
	log_printf_init(&log_buf);
#elif (defined LOG_TYPE_BUFFER)
	log_buffer_init();
#endif
	UART1_init(LOG_BAUDRATE, sys_freq);
#elif (defined LOG_TYPE_SERDES)
	log_serdes_init(const char* fmt, ...)
#endif
}

void log_time_init(void) { startCNT64 = bsp_get_SysTickCNT(); }

size_t _create_log_string(char* dest, size_t dest_size, const char* fmt,
						  va_list va_args);
size_t _create_log_string(char* dest, size_t dest_size, const char* fmt,
						  va_list va_args)
{
	int print_size1;
	int print_size2;
	uint64_t delta;
	uint64_t deltaCNT64;
	uint32_t sec;
	uint32_t msec;
	uint32_t usec;
	uint32_t tick_freq;

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
	print_size1 = npf_snprintf(dest, sizeof(dest), "0x%08X ", (uint32_t)(delta));
#else
	print_size1 = npf_snprintf(dest, sizeof(dest), "%02lus %03lums %03luus ", sec,
							   msec, usec);
#endif

	print_size2 =
		npf_vsnprintf(&dest[print_size1], dest_size - print_size1, fmt, va_args);

	print_size2 += print_size1;

	return print_size2;
}

void _write(char* buffer, size_t size);
void _write(char* buffer, size_t size)
{
#if (defined LOG_TYPE_BUFFER)
	_write_to_buffer(buffer, size);
#elif (defined LOG_TYPE_PRINTF)
	_write_to_stdout(buffer, size);
#elif (defined LOG_TYPE_SERDES)
	_write_to_serdes(buffer, size);
#endif
}

void _log(const char* fmt, ...)
{
	char temp_buffer[512];
	va_list va_args;
	int print_size1;
	int print_size2;
	uint64_t delta;
	uint64_t deltaCNT64;
	uint32_t sec;
	uint32_t msec;
	uint32_t usec;
	uint32_t tick_freq;

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
							   "%lus %lums %luus ", sec, msec, usec);
#endif

	va_start(va_args, fmt);
	print_size2 = npf_vsnprintf(&temp_buffer[print_size1],
								sizeof(temp_buffer) - print_size1, fmt, va_args);
	va_end(va_args);

	print_size2 += print_size1;

	if (print_size2 == 0)
	{
		return;
	}

	_write(temp_buffer, print_size2);
}

void log_if(uint8_t log_prio, uint8_t log_id, const char* fmt, ...)
{
	char temp_buffer[512];
	va_list va_args;
	int print_size1;
	int print_size2;
	uint64_t delta;
	uint64_t deltaCNT64;
	uint32_t sec;
	uint32_t msec;
	uint32_t usec;
	uint32_t tick_freq;

#if (defined LOG_FILTER_IDS)
	for (size_t i = 0; i < log_filter_size; ++i)
	{
#if (defined LOG_LEVEL)
		if (log_id != log_ids[i] || log_prio > LOG_LEVEL)
		{
			return;
		}
#else
		if (log_id != log_ids[i])
		{
			return;
		}
#endif
	}
#elif (defined LOG_LEVEL)
	if (log_prio > LOG_LEVEL)
	{
		return;
	}
#endif

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
							   "%02lus %03lums %03luus ", sec, msec, usec);
#endif

	va_start(va_args, fmt);
	print_size2 = npf_vsnprintf(&temp_buffer[print_size1],
								sizeof(temp_buffer) - print_size1, fmt, va_args);
	va_end(va_args);

	print_size2 += print_size1;

	if (print_size2 == 0)
	{
		return;
	}

	_write(temp_buffer, print_size2);
}
