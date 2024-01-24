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

#ifndef USB_DESCRIPTORS_H
#define USB_DESCRIPTORS_H

// Disable warnings in bsp arising from -pedantic -Wall -Wconversion
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic ignored "-Wvariadic-macros"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#include "CH56xSFR.h"
#pragma GCC diagnostic pop
#pragma GCC diagnostic pop
#pragma GCC diagnostic pop

// endpoint directions and endpoint address mask for bEndpointAddress field of
// endpoint descriptor
#define ENDPOINT_DESCRIPTOR_ADDRESS_IN ((uint8_t)(1) << 7)
#define ENDPOINT_DESCRIPTOR_ADDRESS_OUT ((uint8_t)(0) << 7)
#define ENDPOINT_DESCRIPTOR_ADDRESS_MASK (0x8f)
// endpoint transfer types for bmAttributes filed of endpoint descriptor
#define ENDPOINT_DESCRIPTOR_TRANSFER_TYPE_MASK (0x03)
#define ENDPOINT_DESCRIPTOR_CONTROL_TRANSFER (0x00)
#define ENDPOINT_DESCRIPTOR_ISOCHRONOUS_TRANSFER (0x01)
#define ENDPOINT_DESCRIPTOR_BULK_TRANSFER (0x02)
#define ENDPOINT_DESCRIPTOR_INTERRUPT_TRANSFER (0x03)

#define USB_DESCR_TYP_BOS 0x0f
#define USB_DESCR_UNSUPPORTED 0xffff
#define INVALID_REQ_CODE 0xFF

/* string descriptor type */
#ifndef USB_DESCR_STRING
#define USB_DESCR_LANGID_STRING 0x00
#define USB_DESCR_VENDOR_STRING 0x01
#define USB_DESCR_PRODUCT_STRING 0x02
#define USB_DESCR_SERIAL_STRING 0x03
#define USB_DESCR_OS_STRING 0xee
#endif

typedef struct __PACKED _USB_STRING_DESCR
{
	uint8_t bLength;
	uint8_t bDescriptorType; // 0x03
} USB_STRING_DESCR;

typedef struct __PACKED _USB_ENDP_COMPANION_DESCR
{
	uint8_t bLength; // 0x06
	uint8_t bDescriptorType; // 0x30
	uint8_t bMaxBurst; // between 0 and 15
	uint8_t bmAttributes; // bulk : max number of streams; isochronous : max
		// number of packet in service interval.
	uint16_t wBytesPerInterval; // interrupt/isochronous : max number of bytes per
		// service interval
} USB_ENDP_COMPANION_DESCR, *PUSB_ENDP_COMPANION_DESCR;

typedef struct __PACKED _USB_BOS_DESCR
{
	uint8_t bLength; // 0x05
	uint8_t bDescriptorType; // 0x0f
	uint16_t wTotalLength; // number of bytes of this descriptor and all its
		// subordinates
	uint8_t bNumDeviceCaps; // number of device capability descriptor contained in
		// this BOS descriptor
} USB_BOS_DESCR, *PUSB_BOS_DESCR;

typedef struct __PACKED _USB_BOS_DEVICE_CAPABILITY_DESCR
{
	uint8_t bLength; // varies, additional data must be added at the end of this
		// descriptor
	uint8_t bDescriptorType; // 0x10
	uint8_t bDevCapabilityType;
} USB_BOS_DEVICE_CAPABILITY_DESCR, *PUSB_BOS_DEVICE_CAPABILITY_DESCR;

#define USB2_EXTENSION_ATTRIBUTES_MASK 0x00000002
#define USB2_EXTENSION_ATTRIBUTES_LPM 0x00000002
typedef struct __PACKED _USB_BOS_USB2_EXTENSION
{
	USB_BOS_DEVICE_CAPABILITY_DESCR capability;
	// uint8_t bLength; //0x07
	// uint8_t bDescriptorType; //0x10
	// uint8_t bDevCapabilityType; // 0x02
	uint32_t bmAttributes;
} USB_BOS_USB2_EXTENSION, *PUSB_BOS_USB2_EXTENSION;

#define SUPERSPEED_USB_DEVICE_CAPABILITY_ATTRIBUTES_MASK 0x02
#define SUPERSPEED_USB_DEVICE_CAPABILITY_ATTRIBUTES_LTM 0x00000002

#define SUPERSPEED_USB_DEVICE_CAPABILITY_SUPPORTED_SPEEDS_MASK 0x7
#define SUPERSPEED_USB_DEVICE_CAPABILITY_SUPPORTED_SPEEDS_LS 0x1
#define SUPERSPEED_USB_DEVICE_CAPABILITY_SUPPORTED_SPEEDS_FS 0x2
#define SUPERSPEED_USB_DEVICE_CAPABILITY_SUPPORTED_SPEEDS_HS 0x4
#define SUPERSPEED_USB_DEVICE_CAPABILITY_SUPPORTED_SPEEDS_SS 0x8

#define SUPERSPEED_USB_DEVICE_CAPABILITY_U1_DEV_EXIT_LAT_MASK 0x0f
#define SUPERSPEED_USB_DEVICE_CAPABILITY_U2_DEV_EXIT_LAT_MASK 0x7ff
typedef struct __PACKED _USB_BOS_SUPERSPEED_USB_DEVICE_CAPABILITY
{
	USB_BOS_DEVICE_CAPABILITY_DESCR capability;
	// uint8_t bLength; //0x0a
	// uint8_t bDescriptorType; //0x10
	// uint8_t bDevCapabilityType; // 0x03
	uint8_t bmAttributes;
	uint16_t wSpeedsSupported;
	uint8_t bFunctionalitySupport;
	uint8_t bU1DevExitLat; // max 0xa (10us)
	uint16_t wU2DevExitLat; // max 0x7ff (2047us)
} USB_BOS_SUPERSPEED_USB_DEVICE_CAPABILITY,
	*PUSB_BOS_SUPERSPEED_USB_DEVICE_CAPABILITY;

// those are the descriptors that can queried by a GET_DESCRIPTOR request.
// note that USB_DEVICE_CONFIG descr are assumed to be followed contiguously by
// all their child descriptors. note that USB_BOS_DESCR is expected to be
// followed contiguously by all its subordinates note that each USB_STRING_DESCR
// is expected to be followed by its payload

typedef struct __PACKED
{
	const USB_DEV_DESCR* usb_device_descr;
	const USB_DEV_QUAL_DESCR* usb_device_qualifier_descr;
	const uint8_t** usb_device_config_descrs;
	const USB_STRING_DESCR* const* usb_string_descrs;
	const USB_BOS_DESCR* usb_bos_descr;
} USB_DEVICE_DESCRIPTORS;

typedef struct __PACKED
{
	const USB_DEV_DESCR* usb2_device_descr;
	const USB_DEV_DESCR* usb3_device_descr;
	const USB_DEV_QUAL_DESCR* usb_device_qualifier_descr;
	const uint8_t** usb2_device_config_descrs;
	const uint8_t** usb3_device_config_descrs;
	const USB_STRING_DESCR* const* usb_string_descrs;
	const USB_BOS_DESCR* usb_bos_descr;
} USB23_DEVICE_DESCRIPTORS;

#endif
