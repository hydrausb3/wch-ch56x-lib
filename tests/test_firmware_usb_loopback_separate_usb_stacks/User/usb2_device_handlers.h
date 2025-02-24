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

#include "definitions.h"
#include "usb2_device.h"
#include "wch-ch56x-lib/logging/logging.h"
#include "wch-ch56x-lib/usb/usb_device.h"

void usb2_board_endp1_tx_complete(TRANSACTION_STATUS status);
void usb2_board_endp1_tx_complete(TRANSACTION_STATUS status) {}
void usb2_board_endp2_tx_complete(TRANSACTION_STATUS status);
void usb2_board_endp2_tx_complete(TRANSACTION_STATUS status) {}
void usb2_board_endp3_tx_complete(TRANSACTION_STATUS status);
void usb2_board_endp3_tx_complete(TRANSACTION_STATUS status) {}
void usb2_board_endp4_tx_complete(TRANSACTION_STATUS status);
void usb2_board_endp4_tx_complete(TRANSACTION_STATUS status) {}
void usb2_board_endp5_tx_complete(TRANSACTION_STATUS status);
void usb2_board_endp5_tx_complete(TRANSACTION_STATUS status) {}
void usb2_board_endp6_tx_complete(TRANSACTION_STATUS status);
void usb2_board_endp6_tx_complete(TRANSACTION_STATUS status) {}

uint8_t usb2_board_endp1_rx_callback(uint8_t* const ptr, uint16_t size);
uint8_t usb2_board_endp1_rx_callback(uint8_t* const ptr, uint16_t size)
{
	endp_tx_set_new_buffer(&usb_device_0, 1, usb2_board_endp1_rx_buffer, size); // loop-back
	LOG_IF_LEVEL(LOG_LEVEL_DEBUG, "Received something of size %d on endp1 \r\n",
				 size);
	return 0x00;
}

uint8_t usb2_board_endp2_rx_callback(uint8_t* const ptr, uint16_t size);
uint8_t usb2_board_endp2_rx_callback(uint8_t* const ptr, uint16_t size)
{
	endp_tx_set_new_buffer(&usb_device_0, 2, usb2_board_endp2_rx_buffer, size); // loop-back
	LOG_IF_LEVEL(LOG_LEVEL_DEBUG, "Received something of size %d on endp2 \r\n",
				 size);
	return 0x00;
}

uint8_t usb2_board_endp3_rx_callback(uint8_t* const ptr, uint16_t size);
uint8_t usb2_board_endp3_rx_callback(uint8_t* const ptr, uint16_t size)
{
	endp_tx_set_new_buffer(&usb_device_0, 3, usb2_board_endp3_rx_buffer, size); // loop-back
	LOG_IF_LEVEL(LOG_LEVEL_DEBUG, "Received something of size %d on endp3 \r\n",
				 size);
	return 0x00;
}

uint8_t usb2_board_endp4_rx_callback(uint8_t* const ptr, uint16_t size);
uint8_t usb2_board_endp4_rx_callback(uint8_t* const ptr, uint16_t size)
{
	endp_tx_set_new_buffer(&usb_device_0, 4, usb2_board_endp4_rx_buffer, size); // loop-back
	LOG_IF_LEVEL(LOG_LEVEL_DEBUG, "Received something of size %d on endp4 \r\n",
				 size);
	return 0x00;
}

uint8_t usb2_board_endp5_rx_callback(uint8_t* const ptr, uint16_t size);
uint8_t usb2_board_endp5_rx_callback(uint8_t* const ptr, uint16_t size)
{
	endp_tx_set_new_buffer(&usb_device_0, 5, usb2_board_endp5_rx_buffer, size); // loop-back
	LOG_IF_LEVEL(LOG_LEVEL_DEBUG, "Received something of size %d on endp5 \r\n",
				 size);
	return 0x00;
}

uint8_t usb2_board_endp6_rx_callback(uint8_t* const ptr, uint16_t size);
uint8_t usb2_board_endp6_rx_callback(uint8_t* const ptr, uint16_t size)
{
	endp_tx_set_new_buffer(&usb_device_0, 6, usb2_board_endp6_rx_buffer, size); // loop-back
	LOG_IF_LEVEL(LOG_LEVEL_DEBUG, "Received something of size %d on endp6 \r\n",
				 size);
	return 0x00;
}
