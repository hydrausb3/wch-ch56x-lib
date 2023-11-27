/********************************** (C) COPYRIGHT
******************************* Copyright (c) 2022 Benjamin VERNOUX Copyright
(c) 2023 Quarkslab

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

#ifndef SERDES_H
#define SERDES_H

#define SERDES_TX_RX_SPEED (SDS_PLL_FREQ_1_20G)

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

#define DMA_SIZE 4096
#ifndef HSPI_SERDES_TX_SIZE_MASK
#define HSPI_SERDES_TX_SIZE_MASK 0x00001fff
#endif

#define SERDES_CUSTOM_REGISTER_MASK 0x7fff
#define SERDES_USER_DEFINED_MASK 0xfffffff

typedef struct serdes_user_handled_t
{
	/**
   * @brief Called on packet reception
   * @param buffer Address of one of the two reception buffer, alternating
   * because of double buffering
   * @param size size of buffer received
   * @param custom_register custom value, 15bits max
   */
	void (*serdes_rx_callback)(uint8_t* buffer, uint16_t size,
							   uint16_t custom_register);
} serdes_user_handled_t;

extern serdes_user_handled_t serdes_user_handled;
typedef enum SERDES_TYPE
{
	SERDES_TYPE_HOST,
	SERDES_TYPE_DEVICE,
} SERDES_TYPE;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize interrupts and config for SerDes
 * @param type SERDES_TYPE_DEVICE for reception, SERDES_TYPE_HOST for
 * transmission
 * @param size Size of buffer to be transmitted, in bytes. MUST be one of : 4,
 * 16, 64, 128, 512, 576, 1024, 2048, 4096
 */
void serdes_init(SERDES_TYPE type, uint16_t size);

/**
 * @brief Send buffer of size
 * @param buffer Address of RAMX buffer. MUST be in RAMX because of DMA.
 */
void serdes_send(uint8_t* buffer, uint16_t size, uint16_t custom_register);

#ifdef __cplusplus
}
#endif

#endif
