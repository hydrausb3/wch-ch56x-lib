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

#include "wch-ch56x-lib/USBDevice/usb_device.h"

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

/**
 * Setting up default handlers here : this prevents the device from crashing and avoid an unnecessary check for handlers that are user-defined.
 */
usb_device_t usb_device_0 = {
	.speed = USB2_HIGHSPEED,
	.endpoints = {
		.endp0_user_handled_control_request =
			_default_endp0_user_handled_control_request,
		.endp0_passthrough_setup_callback =
			_default_endp0_passthrough_setup_callback,
		.tx_complete = { _default_endp_tx_complete,
						 _default_endp_tx_complete,
						 _default_endp_tx_complete,
						 _default_endp_tx_complete,
						 _default_endp_tx_complete,
						 _default_endp_tx_complete,
						 _default_endp_tx_complete,
						 _default_endp_tx_complete },
		.rx_callback = { _default_endp_rx_callback,
						 _default_endp_rx_callback,
						 _default_endp_rx_callback,
						 _default_endp_rx_callback,
						 _default_endp_rx_callback,
						 _default_endp_rx_callback,
						 _default_endp_rx_callback,
						 _default_endp_rx_callback } }
};

usb_device_t usb_device_1 = {
	.speed = USB2_HIGHSPEED,
	.endpoints = {
		.endp0_user_handled_control_request =
			_default_endp0_user_handled_control_request,
		.endp0_passthrough_setup_callback =
			_default_endp0_passthrough_setup_callback,
		.tx_complete = { _default_endp_tx_complete,
						 _default_endp_tx_complete,
						 _default_endp_tx_complete,
						 _default_endp_tx_complete,
						 _default_endp_tx_complete,
						 _default_endp_tx_complete,
						 _default_endp_tx_complete,
						 _default_endp_tx_complete },
		.rx_callback = { _default_endp_rx_callback,
						 _default_endp_rx_callback,
						 _default_endp_rx_callback,
						 _default_endp_rx_callback,
						 _default_endp_rx_callback,
						 _default_endp_rx_callback,
						 _default_endp_rx_callback,
						 _default_endp_rx_callback } }
};

void usb_device_set_addr(usb_device_t* usb_device, uint8_t addr)
{
	if (addr <= 0x7f)
	{
		usb_device->addr = addr;
	}
}

void usb_device_set_usb2_device_descriptor(usb_device_t* usb_device, const USB_DEV_DESCR* descriptor)
{
	usb_device->usb_descriptors.usb2_device_descr = descriptor;
}

void usb_device_set_usb3_device_descriptor(usb_device_t* usb_device, const USB_DEV_DESCR* descriptor)
{
	usb_device->usb_descriptors.usb3_device_descr = descriptor;
}

void usb_device_set_device_qualifier_descriptor(usb_device_t* usb_device,
												const USB_DEV_QUAL_DESCR* descriptor)
{
	usb_device->usb_descriptors.usb_device_qualifier_descr = descriptor;
}

void usb_device_set_usb2_config_descriptors(usb_device_t* usb_device, const USB_DEVICE_CONFIG** descriptors)
{
	usb_device->usb_descriptors.usb2_device_config_descrs = descriptors;
}

void usb_device_set_usb3_config_descriptors(usb_device_t* usb_device, const USB_DEVICE_CONFIG** descriptors)
{
	usb_device->usb_descriptors.usb3_device_config_descrs = descriptors;
}

void usb_device_set_bos_descriptor(usb_device_t* usb_device, const USB_BOS_DESCR* const descriptor)
{
	usb_device->usb_descriptors.usb_bos_descr = descriptor;
}

void usb_device_set_string_descriptors(usb_device_t* usb_device,
									   const USB_STRING_DESCR* const* const descriptors)
{
	usb_device->usb_descriptors.usb_string_descrs = descriptors;
}

void usb_device_set_endpoint_mask(usb_device_t* usb_device, uint32_t endpoint_mask)
{
	usb_device->endpoint_mask = endpoint_mask;
}

void endp_rx_set_state(usb_device_t* usb_device, uint8_t endp_num, uint8_t state)
{
	volatile USB_ENDPOINT* ep = &usb_device->endpoints.rx[endp_num];

	if (ep != NULL)
	{
		ep->state = state;
		if (usb_device->speed == USB30_SUPERSPEED)
			; // not implemented
		// usb3_endpoints_backend_handled.usb3_endp_rx_set_state_callback(endp_num);
		else
			usb2_endpoints_backend_handled.usb2_endp_rx_set_state_callback(endp_num);
	}
}

void endp_tx_set_state(usb_device_t* usb_device, uint8_t endp_num, uint8_t state)
{
	volatile USB_ENDPOINT* ep = &usb_device->endpoints.tx[endp_num];

	if (ep != NULL)
	{
		ep->state = state;
		if (usb_device->speed == USB30_SUPERSPEED)
			// not implemented
			; // usb3_endpoints_backend_handled.usb3_endp_tx_set_state_callback(endp_num);
		else
			usb2_endpoints_backend_handled.usb2_endp_tx_set_state_callback(endp_num);
	}
}
