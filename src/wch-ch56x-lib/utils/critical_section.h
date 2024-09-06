/********************************** (C) COPYRIGHT *******************************
Copyright (c) 2024 Quarkslab

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

#ifndef CRITICAL_SECTION_H
#define CRITICAL_SECTION_H

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

/**
 * Totally based on FreeRTOS
*/

extern size_t bsp_critical_nesting;
#define BSP_ENTER_CRITICAL()     \
	{                            \
		bsp_disable_interrupt(); \
		bsp_critical_nesting++;  \
	}

#define BSP_EXIT_CRITICAL()            \
	{                                  \
		bsp_critical_nesting--;        \
		if (bsp_critical_nesting == 0) \
		{                              \
			bsp_enable_interrupt();    \
		}                              \
	}

#endif
