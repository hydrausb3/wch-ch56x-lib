/********************************** (C) COPYRIGHT
******************************* Copyright (c) 2021 Nanjing Qinheng
Microelectronics Co., Ltd. Copyright (c) 2022 Benjamin VERNOUX Copyright (c)
2023 Quarkslab

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

#ifndef HSPI_H
#define HSPI_H

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

#ifdef __cplusplus
extern "C" {
#endif

typedef enum HSPI_DATASIZE
{
	HSPI_DATASIZE_8,
	HSPI_DATASIZE_16,
	HSPI_DATASIZE_32
} HSPI_DATASIZE;

typedef enum HSPI_TYPE
{
	HSPI_TYPE_HOST,
	HSPI_TYPE_DEVICE,
} HSPI_TYPE;

typedef struct hspi_user_handled_t
{
	/**
   * @brief Called when a single packet has been received. Size should be the
   * same as set in hspi_init
   * @param buffer address of one of the two reception buffer, alternating
   * because of double buffering
   * @param custom_register only the first 26bits are valid
   */
	void (*hspi_rx_callback)(uint8_t* buffer, uint16_t size,
							 uint16_t custom_register);

	void (*hspi_err_crc_num_mismatch_callback)(void);
} hspi_user_handled_t;

extern hspi_user_handled_t hspi_user_handled;

#ifndef HSPI_SERDES_TX_SIZE_MASK
#define HSPI_SERDES_TX_SIZE_MASK 0x0001fff
#endif

#define HSPI_USER_DEFINED_MASK 0x03ffffff

/**
 * @brief Initialize interrupts and parameters
 * @param type HSPI_TYPE_DEVICE for reception, HSPI_TYPE_HOST for transmission
 * @param datasize Number of data lines used : HSPI_DATASIZE_8 (8bits)
 * HSPI_DATASIZE_16 (16bits) or HSPI_DATASIZE_32 (32bits)
 * @param size Size of the exchanged buffer in bytes, between 0 and 4096
 * included
 */
void hspi_init(HSPI_TYPE type, HSPI_DATASIZE datasize, uint16_t size);

/**
 * @brief Set new RAMX buffer to be sent. MUST be in RAMX because of DMA.
 * @param custom_register Must be 13bits max
 */
bool hspi_send(uint8_t* buffer, uint16_t size, uint16_t custom_register);

#endif

#ifdef __cplusplus
}
#endif
