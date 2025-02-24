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

#include "wch-ch56x-lib/HSPIDevice/hspi.h"
#include "wch-ch56x-lib/logging/logging.h"
#include "wch-ch56x-lib/usb/usb20.h"
#include "wch-ch56x-lib/usb/usb30.h"
#include "wch-ch56x-lib/usb/usb_descriptors.h"
#include "wch-ch56x-lib/usb/usb_device.h"
#include "wch-ch56x-lib/usb/usb_endpoints.h"

#undef FREQ_SYS
/* System clock / MCU frequency in Hz (lowest possible speed 15MHz) */
#define FREQ_SYS (120000000)

#define ENDP_1_15_MAX_PACKET_SIZE 512

__attribute__((aligned(16))) uint8_t endp0_buffer[ENDP_1_15_MAX_PACKET_SIZE]
	__attribute__((section(".DMADATA")));
__attribute__((aligned(16))) uint8_t endp1_tx_buffer[ENDP_1_15_MAX_PACKET_SIZE]
	__attribute__((section(".DMADATA")));

struct usb_descriptors
{
	USB_DEV_DESCR usb_device_descr;
	struct __PACKED
	{
		USB_CFG_DESCR usb_cfg_descr;
		USB_ITF_DESCR usb_itf_descr;
		USB_ENDP_DESCR usb_endp_descr_1_tx;
	} other_descr;
} usb_descriptors;

void init_usb_descriptors(void);

void init_usb_descriptors(void)
{
	usb_descriptors.usb_device_descr = (USB_DEV_DESCR){
		.bLength = 0x12,
		.bDescriptorType = 0x01, // device descriptor type
		.bcdUSB = 0x0200, // usb2.0
		.bDeviceClass = 0x00,
		.bDeviceSubClass = 0x00,
		.bDeviceProtocol = 0x00,
		.bMaxPacketSize0 = 64,
		.bcdDevice = 0x0001,
		.idVendor =
			0x16c0, // https://github.com/obdev/v-usb/blob/master/usbdrv/usb-ids-for-free.txt
		.idProduct = 0x27d8,
		.iProduct = 0x00,
		.iManufacturer = 0x00,
		.iSerialNumber = 0x00,
		.bNumConfigurations = 0x01
	};

	usb_descriptors.other_descr.usb_cfg_descr = (USB_CFG_DESCR){
		.bLength = 0x09,
		.bDescriptorType = 0x02,
		.wTotalLength = sizeof(usb_descriptors.other_descr),
		.bNumInterfaces = 0x01,
		.bConfigurationValue = 0x01,
		.iConfiguration = 0x00,
		.bmAttributes = 0xa0, // supports remote wake-up
		.MaxPower = 0x64 // 200ma
	};

	usb_descriptors.other_descr.usb_itf_descr =
		(USB_ITF_DESCR){ .bLength = 0x09,
						 .bDescriptorType = 0x04,
						 .bInterfaceNumber = 0x00,
						 .bAlternateSetting = 0x00,
						 .bNumEndpoints = 0x01,
						 .bInterfaceClass = 0xff, // vendor-specific
						 .bInterfaceSubClass = 0xff,
						 .bInterfaceProtocol = 0xff,
						 .iInterface = 0x00 };

	usb_descriptors.other_descr.usb_endp_descr_1_tx = (USB_ENDP_DESCR){
		.bLength = 0x07,
		.bDescriptorType = 0x05,
		.bEndpointAddress = (ENDPOINT_DESCRIPTOR_ADDRESS_IN | 0x01) &
							ENDPOINT_DESCRIPTOR_ADDRESS_MASK,
		.bmAttributes = ENDPOINT_DESCRIPTOR_BULK_TRANSFER,
		.wMaxPacketSizeL = (sizeof(endp1_tx_buffer) & 0x00ff),
		.wMaxPacketSizeH = (sizeof(endp1_tx_buffer) & 0xff00) >> 8, // 512 bytes
		.bInterval = 0
	};
}

const uint8_t* usb_device_configs[1];

void endp1_tx_complete(TRANSACTION_STATUS status);
void endp1_tx_complete(TRANSACTION_STATUS status) {}

void hspi_rx_callback(uint8_t* buffer, uint16_t size, uint16_t custom_register);
void hspi_rx_callback(uint8_t* buffer, uint16_t size,
					  uint16_t custom_register)
{
	endp_tx_set_new_buffer(&usb_device_0, 1, buffer, size);
}

void init_endpoints(void);
void init_endpoints(void)
{
	usb_device_0.endpoints.rx[0].buffer = endp0_buffer;
	usb_device_0.endpoints.rx[0].max_packet_size = 512;
	usb_device_0.endpoints.rx[0].max_burst = 1;
	usb_device_0.endpoints.rx[0].max_packet_size_with_burst = sizeof(endp0_buffer);
	usb_device_0.endpoints.rx[0].state = ENDP_STATE_ACK;

	usb_device_0.endpoints.tx[1].buffer = NULL;
	usb_device_0.endpoints.tx[1].max_packet_size = ENDP_1_15_MAX_PACKET_SIZE;
	usb_device_0.endpoints.tx[1].max_burst = 0;
	usb_device_0.endpoints.tx[1].max_packet_size_with_burst = sizeof(endp1_tx_buffer);
	usb_device_0.endpoints.tx[1].state = ENDP_STATE_NAK;
}

bool_t is_board1; /* Return true or false */

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
	hspi_user_handled.hspi_rx_callback = hspi_rx_callback;

	if (is_board1)
	{
		// Finish initializing the descriptor parameters
		init_usb_descriptors();
		usb_device_configs[0] = (uint8_t*)&usb_descriptors.other_descr;

		// Set the USB device parameters
		usb_device_set_usb2_device_descriptor(&usb_device_0, &usb_descriptors.usb_device_descr);
		usb_device_set_usb2_config_descriptors(&usb_device_0, usb_device_configs);
		usb_device_set_endpoint_mask(&usb_device_0, ENDPOINT_1_TX);

		init_endpoints();

		// Initialize USB device, force to USB2.0 for now.
		usb2_device_init();

		hspi_init(HSPI_TYPE_DEVICE, HSPI_DATASIZE_32, sizeof(endp1_tx_buffer));
	}
	else
	{
		hspi_init(HSPI_TYPE_HOST, HSPI_DATASIZE_32, sizeof(endp1_tx_buffer));

		for (size_t i = 0; i < sizeof(endp1_tx_buffer); ++i)
		{
			endp1_tx_buffer[i] = i;
		}

		while (1)
		{
			if (bsp_ubtn())
			{
				hspi_send(endp1_tx_buffer, sizeof(endp1_tx_buffer), 0);
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
