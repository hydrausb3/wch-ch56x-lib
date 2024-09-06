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

#include "wch-ch56x-lib/HSPIDeviceScheduled/hspi_scheduled.h"
#include "wch-ch56x-lib/interrupt_queue/interrupt_queue.h"
#include "wch-ch56x-lib/logging/log_to_buffer.h"
#include "wch-ch56x-lib/logging/logging.h"
#include "wch-ch56x-lib/memory/fifo.h"
#include "wch-ch56x-lib/memory/pool.h"
#include "wch-ch56x-lib/utils/critical_section.h"
#include <stdint.h>

__attribute__((aligned(16))) uint8_t* hspi_rx_buffer_0;
__attribute__((aligned(16))) uint8_t* hspi_rx_buffer_1;
__attribute__((aligned(16))) uint8_t* hspi_tx_buffer_0;
__attribute__((aligned(16))) uint8_t* hspi_tx_buffer_1;

#define HSPI_DMA0_MASK 1 << 0
#define HSPI_DMA1_MASK 1 << 1
#define HSPI_MAX_SCHEDULED INTERRUPT_QUEUE_SIZE

void _default_hspi_rx_callback(uint8_t* buffer, uint16_t size,
							   uint16_t custom_register);
void _default_hspi_rx_callback(uint8_t* buffer, uint16_t size,
							   uint16_t custom_register) {}
void _default_hspi_err_crc_num_mismatch_callback(void);
void _default_hspi_err_crc_num_mismatch_callback(void) {}

hspi_scheduled_user_handled_t hspi_scheduled_user_handled = {
	.hspi_rx_callback = _default_hspi_rx_callback,
	.hspi_err_crc_num_mismatch_callback =
		_default_hspi_err_crc_num_mismatch_callback
};

typedef enum HSPI_CURRENT_DMA
{
	HSPI_DMA_0,
	HSPI_DMA_1
} hspi_scheduled_current_dma_t;

typedef struct __attribute__((packed)) HSPI_TASK
{
	bool in_use;
	struct
	{
		uint8_t* buffer;
		uint16_t size;
		uint16_t custom_register;
	} args;
} hspi_args_t;

HYDRA_POOL_DEF(hspi_arg_pool, hspi_args_t, HSPI_MAX_SCHEDULED);
uint16_t hspi_packet_size = 0;
volatile bool hspi_transmission_finished = true;

void hspi_init_args_pool(void);
void hspi_init_args_pool(void) { hydra_pool_clean(&hspi_arg_pool); }

void hspi_doubledma_init(HSPI_ModeTypeDef mode_type, uint8_t mode_data,
						 uint8_t* DMA0_addr, uint8_t* DMA1_addr,
						 uint16_t DMA_addr_len)
{
	/* HSPI GPIO configuration */
	// TX GPIO PA9/PA11/PA21 Push-pull output
	R32_PA_DIR |= (1 << 9) | (1 << 11) | (1 << 21);
	// clk 16mA
	R32_PA_DRV |= (1 << 11);
	// RX GPIO PA10 Push-pull output
	R32_PA_DIR |= (1 << 10);
	bsp_wait_us_delay(1); /* Wait "GPIO stabilization" mandatory before HSPI
                           configuration to avoid potential CRC error */

	R8_HSPI_CFG &= ~(RB_HSPI_MODE | RB_HSPI_MSK_SIZE); // Clear HSPI Configuration
	if (mode_type == HSPI_HOST)
	{
		R8_HSPI_CFG |= RB_HSPI_MODE; // Host
	}
	else
	{
		R8_HSPI_CFG &= ~(RB_HSPI_MODE); // Device
	}
	// Data size (8/16 or 32bits)
	R8_HSPI_CFG |= mode_data;

	// ACk mode 0 (Hardware auto-answer mode is used in burst mode, not in normal
	// mode)
	R8_HSPI_CFG &= ~RB_HSPI_HW_ACK;

	R8_HSPI_CFG |= RB_HSPI_TX_TOG_EN;
	R8_HSPI_CFG |= RB_HSPI_RX_TOG_EN;

	// DUDMA 1 Enable dual DMA function Default enabled
	R8_HSPI_CFG |= RB_HSPI_DUALDMA;

	// Enable fast DMA request
	R8_HSPI_AUX |= RB_HSPI_REQ_FT;

	// TX sampling edge
	R8_HSPI_AUX |= RB_HSPI_TCK_MOD; // falling edge sampling

	// Hardware Auto ack time disabled
	R8_HSPI_AUX &= ~RB_HSPI_ACK_TX_MOD;

	// Delay time
	R8_HSPI_AUX &= ~RB_HSPI_ACK_CNT_SEL; //  Delay 2T

	// clear ALL_CLR  TRX_RST  (reset)
	R8_HSPI_CTRL &= ~(RB_HSPI_ALL_CLR | RB_HSPI_TRX_RST);

	// Enable Interrupt
	//  R8_HSPI_INT_EN |= RB_HSPI_IE_T_DONE;  // Single packet sending completed
	R8_HSPI_INT_EN |= RB_HSPI_IE_FIFO_OV;

	// HSPI_DEVICE
	R8_HSPI_INT_EN |= RB_HSPI_IE_R_DONE; // Single packet reception completed
	R8_HSPI_INT_EN |= RB_HSPI_IE_FIFO_OV;

	// reset packet number check
	if ((R8_HSPI_RX_SC & RB_HSPI_RX_TOG) == RB_HSPI_RX_TOG)
	{
		R8_HSPI_RX_SC = RB_HSPI_RX_TOG;
	}
	R8_HSPI_RX_SC = 0x00;
	if ((R8_HSPI_TX_SC & RB_HSPI_TX_TOG) == RB_HSPI_TX_TOG)
	{
		R8_HSPI_TX_SC = RB_HSPI_TX_TOG;
	}
	R8_HSPI_TX_SC = 0x00;

	// Config TX customized Header
	R32_HSPI_UDF0 = 0x3ABCDEF; // UDF0
	R32_HSPI_UDF1 = 0x3456789; // UDF1

	// addr0 DMA TX RX addr
	R32_HSPI_TX_ADDR0 = 0;
	R32_HSPI_RX_ADDR0 = (vuint32_t)DMA0_addr;
	// addr1 DMA TX RX addr
	R32_HSPI_TX_ADDR1 = 0;
	R32_HSPI_RX_ADDR1 = (vuint32_t)DMA1_addr;

	// addr0 TX DMA len
	R16_HSPI_DMA_LEN0 =
		DMA_addr_len -
		1; // "Actual transmission length (LEN0+1)." per WCH's datasheet
	// addr1 TX DMA len
	R16_HSPI_DMA_LEN1 = DMA_addr_len - 1;

	// addr0 RX DMA len
	R16_HSPI_RX_LEN0 = DMA_addr_len; // 0 means max size 4096 bytes
	// addr1 RX DMA len
	R16_HSPI_RX_LEN1 = DMA_addr_len; // 0 means max size 4096 bytes

	// Burst Disabled
	R16_HSPI_BURST_CFG = 0x0000;

	// Enable HSPI DMA
	R8_HSPI_CTRL |= RB_HSPI_ENABLE | RB_HSPI_DMA_EN;

	PFIC_EnableIRQ(HSPI_IRQn);
}

void hspi_reinit_buffers(void)
{
	hspi_transmission_finished = true;
	hspi_init_args_pool();
	hspi_rx_buffer_0 = ramx_pool_alloc_bytes(hspi_packet_size);
	hspi_rx_buffer_1 = ramx_pool_alloc_bytes(hspi_packet_size);
	hspi_tx_buffer_0 = ramx_pool_alloc_bytes(hspi_packet_size);
	hspi_tx_buffer_1 = ramx_pool_alloc_bytes(hspi_packet_size);
	// addr0 DMA TX RX addr
	R32_HSPI_TX_ADDR0 = 0;
	R32_HSPI_RX_ADDR0 = (vuint32_t)hspi_rx_buffer_0;
	// addr1 DMA TX RX addr
	R32_HSPI_TX_ADDR1 = 0;
	R32_HSPI_RX_ADDR1 = (vuint32_t)hspi_rx_buffer_1;
}

void hspi_init(HSPI_TYPE type, HSPI_DATASIZE datasize, uint16_t size)
{
	hspi_transmission_finished = true;
	hspi_init_args_pool();
	hspi_packet_size = size;
	hspi_rx_buffer_0 = ramx_pool_alloc_bytes(hspi_packet_size);
	hspi_rx_buffer_1 = ramx_pool_alloc_bytes(hspi_packet_size);
	hspi_tx_buffer_0 = ramx_pool_alloc_bytes(hspi_packet_size);
	hspi_tx_buffer_1 = ramx_pool_alloc_bytes(hspi_packet_size);

	switch (datasize)
	{
	case HSPI_DATASIZE_8:
		hspi_doubledma_init(type == HSPI_TYPE_HOST ? HSPI_HOST : HSPI_DEVICE,
							RB_HSPI_DAT8_MOD, hspi_rx_buffer_0, hspi_rx_buffer_1,
							hspi_packet_size);
		break;
	case HSPI_DATASIZE_16:
		hspi_doubledma_init(type == HSPI_TYPE_HOST ? HSPI_HOST : HSPI_DEVICE,
							RB_HSPI_DAT16_MOD, hspi_rx_buffer_0, hspi_rx_buffer_1,
							hspi_packet_size);
		break;
	case HSPI_DATASIZE_32:
		hspi_doubledma_init(type == HSPI_TYPE_HOST ? HSPI_HOST : HSPI_DEVICE,
							RB_HSPI_DAT32_MOD, hspi_rx_buffer_0, hspi_rx_buffer_1,
							hspi_packet_size);
		break;
	default:
		return;
		break;
	}
}

void _hspi_cleanup(uint8_t* data);
void _hspi_cleanup(uint8_t* data)
{
	hspi_args_t* hspi_task_args = (hspi_args_t*)data;
	ramx_pool_free(hspi_task_args->args.buffer);
	hydra_pool_free(&hspi_arg_pool, hspi_task_args);
}

bool _hspi_rx_callback(uint8_t* data);
bool _hspi_rx_callback(uint8_t* data)
{
	hspi_args_t* hspi_task_args = (hspi_args_t*)data;
	if (hspi_task_args == NULL)
		return false;
	hspi_scheduled_user_handled.hspi_rx_callback(
		hspi_task_args->args.buffer, hspi_task_args->args.size,
		hspi_task_args->args.custom_register);
	return true;
}

bool _hspi_send(uint8_t* data);
bool _hspi_send(uint8_t* data)
{
	hspi_args_t* hspi_task_args = (hspi_args_t*)data;

	if (hspi_task_args == NULL)
		return false;

	BSP_ENTER_CRITICAL();
	if (R8_HSPI_TX_SC & RB_HSPI_TX_TOG)
	{
		R32_HSPI_TX_ADDR1 = (vuint32_t)hspi_task_args->args.buffer;
		R32_HSPI_UDF1 = ((hspi_task_args->args.size & HSPI_SERDES_TX_SIZE_MASK) |
						 ((hspi_task_args->args.custom_register << 13) &
						  ~HSPI_SERDES_TX_SIZE_MASK)) &
						HSPI_USER_DEFINED_MASK;
		// uint8_t* tx0_addr = hspi_task_args->args.buffer;
		// LOG_IF(LOG_LEVEL_DEBUG, LOG_ID_HSPI, "R32_HSPI_TX_ADDR1 DMA1 addr %x size
		// %d %d %d %d %d %d\r\n", tx0_addr, hspi_task_args->args.size, tx0_addr[0],
		// tx0_addr[1], tx0_addr[2], tx0_addr[3], tx0_addr[4]);
	}
	else
	{
		R32_HSPI_TX_ADDR0 = (vuint32_t)hspi_task_args->args.buffer;
		R32_HSPI_UDF0 = ((hspi_task_args->args.size & HSPI_SERDES_TX_SIZE_MASK) |
						 ((hspi_task_args->args.custom_register << 13) &
						  ~HSPI_SERDES_TX_SIZE_MASK)) &
						HSPI_USER_DEFINED_MASK;
		// uint8_t* tx1_addr = hspi_task_args->args.buffer;
		// LOG_IF(LOG_LEVEL_DEBUG, LOG_ID_HSPI, "R32_HSPI_TX_ADDR0 DMA0 addr %x size
		// %d %d %d %d %d %d\r\n", tx1_addr, hspi_task_args->args.size, tx1_addr[0],
		// tx1_addr[1], tx1_addr[2], tx1_addr[3], tx1_addr[4]);
	}
	hspi_transmission_finished = false;
	BSP_EXIT_CRITICAL();

	HSPI_DMA_Tx();

	while (!(R8_HSPI_INT_FLAG & RB_HSPI_IF_T_DONE))
	{
	}

	R8_HSPI_INT_FLAG = RB_HSPI_IF_T_DONE;
	hydra_interrupt_queue_task_t task;

	// if transmissions happen too fast, the other side could still be in an
	// interrupt and miss this transmission. this adds a high-enough delay to try
	// to prevent this WCH might have been able to prevent this by using HSPI's
	// HTRDY line to check if the receiving side is ready ... this must be done on
	// hardware to prevent the DMA from shifting registers too fast (at least in
	// some mode)
	if (hydra_interrupt_queue_peek_next_task_prio0(&task))
	{
		if (task.task == _hspi_send)
		{
			LOG_IF(LOG_LEVEL_DEBUG, LOG_ID_HSPI,
				   "adding delay because of consecutive _hspi_send\r\n");
			bsp_wait_us_delay(
				500); // hope this is enough to prevent any miss on reception end
		}
	}

	return true;
}

bool hspi_send(uint8_t* buffer, uint16_t size, uint16_t custom_register)
{
	hspi_args_t* hspi_task_args = hydra_pool_get(&hspi_arg_pool);
	if (hspi_task_args == NULL)
		return false;

	// only copy to a new buffer if buffer is not in ramx_pool already
	if (!ramx_address_in_pool(buffer))
	{
		uint8_t* new_buffer = ramx_pool_alloc_bytes(hspi_packet_size);
		if (new_buffer == NULL)
			return false;
		memcpy(new_buffer, buffer, size);
		hspi_task_args->args.buffer = new_buffer;
	}
	else
	{
		ramx_take_ownership(buffer);
		hspi_task_args->args.buffer = buffer;
	}
	hspi_task_args->args.size = size;
	hspi_task_args->args.custom_register = custom_register;
	hydra_interrupt_queue_set_next_task(_hspi_send, (uint8_t*)hspi_task_args,
										_hspi_cleanup);
	return true;
}

__attribute__((interrupt("WCH-Interrupt-fast"))) void HSPI_IRQHandler(void);
__attribute__((interrupt("WCH-Interrupt-fast"))) void HSPI_IRQHandler(void)
{
	// LOG_IF(LOG_LEVEL_DEBUG, LOG_ID_HSPI, "RB_HSPI_RX_NUM %d  RB_HSPI_TX_NUM %d
	// HSPI_RX_LEN0 %d HSPI_RX_LEN1 %d \r\n", HSPI_RX_SC & RB_HSPI_RX_NUM,
	// HSPI_TX_SC & RB_HSPI_TX_NUM, R16_HSPI_DMA_LEN0, R16_HSPI_DMA_LEN1);

	if (R8_HSPI_INT_FLAG & RB_HSPI_IF_R_DONE)
	{
		R8_HSPI_INT_FLAG = RB_HSPI_IF_R_DONE; // Clear Interrupt

		vuint32_t udf0 = R32_HSPI_UDF0;
		vuint32_t udf1 = R32_HSPI_UDF1;

		if ((R8_HSPI_RTX_STATUS & (RB_HSPI_CRC_ERR | RB_HSPI_NUM_MIS)) == 0)
		{
			if (R8_HSPI_RX_SC & RB_HSPI_RX_TOG)
			{
				hspi_args_t* hspi_task_args = hydra_pool_get(&hspi_arg_pool);
				if (hspi_task_args != NULL)
				{
					// LOG_IF(LOG_LEVEL_DEBUG, LOG_ID_HSPI, "R8_HSPI_RX_SC DMA0 addr %x %d
					// %d %d %d %d / DMA1 %x %d %d %d %d %d\r\n", hspi_rx_buffer_0,
					// hspi_rx_buffer_0[0], hspi_rx_buffer_0[1], hspi_rx_buffer_0[2],
					// hspi_rx_buffer_0[3], hspi_rx_buffer_0[4], hspi_rx_buffer_1,
					// hspi_rx_buffer_1[0], hspi_rx_buffer_1[1], hspi_rx_buffer_1[2],
					// hspi_rx_buffer_1[3], hspi_rx_buffer_1[4]);
					hspi_task_args->args.buffer = hspi_rx_buffer_0;
					hspi_task_args->args.size =
						(udf0 & HSPI_USER_DEFINED_MASK) & HSPI_SERDES_TX_SIZE_MASK;
					hspi_task_args->args.custom_register =
						(udf0 & HSPI_USER_DEFINED_MASK) >> 13;
					hydra_interrupt_queue_set_next_task(
						_hspi_rx_callback, (uint8_t*)hspi_task_args, _hspi_cleanup);
				}
				hspi_rx_buffer_0 = ramx_pool_alloc_bytes(hspi_packet_size);
				R32_HSPI_RX_ADDR0 = (vuint32_t)hspi_rx_buffer_0;
			}
			else
			{
				hspi_args_t* hspi_task_args = hydra_pool_get(&hspi_arg_pool);
				if (hspi_task_args != NULL)
				{
					// LOG_IF(LOG_LEVEL_DEBUG, LOG_ID_HSPI, "R8_HSPI_RX_SC DMA1 addr %x %d
					// %d %d %d %d / DMA0 %x %d %d %d %d %d\r\n", hspi_rx_buffer_1,
					// hspi_rx_buffer_1[0], hspi_rx_buffer_1[1], hspi_rx_buffer_1[2],
					// hspi_rx_buffer_1[3], hspi_rx_buffer_1[4], hspi_rx_buffer_0,
					// hspi_rx_buffer_0[0], hspi_rx_buffer_0[1], hspi_rx_buffer_0[2],
					// hspi_rx_buffer_0[3], hspi_rx_buffer_0[4]);
					hspi_task_args->args.buffer = hspi_rx_buffer_1;
					hspi_task_args->args.size =
						(udf1 & HSPI_USER_DEFINED_MASK) & HSPI_SERDES_TX_SIZE_MASK;
					hspi_task_args->args.custom_register =
						(udf1 & HSPI_USER_DEFINED_MASK) >> 13;
					hydra_interrupt_queue_set_next_task(
						_hspi_rx_callback, (uint8_t*)hspi_task_args, _hspi_cleanup);
				}
				hspi_rx_buffer_1 = ramx_pool_alloc_bytes(hspi_packet_size);
				R32_HSPI_RX_ADDR1 = (vuint32_t)hspi_rx_buffer_1;
			}
		}
		else
		{
			LOG_IF(LOG_LEVEL_DEBUG, LOG_ID_HSPI,
				   "ERROR : either CRC or NUM toggle mismatch \r\n");
			hspi_scheduled_user_handled.hspi_err_crc_num_mismatch_callback();
		}
	}

	if (R8_HSPI_INT_FLAG & RB_HSPI_IF_FIFO_OV)
	{
		LOG_IF(LOG_LEVEL_DEBUG, LOG_ID_HSPI, "RB_HSPI_IF_FIFO_OV \r\n");
		R8_HSPI_INT_FLAG = RB_HSPI_IF_FIFO_OV; // Clear Interrupt
	}

	if (R8_HSPI_INT_FLAG & RB_HSPI_IF_B_DONE)
	{
		LOG_IF(LOG_LEVEL_DEBUG, LOG_ID_HSPI, "RB_HSPI_IF_B_DONE \r\n");
		R8_HSPI_INT_FLAG = RB_HSPI_IF_B_DONE; // Clear Interrupt
	}
}
