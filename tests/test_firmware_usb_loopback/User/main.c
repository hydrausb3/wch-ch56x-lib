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
#include "wch-ch56x-lib/logging/logging.h"
#include "wch-ch56x-lib/USBDevice/usb20.h"
#include "wch-ch56x-lib/USBDevice/usb30.h"
#include "wch-ch56x-lib/USBDevice/usb_descriptors.h"
#include "wch-ch56x-lib/USBDevice/usb_endpoints.h"

#undef FREQ_SYS
/* System clock / MCU frequency in Hz (lowest possible speed 15MHz) */
#define FREQ_SYS (120000000)

void endp1_tx_complete(TRANSACTION_STATUS status);
void endp1_tx_complete(TRANSACTION_STATUS status) {}
void endp2_tx_complete(TRANSACTION_STATUS status);
void endp2_tx_complete(TRANSACTION_STATUS status) {}
void endp3_tx_complete(TRANSACTION_STATUS status);
void endp3_tx_complete(TRANSACTION_STATUS status) {}
void endp4_tx_complete(TRANSACTION_STATUS status);
void endp4_tx_complete(TRANSACTION_STATUS status) {}
void endp5_tx_complete(TRANSACTION_STATUS status);
void endp5_tx_complete(TRANSACTION_STATUS status) {}
void endp6_tx_complete(TRANSACTION_STATUS status);
void endp6_tx_complete(TRANSACTION_STATUS status) {}
void endp7_tx_complete(TRANSACTION_STATUS status);
void endp7_tx_complete(TRANSACTION_STATUS status) {}

uint8_t endp1_rx_callback(uint8_t* const ptr, uint16_t size);
uint8_t endp1_rx_callback(uint8_t* const ptr, uint16_t size)
{
	endp_tx_set_new_buffer(&usb_device_0, 1, endp1_rx_buffer, size); // loop-back
	LOG_IF_LEVEL(LOG_LEVEL_DEBUG, "Received something of size %d on endp1 \r\n",
				 size);
	return 0x00;
}

uint8_t endp2_rx_callback(uint8_t* const ptr, uint16_t size);
uint8_t endp2_rx_callback(uint8_t* const ptr, uint16_t size)
{
	endp_tx_set_new_buffer(&usb_device_0, 2, endp2_rx_buffer, size); // loop-back
	LOG_IF_LEVEL(LOG_LEVEL_DEBUG, "Received something of size %d on endp2 \r\n",
				 size);
	return 0x00;
}

uint8_t endp3_rx_callback(uint8_t* const ptr, uint16_t size);
uint8_t endp3_rx_callback(uint8_t* const ptr, uint16_t size)
{
	endp_tx_set_new_buffer(&usb_device_0, 3, endp3_rx_buffer, size); // loop-back
	LOG_IF_LEVEL(LOG_LEVEL_DEBUG, "Received something of size %d on endp3 \r\n",
				 size);
	return 0x00;
}

uint8_t endp4_rx_callback(uint8_t* const ptr, uint16_t size);
uint8_t endp4_rx_callback(uint8_t* const ptr, uint16_t size)
{
	endp_tx_set_new_buffer(&usb_device_0, 4, endp4_rx_buffer, size); // loop-back
	LOG_IF_LEVEL(LOG_LEVEL_DEBUG, "Received something of size %d on endp4 \r\n",
				 size);
	return 0x00;
}

uint8_t endp5_rx_callback(uint8_t* const ptr, uint16_t size);
uint8_t endp5_rx_callback(uint8_t* const ptr, uint16_t size)
{
	endp_tx_set_new_buffer(&usb_device_0, 5, endp5_rx_buffer, size); // loop-back
	LOG_IF_LEVEL(LOG_LEVEL_DEBUG, "Received something of size %d on endp5 \r\n",
				 size);
	return 0x00;
}

uint8_t endp6_rx_callback(uint8_t* const ptr, uint16_t size);
uint8_t endp6_rx_callback(uint8_t* const ptr, uint16_t size)
{
	endp_tx_set_new_buffer(&usb_device_0, 6, endp6_rx_buffer, size); // loop-back
	LOG_IF_LEVEL(LOG_LEVEL_DEBUG, "Received something of size %d on endp6 \r\n",
				 size);
	return 0x00;
}

uint8_t endp7_rx_callback(uint8_t* const ptr, uint16_t size);
uint8_t endp7_rx_callback(uint8_t* const ptr, uint16_t size)
{
	endp_tx_set_new_buffer(&usb_device_0, 7, endp7_rx_buffer, size); // loop-back
	LOG_IF_LEVEL(LOG_LEVEL_DEBUG, "Received something of size %d on endp7 \r\n",
				 size);
	return 0x00;
}

/* Blink time in ms */
#define BLINK_FAST (50) // Blink LED each 100ms (50*2)
#define BLINK_USB3 (250) // Blink LED each 500ms (250*2)
#define BLINK_USB2 (1000) // Blink LED each 2000ms (1000*2)
int blink_ms = BLINK_USB2;
USB_DEVICE_SPEED usb_device_old_speed = -1;

uint16_t endp0_user_handled_control_request(USB_SETUP* request,
											uint8_t** buffer);
uint16_t endp0_user_handled_control_request(USB_SETUP* request,
											uint8_t** buffer)
{
	return 0xffff;
}

void endp0_passthrough_setup_callback(uint8_t* ptr, uint16_t size);
void endp0_passthrough_setup_callback(uint8_t* ptr, uint16_t size) {}

void usb2_device_handle_bus_reset(void);
void usb2_device_handle_bus_reset(void)
{
	LOG_IF_LEVEL(LOG_LEVEL_DEBUG, "bus reset \r\n");
}
/*********************************************************************
 * @fn      main
 *
 * @brief   Main program.
 *
 * @return  none
 */
int main()
{
	// Initialize board
	bsp_gpio_init();
	bsp_init(FREQ_SYS);

	LOG_INIT(FREQ_SYS);

	usb_device_0.endpoints.endp0_user_handled_control_request = endp0_user_handled_control_request;
	usb_device_0.endpoints.endp0_passthrough_setup_callback = endp0_passthrough_setup_callback;
	usb_device_0.endpoints.tx_complete[1] = endp1_tx_complete;
	usb_device_0.endpoints.tx_complete[2] = endp2_tx_complete;
	usb_device_0.endpoints.tx_complete[3] = endp3_tx_complete;
	usb_device_0.endpoints.tx_complete[4] = endp4_tx_complete;
	usb_device_0.endpoints.tx_complete[5] = endp5_tx_complete;
	usb_device_0.endpoints.tx_complete[6] = endp6_tx_complete;
	usb_device_0.endpoints.tx_complete[7] = endp7_tx_complete;
	usb_device_0.endpoints.rx_callback[1] = endp1_rx_callback;
	usb_device_0.endpoints.rx_callback[2] = endp2_rx_callback;
	usb_device_0.endpoints.rx_callback[3] = endp3_rx_callback;
	usb_device_0.endpoints.rx_callback[4] = endp4_rx_callback;
	usb_device_0.endpoints.rx_callback[5] = endp5_rx_callback;
	usb_device_0.endpoints.rx_callback[6] = endp6_rx_callback;
	usb_device_0.endpoints.rx_callback[7] = endp7_rx_callback;

	usb2_user_handled.usb2_device_handle_bus_reset =
		&usb2_device_handle_bus_reset;

	// Finish initializing the descriptor parameters
	init_usb3_descriptors();
	init_usb2_descriptors();
	init_string_descriptors();

	// Set the USB device parameters
	usb_device_set_usb3_device_descriptor(&usb_device_0, &usb3_descriptors.usb_device_descr);
	usb_device_set_usb2_device_descriptor(&usb_device_0, &usb2_descriptors.usb_device_descr);
	usb_device_set_usb3_config_descriptors(&usb_device_0, usb3_device_configs);
	usb_device_set_usb2_config_descriptors(&usb_device_0, usb2_device_configs);
	usb_device_set_bos_descriptor(&usb_device_0, &usb3_descriptors.capabilities.usb_bos_descr);
	usb_device_set_string_descriptors(&usb_device_0, device_string_descriptors);
	usb_device_set_endpoint_mask(&usb_device_0, ENDPOINT_1_RX | ENDPOINT_1_TX | ENDPOINT_2_RX |
													ENDPOINT_2_TX | ENDPOINT_3_RX | ENDPOINT_3_TX |
													ENDPOINT_4_RX | ENDPOINT_4_TX | ENDPOINT_5_RX |
													ENDPOINT_5_TX | ENDPOINT_6_RX | ENDPOINT_6_TX |
													ENDPOINT_7_RX | ENDPOINT_7_TX);

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
						LOG_IF_LEVEL(LOG_LEVEL_DEBUG, "USB2\n");
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
						LOG_IF_LEVEL(LOG_LEVEL_DEBUG, "USB3\n");
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
