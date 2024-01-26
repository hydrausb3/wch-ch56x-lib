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

#include "usb2_device.h"

uint8_t test_firmware_loopback_usb2_board[] = {
	'H',
	0x00,
	'y',
	0x00,
	'd',
	0x00,
	'r',
	0x00,
	'a',
	0x00,
	'd',
	0x00,
	'a',
	0x00,
	'n',
	0x00,
	'c',
	0x00,
	'e',
	0x00,
	'r',
	0x00,
	' ',
	0x00,
	'T',
	0x00,
	'e',
	0x00,
	's',
	0x00,
	't',
	0x00,
	' ',
	0x00,
	'L',
	0x00,
	'o',
	0x00,
	'o',
	0x00,
	'p',
	0x00,
	'b',
	0x00,
	'a',
	0x00,
	'c',
	0x00,
	'k',
	0x00,
	' ',
	0x00,
	'U',
	0x00,
	'S',
	0x00,
	'B',
	0x00,
	'2',
	0x00,
	' ',
	0x00,
	'B',
	0x00,
	'o',
	0x00,
	'a',
	0x00,
	'r',
	0x00,
	'd',
	0x00
};

struct usb2_board_string_descriptors
{
	USB_STRING_DESCR lang_ids_descriptor;
	uint16_t lang_ids[1];
	USB_STRING_DESCR product_string_descriptor;
	uint8_t hydradancer_product_string_descriptor[sizeof(
		test_firmware_loopback_usb2_board)];
} usb2_board_string_descriptors;

const USB_STRING_DESCR* usb2_board_string_descriptors_array[2];

__attribute__((aligned(16)))
uint8_t usb2_board_endp0_buffer[512]
	__attribute__((section(".DMADATA")));
__attribute__((aligned(16)))
uint8_t usb2_board_endp1_tx_buffer[ENDP_1_15_MAX_PACKET_SIZE * DEF_ENDP_IN_BURST_LEVEL]
	__attribute__((section(".DMADATA")));
__attribute__((aligned(16)))
uint8_t usb2_board_endp1_rx_buffer[ENDP_1_15_MAX_PACKET_SIZE * DEF_ENDP_OUT_BURST_LEVEL]
	__attribute__((section(".DMADATA")));

__attribute__((aligned(16)))
uint8_t usb2_board_endp2_tx_buffer[ENDP_1_15_MAX_PACKET_SIZE * DEF_ENDP_IN_BURST_LEVEL]
	__attribute__((section(".DMADATA")));
__attribute__((aligned(16)))
uint8_t usb2_board_endp2_rx_buffer[ENDP_1_15_MAX_PACKET_SIZE * DEF_ENDP_OUT_BURST_LEVEL]
	__attribute__((section(".DMADATA")));

__attribute__((aligned(16)))
uint8_t usb2_board_endp3_tx_buffer[ENDP_1_15_MAX_PACKET_SIZE * DEF_ENDP_IN_BURST_LEVEL]
	__attribute__((section(".DMADATA")));
__attribute__((aligned(16)))
uint8_t usb2_board_endp3_rx_buffer[ENDP_1_15_MAX_PACKET_SIZE * DEF_ENDP_OUT_BURST_LEVEL]
	__attribute__((section(".DMADATA")));

__attribute__((aligned(16)))
uint8_t usb2_board_endp4_tx_buffer[ENDP_1_15_MAX_PACKET_SIZE * DEF_ENDP_IN_BURST_LEVEL]
	__attribute__((section(".DMADATA")));
__attribute__((aligned(16)))
uint8_t usb2_board_endp4_rx_buffer[ENDP_1_15_MAX_PACKET_SIZE * DEF_ENDP_OUT_BURST_LEVEL]
	__attribute__((section(".DMADATA")));

__attribute__((aligned(16)))
uint8_t usb2_board_endp5_tx_buffer[ENDP_1_15_MAX_PACKET_SIZE * DEF_ENDP_IN_BURST_LEVEL]
	__attribute__((section(".DMADATA")));
__attribute__((aligned(16)))
uint8_t usb2_board_endp5_rx_buffer[ENDP_1_15_MAX_PACKET_SIZE * DEF_ENDP_OUT_BURST_LEVEL]
	__attribute__((section(".DMADATA")));

__attribute__((aligned(16)))
uint8_t usb2_board_endp6_tx_buffer[ENDP_1_15_MAX_PACKET_SIZE * DEF_ENDP_IN_BURST_LEVEL]
	__attribute__((section(".DMADATA")));
__attribute__((aligned(16)))
uint8_t usb2_board_endp6_rx_buffer[ENDP_1_15_MAX_PACKET_SIZE * DEF_ENDP_OUT_BURST_LEVEL]
	__attribute__((section(".DMADATA")));

void usb2_board_init_string_descriptors(void)
{
	usb2_board_string_descriptors.lang_ids_descriptor = (USB_STRING_DESCR){
		.bLength =
			sizeof(USB_STRING_DESCR) + sizeof(usb2_board_string_descriptors.lang_ids),
		.bDescriptorType = 0x03, // String Descriptor
	};

	usb2_board_string_descriptors.lang_ids[0] = 0x0409;

	usb2_board_string_descriptors.product_string_descriptor = (USB_STRING_DESCR){
		.bLength = sizeof(USB_STRING_DESCR) +
				   sizeof(test_firmware_loopback_usb2_board),
		.bDescriptorType = 0x03, // String Descriptor
	};

	memcpy(&usb2_board_string_descriptors.hydradancer_product_string_descriptor,
		   test_firmware_loopback_usb2_board,
		   sizeof(test_firmware_loopback_usb2_board));

	usb2_board_string_descriptors_array[0] = &usb2_board_string_descriptors.lang_ids_descriptor;
	usb2_board_string_descriptors_array[1] =
		&usb2_board_string_descriptors.product_string_descriptor;
}

void usb2_board_init_endpoints(void)
{
	usb_device_0.endpoints.rx[0].buffer = usb2_board_endp0_buffer;
	usb_device_0.endpoints.rx[0].max_packet_size = 512;
	usb_device_0.endpoints.rx[0].max_burst = 1;
	usb_device_0.endpoints.rx[0].max_packet_size_with_burst = sizeof(usb2_board_endp0_buffer);

	usb_device_0.endpoints.rx[1].buffer = usb2_board_endp1_rx_buffer;
	usb_device_0.endpoints.rx[1].max_packet_size = USB2_ENDP_1_15_MAX_PACKET_SIZE;
	usb_device_0.endpoints.rx[1].max_burst = 1;
	usb_device_0.endpoints.rx[1].max_packet_size_with_burst = USB2_ENDP_1_15_MAX_PACKET_SIZE;

	usb_device_0.endpoints.tx[1].buffer = NULL;
	usb_device_0.endpoints.tx[1].max_packet_size = USB2_ENDP_1_15_MAX_PACKET_SIZE;
	usb_device_0.endpoints.tx[1].max_burst = 1;
	usb_device_0.endpoints.tx[1].max_packet_size_with_burst = USB2_ENDP_1_15_MAX_PACKET_SIZE;

	usb_device_0.endpoints.rx[2].buffer = usb2_board_endp2_rx_buffer;
	usb_device_0.endpoints.rx[2].max_packet_size = USB2_ENDP_1_15_MAX_PACKET_SIZE;
	usb_device_0.endpoints.rx[2].max_burst = 1;
	usb_device_0.endpoints.rx[2].max_packet_size_with_burst = USB2_ENDP_1_15_MAX_PACKET_SIZE;

	usb_device_0.endpoints.tx[2].buffer = NULL;
	usb_device_0.endpoints.tx[2].max_packet_size = USB2_ENDP_1_15_MAX_PACKET_SIZE;
	usb_device_0.endpoints.tx[2].max_burst = 1;
	usb_device_0.endpoints.tx[2].max_packet_size_with_burst = USB2_ENDP_1_15_MAX_PACKET_SIZE;

	usb_device_0.endpoints.rx[3].buffer = usb2_board_endp3_rx_buffer;
	usb_device_0.endpoints.rx[3].max_packet_size = USB2_ENDP_1_15_MAX_PACKET_SIZE;
	usb_device_0.endpoints.rx[3].max_burst = 1;
	usb_device_0.endpoints.rx[3].max_packet_size_with_burst = USB2_ENDP_1_15_MAX_PACKET_SIZE;

	usb_device_0.endpoints.tx[3].buffer = NULL;
	usb_device_0.endpoints.tx[3].max_packet_size = USB2_ENDP_1_15_MAX_PACKET_SIZE;
	usb_device_0.endpoints.tx[3].max_burst = 1;
	usb_device_0.endpoints.tx[3].max_packet_size_with_burst = USB2_ENDP_1_15_MAX_PACKET_SIZE;

	usb_device_0.endpoints.rx[4].buffer = usb2_board_endp4_rx_buffer;
	usb_device_0.endpoints.rx[4].max_packet_size = USB2_ENDP_1_15_MAX_PACKET_SIZE;
	usb_device_0.endpoints.rx[4].max_burst = 1;
	usb_device_0.endpoints.rx[4].max_packet_size_with_burst = USB2_ENDP_1_15_MAX_PACKET_SIZE;

	usb_device_0.endpoints.tx[4].buffer = NULL;
	usb_device_0.endpoints.tx[4].max_packet_size = USB2_ENDP_1_15_MAX_PACKET_SIZE;
	usb_device_0.endpoints.tx[4].max_burst = 1;
	usb_device_0.endpoints.tx[4].max_packet_size_with_burst = USB2_ENDP_1_15_MAX_PACKET_SIZE;

	usb_device_0.endpoints.rx[5].buffer = usb2_board_endp5_rx_buffer;
	usb_device_0.endpoints.rx[5].max_packet_size = USB2_ENDP_1_15_MAX_PACKET_SIZE;
	usb_device_0.endpoints.rx[5].max_burst = 1;
	usb_device_0.endpoints.rx[5].max_packet_size_with_burst = USB2_ENDP_1_15_MAX_PACKET_SIZE;

	usb_device_0.endpoints.tx[5].buffer = NULL;
	usb_device_0.endpoints.tx[5].max_packet_size = USB2_ENDP_1_15_MAX_PACKET_SIZE;
	usb_device_0.endpoints.tx[5].max_burst = 1;
	usb_device_0.endpoints.tx[5].max_packet_size_with_burst = USB2_ENDP_1_15_MAX_PACKET_SIZE;

	usb_device_0.endpoints.rx[6].buffer = usb2_board_endp6_rx_buffer;
	usb_device_0.endpoints.rx[6].max_packet_size = USB2_ENDP_1_15_MAX_PACKET_SIZE;
	usb_device_0.endpoints.rx[6].max_burst = 1;
	usb_device_0.endpoints.rx[6].max_packet_size_with_burst = USB2_ENDP_1_15_MAX_PACKET_SIZE;

	usb_device_0.endpoints.tx[6].buffer = NULL;
	usb_device_0.endpoints.tx[6].max_packet_size = USB2_ENDP_1_15_MAX_PACKET_SIZE;
	usb_device_0.endpoints.tx[6].max_burst = 1;
	usb_device_0.endpoints.tx[6].max_packet_size_with_burst = USB2_ENDP_1_15_MAX_PACKET_SIZE;
}
