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

********************************************************************************/

#include "wch-ch56x-lib/hspi/hspi.h"
#include "wch-ch56x-lib/logging/logging.h"
#include <stdint.h>

// must be contiguous
uint8_t* dma_buffer_0 = NULL;
uint8_t* dma_buffer_1 = NULL;

__attribute__((aligned(16))) uint8_t hspi_rx_buffer_0[4096]
	__attribute__((section(".DMADATA")));
__attribute__((aligned(16))) uint8_t hspi_rx_buffer_1[4096]
	__attribute__((section(".DMADATA")));

void _default_hspi_rx_callback(uint8_t* buffer, uint16_t size,
							   uint16_t custom_register);
void _default_hspi_rx_callback(uint8_t* buffer, uint16_t size,
							   uint16_t custom_register) {}
void _default_hspi_err_crc_num_mismatch_callback(void);
void _default_hspi_err_crc_num_mismatch_callback(void) {}

hspi_user_handled_t hspi_user_handled = {
	.hspi_rx_callback = NULL,
	.hspi_err_crc_num_mismatch_callback = NULL
};

typedef enum CURRENT_DMA_BUFFER
{
	HSPI_DMA_BUFFER_0,
	HSPI_DMA_BUFFER_1
} CURRENT_DMA_BUFFER;

CURRENT_DMA_BUFFER current_dma_buffer = HSPI_DMA_BUFFER_0;

void hspi_init(HSPI_TYPE type, HSPI_DATASIZE datasize, uint16_t size)
{
	if (type == HSPI_TYPE_DEVICE)
	{
		dma_buffer_0 = hspi_rx_buffer_0;
		dma_buffer_1 = hspi_rx_buffer_1;
	}

	switch (datasize)
	{
	case HSPI_DATASIZE_8:
		HSPI_DoubleDMA_Init(type == HSPI_TYPE_HOST ? HSPI_HOST : HSPI_DEVICE,
							RB_HSPI_DAT8_MOD, (vuint32_t)dma_buffer_0,
							(vuint32_t)dma_buffer_1, size);
		break;
	case HSPI_DATASIZE_16:
		HSPI_DoubleDMA_Init(type == HSPI_TYPE_HOST ? HSPI_HOST : HSPI_DEVICE,
							RB_HSPI_DAT16_MOD, (vuint32_t)dma_buffer_0,
							(vuint32_t)dma_buffer_1, size);
		break;
	case HSPI_DATASIZE_32:
		HSPI_DoubleDMA_Init(type == HSPI_TYPE_HOST ? HSPI_HOST : HSPI_DEVICE,
							RB_HSPI_DAT32_MOD, (vuint32_t)dma_buffer_0,
							(vuint32_t)dma_buffer_1, size);
		break;
	default:
		return;
		break;
	}
}

bool hspi_send(uint8_t* buffer, uint16_t size, uint16_t custom_register)
{
	if (current_dma_buffer == HSPI_DMA_BUFFER_0)
	{
		R32_HSPI_TX_ADDR0 = (vuint32_t)buffer;
		R32_HSPI_UDF0 = ((size & HSPI_SERDES_TX_SIZE_MASK) |
						 ((custom_register << 13) & ~HSPI_SERDES_TX_SIZE_MASK)) &
						HSPI_USER_DEFINED_MASK;
	}
	else if (current_dma_buffer == HSPI_DMA_BUFFER_1)
	{
		R32_HSPI_TX_ADDR1 = (vuint32_t)buffer;
		R32_HSPI_UDF1 = ((size & HSPI_SERDES_TX_SIZE_MASK) |
						 ((custom_register << 13) & ~HSPI_SERDES_TX_SIZE_MASK)) &
						HSPI_USER_DEFINED_MASK;
	}
	else
	{
		return false;
	}

	HSPI_DMA_Tx();

	return true;
}

__attribute__((interrupt("WCH-Interrupt-fast"))) void HSPI_IRQHandler(void);
__attribute__((interrupt("WCH-Interrupt-fast"))) void HSPI_IRQHandler(void)
{
	if (R8_HSPI_INT_FLAG & RB_HSPI_IF_T_DONE)
	{
		R8_HSPI_INT_FLAG = RB_HSPI_IF_T_DONE; // Clear Interrupt

		if (current_dma_buffer == HSPI_DMA_BUFFER_0)
		{
			current_dma_buffer = HSPI_DMA_BUFFER_1;
		}
		else if (current_dma_buffer == HSPI_DMA_BUFFER_1)
		{
			current_dma_buffer = HSPI_DMA_BUFFER_0;
		}
	}
	else if (R8_HSPI_INT_FLAG & RB_HSPI_IF_R_DONE)
	{
		R8_HSPI_INT_FLAG = RB_HSPI_IF_R_DONE; // Clear Interrupt

		if ((R8_HSPI_RTX_STATUS & (RB_HSPI_CRC_ERR | RB_HSPI_NUM_MIS)) == 0)
		{
			if (current_dma_buffer == HSPI_DMA_BUFFER_0)
			{
				current_dma_buffer = HSPI_DMA_BUFFER_1;
				hspi_user_handled.hspi_rx_callback(
					hspi_rx_buffer_0, R32_HSPI_UDF0 & HSPI_SERDES_TX_SIZE_MASK,
					(R32_HSPI_UDF0 & ~HSPI_SERDES_TX_SIZE_MASK) >> 13);
			}
			else if (current_dma_buffer == HSPI_DMA_BUFFER_1)
			{
				current_dma_buffer = HSPI_DMA_BUFFER_0;
				hspi_user_handled.hspi_rx_callback(
					hspi_rx_buffer_1, R32_HSPI_UDF1 & HSPI_SERDES_TX_SIZE_MASK,
					(R32_HSPI_UDF1 & ~HSPI_SERDES_TX_SIZE_MASK) >> 13);
			}
		}
		else
		{
			LOG_IF(LOG_LEVEL_DEBUG, LOG_ID_HSPI,
				   "ERROR : either CRC or NUM toggle mismatch \r\n");
			hspi_user_handled.hspi_err_crc_num_mismatch_callback();
		}
	}
}
