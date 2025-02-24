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

#include "usb2_device_descriptors.h"
#include "usb3_device_descriptors.h"
#include "usb_device.h"
#include "wch-ch56x-lib/HSPIDevice/hspi.h"
#include "wch-ch56x-lib/logging/logging.h"
#include "wch-ch56x-lib/SerDesDevice/serdes.h"
#include "wch-ch56x-lib/usb/usb20.h"
#include "wch-ch56x-lib/usb/usb30.h"
#include "wch-ch56x-lib/usb/usb_descriptors.h"
#include "wch-ch56x-lib/usb/usb_endpoints.h"

#undef FREQ_SYS
/* System clock / MCU frequency in Hz (lowest possible speed 15MHz) */
#define FREQ_SYS (120000000)

bool_t is_board1;

void hspi_rx_callback(uint8_t* buffer, uint16_t size, uint16_t custom_register);
void hspi_rx_callback(uint8_t* buffer, uint16_t size,
					  uint16_t custom_register)
{
	serdes_send(buffer, size, 0);
}

void serdes_rx_callback(uint8_t* buffer, uint16_t size,
						uint16_t custom_register);
void serdes_rx_callback(uint8_t* buffer, uint16_t size,
						uint16_t custom_register)
{
	endp_tx_set_new_buffer(&usb_device_0, 1, buffer, size);
}

uint8_t endp1_rx_callback(uint8_t* const ptr, uint16_t size);
uint8_t endp1_rx_callback(uint8_t* const ptr, uint16_t size)
{
	hspi_send(endp1_rx_buffer, size, 0);
	return 0x00;
}

void hspi_err_crc_num_mismatch_callback(void);
void hspi_err_crc_num_mismatch_callback(void)
{
	if (is_board1)
	{
		hspi_init(HSPI_TYPE_HOST, HSPI_DATASIZE_32, sizeof(endp1_rx_buffer));
	}
	else
	{
		hspi_init(HSPI_TYPE_DEVICE, HSPI_DATASIZE_32, sizeof(endp1_rx_buffer));
	}
}

/* Blink time in ms */
#define BLINK_FAST (50) // Blink LED each 100ms (50*2)
#define BLINK_USB3 (250) // Blink LED each 500ms (250*2)
#define BLINK_USB2 (1000) // Blink LED each 2000ms (1000*2)
int blink_ms = BLINK_USB2;
USB_DEVICE_SPEED usb_device_old_speed = -1;

/*********************************************************************
 * @fn      main
 *
 * @brief   Main program.
 *
 * @return  none
 */
int main()
{
	uint32_t err;

	/* Configure GPIO In/Out default/safe state for the board */
	bsp_gpio_init();
	/* Init BSP (MCU Frequency & SysTick) */
	bsp_init(FREQ_SYS);

	LOG_INIT(FREQ_SYS);

	/******************************************/
	/* Start Synchronization between 2 Boards */
	/* J3 MOSI(PA14) & J3 SCS(PA12) signals   */
	/******************************************/
	if (bsp_switch() == 0)
	{
		is_board1 = false;
		err = bsp_sync2boards(PA14, PA12, BSP_BOARD2);
	}
	else
	{
		is_board1 = true;
		err = bsp_sync2boards(PA14, PA12, BSP_BOARD1);
	}
	if (err > 0)
		LOG("SYNC %08d\n", err);
	else
		LOG("SYNC Err Timeout\n");
	log_time_init(); // Reinit log time after synchro
	/* Test Synchronization to be checked with Oscilloscope/LA */
	bsp_uled_on();
	bsp_uled_off();
	/****************************************/
	/* End Synchronization between 2 Boards */
	/****************************************/

	usb_device_0.endpoints.rx_callback[1] = endp1_rx_callback;
	hspi_user_handled.hspi_rx_callback = hspi_rx_callback;
	hspi_user_handled.hspi_err_crc_num_mismatch_callback =
		hspi_err_crc_num_mismatch_callback;
	serdes_user_handled.serdes_rx_callback = serdes_rx_callback;

	if (is_board1)
	{
		// Finish initializing the descriptor parameters
		init_usb3_descriptors();
		init_usb2_descriptors();
		init_string_descriptors();

		// Set the USB device parameters
		usb_device_set_usb3_device_descriptor(&usb_device_0, &usb3_descriptors.usb_device_descr);
		usb_device_set_usb2_device_descriptor(&usb_device_0, &usb2_descriptors.usb_device_descr);
		usb_device_set_usb3_config_descriptors(&usb_device_0, usb3_device_configs);
		usb_device_set_usb2_config_descriptors(&usb_device_0, usb2_device_configs);
		usb_device_set_bos_descriptor(&usb_device_0,
									  &usb3_descriptors.capabilities.usb_bos_descr);
		usb_device_set_string_descriptors(&usb_device_0, device_string_descriptors);
		usb_device_set_endpoint_mask(&usb_device_0, ENDPOINT_1_RX | ENDPOINT_1_TX);

		init_endpoints();

		if (!bsp_ubtn())
		{
			usb_device_0.speed = USB30_SUPERSPEED;
			usb30_device_init(false);
		}
		else
		{
			usb_device_0.speed = USB2_HIGHSPEED;
			usb2_device_init();
		}

		hspi_init(HSPI_TYPE_HOST, HSPI_DATASIZE_32, sizeof(endp1_rx_buffer));
		serdes_init(SERDES_TYPE_DEVICE, sizeof(endp1_rx_buffer));

		// Infinite loop USB2/USB3 managed with Interrupt
		while (1)
		{
			if (bsp_ubtn())
			{
				blink_ms = BLINK_FAST;
				bsp_uled_on();
				bsp_wait_ms_delay(blink_ms);
				bsp_uled_off();
				bsp_wait_ms_delay(blink_ms);
				LOG_DUMP();
			}
			else
			{
				if (usb_device_0.state == CONFIGURED)
				{
					switch (usb_device_0.speed)
					{
					case USB2_LOWSPEED: // USB2
					case USB2_FULLSPEED:
					case USB2_HIGHSPEED: {
						if (usb_device_0.speed != usb_device_old_speed)
						{
							usb_device_old_speed = usb_device_0.speed;
							LOG("USB2\n");
						}
						blink_ms = BLINK_USB2;
						bsp_uled_on();
						bsp_wait_ms_delay(blink_ms);
						bsp_uled_off();
						bsp_wait_ms_delay(blink_ms);
					}
					break;
					case USB30_SUPERSPEED: // USB3
					{
						if (usb_device_0.speed != usb_device_old_speed)
						{
							usb_device_old_speed = usb_device_0.speed;
							LOG("USB3\n");
						}
						blink_ms = BLINK_USB3;
						bsp_uled_on();
						bsp_wait_ms_delay(blink_ms);
						bsp_uled_off();
						bsp_wait_ms_delay(blink_ms);
					}
					break;
					case SPEED_NONE:
					default:
						bsp_uled_on(); // LED is steady until USB3 SS or USB2 HS is ready
						break;
					}
				}
				else
				{
					bsp_uled_on(); // LED is steady until USB3 SS or USB2 HS is ready
				}
			}
		}
	}
	else
	{
		hspi_init(HSPI_TYPE_DEVICE, HSPI_DATASIZE_32, sizeof(endp1_rx_buffer));
		serdes_init(SERDES_TYPE_HOST, sizeof(endp1_rx_buffer));
	}
}

__attribute__((interrupt("WCH-Interrupt-fast"))) void WDOG_IRQHandler(void);
__attribute__((interrupt("WCH-Interrupt-fast"))) void WDOG_IRQHandler(void)
{
	LOG_DUMP();

	LOG_IF_LEVEL(LOG_LEVEL_CRITICAL,
				 "WDOG_IRQHandler\r\n"
				 " SP=0x%08X\r\n"
				 " MIE=0x%08X\r\n"
				 " MSTATUS=0x%08X\r\n"
				 " MCAUSE=0x%08X\r\n"
				 " MVENDORID=0x%08X\r\n"
				 " MARCHID=0x%08X\r\n"
				 " MISA=0x%08X\r\n"
				 " MIMPID=0x%08X\r\n"
				 " MHARTID=0x%08X\r\n"
				 " MEPC=0x%08X\r\n"
				 " MSCRATCH=0x%08X\r\n"
				 " MTVEC=0x%08X\r\n",
				 __get_SP(), __get_MIE(), __get_MSTATUS(), __get_MCAUSE(),
				 __get_MVENDORID(), __get_MARCHID(), __get_MISA(), __get_MIMPID(),
				 __get_MHARTID(), __get_MEPC(), __get_MSCRATCH(), __get_MTVEC());

	LOG_DUMP();

	bsp_wait_ms_delay(100000000);
}

/*********************************************************************
 * @fn      HardFault_Handler
 *
 * @brief   Example of basic HardFault Handler called if an exception occurs
 *
 * @return  none
 */
__attribute__((interrupt("WCH-Interrupt-fast"))) void HardFault_Handler(void);
__attribute__((interrupt("WCH-Interrupt-fast"))) void HardFault_Handler(void)
{
	LOG_DUMP();

	// asm("ebreak"); to trigger a breakpoint and test hardfault_handler
	LOG_IF_LEVEL(LOG_LEVEL_CRITICAL,
				 "HardFault_Handler\r\n"
				 " SP=0x%08X\r\n"
				 " MIE=0x%08X\r\n"
				 " MSTATUS=0x%08X\r\n"
				 " MCAUSE=0x%08X\r\n"
				 " MVENDORID=0x%08X\r\n"
				 " MARCHID=0x%08X\r\n"
				 " MISA=0x%08X\r\n"
				 " MIMPID=0x%08X\r\n"
				 " MHARTID=0x%08X\r\n"
				 " MEPC=0x%08X\r\n"
				 " MSCRATCH=0x%08X\r\n"
				 " MTVEC=0x%08X\r\n",
				 __get_SP(), __get_MIE(), __get_MSTATUS(), __get_MCAUSE(),
				 __get_MVENDORID(), __get_MARCHID(), __get_MISA(), __get_MIMPID(),
				 __get_MHARTID(), __get_MEPC(), __get_MSCRATCH(), __get_MTVEC());

	LOG_DUMP();

	bsp_wait_ms_delay(100000000);
}
