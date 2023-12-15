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

#include "wch-ch56x-lib/USBDevice/usb_descriptors.h"
#include "wch-ch56x-lib/USBDevice/usb_types.h"
#include <stdint.h>

typedef struct USB_DEVICE
{
	volatile uint8_t addr;
	volatile USB2_DEVICE_DESCRIPTORS usb2_descriptors;
	volatile USB3_DEVICE_DESCRIPTORS usb3_descriptors;
	volatile uint8_t current_config;
	volatile USB_DEVICE_INTERFACE current_interface;
	volatile USB_DEVICE_STATE state;
	volatile USB_DEVICE_SPEED speed;
	volatile USB2_SPEED usb2_speed;
	volatile uint32_t endpoint_mask;
} USB_DEVICE;

extern USB_DEVICE usb_device;

#ifdef __cplusplus
extern "C" {
#endif

void usb_device_set_addr(uint8_t addr);

/**
 * @brief USB_DEV_DESCR should be followed by all its child descriptors.
 * @param descriptor
 */
void usb2_device_set_device_descriptor(const USB_DEV_DESCR* descriptor);
void usb3_device_set_device_descriptor(const USB_DEV_DESCR* descriptor);

void usb2_device_set_device_qualifier_descriptor(
	const USB_DEV_QUAL_DESCR* descriptor);
void usb3_device_set_device_qualifier_descriptor(
	const USB_DEV_QUAL_DESCR* descriptor);

/**
 * @brief Each USB_DEVICE_CONFIG should be followed contiguously by all its
 * child descriptors.
 * @param descriptors
 */
void usb2_device_set_config_descriptors(const USB_DEVICE_CONFIG** descriptors);
void usb3_device_set_config_descriptors(const USB_DEVICE_CONFIG** descriptors);

/**
 * @brief Descriptor should be followed by all its child descriptors
 * contiguously.
 * @param descriptor
 */
void usb3_device_set_bos_descriptor(const USB_BOS_DESCR* const descriptor);

/**
 * @param descriptors Each USB_STRING_DESCR must be followed contiguously by its
 * payload (string).
 */
void usb2_device_set_string_descriptors(
	const USB_STRING_DESCR* const* const descriptors);
void usb3_device_set_string_descriptors(
	const USB_STRING_DESCR* const* const descriptors);

/**
 * @brief Configure which endpoints are to be activated by the backend. See
 * usb_types.h for a list of endpoints.
 * @param endpoint_mask
 */
void usb_device_set_endpoint_mask(uint32_t endpoint_mask);

#ifdef __cplusplus
}
#endif

#endif
