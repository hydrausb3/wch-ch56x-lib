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

#ifndef USB3_DEVICE_USER_H
#define USB3_DEVICE_USER_H

#include "definitions.h"
#include "wch-ch56x-lib/usb/usb_descriptors.h"
#include "wch-ch56x-lib/usb/usb_device.h"
#include "wch-ch56x-lib/usb/usb_endpoints.h"

extern const USB_STRING_DESCR* usb3_board_string_descriptors_array[2];

extern __attribute__((aligned(16)))
uint8_t usb3_board_endp0_buffer[512]
	__attribute__((section(".DMADATA")));
extern __attribute__((aligned(16)))
uint8_t usb3_board_endp1_tx_buffer[ENDP_1_15_MAX_PACKET_SIZE * DEF_ENDP_IN_BURST_LEVEL]
	__attribute__((section(".DMADATA")));
extern __attribute__((aligned(16)))
uint8_t usb3_board_endp1_rx_buffer[ENDP_1_15_MAX_PACKET_SIZE * DEF_ENDP_OUT_BURST_LEVEL]
	__attribute__((section(".DMADATA")));

extern __attribute__((aligned(16)))
uint8_t usb3_board_endp2_tx_buffer[ENDP_1_15_MAX_PACKET_SIZE * DEF_ENDP_IN_BURST_LEVEL]
	__attribute__((section(".DMADATA")));
extern __attribute__((aligned(16)))
uint8_t usb3_board_endp2_rx_buffer[ENDP_1_15_MAX_PACKET_SIZE * DEF_ENDP_OUT_BURST_LEVEL]
	__attribute__((section(".DMADATA")));

extern __attribute__((aligned(16)))
uint8_t usb3_board_endp3_tx_buffer[ENDP_1_15_MAX_PACKET_SIZE * DEF_ENDP_IN_BURST_LEVEL]
	__attribute__((section(".DMADATA")));
extern __attribute__((aligned(16)))
uint8_t usb3_board_endp3_rx_buffer[ENDP_1_15_MAX_PACKET_SIZE * DEF_ENDP_OUT_BURST_LEVEL]
	__attribute__((section(".DMADATA")));

extern __attribute__((aligned(16)))
uint8_t usb3_board_endp4_tx_buffer[ENDP_1_15_MAX_PACKET_SIZE * DEF_ENDP_IN_BURST_LEVEL]
	__attribute__((section(".DMADATA")));
extern __attribute__((aligned(16)))
uint8_t usb3_board_endp4_rx_buffer[ENDP_1_15_MAX_PACKET_SIZE * DEF_ENDP_OUT_BURST_LEVEL]
	__attribute__((section(".DMADATA")));

void usb3_board_init_string_descriptors(void);

void usb3_board_init_endpoints(void);
#endif
