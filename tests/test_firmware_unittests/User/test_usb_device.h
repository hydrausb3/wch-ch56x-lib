#ifndef TEST_USBDEVICE_H
#define TEST_USBDEVICE_H

#include "wch-ch56x-lib/USBDevice/usb_device.h"
#include "wch-ch56x-lib/USBDevice/usb_endpoints.h"

bool test_usb_device(void);

uint8_t data[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

#define ENDP_1_15_MAX_PACKET_SIZE 512
#define DEF_ENDP_OUT_BURST_LEVEL 1

bool test_usb_device(void)
{
	endp1_tx.buffer = NULL;
	endp1_tx.max_packet_size = ENDP_1_15_MAX_PACKET_SIZE;
	endp1_tx.max_burst = DEF_ENDP_OUT_BURST_LEVEL;
	endp1_tx.max_packet_size_with_burst = ENDP_1_15_MAX_PACKET_SIZE;

	endp2_tx.buffer = NULL;
	endp2_tx.max_packet_size = ENDP_1_15_MAX_PACKET_SIZE;
	endp2_tx.max_burst = DEF_ENDP_OUT_BURST_LEVEL;
	endp2_tx.max_packet_size_with_burst = ENDP_1_15_MAX_PACKET_SIZE;

	endp3_tx.buffer = NULL;
	endp3_tx.max_packet_size = ENDP_1_15_MAX_PACKET_SIZE;
	endp3_tx.max_burst = DEF_ENDP_OUT_BURST_LEVEL;
	endp3_tx.max_packet_size_with_burst = ENDP_1_15_MAX_PACKET_SIZE;

	endp4_tx.buffer = NULL;
	endp4_tx.max_packet_size = ENDP_1_15_MAX_PACKET_SIZE;
	endp4_tx.max_burst = DEF_ENDP_OUT_BURST_LEVEL;
	endp4_tx.max_packet_size_with_burst = ENDP_1_15_MAX_PACKET_SIZE;

	endp5_tx.buffer = NULL;
	endp5_tx.max_packet_size = ENDP_1_15_MAX_PACKET_SIZE;
	endp5_tx.max_burst = DEF_ENDP_OUT_BURST_LEVEL;
	endp5_tx.max_packet_size_with_burst = ENDP_1_15_MAX_PACKET_SIZE;

	endp6_tx.buffer = NULL;
	endp6_tx.max_packet_size = ENDP_1_15_MAX_PACKET_SIZE;
	endp6_tx.max_burst = DEF_ENDP_OUT_BURST_LEVEL;
	endp6_tx.max_packet_size_with_burst = ENDP_1_15_MAX_PACKET_SIZE;

	endp7_tx.buffer = NULL;
	endp7_tx.max_packet_size = ENDP_1_15_MAX_PACKET_SIZE;
	endp7_tx.max_burst = DEF_ENDP_OUT_BURST_LEVEL;
	endp7_tx.max_packet_size_with_burst = ENDP_1_15_MAX_PACKET_SIZE;

	bool success = true;

	usb_device_set_addr(1);

	if (usb_device.addr != 1)
		success = false;

	usb_device_set_addr(0);

	if (usb_device.addr != 0)
		success = false;

	endp1_tx_set_new_buffer(data, sizeof(data));
	endp2_tx_set_new_buffer(data, sizeof(data));
	endp3_tx_set_new_buffer(data, sizeof(data));
	endp4_tx_set_new_buffer(data, sizeof(data));
	endp5_tx_set_new_buffer(data, sizeof(data));
	endp6_tx_set_new_buffer(data, sizeof(data));
	endp7_tx_set_new_buffer(data, sizeof(data));

	bool endp_set_buffer_success =
		(endp1_tx.buffer == data) && (endp2_tx.buffer == data);
	if (!endp_set_buffer_success)
		success = false;

	return success;
}

#endif
