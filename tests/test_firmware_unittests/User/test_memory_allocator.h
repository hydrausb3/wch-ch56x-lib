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

#ifndef TEST_MEMORY_ALLOCATOR_H
#define TEST_MEMORY_ALLOCATOR_H

#include "wch-ch56x-lib/logging/logging.h"
#include "wch-ch56x-lib/memory/ramx_alloc.h"

bool test_memory_allocator_ramx_alloc_bytes(void);

bool test_memory_allocator_ramx_alloc_bytes(void)
{
	ramx_pool_init();

	uint8_t* mem = ramx_pool_alloc_bytes(4);

	*((int*)(void*)mem) = 33;

	LOG("mem : %d \r\n", *((int*)(void*)mem));
	LOG("free : %d, used: %d \r\n", ramx_pool_stats_free(),
		ramx_pool_stats_used());

	ramx_pool_free(mem);

	LOG("mem : %d \r\n", *((int*)(void*)mem));
	LOG("free : %d, used: %d \r\n", ramx_pool_stats_free(),
		ramx_pool_stats_used());

	return ramx_pool_stats_free() == POOL_BLOCK_NUM &&
		   ramx_pool_stats_used() == 0;
}

bool test_memory_allocator_ramx_alloc_all_blocks(void);

bool test_memory_allocator_ramx_alloc_all_blocks(void)
{
	ramx_pool_init();

	uint8_t* mem = ramx_pool_alloc_blocks(20);

	LOG("mem : %d \r\n", *((int*)(void*)mem));
	LOG("free : %d, used: %d \r\n", ramx_pool_stats_free(),
		ramx_pool_stats_used());

	ramx_pool_free(mem);

	LOG("mem : %d \r\n", *((int*)(void*)mem));
	LOG("free : %d, used: %d \r\n", ramx_pool_stats_free(),
		ramx_pool_stats_used());

	return ramx_pool_stats_free() == POOL_BLOCK_NUM &&
		   ramx_pool_stats_used() == 0;
}

bool test_memory_allocator_ramx_alloc_too_many_blocks(void);

bool test_memory_allocator_ramx_alloc_too_many_blocks(void)
{
	ramx_pool_init();

	uint8_t* mem = ramx_pool_alloc_blocks(25);

	LOG("mem ptr : %d \r\n", mem);
	LOG("free : %d, used: %d \r\n", ramx_pool_stats_free(),
		ramx_pool_stats_used());

	ramx_pool_free(mem);

	LOG("free : %d, used: %d \r\n", ramx_pool_stats_free(),
		ramx_pool_stats_used());

	return ramx_pool_stats_free() == POOL_BLOCK_NUM &&
		   ramx_pool_stats_used() == 0;
}

bool test_memory_allocator_ramx_alloc_double_free(void);
bool test_memory_allocator_ramx_alloc_double_free(void)
{
	ramx_pool_init();

	uint8_t* mem = ramx_pool_alloc_blocks(1);

	LOG("mem ptr : %x \r\n", mem);
	LOG("free : %d, used: %d \r\n", ramx_pool_stats_free(),
		ramx_pool_stats_used());

	ramx_pool_free(mem);
	ramx_pool_free(mem);

	LOG("free : %d, used: %d \r\n", ramx_pool_stats_free(),
		ramx_pool_stats_used());

	return ramx_pool_stats_free() == POOL_BLOCK_NUM &&
		   ramx_pool_stats_used() == 0;
}
#endif
