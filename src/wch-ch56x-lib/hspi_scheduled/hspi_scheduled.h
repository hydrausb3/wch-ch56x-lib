/********************************** (C) COPYRIGHT *******************************
Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
Copyright (c) 2022 Benjamin VERNOUX
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

/**
hspi_scheduled uses an interrupt queue to process RX and TX interrupts.
It uses a statically allocated pool to save call arguments for use when the
function is scheduled.
*/

#ifndef HSPI_DEVICE_SCHEDULED_H
#define HSPI_DEVICE_SCHEDULED_H

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

#include "wch-ch56x-lib/memory/ramx_alloc.h"

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

#ifndef HSPI_SERDES_TX_SIZE_MASK
#define HSPI_SERDES_TX_SIZE_MASK 0x0001fff
#endif
#define HSPI_ACK 0x0001fff

#define HSPI_USER_DEFINED_MASK 0x03ffffff

typedef struct hspi_scheduled_user_handled_t
{
	/**
   * @brief Programmed on interrupt_queue when a single packet has been
   * received. Size should be the same as set in hspi_send on the other side.
   * buffer points to a block in ramx_pool, it's the user's responsibility to
   * free it.
   * @param buffer address of one of the two reception buffer, alternating
   * because of double buffering
   * @param custom_register only the first 26bits are valid
   */
	void (*hspi_rx_callback)(uint8_t* buffer, uint16_t size,
							 uint16_t custom_register);

	/**
   * @brief Called when a CRC or NUM mismatch happens
   * @param
   */
	void (*hspi_err_crc_num_mismatch_callback)(void);
} hspi_scheduled_user_handled_t;

extern hspi_scheduled_user_handled_t hspi_scheduled_user_handled;

/**
 * @fn     hspi_doubledma_init
 *
 * @brief  hspi_doubledma_init mode (Double DMA for RX & TX with toggle Addr0/1)
 *
 * @param  mode_type -  Priority mode
 *                  HSPI_HOST - Host(Up) send data to Device, will have priority
 * over HSPI_DEVICE HSPI_DEVICE - Device(Down) receive data from Host Note that
 * both host/device will be set up, to be able to use the link in half-duplex
 *         data_mode -  Data bit width
 *                  RB_HSPI_DAT8_MOD  - 8bits data
 *                  RB_HSPI_DAT16_MOD - 16bits data
 *                  RB_HSPI_DAT32_MOD - 32bits data
 *          DMA0_addr - RX start address for DMA0
 *                  DMA0 starting address (shall be sized to contains up to 4096
 * bytes) DMA1_addr - RX start address for DMA1 DMA1 starting address (shall be
 * sized to contains up to 4096 bytes) DMA_addr_len Len in bytes of data to
 * TX(max 4096) can be set to 0 for RX
 *
 * @return   None
 */
void hspi_doubledma_init(HSPI_ModeTypeDef mode_type, uint8_t mode_data,
						 uint8_t* DMA0_addr, uint8_t* DMA1_addr,
						 uint16_t DMA_addr_len);

/**
 * @brief Initialize interrupts and parameters
 * @param type HSPI_TYPE_HOST to have priority on the line, the other side must
 * be HSPI_TYPE_DEVICE. This does not prevent the link to be half-duplex.
 * @param datasize Number of data lines used : HSPI_DATASIZE_8 (8bits)
 * HSPI_DATASIZE_16 (16bits) or HSPI_DATASIZE_32 (32bits)
 * @param size Size of the exchanged buffer in bytes, between 0 and 4096
 * included
 */
void hspi_init(HSPI_TYPE type, HSPI_DATASIZE datasize, uint16_t size);

/**
 * @brief Reset the internal buffers
 * @param
 */
void hspi_reinit_buffers(void);

/**
 * @brief Set the new buffer to be sent, will schedule it using interrupt_queue.
 * If buffer is in ramx_pool already, no copy will happen and hspi_send will
 * take a reference an free the buffer when finished.
 * @param custom_register Must be 26bits max
 */
bool hspi_send(uint8_t* buffer, uint16_t size, uint16_t custom_register);

#ifdef __cplusplus
}
#endif

#endif