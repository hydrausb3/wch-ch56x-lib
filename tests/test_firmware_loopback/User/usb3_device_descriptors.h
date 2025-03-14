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

#ifndef USB3_DEVICE_DESCRIPTOR_H
#define USB3_DEVICE_DESCRIPTOR_H

#include "definitions.h"
#include "wch-ch56x-lib/usb/usb_descriptors.h"

uint8_t device_capabilities_1[4] = { 0x1e, 0xf4, 0x00, 0x00 };
uint8_t device_capabilities_2[7] = { 0x00, 0x0c, 0x00, 0x02, 0x0a, 0xff, 0x07 };

const uint8_t* usb3_device_configs[1];

struct usb3_descriptors
{
	USB_DEV_DESCR usb_device_descr;
	struct __PACKED
	{
		USB_CFG_DESCR usb_cfg_descr;
		USB_ITF_DESCR usb_itf_descr;
		USB_ENDP_DESCR usb_endp_descr_1;
		USB_ENDP_COMPANION_DESCR usb_endp_companion_descr_1;
		USB_ENDP_DESCR usb_endp_descr_1_tx;
		USB_ENDP_COMPANION_DESCR usb_endp_companion_descr_1_tx;
	} other_descr;
	struct __PACKED
	{
		USB_BOS_DESCR usb_bos_descr;
		struct __PACKED
		{
			USB_BOS_DEVICE_CAPABILITY_DESCR usb_bos_device_capability_descr;
			uint8_t capabilities[4];
		} capability_1;
		struct __PACKED
		{
			USB_BOS_DEVICE_CAPABILITY_DESCR usb_bos_device_capability_descr;
			uint8_t capabilities[7];
		} capability_2;
	} capabilities;
} usb3_descriptors;

void init_usb3_descriptors(void);

void init_usb3_descriptors(void)
{
	usb3_descriptors.usb_device_descr = (USB_DEV_DESCR){
		.bLength = 0x12,
		.bDescriptorType = 0x01, // device descriptor type
		.bcdUSB = 0x0300, // usb3.0
		.bDeviceClass = 0x00,
		.bDeviceSubClass = 0x00,
		.bDeviceProtocol = 0x00,
		.bMaxPacketSize0 = 9, // this is a requirement for usb3 (max packet size =
		// 2^9 thus the 9)
		.bcdDevice = 0x0001,
		.idVendor =
			0x16c0, // https://github.com/obdev/v-usb/blob/master/usbdrv/usb-ids-for-free.txt
		.idProduct = 0x27d8,
		.iProduct = 0x01,
		.iManufacturer = 0x00,
		.iSerialNumber = 0x00,
		.bNumConfigurations = 0x01
	};

	usb3_descriptors.other_descr.usb_cfg_descr = (USB_CFG_DESCR){
		.bLength = 0x09,
		.bDescriptorType = 0x02,
		.wTotalLength = sizeof(usb3_descriptors.other_descr),
		.bNumInterfaces = 0x01,
		.bConfigurationValue = 0x01,
		.iConfiguration = 0x00,
		.bmAttributes = 0xe0, // self-powered, supports remote wake-up
		.MaxPower = 0x64 // 200ma
	};

	usb3_descriptors.other_descr.usb_itf_descr =
		(USB_ITF_DESCR){ .bLength = 0x09,
						 .bDescriptorType = 0x04,
						 .bInterfaceNumber = 0x00,
						 .bAlternateSetting = 0x00,
						 .bNumEndpoints = 0x02,
						 .bInterfaceClass = 0xff, // vendor-specific
						 .bInterfaceSubClass = 0xff,
						 .bInterfaceProtocol = 0xff,
						 .iInterface = 0x00 };

	usb3_descriptors.other_descr.usb_endp_descr_1 = (USB_ENDP_DESCR){
		.bLength = 0x07,
		.bDescriptorType = 0x05,
		.bEndpointAddress = (ENDPOINT_DESCRIPTOR_ADDRESS_OUT | 0x01) &
							ENDPOINT_DESCRIPTOR_ADDRESS_MASK,
		.bmAttributes = ENDPOINT_DESCRIPTOR_BULK_TRANSFER,
		.wMaxPacketSizeL = 0x00,
		.wMaxPacketSizeH = 0x04, // 1024 bytes
		.bInterval = 50 // 50ms
	};

	usb3_descriptors.other_descr.usb_endp_companion_descr_1 =
		(USB_ENDP_COMPANION_DESCR){
			.bLength = 0x06,
			.bDescriptorType = 0x30,
			.bMaxBurst = DEF_ENDP_IN_BURST_LEVEL - 1,
			.bmAttributes = 0x00,
			.wBytesPerInterval = 0x00,
		};

	usb3_descriptors.other_descr.usb_endp_descr_1_tx = (USB_ENDP_DESCR){
		.bLength = 0x07,
		.bDescriptorType = 0x05,
		.bEndpointAddress = (ENDPOINT_DESCRIPTOR_ADDRESS_IN | 0x01) &
							ENDPOINT_DESCRIPTOR_ADDRESS_MASK,
		.bmAttributes = ENDPOINT_DESCRIPTOR_BULK_TRANSFER,
		.wMaxPacketSizeL = 0x00,
		.wMaxPacketSizeH = 0x04, // 1024 bytes
		.bInterval = 50 // 50ms
	};

	usb3_descriptors.other_descr.usb_endp_companion_descr_1_tx =
		(USB_ENDP_COMPANION_DESCR){
			.bLength = 0x06,
			.bDescriptorType = 0x30,
			.bMaxBurst = DEF_ENDP_IN_BURST_LEVEL - 1,
			.bmAttributes = 0x00,
			.wBytesPerInterval = 0x00,
		};

	usb3_descriptors.capabilities.usb_bos_descr = (USB_BOS_DESCR){
		.bLength = 0x05,
		.bDescriptorType = 0x0f,
		.wTotalLength = sizeof(
			usb3_descriptors.capabilities), // number of bytes of this descriptor
		// and all its subordinates
		.bNumDeviceCaps = 0x02 // number of device capability descriptor contained
		// in this BOS descriptor
	};

	usb3_descriptors.capabilities.capability_1.usb_bos_device_capability_descr =
		(USB_BOS_DEVICE_CAPABILITY_DESCR){
			.bLength = 0x07,
			.bDescriptorType = 0x10,
			.bDevCapabilityType = 0x02 // USB2.0 Extension
		};

	usb3_descriptors.capabilities.capability_2.usb_bos_device_capability_descr =
		(USB_BOS_DEVICE_CAPABILITY_DESCR){
			.bLength = 0x0a,
			.bDescriptorType = 0x10,
			.bDevCapabilityType = 0x03 // USB Superspeed
		};

	memcpy(usb3_descriptors.capabilities.capability_1.capabilities,
		   device_capabilities_1, 4);
	memcpy(usb3_descriptors.capabilities.capability_2.capabilities,
		   device_capabilities_2, 7);
	usb3_device_configs[0] = (uint8_t*)&usb3_descriptors.other_descr;
}

#endif
