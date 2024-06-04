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

#ifndef TEST_HYDRA_INTERRUPT_QUEUE_H
#define TEST_HYDRA_INTERRUPT_QUEUE_H
#include "wch-ch56x-lib/interrupt_queue/interrupt_queue.h"
#include "wch-ch56x-lib/logging/logging.h"

// void hydra_interrupt_queue_init(void);

// hydra_interrupt_queue_task_t* interrupt_queue_get_next_free_task(void);

// bool hydra_interrupt_queue_set_next_task(bool (*func)(uint8_t*), uint8_t*
// args);

// hydra_interrupt_queue_task_t* interrupt_queue_get_next_task(void);

// void interrupt_queue_free_task(hydra_interrupt_queue_task_t* task);

// void hydra_interrupt_queue_run(void);

size_t num_run = 0;

bool some_random_function(uint8_t* args);
bool some_random_function(uint8_t* args)
{
	*(size_t*)(void*)(args) += 1;
	LOG("\rexecuting random function n°%d \r\n", *(size_t*)(void*)(args));
	return true;
}

bool test_interrupt_queue_set_tasks(void);
bool test_interrupt_queue_set_tasks(void)
{
	hydra_interrupt_queue_init();

	hydra_interrupt_queue_set_next_task(some_random_function, (uint8_t*)&num_run,
										NULL);
	hydra_interrupt_queue_set_next_task(some_random_function, (uint8_t*)&num_run,
										NULL);
	hydra_interrupt_queue_set_next_task(some_random_function, (uint8_t*)&num_run,
										NULL);

	hydra_interrupt_queue_run();
	hydra_interrupt_queue_run();
	hydra_interrupt_queue_run();
	hydra_interrupt_queue_run();
	hydra_interrupt_queue_run();

	return num_run == 3;
}

bool test_interrupt_queue_overflow(void);
bool test_interrupt_queue_overflow(void)
{
	num_run = 0;
	hydra_interrupt_queue_init();

	bool result = true;

	for (int i = 0; i < INTERRUPT_QUEUE_SIZE + 5; ++i)
	{
		result = hydra_interrupt_queue_set_next_task(some_random_function,
													 (uint8_t*)&num_run, NULL);
	}

	for (int i = 0; i < INTERRUPT_QUEUE_SIZE + 5; ++i)
	{
		hydra_interrupt_queue_run();
	}

	return num_run == 20 && result == false;
}

bool test_interrupt_queue_stress(void);
bool test_interrupt_queue_stress(void)
{
	num_run = 0;
	hydra_interrupt_queue_init();

	bool result = true;

	size_t num_runs = 100;
	for (size_t j = 0; j < num_runs; ++j)
	{
		for (int i = 0; i < INTERRUPT_QUEUE_SIZE; ++i)
		{
			result = hydra_interrupt_queue_set_next_task(some_random_function,
														 (uint8_t*)&num_run, NULL);
			if (!result)
				return false;
		}

		for (int i = 0; i < INTERRUPT_QUEUE_SIZE; ++i)
		{
			hydra_interrupt_queue_run();
		}
	}

	return num_run == INTERRUPT_QUEUE_SIZE * num_runs && result == true;
}
#endif
