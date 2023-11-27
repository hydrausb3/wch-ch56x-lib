#ifndef USB_DEVICE_USER_H
#define USB_DEVICE_USER_H

#include "definitions.h"
#include "wch-ch56x-lib/USBDevice/usb_descriptors.h"
#include "wch-ch56x-lib/USBDevice/usb_endpoints.h"

uint8_t hydradancer_product_string_descriptor[] = {
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
	'F',
	0x00,
	'i',
	0x00,
	'r',
	0x00,
	'm',
	0x00,
	'w',
	0x00,
	'a',
	0x00,
	'r',
	0x00,
	'e',
	0x00,
	',',
	0x00,
	' ',
	0x00,
	'b',
	0x00,
	'i',
	0x00,
	'd',
	0x00,
	'i',
	0x00,
	'r',
	0x00,
	'e',
	0x00,
	'c',
	0x00,
	't',
	0x00,
	'i',
	0x00,
	'o',
	0x00,
	'n',
	0x00,
	'n',
	0x00,
	'a',
	0x00,
	'l',
	0x00,
	',',
	0x00,
	' ',
	0x00,
	'a',
	0x00,
	'l',
	0x00,
	'l',
	0x00,
	' ',
	0x00,
	'e',
	0x00,
	'n',
	0x00,
	'd',
	0x00,
	'p',
	0x00,
	'o',
	0x00,
	'i',
	0x00,
	'n',
	0x00,
	't',
	0x00,
	's',
	0x00,
};

struct usb_string_descriptors
{
	USB_STRING_DESCR lang_ids_descriptor;
	uint16_t lang_ids[1];
	USB_STRING_DESCR product_string_descriptor;
	uint8_t hydradancer_product_string_descriptor[sizeof(
		hydradancer_product_string_descriptor)];
} usb_string_descriptors;

const USB_STRING_DESCR* device_string_descriptors[2];

__attribute__((aligned(16)))
uint8_t endp1_tx_buffer[ENDP_1_15_MAX_PACKET_SIZE * DEF_ENDP_IN_BURST_LEVEL]
	__attribute__((section(".DMADATA")));
__attribute__((aligned(16)))
uint8_t endp1_rx_buffer[ENDP_1_15_MAX_PACKET_SIZE * DEF_ENDP_OUT_BURST_LEVEL]
	__attribute__((section(".DMADATA")));

void init_string_descriptors(void);
void init_string_descriptors(void)
{
	usb_string_descriptors.lang_ids_descriptor = (USB_STRING_DESCR){
		.bLength =
			sizeof(USB_STRING_DESCR) + sizeof(usb_string_descriptors.lang_ids),
		.bDescriptorType = 0x03, // String Descriptor
	};

	usb_string_descriptors.lang_ids[0] = 0x0409;

	usb_string_descriptors.product_string_descriptor = (USB_STRING_DESCR){
		.bLength = sizeof(USB_STRING_DESCR) +
				   sizeof(hydradancer_product_string_descriptor),
		.bDescriptorType = 0x03, // String Descriptor
	};

	memcpy(&usb_string_descriptors.hydradancer_product_string_descriptor,
		   hydradancer_product_string_descriptor,
		   sizeof(hydradancer_product_string_descriptor));

	device_string_descriptors[0] = &usb_string_descriptors.lang_ids_descriptor;
	device_string_descriptors[1] =
		&usb_string_descriptors.product_string_descriptor;
}

void init_endpoints(void);
void init_endpoints(void)
{
	endp1_rx.buffer = endp1_rx_buffer;
	endp1_rx.max_packet_size = ENDP_1_15_MAX_PACKET_SIZE;
	endp1_rx.max_burst = DEF_ENDP_OUT_BURST_LEVEL;
	endp1_rx.max_packet_size_with_burst = sizeof(endp1_rx_buffer);

	endp1_tx.buffer = NULL;
	endp1_tx.max_packet_size = ENDP_1_15_MAX_PACKET_SIZE;
	endp1_tx.max_burst = DEF_ENDP_OUT_BURST_LEVEL;
	endp1_tx.max_packet_size_with_burst = sizeof(endp1_rx_buffer);
}

#endif
