#ifndef TEST_POOL_H
#define TEST_POOL_H
#include "wch-ch56x-lib/memory/pool.h"

typedef struct custom_type
{
	uint8_t a;
	uint8_t b;
} custom_type_t;

#define TEST_POOL_SIZE 20

HYDRA_POOL_DEF(pool, custom_type_t, TEST_POOL_SIZE);

bool test_pool_alloc_max(void);
bool test_pool_alloc_max(void)
{
	for (int i = 0; i < TEST_POOL_SIZE; ++i)
	{
		custom_type_t* member = (custom_type_t*)hydra_pool_get(&pool);
		member->a = i;
		member->b = i;
	}

	for (int i = 0; i < TEST_POOL_SIZE; ++i)
	{
		custom_type_t* member =
			(custom_type_t*)&pool_pool_members[i * sizeof(custom_type_t)];

		if (member->a != i || member->b != i)
			return false;
	}
	return true;
}

bool test_pool_overflow(void);
bool test_pool_overflow(void)
{
	for (int i = 0; i < 2 * TEST_POOL_SIZE; ++i)
	{
		custom_type_t* member = (custom_type_t*)hydra_pool_get(&pool);

		if (member != NULL)
		{
			member->a = i;
			member->b = i;
		}
	}

	for (int i = 0; i < TEST_POOL_SIZE; ++i)
	{
		custom_type_t* member =
			(custom_type_t*)&pool_pool_members[i * sizeof(custom_type_t)];

		if (member->a != i || member->b != i)
			return false;
	}
	return true;
}

#endif
