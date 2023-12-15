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

/* Taken from linux kernel */
#define DIV_ROUND_UP(n, d) (((n) + (d)-1) / (d))

__attribute__((aligned(16))) static uint8_t ramx_pool_buf[POOL_BUFFER_SIZE]
	__attribute__((section(".DMADATA")));
static pool_t ramx_pool;

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

/**
 * @brief Allocates a number of contiguous blocks
 * @param  num_blocks: number of contiguous blocks to allocate
 * @retval Pointer to the starting buffer, 0 if requested size is not available
 */
/*
 * This function tries to allocate contiguous blocks by looking at the free
 * blocks list.
 * If found, it will mark the blocks as used (number of allocated blocks).
 */
void* ramx_pool_alloc_blocks(uint8_t num_blocks)
{
	uint32_t i, j;
	uint8_t space_found;

	if (num_blocks == 0)
	{
		return 0;
	}
	if (num_blocks > POOL_BLOCK_NUM)
	{
		return 0;
	}

	space_found = 1;
	for (i = 0; i <= (uint32_t)POOL_BLOCK_NUM - num_blocks; i++)
	{
		if (ramx_pool.blocks[i] == 0)
		{
			for (j = 1; j < num_blocks; j++)
			{
				if (ramx_pool.blocks[i + j] != 0)
				{
					// i += j ?
					space_found = 0;
					break;
				}
				else
				{
					space_found = 1;
				}
			}
			if (space_found == 1)
			{
				for (j = 0; j < num_blocks; j++)
				{
					ramx_pool.blocks[i + j] = num_blocks;
					ramx_pool.blocks_reference_counter[i + j] = 1;
				}
				ramx_pool.blocks_used += num_blocks;
				return ramx_pool.pool + (ramx_pool.block_size * i);
			}
		}
	}
	return 0;
}

/**
 * @brief  Helper function to allocate a buffer of at least n bytes
 * @param  bytes: Number of bytes requested
 * @retval Pointer to the starting buffer, 0 if requested size is not available
 */
/*
 * This helper function will allocate enough pool blocks to store the requested
 * number of bytes.
 */
void* ramx_pool_alloc_bytes(uint32_t num_bytes)
{
	uint8_t blocks_needed = DIV_ROUND_UP(num_bytes, POOL_BLOCK_SIZE);
	return ramx_pool_alloc_blocks(blocks_needed);
}

/**
 * @brief Takes ownership of the blocks referenced by the pointer.
 * ramx_pool_free must be called to release ownership and free the blocks if
 * possible.
 */
void ramx_take_ownership(void* ptr)
{
	uint8_t block_index, num_blocks, i;

	if (ptr == 0)
	{
		return;
	}

	block_index =
		((uint32_t)ptr - (uint32_t)ramx_pool.pool) / ramx_pool.block_size;
	num_blocks = ramx_pool.blocks[block_index];

	for (i = 0; i < num_blocks; i++)
	{
		ramx_pool.blocks_reference_counter[block_index + i] += 1;
	}
}

/**
 * @brief  Free owner from it's ownership on the blocks. Free the blocks if
 * there are no more owners.
 * @param  ptr: pointer to the beginning of the buffer to be freed
 * @retval None
 */

void ramx_pool_free(void* ptr)
{
	uint8_t block_index, num_blocks, i;
	uint8_t num_freed = 0;

	if (ptr == 0)
	{
		return;
	}

	block_index =
		((uint32_t)ptr - (uint32_t)ramx_pool.pool) / ramx_pool.block_size;
	num_blocks = ramx_pool.blocks[block_index];

	for (i = 0; i < num_blocks; i++)
	{
		if (ramx_pool.blocks_reference_counter[block_index + i] <= 1)
		{
			ramx_pool.blocks[block_index + i] = 0;
			num_freed += 1;
		}
		else
		{
			ramx_pool.blocks_reference_counter[block_index + i] -= 1;
		}
	}
	ramx_pool.blocks_used -= num_freed;
}

bool ramx_address_in_pool(void* ptr)
{
	return (uint8_t*)ptr < ramx_pool.end_pool &&
		   (uint8_t*)ptr >= ramx_pool.pool;
}

uint8_t ramx_pool_stats_free()
{
	return ramx_pool.pool_size - ramx_pool.blocks_used;
}

uint8_t ramx_pool_stats_used() { return ramx_pool.blocks_used; }

uint8_t* ramx_pool_stats_blocks() { return ramx_pool.blocks; }
