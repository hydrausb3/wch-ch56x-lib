/********************************** (C) COPYRIGHT *******************************
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

#include "wch-ch56x-lib/SerDesDevice/serdes.h"
#include "wch-ch56x-lib/logging/logging.h"

volatile uint32_t SDS_RX_LEN0 = 0;
volatile uint32_t SDS_RX_LEN1 = 0;

__attribute__((aligned(16))) uint8_t RX_DMA_0[DMA_SIZE]
	__attribute((section(".DMADATA")));
__attribute__((aligned(16))) uint8_t RX_DMA_1[DMA_SIZE]
	__attribute__((section(".DMADATA")));

uint8_t* current_rx_dma = RX_DMA_0;
uint16_t serdes_max_packet_size = 0;

void _default_serdes_rx_callback(uint8_t* buffer, uint16_t size,
								 uint16_t custom_register);
void _default_serdes_rx_callback(uint8_t* buffer, uint16_t size,
								 uint16_t custom_register) {}

serdes_user_handled_t serdes_user_handled = { .serdes_rx_callback =
												  _default_serdes_rx_callback };

void serdes_init(SERDES_TYPE type, uint16_t max_packet_size)
{
	// setup SerDes
	current_rx_dma = RX_DMA_0;
	serdes_max_packet_size = max_packet_size;

	if (type == SERDES_TYPE_HOST)
	{
		SerDes_Tx_Init(SERDES_TX_RX_SPEED);
	}
	else if (type == SERDES_TYPE_DEVICE)
	{
		PFIC_EnableIRQ(INT_ID_SERDES);
		SerDes_Rx_Init(SERDES_TX_RX_SPEED);
		SerDes_DoubleDMA_Rx_CFG((vuint32_t)RX_DMA_0, (vuint32_t)RX_DMA_1);
		SerDes_EnableIT(SDS_RX_INT_EN | SDS_RX_ERR_EN | SDS_FIFO_OV_EN);
		SerDes_ClearIT(ALL_INT_TYPE);
	}
}

void serdes_send(uint8_t* buffer, uint16_t size, uint16_t custom_register)
{
	SerDes_DMA_Tx_CFG(
		(vuint32_t)buffer, serdes_max_packet_size,
		((size & HSPI_SERDES_TX_SIZE_MASK) | (custom_register << 13)) &
			SERDES_USER_DEFINED_MASK);
	SerDes_DMA_Tx();
	SerDes_Wait_Txdone();
}

__attribute__((interrupt("WCH-Interrupt-fast"))) void SERDES_IRQHandler(void);
__attribute__((interrupt("WCH-Interrupt-fast"))) void SERDES_IRQHandler(void)
{
	uint32_t sds_it_status;
	sds_it_status = SerDes_StatusIT();
	if (sds_it_status & SDS_RX_INT_FLG)
	{
		SDS_RX_LEN0 = SDS->SDS_RX_LEN0;
		SDS_RX_LEN1 = SDS->SDS_RX_LEN1;

		// switch between DMA buffers, new data will be put in those alternatively
		if (current_rx_dma == RX_DMA_0)
		{
			serdes_user_handled.serdes_rx_callback(
				current_rx_dma, SDS->SDS_DATA0 & HSPI_SERDES_TX_SIZE_MASK,
				SDS->SDS_DATA0 >> 13);
			current_rx_dma = RX_DMA_1;
		}
		else
		{
			serdes_user_handled.serdes_rx_callback(
				current_rx_dma, SDS->SDS_DATA1 & HSPI_SERDES_TX_SIZE_MASK,
				SDS->SDS_DATA1 >> 13);
			current_rx_dma = RX_DMA_0;
		}
	}

	SerDes_ClearIT(ALL_INT_FLG);
}
