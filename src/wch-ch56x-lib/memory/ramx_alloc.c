/********************************** (C) COPYRIGHT *******************************
Copyright (c) 2020 Nicolas OBERLI (HydraBus/HydraNFC)
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

#include "wch-ch56x-lib/memory/ramx_alloc.h"

__attribute__((aligned(16))) static uint8_t ramx_pool_buf[POOL_BUFFER_SIZE]
	__attribute__((section(".DMADATA")));
pool_t ramx_pool;

/**
 * @brief  Init pool allocator
 * @retval None
 */
/*
 * This function initiates the pool allocator.
 * The number of blocks must be lower than 256
 */
void ramx_pool_init(void)
{
	uint32_t i;

	ramx_pool.pool_size = POOL_BLOCK_NUM;
	ramx_pool.block_size = POOL_BLOCK_SIZE;
	ramx_pool.blocks_used = 0;
	ramx_pool.pool = ramx_pool_buf;
	ramx_pool.end_pool = ramx_pool_buf + POOL_BLOCK_NUM * POOL_BLOCK_SIZE;

	for (i = 0; i < sizeof(ramx_pool.blocks); i++)
	{
		ramx_pool.blocks[i] = 0;
		ramx_pool.blocks_reference_counter[i] = 0;
	}
}
