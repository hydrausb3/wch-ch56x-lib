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

#ifndef HYDRA_POOL_H
#define HYDRA_POOL_H

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
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
Pool requirements:
    -> can be used with complex types
    -> safe for use with interrupts : this is achieved by disabling interrupts
    -> wch569 is single-core so no need for concurrent access protections
    -> fast : this might depend on the context
*/

typedef struct hydra_pool_t
{
	uint8_t* pool_members;
	bool* pool_manager;
	uint16_t size;
	size_t type_size;
} hydra_pool_t;

#define HYDRA_POOL_DEF(_name, _type, _size)                      \
	uint8_t _name##_pool_members[_size * sizeof(_type)];         \
	bool _name##_pool_manager[_size];                            \
	hydra_pool_t _name = { .pool_members = _name##_pool_members, \
						   .pool_manager = _name##_pool_manager, \
						   .size = _size,                        \
						   .type_size = sizeof(_type) }

#define HYDRA_POOL_DECLR(_name) \
	extern hydra_pool_t _name;

/**
 * @brief Get a free pool member
 * @param pool pointer to the pool
 * @return pointer to a free pool member, or NULL is none are available
 */
__attribute__((always_inline)) static inline void*
hydra_pool_get(hydra_pool_t* pool)
{
	bsp_disable_interrupt();
	for (uint16_t i = 0; i < pool->size; ++i)
	{
		if (!pool->pool_manager[i])
		{
			pool->pool_manager[i] = true;
			bsp_enable_interrupt();
			return (void*)(pool->pool_members + i * pool->type_size);
		}
	}
	bsp_enable_interrupt();
	return NULL;
}

/**
 * @brief Free pool member
 * @param pool pointer to the pool
 * @param ptr pointer to the member to free
 */
__attribute__((always_inline)) static inline void
hydra_pool_free(hydra_pool_t* pool, void* ptr)
{
	uint16_t i = ((uint8_t*)(ptr)-pool->pool_members) / pool->type_size;
	bsp_disable_interrupt();
	if (i > pool->size || !pool->pool_manager[i])
	{
		bsp_enable_interrupt();
		return;
	}
	pool->pool_manager[i] = false;
	bsp_enable_interrupt();
}

/**
 * @brief Reset the pool, set members to 0
 * @param pool pointer to the pool
 */
__attribute__((always_inline)) static inline void
hydra_pool_clean(hydra_pool_t* pool)
{
	bsp_disable_interrupt();
	for (uint16_t i = 0; i < pool->size; ++i)
	{
		pool->pool_manager[i] = false;
	}
	memset(pool->pool_members, 0, pool->size * pool->type_size);
	bsp_enable_interrupt();
}

#ifdef __cplusplus
}
#endif

#endif
