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
	usb_device_0.endpoints.tx[1].buffer = NULL;
	usb_device_0.endpoints.tx[1].max_packet_size = ENDP_1_15_MAX_PACKET_SIZE;
	usb_device_0.endpoints.tx[1].max_burst = DEF_ENDP_OUT_BURST_LEVEL;
	usb_device_0.endpoints.tx[1].max_packet_size_with_burst = ENDP_1_15_MAX_PACKET_SIZE;

	usb_device_0.endpoints.tx[2].buffer = NULL;
	usb_device_0.endpoints.tx[2].max_packet_size = ENDP_1_15_MAX_PACKET_SIZE;
	usb_device_0.endpoints.tx[2].max_burst = DEF_ENDP_OUT_BURST_LEVEL;
	usb_device_0.endpoints.tx[2].max_packet_size_with_burst = ENDP_1_15_MAX_PACKET_SIZE;

	usb_device_0.endpoints.tx[3].buffer = NULL;
	usb_device_0.endpoints.tx[3].max_packet_size = ENDP_1_15_MAX_PACKET_SIZE;
	usb_device_0.endpoints.tx[3].max_burst = DEF_ENDP_OUT_BURST_LEVEL;
	usb_device_0.endpoints.tx[3].max_packet_size_with_burst = ENDP_1_15_MAX_PACKET_SIZE;

	usb_device_0.endpoints.tx[4].buffer = NULL;
	usb_device_0.endpoints.tx[4].max_packet_size = ENDP_1_15_MAX_PACKET_SIZE;
	usb_device_0.endpoints.tx[4].max_burst = DEF_ENDP_OUT_BURST_LEVEL;
	usb_device_0.endpoints.tx[4].max_packet_size_with_burst = ENDP_1_15_MAX_PACKET_SIZE;

	usb_device_0.endpoints.tx[5].buffer = NULL;
	usb_device_0.endpoints.tx[5].max_packet_size = ENDP_1_15_MAX_PACKET_SIZE;
	usb_device_0.endpoints.tx[5].max_burst = DEF_ENDP_OUT_BURST_LEVEL;
	usb_device_0.endpoints.tx[5].max_packet_size_with_burst = ENDP_1_15_MAX_PACKET_SIZE;

	usb_device_0.endpoints.tx[6].buffer = NULL;
	usb_device_0.endpoints.tx[6].max_packet_size = ENDP_1_15_MAX_PACKET_SIZE;
	usb_device_0.endpoints.tx[6].max_burst = DEF_ENDP_OUT_BURST_LEVEL;
	usb_device_0.endpoints.tx[6].max_packet_size_with_burst = ENDP_1_15_MAX_PACKET_SIZE;

	usb_device_0.endpoints.tx[7].buffer = NULL;
	usb_device_0.endpoints.tx[7].max_packet_size = ENDP_1_15_MAX_PACKET_SIZE;
	usb_device_0.endpoints.tx[7].max_burst = DEF_ENDP_OUT_BURST_LEVEL;
	usb_device_0.endpoints.tx[7].max_packet_size_with_burst = ENDP_1_15_MAX_PACKET_SIZE;

	bool success = true;

	usb_device_set_addr(&usb_device_0, 1);

	if (usb_device_0.addr != 1)
		success = false;

	usb_device_set_addr(&usb_device_0, 0);

	if (usb_device_0.addr != 0)
		success = false;

	endp_tx_set_new_buffer(&usb_device_0, 1, data, sizeof(data));
	endp_tx_set_new_buffer(&usb_device_0, 2, data, sizeof(data));
	endp_tx_set_new_buffer(&usb_device_0, 3, data, sizeof(data));
	endp_tx_set_new_buffer(&usb_device_0, 4, data, sizeof(data));
	endp_tx_set_new_buffer(&usb_device_0, 5, data, sizeof(data));
	endp_tx_set_new_buffer(&usb_device_0, 6, data, sizeof(data));
	endp_tx_set_new_buffer(&usb_device_0, 7, data, sizeof(data));

	bool endp_set_buffer_success =
		(usb_device_0.endpoints.tx[1].buffer == data) && (usb_device_0.endpoints.tx[2].buffer == data);
	if (!endp_set_buffer_success)
		success = false;

	return success;
}

#endif
