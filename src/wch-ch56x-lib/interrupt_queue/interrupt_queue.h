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

#define INTERRUPT_QUEUE_LOW_PRIO 0
#define HYDRA_INTERRUPT_QUEUE_HIGH_PRIO 1

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
void hydra_interrupt_queue_run(void);

/**
 * @brief Free all task from interrupt_queue and call their cleanup task.
 */
void hydra_interrupt_queue_free_all(void);

/**
 * @brief Set next task
 * @param func The task to be executed, will be passed "args" pointer when
 * called, to pass any required data
 * @param args Pointer to data to be passed to func. Should live as long as func
 * is scheduled
 * @param cleanup Functions called after func has been executed or when freeing
 * all tasks. Allows cleaning up args for instance.
 * @param prio Either INTERRUPT_QUEUE_LOW_PRIO or
 * HYDRA_INTERRUPT_QUEUE_HIGH_PRIO.
 * @return
 */
bool hydra_interrupt_queue_set_next_task(bool (*func)(uint8_t*), uint8_t* args,
										 void (*cleanup)(uint8_t*),
										 uint8_t prio);

/**
 * @brief Peek the next task to be scheduled, to check which function is coming
 * up next for instance.
 * @param task task address where the next task will be copied
 * @return
 */
bool hydra_interrupt_queue_peek_next_task_prio0(
	hydra_interrupt_queue_task_t* task);

#ifdef __cplusplus
}
#endif

#endif
