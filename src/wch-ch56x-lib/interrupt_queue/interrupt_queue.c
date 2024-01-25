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

#include "wch-ch56x-lib/interrupt_queue/interrupt_queue.h"
#include "wch-ch56x-lib/logging/logging.h"
#include "wch-ch56x-lib/memory/fifo.h"

HYDRA_FIFO_DEF(task_queue_prio0, hydra_interrupt_queue_task_t,
			   INTERRUPT_QUEUE_SIZE);
HYDRA_FIFO_DEF(task_queue_prio1, hydra_interrupt_queue_task_t,
			   INTERRUPT_QUEUE_SIZE);

hydra_fifo_t* task_queues[] = { &task_queue_prio1, &task_queue_prio0 };

void hydra_interrupt_queue_init(void)
{
	fifo_clean(&task_queue_prio0);
	fifo_clean(&task_queue_prio1);
}

bool hydra_interrupt_queue_peek_next_task_prio0(
	hydra_interrupt_queue_task_t* task)
{
	uint16_t read = fifo_peek_n(&task_queue_prio0, task, 1);
	return read > 0 ? true : false;
}

bool hydra_interrupt_queue_set_next_task(bool (*func)(uint8_t*), uint8_t* args,
										 void (*cleanup)(uint8_t*),
										 uint8_t prio)
{
	LOG_IF(LOG_LEVEL_DEBUG, LOG_ID_INTERRUPT_QUEUE,
		   "hydra_interrupt_queue_set_next_task prio %d\r\n", prio);
	hydra_interrupt_queue_task_t task;
	task.task = func;
	task.args = args;
	task.cleanup = cleanup;

	uint16_t written = 0;
	if (prio == 0)
	{
		written = fifo_write(&task_queue_prio0, &task, 1);
	}
	else if (prio == 1)
	{
		written = fifo_write(&task_queue_prio1, &task, 1);
	}
	else
	{
		return false;
	}
	if (written == 0)
		return false;

	return true;
}

void hydra_interrupt_queue_run(void)
{
	for (size_t i = 0; i < sizeof(task_queues) / sizeof(hydra_fifo_t*); ++i)
	{
		hydra_interrupt_queue_task_t task;
		LOG_IF(LOG_LEVEL_TRACE, LOG_ID_INTERRUPT_QUEUE,
			   "hydra_interrupt_queue_run prio %d \r\n",
			   sizeof(task_queues) / sizeof(hydra_fifo_t*) - 1 - i);
		if (fifo_read_n(task_queues[i], &task, 1))
		{
			LOG_IF(LOG_LEVEL_TRACE, LOG_ID_INTERRUPT_QUEUE,
				   "hydra_interrupt_queue_run prio %d executing task\r\n",
				   sizeof(task_queues) / sizeof(hydra_fifo_t*) - 1 - i);
			task.task(task.args);
			if (task.cleanup)
				task.cleanup(task.args);
			break;
		}
	}
}

void hydra_interrupt_queue_free_all(void)
{
	for (size_t i = 0; i < sizeof(task_queues) / sizeof(hydra_fifo_t*); ++i)
	{
		hydra_interrupt_queue_task_t task;
		while (fifo_read_n(task_queues[i], &task, 1))
		{
			// LOG_IF(LOG_LEVEL_DEBUG, LOG_ID_INTERRUPT_QUEUE,
			// "interrupt_queue_get_next_task prio %d \r\n", sizeof(task_queues) /
			// sizeof(hydra_fifo_t*) - 1 - i);
			if (task.cleanup)
				task.cleanup(task.args);
		}
	}
}
