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

HYDRA_FIFO_DEF(task_queue, hydra_interrupt_queue_task_t,
			   INTERRUPT_QUEUE_SIZE);

hydra_fifo_t* task_queues[] = { &task_queue };

void hydra_interrupt_queue_init(void)
{
	fifo_clean(&task_queue);
}
