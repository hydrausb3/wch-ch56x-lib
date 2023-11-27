/********************************** (C) COPYRIGHT
******************************* Copyright (c) 2023 Quarkslab

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

USB_DEVICE usb_device = { .usb2_speed = USB2_HIGHSPEED };

void usb_device_set_addr(uint8_t addr)
{
	if (addr <= 0x7f)
	{
		usb_device.addr = addr;
	}
}

void usb2_device_set_device_descriptor(const USB_DEV_DESCR* descriptor)
{
	usb_device.usb2_descriptors.usb_device_descr = descriptor;
}

void usb3_device_set_device_descriptor(const USB_DEV_DESCR* descriptor)
{
	usb_device.usb3_descriptors.usb_device_descr = descriptor;
}

void usb2_device_set_device_qualifier_descriptor(
	const USB_DEV_QUAL_DESCR* descriptor)
{
	usb_device.usb2_descriptors.usb_device_qualifier_descr = descriptor;
}

void usb3_device_set_device_qualifier_descriptor(
	const USB_DEV_QUAL_DESCR* descriptor)
{
	usb_device.usb3_descriptors.usb_device_qualifier_descr = descriptor;
}

void usb2_device_set_config_descriptors(const USB_DEVICE_CONFIG** descriptors)
{
	usb_device.usb2_descriptors.usb_device_config_descrs = descriptors;
}

void usb3_device_set_config_descriptors(const USB_DEVICE_CONFIG** descriptors)
{
	usb_device.usb3_descriptors.usb_device_config_descrs = descriptors;
}

void usb3_device_set_bos_descriptor(const USB_BOS_DESCR* const descriptor)
{
	usb_device.usb3_descriptors.usb_bos_descr = descriptor;
}

void usb2_device_set_string_descriptors(
	const USB_STRING_DESCR* const* const descriptors)
{
	usb_device.usb2_descriptors.usb_string_descrs = descriptors;
}

void usb3_device_set_string_descriptors(
	const USB_STRING_DESCR* const* const descriptors)
{
	usb_device.usb3_descriptors.usb_string_descrs = descriptors;
}

void usb_device_set_endpoint_mask(uint32_t endpoint_mask)
{
	usb_device.endpoint_mask = endpoint_mask;
}
