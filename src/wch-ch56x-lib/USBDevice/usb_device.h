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

/**
USB_Device provides functions and variables to handle the functionalities of the
device.

Which means
* Configuration of the descriptors
* Configuration of the endpoints (which are activated, what type of transfer)
* Memory management for the endpoints. A USB Endpoint does two things : sending
data and receiving data. Functions are needed to transmit data, get a callback
when data has been received and a callback for when data transmission is
finished.

The goal is to have a shared interface for USB communication, and do the
USB3.0/USB2.0 management in the back.

*/

#ifndef USB_DEVICE_H
#define USB_DEVICE_H

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
#include "wch-ch56x-lib/USBDevice/usb_descriptors.h"
#include "wch-ch56x-lib/USBDevice/usb_endpoints.h"
#include "wch-ch56x-lib/USBDevice/usb_types.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct USB_DEVICE
{
	volatile uint8_t addr;
	volatile USB23_DEVICE_DESCRIPTORS usb_descriptors;
	volatile uint8_t current_config;
	volatile USB_DEVICE_INTERFACE current_interface;
	volatile USB_DEVICE_STATE state;
	volatile USB_DEVICE_SPEED speed;
	volatile uint32_t endpoint_mask;
	volatile usb_endpoints_t endpoints;
} usb_device_t;

extern usb_device_t usb_device_0;
extern usb_device_t usb_device_1;

void usb_device_set_addr(usb_device_t* usb_device, uint8_t addr);

/**
 * @brief USB_DEV_DESCR should be followed by all its child descriptors.
 * @param descriptor
 */
void usb_device_set_usb2_device_descriptor(usb_device_t* usb_device, const USB_DEV_DESCR* descriptor);

/**
 * @brief USB_DEV_DESCR should be followed by all its child descriptors.
 * @param descriptor
 */
void usb_device_set_usb3_device_descriptor(usb_device_t* usb_device, const USB_DEV_DESCR* descriptor);

void usb_device_set_device_qualifier_descriptor(usb_device_t* usb_device,
												const USB_DEV_QUAL_DESCR* descriptor);

/**
 * @brief Each USB_DEVICE_CONFIG should be followed contiguously by all its
 * child descriptors.
 * @param descriptors
 */
void usb_device_set_usb2_config_descriptors(usb_device_t* usb_device, const USB_DEVICE_CONFIG** descriptors);

/**
 * @brief Each USB_DEVICE_CONFIG should be followed contiguously by all its
 * child descriptors.
 * @param descriptors
 */
void usb_device_set_usb3_config_descriptors(usb_device_t* usb_device, const USB_DEVICE_CONFIG** descriptors);

/**
 * @brief Descriptor should be followed by all its child descriptors
 * contiguously.
 * @param descriptor
 */
void usb_device_set_bos_descriptor(usb_device_t* usb_device, const USB_BOS_DESCR* const descriptor);

/**
 * @param descriptors Each USB_STRING_DESCR must be followed contiguously by its
 * payload (string).
 */
void usb_device_set_string_descriptors(usb_device_t* usb_device,
									   const USB_STRING_DESCR* const* const descriptors);

/**
 * @brief Configure which endpoints are to be activated by the backend. See
 * usb_types.h for a list of endpoints.
 * @param endpoint_mask
 */
void usb_device_set_endpoint_mask(usb_device_t* usb_device, uint32_t endpoint_mask);

/**
 * @brief Set new RAMX buffer that contains next data to be sent. The buffer
 * must remain valid until it has been transmitted (after endp*_tx_complete)
 * @arg ptr pointer to the buffer to be sent by this endpoint
 * @arg size size of the buffer to be sent
 */
/**
 * @brief Set new RAMX buffer that contains next data to be sent. The buffer
 * must remain valid until it has been transmitted (after endp*_tx_complete)
 * @arg ptr pointer to the buffer to be sent by this endpoint
 * @arg size size of the buffer to be sent
 */
__attribute__((always_inline)) inline static bool
endp_tx_set_new_buffer(usb_device_t* usb_device, uint8_t endp_num, uint8_t* const ptr, uint16_t size)
{
	bsp_disable_interrupt();
	volatile USB_ENDPOINT* ep = &usb_device->endpoints.tx[endp_num];
	if (size > ep->max_packet_size_with_burst || ptr == 0)
	{
		bsp_enable_interrupt();
		return false;
	}

	ep->buffer = ptr;

	if (endp_num == 0)
	{
		if (usb_device->endpoints.rx[0].buffer == NULL) return false;
		memcpy(usb_device->endpoints.rx[0].buffer, ptr, size);
	}

	bsp_enable_interrupt();
	if (usb_device->speed == USB30_SUPERSPEED)
		usb3_endpoints_backend_handled.usb3_endp_tx_ready(endp_num, size);
	else
		usb2_endpoints_backend_handled.usb2_endp_tx_ready(endp_num, size);

	return true;
}

/**
 * @brief Set the current state of the RX endpoint. This will affect the next
 * received OUT request.
 * @param endp_num endpoint number
 * @param state either ENDP_STATE_ACK, ENDP_STATE_NAK or ENDP_STATE_STALL
 */
void endp_rx_set_state(usb_device_t* usb_device, uint8_t endp_num, uint8_t state);

/**
 * @brief Set the current state of the TX endpoint. This will affect the next
 * received IN request.
 * @param endp_num endpoint number
 * @param state either ENDP_STATE_ACK, ENDP_STATE_NAK or ENDP_STATE_STALL
 */
void endp_tx_set_state(usb_device_t* usb_device, uint8_t endp_num, uint8_t state);

#ifdef __cplusplus
}
#endif

#endif
