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

#include "wch-ch56x-lib/USBDevice/usb_endpoints.h"
#include "wch-ch56x-lib/USBDevice/usb_device.h"

__attribute__((aligned(16))) uint8_t endp0_buffer[512]
	__attribute__((section(".DMADATA")));

volatile USB_ENDPOINT endp0 = {
	.buffer = endp0_buffer,
	.max_burst = 0,
	.max_packet_size = sizeof(endp0_buffer),
	.max_packet_size_with_burst =
		sizeof(endp0_buffer) // should be improved : USB2 cannot handle 512
	// bytes, so the check is too permissive
};

volatile USB_ENDPOINT endp1_tx;
volatile USB_ENDPOINT endp1_rx;
volatile USB_ENDPOINT endp2_tx;
volatile USB_ENDPOINT endp2_rx;
volatile USB_ENDPOINT endp3_tx;
volatile USB_ENDPOINT endp3_rx;
volatile USB_ENDPOINT endp4_tx;
volatile USB_ENDPOINT endp4_rx;
volatile USB_ENDPOINT endp5_tx;
volatile USB_ENDPOINT endp5_rx;
volatile USB_ENDPOINT endp6_tx;
volatile USB_ENDPOINT endp6_rx;
volatile USB_ENDPOINT endp7_tx;
volatile USB_ENDPOINT endp7_rx;

uint16_t _default_endp0_user_handled_control_request(USB_SETUP* request,
													 uint8_t** buffer);
uint16_t _default_endp0_user_handled_control_request(USB_SETUP* request,
													 uint8_t** buffer)
{
	return 0xffff;
}
void _default_endp0_passthrough_setup_callback(uint8_t* ptr, uint16_t size);
void _default_endp0_passthrough_setup_callback(uint8_t* ptr, uint16_t size) {}
void _default_endp_tx_complete(TRANSACTION_STATUS status);
void _default_endp_tx_complete(TRANSACTION_STATUS status) {}
uint8_t _default_endp_rx_callback(uint8_t* const ptr, uint16_t size);
uint8_t _default_endp_rx_callback(uint8_t* const ptr, uint16_t size)
{
	return ENDP_STATE_STALL;
}

usb_endpoints_user_handled_t usb_endpoints_user_handled = {
	.endp0_user_handled_control_request =
		_default_endp0_user_handled_control_request,
	.endp0_passthrough_setup_callback =
		_default_endp0_passthrough_setup_callback,
	.endp0_tx_complete = _default_endp_tx_complete,
	.endp1_tx_complete = _default_endp_tx_complete,
	.endp2_tx_complete = _default_endp_tx_complete,
	.endp3_tx_complete = _default_endp_tx_complete,
	.endp4_tx_complete = _default_endp_tx_complete,
	.endp5_tx_complete = _default_endp_tx_complete,
	.endp6_tx_complete = _default_endp_tx_complete,
	.endp7_tx_complete = _default_endp_tx_complete,
	.endp0_rx_callback = _default_endp_rx_callback,
	.endp1_rx_callback = _default_endp_rx_callback,
	.endp2_rx_callback = _default_endp_rx_callback,
	.endp3_rx_callback = _default_endp_rx_callback,
	.endp4_rx_callback = _default_endp_rx_callback,
	.endp5_rx_callback = _default_endp_rx_callback,
	.endp6_rx_callback = _default_endp_rx_callback,
	.endp7_rx_callback = _default_endp_rx_callback
};

void endp_rx_set_state(uint8_t endp_num, uint8_t state)
{
	volatile USB_ENDPOINT* endp = usb_get_rx_endp(endp_num);

	if (endp != NULL)
	{
		endp->state = state;
		if (usb_device.speed == SPEED_USB30)
			; // not implemented
		// usb3_endpoints_backend_handled.usb3_endp_rx_set_state_callback(endp_num);
		else
			usb2_endpoints_backend_handled.usb2_endp_rx_set_state_callback(endp_num);
	}
}

void endp_tx_set_state(uint8_t endp_num, uint8_t state)
{
	volatile USB_ENDPOINT* endp = usb_get_tx_endp(endp_num);

	if (endp != NULL)
	{
		endp->state = state;
		if (usb_device.speed == SPEED_USB30)
			// not implemented
			; // usb3_endpoints_backend_handled.usb3_endp_tx_set_state_callback(endp_num);
		else
			usb2_endpoints_backend_handled.usb2_endp_tx_set_state_callback(endp_num);
	}
}

volatile USB_ENDPOINT* usb_get_tx_endp(uint8_t endp)
{
	switch (endp)
	{
	case ENDP_0:
		return &endp0;
	case ENDP_1:
		return &endp1_tx;
	case ENDP_2:
		return &endp2_tx;
	case ENDP_3:
		return &endp3_tx;
	case ENDP_4:
		return &endp4_tx;
	case ENDP_5:
		return &endp5_tx;
	case ENDP_6:
		return &endp6_tx;
	case ENDP_7:
		return &endp7_tx;
	default:
		return NULL;
	}
	return NULL;
}
