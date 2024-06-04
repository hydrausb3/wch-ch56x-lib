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

#ifndef HYDRA_INTERRUPT_QUEUE_H
#define HYDRA_INTERRUPT_QUEUE_H
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

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "wch-ch56x-lib/logging/logging.h"
#include "wch-ch56x-lib/memory/fifo.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct HYDRA_INTERRUPT_QUEUE_TASK
{
	bool in_use;
	bool (*task)(uint8_t*);
	uint8_t* args;
	void (*cleanup)(uint8_t*);
} hydra_interrupt_queue_task_t;

HYDRA_FIFO_DECLR(task_queue, hydra_interrupt_queue_task_t,
				 INTERRUPT_QUEUE_SIZE);

/**
 * @brief Initialize (or reset) internal state of interrupt_queue.
 * @param
 */
void hydra_interrupt_queue_init(void);

/**
 * @brief Run next task if there are any. Starts with
 * HYDRA_INTERRUPT_QUEUE_HIGH_PRIO queue, then INTERRUPT_QUEUE_LOW_PRIO.
 * @param
 */
__attribute__((always_inline)) static inline void hydra_interrupt_queue_run(void)
{
	hydra_interrupt_queue_task_t task;
	LOG_IF(LOG_LEVEL_TRACE, LOG_ID_INTERRUPT_QUEUE,
		   "hydra_interrupt_queue_run \r\n");
	if (fifo_read_n(&task_queue, &task, 1))
	{
		LOG_IF(LOG_LEVEL_TRACE, LOG_ID_INTERRUPT_QUEUE,
			   "hydra_interrupt_queue_run executing task\r\n");
		task.task(task.args);
		if (task.cleanup)
			task.cleanup(task.args);
	}
}

/**
 * @brief Free all task from interrupt_queue and call their cleanup task.
 */
__attribute__((always_inline)) static inline void hydra_interrupt_queue_free_all(void)
{
	hydra_interrupt_queue_task_t task;
	while (fifo_read_n(&task_queue, &task, 1))
	{
		if (task.cleanup)
			task.cleanup(task.args);
	}
}

/**
 * @brief Set next task
 * @param func The task to be executed, will be passed "args" pointer when
 * called, to pass any required data
 * @param args Pointer to data to be passed to func. Should live as long as func
 * is scheduled
 * @param cleanup Functions called after func has been executed or when freeing
 * all tasks. Allows cleaning up args for instance.
 * @return
 */
__attribute__((always_inline)) static inline bool hydra_interrupt_queue_set_next_task(bool (*func)(uint8_t*), uint8_t* args,
																					  void (*cleanup)(uint8_t*))
{
	LOG_IF(LOG_LEVEL_TRACE, LOG_ID_INTERRUPT_QUEUE,
		   "hydra_interrupt_queue_set_next_task %d\r\n");
	hydra_interrupt_queue_task_t task;
	task.task = func;
	task.args = args;
	task.cleanup = cleanup;

	uint16_t written = 0;

	written = fifo_write(&task_queue, &task, 1);

	if (written == 0)
	{
		LOG_IF_LEVEL(LOG_LEVEL_CRITICAL, "Interrupt queue is full\r\n");
		return false;
	}

	return true;
}

/**
 * @brief Peek the next task to be scheduled, to check which function is coming
 * up next for instance.
 * @param task task address where the next task will be copied
 * @return
 */
__attribute__((always_inline)) static inline bool hydra_interrupt_queue_peek_next_task_prio0(
	hydra_interrupt_queue_task_t* task)
{
	uint16_t read = fifo_peek_n(&task_queue, task, 1);
	return read > 0 ? true : false;
}

#ifdef __cplusplus
}
#endif

#endif
