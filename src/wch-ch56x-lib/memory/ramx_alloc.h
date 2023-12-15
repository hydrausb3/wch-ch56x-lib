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

#ifndef _ALLOC_H_
#define _ALLOC_H_

#include <stdbool.h>
#include <stdint.h>

/**
You must pass the following defines to your compiler
#define POOL_BLOCK_SIZE size // the size of each individual block
#define POOL_BLOCK_NUM num // number of blocks to be allocated, no more than 256
*/
#define POOL_BUFFER_SIZE (POOL_BLOCK_SIZE * POOL_BLOCK_NUM)

#ifdef __cplusplus
extern "C" {
#endif

typedef struct pool
{
	uint8_t* pool;
	uint8_t* end_pool;
	uint32_t block_size; // Block size in bytes
	uint8_t pool_size; // Total number of blocks
	uint8_t blocks_used; // Number of used blocks
	uint8_t blocks[POOL_BLOCK_NUM]; // Blocks status
	uint8_t blocks_reference_counter[POOL_BLOCK_NUM]; // Blocks status
} pool_t;

/**
 * @brief Reset ramx pool state.
 * @param
 */
void ramx_pool_init(void);

/**
 * @brief Allocate the min number of blocks that bytes can fit in. Not very
 * efficient if bytes is not a multiple of block_size.
 * @param bytes
 * @return
 */
void* ramx_pool_alloc_bytes(uint32_t bytes);

/**
 * @brief Allocate a number of blocks
 * @param num_blocks
 * @return
 */
void* ramx_pool_alloc_blocks(uint8_t num_blocks);

/**
 * @brief Increment the reference count on the blocks referenced by ptr.
 * @param ptr
 */
void ramx_take_ownership(void* ptr);

/**
 * @brief Decreases the reference count on the blocks referenced by ptr, and
 * deallocate them when the reference count goes to zero.
 * @param ptr
 */
void ramx_pool_free(void* ptr);

/**
 * @brief Check if ptr is in the pool's address range. Useful to avoid
 * unnecessary memcpy.
 * @param ptr
 * @return
 */
bool ramx_address_in_pool(void* ptr);

/**
 * @brief Get the number of free blocks.
 * @param
 * @return
 */
uint8_t ramx_pool_stats_free(void);

/**
 * @brief Get the number of used blocks.
 * @param
 * @return
 */
uint8_t ramx_pool_stats_used(void);

/**
 * @brief Get a pointer to the blocks states.
 * @param
 * @return
 */
uint8_t* ramx_pool_stats_blocks(void);

#ifdef __cplusplus
}
#endif

#endif /* _ALLOC_H_ */
