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
#include "usb_device.h"
#include "wch-ch56x-lib/logging/logging.h"
#include "wch-ch56x-lib/SerDesDevice/serdes.h"
#include "wch-ch56x-lib/USBDevice/usb20.h"
#include "wch-ch56x-lib/USBDevice/usb_descriptors.h"
#include "wch-ch56x-lib/USBDevice/usb_endpoints.h"

#undef FREQ_SYS
/* System clock / MCU frequency in Hz (lowest possible speed 15MHz) */
#define FREQ_SYS (120000000)

bool_t is_board1;

__attribute__((aligned(16)))
uint8_t buf_0[ENDP_1_15_MAX_PACKET_SIZE * DEF_ENDP_IN_BURST_LEVEL]
	__attribute__((section(".DMADATA")));
__attribute__((aligned(16))) uint8_t ep_in_status
	__attribute__((section(".DMADATA")));

void endp1_tx_complete(TRANSACTION_STATUS status);
void endp1_tx_complete(TRANSACTION_STATUS status) {}

void serdes_rx_callback(uint8_t* buffer, uint16_t size,
						uint16_t custom_register);
void serdes_rx_callback(uint8_t* buffer, uint16_t size,
						uint16_t custom_register)
{
	endp_tx_set_new_buffer(&usb_device_0, 1, buffer, size);
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

	usb_device_0.endpoints.tx_complete[1] = endp1_tx_complete;
	serdes_user_handled.serdes_rx_callback = serdes_rx_callback;

	// Finish initializing the descriptor parameters
	init_usb2_descriptors();
	init_string_descriptors();

	// Set the USB device parameters
	usb_device_set_usb2_device_descriptor(&usb_device_0, &usb2_descriptors.usb_device_descr);
	usb_device_set_usb2_config_descriptors(&usb_device_0, usb2_device_configs);
	usb_device_set_string_descriptors(&usb_device_0, device_string_descriptors);
	usb_device_set_endpoint_mask(&usb_device_0, ENDPOINT_1_RX | ENDPOINT_1_TX | ENDPOINT_2_RX |
													ENDPOINT_2_TX);

	init_endpoints();

	usb2_device_init();

	serdes_init(SERDES_TYPE_DEVICE, 4096);

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
