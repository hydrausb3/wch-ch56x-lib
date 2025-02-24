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

#ifndef USB2_LS_DEVICE_DESCRIPTOR_H
#define USB2_LS_DEVICE_DESCRIPTOR_H

#include "definitions.h"
#include "wch-ch56x-lib/usb/usb_descriptors.h"

const uint8_t* usb2_ls_device_configs[1];

struct usb2_ls_descriptors
{
	USB_DEV_DESCR usb_device_descr;
	struct __PACKED
	{
		USB_CFG_DESCR usb_cfg_descr;
		USB_ITF_DESCR usb_itf_descr;
		USB_ENDP_DESCR usb_endp_descr_1;
		USB_ENDP_DESCR usb_endp_descr_2_tx;
	} other_descr;
} usb2_ls_descriptors;

void init_usb2_ls_descriptors(void);
void init_usb2_ls_descriptors(void)
{
	usb2_ls_descriptors.usb_device_descr = (USB_DEV_DESCR){
		.bLength = 0x12,
		.bDescriptorType = 0x01, // device descriptor type
		.bcdUSB = 0x0100, // usb2.0
		.bDeviceClass = 0x00,
		.bDeviceSubClass = 0x00,
		.bDeviceProtocol = 0x00,
		.bMaxPacketSize0 = 8,
		.bcdDevice = 0x0001,
		.idVendor =
			0x1209, // https://github.com/obdev/v-usb/blob/master/usbdrv/usb-ids-for-free.txt
		.idProduct = 0x0001,
		.iProduct = 0x01,
		.iManufacturer = 0x00,
		.iSerialNumber = 0x00,
		.bNumConfigurations = 0x01
	};

	usb2_ls_descriptors.other_descr.usb_cfg_descr = (USB_CFG_DESCR){
		.bLength = 0x09,
		.bDescriptorType = 0x02,
		.wTotalLength = sizeof(usb2_ls_descriptors.other_descr),
		.bNumInterfaces = 0x01,
		.bConfigurationValue = 0x01,
		.iConfiguration = 0x00,
		.bmAttributes = 0xa0, // supports remote wake-up
		.MaxPower = 0x64 // 200ma
	};

	usb2_ls_descriptors.other_descr.usb_itf_descr =
		(USB_ITF_DESCR){ .bLength = 0x09,
						 .bDescriptorType = 0x04,
						 .bInterfaceNumber = 0x00,
						 .bAlternateSetting = 0x00,
						 .bNumEndpoints = 0x02,
						 .bInterfaceClass = 0xff, // vendor-specific
						 .bInterfaceSubClass = 0xff,
						 .bInterfaceProtocol = 0xff,
						 .iInterface = 0x00 };

	usb2_ls_descriptors.other_descr.usb_endp_descr_1 = (USB_ENDP_DESCR){
		.bLength = 0x07,
		.bDescriptorType = 0x05,
		.bEndpointAddress = (ENDPOINT_DESCRIPTOR_ADDRESS_OUT | 0x01) &
							ENDPOINT_DESCRIPTOR_ADDRESS_MASK,
		.bmAttributes = ENDPOINT_DESCRIPTOR_INTERRUPT_TRANSFER,
		.wMaxPacketSizeL = 0x08,
		.wMaxPacketSizeH = 0x00, // 8 bytes
		.bInterval = 255 // max NAK rate
	};

	usb2_ls_descriptors.other_descr.usb_endp_descr_2_tx = (USB_ENDP_DESCR){
		.bLength = 0x07,
		.bDescriptorType = 0x05,
		.bEndpointAddress = (ENDPOINT_DESCRIPTOR_ADDRESS_IN | 0x02) &
							ENDPOINT_DESCRIPTOR_ADDRESS_MASK,
		.bmAttributes = ENDPOINT_DESCRIPTOR_INTERRUPT_TRANSFER,
		.wMaxPacketSizeL = 0x08,
		.wMaxPacketSizeH = 0x00, // 8 bytes
		.bInterval = 0
	};

	usb2_ls_device_configs[0] = (uint8_t*)&usb2_ls_descriptors.other_descr;
}

#endif
