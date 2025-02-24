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
#include "usb_device.h"
#include "wch-ch56x-lib/logging/logging.h"
#include "wch-ch56x-lib/usb/usb20.h"
#include "wch-ch56x-lib/usb/usb30.h"
#include "wch-ch56x-lib/usb/usb_descriptors.h"
#include "wch-ch56x-lib/usb/usb_endpoints.h"

#undef FREQ_SYS
/* System clock / MCU frequency in Hz (lowest possible speed 15MHz) */
#define FREQ_SYS (120000000)

void endp_tx_complete(TRANSACTION_STATUS status);
void endp_tx_complete(TRANSACTION_STATUS status) {}

uint8_t endp1_rx_callback(uint8_t* const ptr, uint16_t size);
uint8_t endp1_rx_callback(uint8_t* const ptr, uint16_t size)
{
	bsp_wait_us_delay(1000);
	endp_tx_set_new_buffer(&usb_device_0, 1, usb_device_0.endpoints.rx[1].buffer, size); // loop-back
	LOG_IF_LEVEL(LOG_LEVEL_DEBUG, "Received something of size %d on endp1 \r\n",
				 size);
	return ENDP_STATE_NAK;
}

uint16_t endp0_user_handled_control_request(USB_SETUP* request,
											uint8_t** buffer);
uint16_t endp0_user_handled_control_request(USB_SETUP* request,
											uint8_t** buffer)
{
	return 0xffff;
}

uint8_t endp0_passthrough_setup_callback(uint8_t* ptr, uint16_t size);
uint8_t endp0_passthrough_setup_callback(uint8_t* ptr, uint16_t size) { return ENDP_STATE_ACK; }

void usb2_device_handle_bus_reset(void);
void usb2_device_handle_bus_reset(void)
{
	LOG_IF_LEVEL(LOG_LEVEL_DEBUG, "bus reset \r\n");
}

#define TMR_CLOCK_CYCLES 100000
#define TMR_US 1000

__attribute__((interrupt("WCH-Interrupt-fast"))) void TMR0_IRQHandler(void);

/*********************************************************************
 * @fn      main
 *
 * @brief   Main program.
 *
 * @return  none
 */
int main()
{
	PFIC_SetPriority(INT_ID_TMR0, 0xa0);
	PFIC_SetPriority(INT_ID_LINK, 0x90);
	PFIC_SetPriority(INT_ID_USBHS, 0xc0);
	PFIC_HaltPushCfg(ENABLE);
	PFIC_INTNestCfg(DISABLE);
	PFIC_SetFastIRQ((uint32_t)LINK_IRQHandler, INT_ID_LINK, 0);
	PFIC_SetFastIRQ((uint32_t)TMR0_IRQHandler, INT_ID_TMR0, 1);
	PFIC_SetFastIRQ((uint32_t)USBHS_IRQHandler, INT_ID_USBHS, 2);

	// Initialize board
	bsp_gpio_init();
	bsp_init(FREQ_SYS);

	LOG_INIT(FREQ_SYS);

	TMR0_TimerInit(bsp_get_nbtick_1us() * TMR_US);
	R8_TMR0_INTER_EN = RB_TMR_IE_CYC_END;
	PFIC_EnableIRQ(TMR0_IRQn);

	usb_device_0.endpoints.endp0_user_handled_control_request = endp0_user_handled_control_request;
	usb_device_0.endpoints.endp0_passthrough_setup_callback = endp0_passthrough_setup_callback;
	usb_device_0.endpoints.tx_complete[1] = endp_tx_complete;
	usb_device_0.endpoints.rx_callback[1] = endp1_rx_callback;

	usb2_user_handled.usb2_device_handle_bus_reset =
		&usb2_device_handle_bus_reset;

	// Finish initializing the descriptor parameters

	init_string_descriptors();

	// Set the USB device parameters

	usb_device_set_string_descriptors(&usb_device_0, device_string_descriptors);

	init_endpoints();

	// // uncomment to test other supported endpoint numbers
	// // NOTE : some endpoints numbers are incompatible and can't be used at the same time
	// init_usb2_descriptors_alt_endpoints();
	// usb_device_set_usb2_device_descriptor(&usb_device_0, &usb2_descriptors_alt_endpoints.usb_device_descr);
	// usb_device_set_usb2_config_descriptors(&usb_device_0, usb2_device_configs_alt_endpoints);
	// usb_device_set_endpoint_mask(&usb_device_0, ENDPOINT_9_RX | ENDPOINT_9_TX | ENDPOINT_10_RX |
	// 												ENDPOINT_10_TX | ENDPOINT_11_RX | ENDPOINT_11_TX |
	// 												ENDPOINT_12_RX | ENDPOINT_12_TX | ENDPOINT_13_RX |
	// 												ENDPOINT_13_TX | ENDPOINT_14_RX | ENDPOINT_14_TX |
	// 												ENDPOINT_15_RX | ENDPOINT_15_TX);
	init_usb2_fs_descriptors();
	usb_device_set_usb2_device_descriptor(&usb_device_0, &usb2_fs_descriptors.usb_device_descr);
	usb_device_set_usb2_config_descriptors(&usb_device_0, usb2_fs_device_configs);
	usb_device_set_endpoint_mask(&usb_device_0, ENDPOINT_1_RX | ENDPOINT_1_TX);
	usb_device_0.speed = USB2_FULLSPEED;
	usb2_device_init();
}

/*******************************************************************************
 * @fn     TMR0_IRQHandler
 * @return None
 */
__attribute__((interrupt("WCH-Interrupt-fast"))) void TMR0_IRQHandler(void)
{
	R8_TMR0_INT_FLAG = RB_TMR_IF_CYC_END;
	LOG("clock \r\n");
	endp_rx_set_state(&usb_device_0, 1, ENDP_STATE_ACK);
	return;
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
