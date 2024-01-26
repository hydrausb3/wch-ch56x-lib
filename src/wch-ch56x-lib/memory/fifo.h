/********************************** (C) COPYRIGHT *******************************
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

#ifndef FIFO_H
#define FIFO_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "wch-ch56x-lib/logging/logging.h"

#ifdef __cplusplus
extern "C" {
#endif
/*

FIFO requirements:
    -> store any type of data, not just uint8_t
    -> the wch569 is single-core, so we don't have to worry about concurrent
access
    -> safe for use in an interrupt context
    There are interrupts so the FIFO should be reentrant. Meaning if a FIFO
function is interrupted, it should resume in a correct state. This can be
achieved by disabling interrupts temporarily, and working on local variables
instead of globals whenever possible. Avoid disabling interrupts as much as
possible.

Principle:
    -> separate read/write heads
    -> no overwrite : no overflow

    1. R=W means the FIFO is empty. R points to the next readable index, W to
the next writable index
    2. if W > R : there are W-R readable index, (size-W) + R - 1 writable
    3. if W < R : there are (size - R) + W readable, R-1 writable

    W cannot join R when writing, otherwise the queue would be empty even though
we just wrote something. That's why the queue length is one more than the queue
capacity.

Limitations:
    -> if you protect the read and write functions from interrupts, the FIFO is
"multi-producer/multi-consumer" : everything is sequential, since we have only
one core and no interruptions that could trigger a reentry.
    -> if you don't protect the read and write functions from interrupts, the
FIFO is still safe but only if your interrupt handler and your user code keep to
one role (either producer or consumer). Ex : if you are reading outside
interrupts, do not read in an interrupt handler. otherwise, the interrupt could
preempt your code while its reading which would put the FIFO in a bad state.
    -> single-core

Got some inspiration from https://github.com/hathach/tinyusb
*/

typedef struct hydra_fifo_t
{
	uint8_t* buffer;
	uint16_t size;
	size_t type_size;
	volatile uint16_t rd_idx;
	volatile uint16_t wr_idx;
} hydra_fifo_t;

#define HYDRA_FIFO_DEF(_name, _type, _size)              \
	uint8_t _name##_buffer[(_size + 1) * sizeof(_type)]; \
	hydra_fifo_t _name = { .buffer = _name##_buffer,     \
						   .size = _size + 1,            \
						   .type_size = sizeof(_type),   \
						   .rd_idx = 0,                  \
						   .wr_idx = 0 }

/**
 * @brief Set this pointer to disable/enable interrupts. Not necessary with
 * single-producer/single-consumer.
 */
extern void (*hydra_fifo_disable_interrupt)(bool);

#define ABS(x) x < 0 ? -x : x
#define MIN(x, y) x < y ? x : y

__attribute__((always_inline)) static inline uint16_t
_fifo_get_count(uint16_t rd_idx, uint16_t wr_idx, uint16_t size);
__attribute__((always_inline)) static inline uint16_t
_fifo_get_count(uint16_t rd_idx, uint16_t wr_idx, uint16_t size)
{
	if (wr_idx >= rd_idx)
	{
		return wr_idx - rd_idx;
	}
	return size - rd_idx + wr_idx;
}

__attribute__((always_inline)) static inline uint16_t
_fifo_get_free_space(uint16_t rd_idx, uint16_t wr_idx, uint16_t size);
__attribute__((always_inline)) static inline uint16_t
_fifo_get_free_space(uint16_t rd_idx, uint16_t wr_idx, uint16_t size)
{
	if (rd_idx > wr_idx)
	{
		return rd_idx - wr_idx - 1;
	}
	return size - wr_idx + rd_idx - 1;
}

__attribute__((always_inline)) static inline uint16_t
_fifo_read_n(hydra_fifo_t* fifo, void* buffer, uint16_t n, uint16_t rd_idx,
			 uint16_t wr_idx);
__attribute__((always_inline)) static inline uint16_t
_fifo_read_n(hydra_fifo_t* fifo, void* buffer, uint16_t n, uint16_t rd_idx,
			 uint16_t wr_idx)
{
	LOG_IF_LEVEL(
		LOG_LEVEL_TRACE,
		"_fifo_read_n rd_idx=%d wr_idx=%d n=%d free_space=%d count%d \r\n",
		rd_idx, wr_idx, n, _fifo_get_free_space(rd_idx, wr_idx, fifo->size),
		_fifo_get_count(rd_idx, wr_idx, fifo->size));

	if (rd_idx == wr_idx || (n > _fifo_get_count(rd_idx, wr_idx, fifo->size)))
		return 0;

	if (wr_idx > rd_idx)
	{
		uint16_t linear_count = MIN(n, wr_idx - rd_idx);
		uint16_t linear_bytes = linear_count * fifo->type_size;
		memcpy(buffer, fifo->buffer + rd_idx * fifo->type_size, linear_bytes);
		LOG_IF_LEVEL(LOG_LEVEL_TRACE,
					 "read rd_idx=%d wr_idx=%d linear_count=%d\r\n", rd_idx, wr_idx,
					 linear_count);
		return linear_count;
	}

	uint16_t linear_count = MIN(n, fifo->size - rd_idx);
	uint16_t linear_bytes = linear_count * fifo->type_size;
	memcpy(buffer, fifo->buffer + rd_idx * fifo->type_size, linear_bytes);
	LOG_IF_LEVEL(LOG_LEVEL_TRACE, "read rd_idx=%d wr_idx=%d linear_count=%d\r\n",
				 rd_idx, wr_idx, linear_count);

	if (n > linear_count)
	{
		uint16_t wrap_count = MIN(wr_idx, n - linear_count);
		uint16_t wrap_bytes = wr_idx * fifo->type_size;
		memcpy((uint8_t*)buffer + linear_bytes, fifo->buffer, wrap_bytes);
		LOG_IF_LEVEL(LOG_LEVEL_TRACE,
					 "read rd_idx=%d wr_idx=%d linear_count=%d, wrap_count=%d\r\n",
					 rd_idx, wr_idx, linear_count, wrap_count);
		return linear_count + wrap_count;
	}
	return linear_count;
}

__attribute__((always_inline)) static inline uint16_t
_fifo_write_n(hydra_fifo_t* fifo, void* buffer, uint16_t n, uint16_t rd_idx,
			  uint16_t wr_idx);
__attribute__((always_inline)) static inline uint16_t
_fifo_write_n(hydra_fifo_t* fifo, void* buffer, uint16_t n, uint16_t rd_idx,
			  uint16_t wr_idx)
{
	LOG_IF_LEVEL(
		LOG_LEVEL_TRACE,
		"_fifo_write_n rd_idx=%d wr_idx=%d n=%d free_space=%d count=%d \r\n",
		rd_idx, wr_idx, n, _fifo_get_free_space(rd_idx, wr_idx, fifo->size),
		_fifo_get_count(rd_idx, wr_idx, fifo->size));

	uint16_t free_space = _fifo_get_free_space(rd_idx, wr_idx, fifo->size);
	if (free_space < n)
		return 0;

	if (rd_idx > wr_idx)
	{
		uint16_t linear_count = MIN(free_space, n);
		uint16_t linear_bytes = linear_count * fifo->type_size;
		memcpy(fifo->buffer + wr_idx * fifo->type_size, buffer, linear_bytes);
		LOG_IF_LEVEL(LOG_LEVEL_TRACE,
					 "write rd_idx=%d wr_idx=%d linear_count=%d\r\n", rd_idx,
					 wr_idx, linear_count);
		return linear_count;
	}

	uint16_t linear_count = MIN(fifo->size - wr_idx, n);
	uint16_t linear_bytes = linear_count * fifo->type_size;
	memcpy(fifo->buffer + wr_idx * fifo->type_size, buffer, linear_bytes);

	if (n > linear_count)
	{
		uint16_t wrap_count = MIN(rd_idx, n - linear_count);
		uint16_t wrap_bytes = wrap_count * fifo->type_size;
		memcpy(fifo->buffer + wr_idx * fifo->type_size + linear_bytes,
			   (uint8_t*)buffer + linear_bytes, wrap_bytes);
		LOG_IF_LEVEL(LOG_LEVEL_TRACE,
					 "write rd_idx=%d wr_idx=%d linear_count=%d, wrap_count=%d\r\n",
					 rd_idx, wr_idx, linear_count, wrap_count);
		return linear_count + wrap_count;
	}
	else
	{
		LOG_IF_LEVEL(LOG_LEVEL_TRACE,
					 "write rd_idx=%d wr_idx=%d linear_count=%d\r\n", rd_idx,
					 wr_idx, linear_count);
	}
	return linear_count;
}

__attribute__((always_inline)) static inline void
_fifo_advance_read(hydra_fifo_t* fifo, uint16_t rd_idx, uint16_t offset);
__attribute__((always_inline)) static inline void
_fifo_advance_read(hydra_fifo_t* fifo, uint16_t rd_idx, uint16_t offset)
{
	uint16_t remaining = fifo->size - rd_idx;
	LOG_IF_LEVEL(LOG_LEVEL_TRACE, "advance_read rd_idx=%d offset=%d\r\n", rd_idx,
				 offset);

	if (offset < remaining)
	{
		fifo->rd_idx += offset;
		return;
	}
	fifo->rd_idx = offset - remaining;
}

__attribute__((always_inline)) static inline void
_fifo_advance_write(hydra_fifo_t* fifo, uint16_t wr_idx, uint16_t offset);
__attribute__((always_inline)) static inline void
_fifo_advance_write(hydra_fifo_t* fifo, uint16_t wr_idx, uint16_t offset)
{
	uint16_t remaining = fifo->size - wr_idx;
	LOG_IF_LEVEL(LOG_LEVEL_TRACE, "advance_write wr_idx=%d offset=%d\r\n", wr_idx,
				 offset);

	if (offset < remaining)
	{
		fifo->wr_idx += offset;
		return;
	}
	fifo->wr_idx = offset - remaining;
}

/**
 * @brief Read all the content of the buffer
 * @param fifo pointer to the fifo
 * @param buffer buffer of size at least the size of sizeof(type)*fifo->size
 * @return number of elements actually read
 */
__attribute__((always_inline)) static inline uint16_t
fifo_read(hydra_fifo_t* fifo, void* buffer)
{
	hydra_fifo_disable_interrupt(true);
	uint16_t rd_idx = fifo->rd_idx;
	uint16_t wr_idx = fifo->wr_idx;
	uint16_t count_read =
		_fifo_read_n(fifo, buffer, _fifo_get_count(rd_idx, wr_idx, fifo->size),
					 rd_idx, wr_idx);
	hydra_fifo_disable_interrupt(false);
	return count_read;
}

/**
 * @brief Read all the content of the buffer
 * @param fifo pointer to the fifo
 * @param buffer buffer of size at least the size of sizeof(type)*fifo->size
 * @param n number of elements to read
 * @return either n or 0. If there are less than n elements in the queue,
 * returns 0
 */
__attribute__((always_inline)) static inline uint16_t
fifo_read_n(hydra_fifo_t* fifo, void* buffer, uint16_t n)
{
	hydra_fifo_disable_interrupt(true);
	uint16_t rd_idx = fifo->rd_idx;
	uint16_t wr_idx = fifo->wr_idx;
	uint16_t count_read = _fifo_read_n(fifo, buffer, n, rd_idx, wr_idx);
	_fifo_advance_read(fifo, rd_idx, count_read);
	hydra_fifo_disable_interrupt(false);
	return count_read;
}

/**
 * @brief Same as fifo_read_n, but does not remove the elements that have been
 * read
 * @param fifo pointer to the fifo
 * @param buffer buffer of size at least the size of sizeof(type)*fifo->size
 * @param n number of elements to read
 * @return either n or 0. If there are less than n elements in the queue,
 * returns 0
 */
__attribute__((always_inline)) static inline uint16_t
fifo_peek_n(hydra_fifo_t* fifo, void* buffer, uint16_t n)
{
	hydra_fifo_disable_interrupt(true);
	uint16_t rd_idx = fifo->rd_idx;
	uint16_t wr_idx = fifo->wr_idx;
	uint16_t count_read = _fifo_read_n(fifo, buffer, n, rd_idx, wr_idx);
	hydra_fifo_disable_interrupt(false);
	return count_read;
}

/**
 * @brief
 * @param fifo pointer to the fifo
 * @param buffer buffer of size at least n
 * @param n number of elements to write
 * @return either n or 0. If there is not enough free space to fit n elements,
 * returns 0
 */
__attribute__((always_inline)) static inline uint16_t
fifo_write(hydra_fifo_t* fifo, void* buffer, uint16_t n)
{
	hydra_fifo_disable_interrupt(true);
	uint16_t rd_idx = fifo->rd_idx;
	uint16_t wr_idx = fifo->wr_idx;
	uint16_t count_written = _fifo_write_n(fifo, buffer, n, rd_idx, wr_idx);
	_fifo_advance_write(fifo, wr_idx, count_written);
	hydra_fifo_disable_interrupt(false);
	return count_written;
}

/**
 * @brief Get the number of elements in the queue
 * @param fifo pointer to the fifo
 * @return
 */
__attribute__((always_inline)) static inline uint16_t
fifo_count(hydra_fifo_t* fifo)
{
	return _fifo_get_count(fifo->rd_idx, fifo->wr_idx, fifo->size);
}

/**
 * @brief Remove all elements in the queue and set to 0
 * @param fifo pointer to the fifo
 */
__attribute__((always_inline)) static inline void
fifo_clean(hydra_fifo_t* fifo)
{
	hydra_fifo_disable_interrupt(true);
	fifo->rd_idx = 0;
	fifo->wr_idx = 0;
	memset(fifo->buffer, 0, fifo->size * fifo->type_size);
	hydra_fifo_disable_interrupt(false);
}

#ifdef __cplusplus
}
#endif

#endif
