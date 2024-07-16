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

#include "usb2_fs_device_descriptors.h"
#include "usb2_hs_device_descriptors.h"
#include "usb2_ls_device_descriptors.h"
#include "usb_device.h"
#include "wch-ch56x-lib/logging/logging.h"
#include "wch-ch56x-lib/USBDevice/usb20.h"
#include "wch-ch56x-lib/USBDevice/usb_descriptors.h"
#include "wch-ch56x-lib/USBDevice/usb_endpoints.h"

#undef FREQ_SYS
/* System clock / MCU frequency in Hz (lowest possible speed 15MHz) */
#define FREQ_SYS (120000000)

#define MIN(a, b) (a <= b ? a : b)

#define EP_IN 2

volatile bool nak_received = false;
volatile uint16_t in_transfer_length = 32;
volatile uint16_t in_transfer_total_sent = 0;
volatile uint16_t in_transfer_last_sent = 0;
volatile uint16_t out_total_sent = 0;
volatile uint16_t last_out_total_sent = 0;

uint8_t last_out_buffer[ENDP_1_15_MAX_PACKET_SIZE * DEF_ENDP_OUT_BURST_LEVEL]
	__attribute__((section(".DMADATA")));
uint8_t temp_in_buffer[ENDP_1_15_MAX_PACKET_SIZE * DEF_ENDP_OUT_BURST_LEVEL]
	__attribute__((section(".DMADATA")));

void generate_data(uint8_t* buffer, uint16_t length);
void generate_data(uint8_t* buffer, uint16_t length)
{
	for (uint16_t i = 0; i < length; ++i)
	{
		buffer[i] = i % 256;
	}
}

void endp2_tx_complete(TRANSACTION_STATUS status);
void endp2_tx_complete(TRANSACTION_STATUS status)
{
	nak_received = false;
	in_transfer_total_sent += in_transfer_last_sent;
}

uint8_t endp1_rx_callback(uint8_t* const ptr, uint16_t size);
uint8_t endp1_rx_callback(uint8_t* const ptr, uint16_t size)
{
	memcpy(last_out_buffer + out_total_sent, ptr, size);
	out_total_sent += size;
	last_out_total_sent = out_total_sent; //because no ZLP is sent for bulk OUT ... meaning we can't detect the end of the transfer
	if (size < usb_device_0.endpoints.rx[1].max_packet_size || size == 0)
	{
		out_total_sent = 0;
	}
	return 0x00;
}

void nak_callback(uint8_t endp_num);
void nak_callback(uint8_t endp_num)
{
	if (!nak_received && endp_num == EP_IN)
	{
		LOG("nak received \r\n");
		if (in_transfer_total_sent == in_transfer_length)
		{
			in_transfer_total_sent = 0;
		}
		nak_received = true;
		in_transfer_last_sent = MIN(in_transfer_length - in_transfer_total_sent, usb_device_0.endpoints.tx[EP_IN].max_packet_size_with_burst);
		memcpy(endp2_tx_buffer, temp_in_buffer + in_transfer_total_sent, in_transfer_last_sent);
		endp_tx_set_new_buffer(&usb_device_0, EP_IN, endp2_tx_buffer, in_transfer_last_sent);
		LOG("end nak received");
	}
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
	if (request->bRequest == 10)
	{
		LOG("writing %x %x %x %x \r\n", endp0_buffer[0], endp0_buffer[1], endp0_buffer[2], endp0_buffer[3]);
		memcpy(last_out_buffer, endp0_buffer, request->wLength);
		last_out_total_sent = request->wLength;
		return 0x00;
	}
	else if (request->bRequest == 20)
	{
		in_transfer_length = request->wValue.bw.bb1 | (request->wIndex.bw.bb1 << 8);
		LOG("requested %d bytes \r\n", in_transfer_length);
		generate_data(usb2_backend_current_device->endpoints.rx[0].buffer, in_transfer_length);
		*buffer = usb2_backend_current_device->endpoints.rx[0].buffer;
		return in_transfer_length;
	}
	else if (request->bRequest == 1)
	{
		in_transfer_length = request->wValue.bw.bb1 | (request->wIndex.bw.bb1 << 8);
		LOG("in transfer length %d \r\n", in_transfer_length);
		generate_data(temp_in_buffer, in_transfer_length);
		in_transfer_total_sent = 0;
		in_transfer_last_sent = 0;
		return 0x00;
	}
	else if (request->bRequest == 2)
	{
		*buffer = last_out_buffer;
		LOG("returning last out buffer %d\r\n", last_out_total_sent);
		return last_out_total_sent;
	}
	else if (request->bRequest == 3)
	{
		LOG("reset out buffer head\r\n");
		out_total_sent = 0;
		return 0x00;
	}
	return 0xffff;
}

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
	LOG("USB stress test\r\n");

	usb_device_0.endpoints.endp0_user_handled_control_request = endp0_user_handled_control_request;
	usb_device_0.endpoints.tx_complete[2] = endp2_tx_complete;
	usb_device_0.endpoints.rx_callback[1] = endp1_rx_callback;
	usb_device_0.endpoints.nak_callback = nak_callback;

	usb2_user_handled.usb2_device_handle_bus_reset =
		&usb2_device_handle_bus_reset;

	if (bsp_ubtn())
	{
		usb_device_0.speed = USB2_FULLSPEED;
	}
	else
	{
		usb_device_0.speed = USB2_HIGHSPEED;
	}

	if (usb_device_0.speed == USB2_LOWSPEED)
	{
		// Finish initializing the descriptor parameters
		init_usb2_ls_descriptors();
		init_string_descriptors();

		// Set the USB device parameters
		usb_device_set_usb2_device_descriptor(&usb_device_0, &usb2_ls_descriptors.usb_device_descr);
		usb_device_set_usb2_config_descriptors(&usb_device_0, usb2_ls_device_configs);
		usb_device_set_string_descriptors(&usb_device_0, device_string_descriptors);
		usb_device_set_endpoint_mask(&usb_device_0, ENDPOINT_1_RX | ENDPOINT_2_TX);

		init_endpoints_ls();

		usb2_device_init();
		usb2_enable_nak(true);
	}
	else if (usb_device_0.speed == USB2_FULLSPEED)
	{
		// Finish initializing the descriptor parameters
		init_usb2_fs_descriptors();
		init_string_descriptors();

		// Set the USB device parameters
		usb_device_set_usb2_device_descriptor(&usb_device_0, &usb2_fs_descriptors.usb_device_descr);
		usb_device_set_usb2_config_descriptors(&usb_device_0, usb2_fs_device_configs);
		usb_device_set_string_descriptors(&usb_device_0, device_string_descriptors);
		usb_device_set_endpoint_mask(&usb_device_0, ENDPOINT_1_RX | ENDPOINT_2_TX);

		init_endpoints_fs();

		usb2_device_init();
		usb2_enable_nak(true);
	}
	else if (usb_device_0.speed == USB2_HIGHSPEED)
	{
		// Finish initializing the descriptor parameters
		init_usb2_hs_descriptors();
		init_string_descriptors();

		// Set the USB device parameters
		usb_device_set_usb2_device_descriptor(&usb_device_0, &usb2_hs_descriptors.usb_device_descr);
		usb_device_set_usb2_config_descriptors(&usb_device_0, usb2_hs_device_configs);
		usb_device_set_string_descriptors(&usb_device_0, device_string_descriptors);
		usb_device_set_endpoint_mask(&usb_device_0, ENDPOINT_1_RX | ENDPOINT_2_TX);

		init_endpoints_hs();

		usb2_device_init();
		usb2_enable_nak(true);
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
				 " MTVEC=0x%08X\r\n"
				 " MTVAL=0x%08X\r\n",
				 __get_SP(), __get_MIE(), __get_MSTATUS(), __get_MCAUSE(),
				 __get_MVENDORID(), __get_MARCHID(), __get_MISA(), __get_MIMPID(),
				 __get_MHARTID(), __get_MEPC(), __get_MSCRATCH(), __get_MTVEC(), __get_MTVAL());

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
	LOG("this might be the return addr of current function %x \r\n", __builtin_return_address(0));
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
				 " MTVEC=0x%08X\r\n"
				 " MTVAL=0x%08X\r\n",
				 __get_SP(), __get_MIE(), __get_MSTATUS(), __get_MCAUSE(),
				 __get_MVENDORID(), __get_MARCHID(), __get_MISA(), __get_MIMPID(),
				 __get_MHARTID(), __get_MEPC(), __get_MSCRATCH(), __get_MTVEC(), __get_MTVAL());

	LOG_DUMP();

	bsp_wait_ms_delay(100000000);
}
