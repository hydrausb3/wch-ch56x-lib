/********************************** (C) COPYRIGHT *******************************
Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
Copyright (c) 2022 Benjamin VERNOUX
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

Provide USB types used to define an abstract USB device.

*/

#ifndef USB_TYPES_H
#define USB_TYPES_H

#include <stdint.h>

typedef union
{
	uint16_t w;
	struct BW
	{
		uint8_t bb1; // low byte
		uint8_t bb0;
	} bw;
} UINT16_UINT8;

/**********control request command***********/
typedef struct __PACKED
{
	uint8_t bRequestType;
	uint8_t bRequest;
	UINT16_UINT8 wValue;
	UINT16_UINT8 wIndex;
	uint16_t wLength;
} USB_SETUP;

typedef uint8_t USB_DEVICE_CONFIG; // must start at 0x01
typedef uint8_t USB_DEVICE_INTERFACE; // must start at 0x00

typedef enum
{
	ENDPOINT_1_TX = 1U << 0,
	ENDPOINT_1_RX = 1U << 1,
	ENDPOINT_2_TX = 1U << 2,
	ENDPOINT_2_RX = 1U << 3,
	ENDPOINT_3_TX = 1U << 4,
	ENDPOINT_3_RX = 1U << 5,
	ENDPOINT_4_TX = 1U << 6,
	ENDPOINT_4_RX = 1U << 7,
	ENDPOINT_5_TX = 1U << 8,
	ENDPOINT_5_RX = 1U << 9,
	ENDPOINT_6_TX = 1U << 10,
	ENDPOINT_6_RX = 1U << 11,
	ENDPOINT_7_TX = 1U << 12,
	ENDPOINT_7_RX = 1U << 13,
	ENDPOINT_8_TX = 1U << 14,
	ENDPOINT_8_RX = 1U << 15,
	ENDPOINT_9_TX = 1U << 16,
	ENDPOINT_9_RX = 1U << 17,
	ENDPOINT_10_TX = 1U << 18,
	ENDPOINT_10_RX = 1U << 19,
	ENDPOINT_11_TX = 1U << 20,
	ENDPOINT_11_RX = 1U << 21,
	ENDPOINT_12_TX = 1U << 22,
	ENDPOINT_12_RX = 1U << 23,
	ENDPOINT_13_TX = 1U << 24,
	ENDPOINT_13_RX = 1U << 25,
	ENDPOINT_14_TX = 1U << 26,
	ENDPOINT_14_RX = 1U << 27,
	ENDPOINT_15_TX = 1U << 28,
	ENDPOINT_15_RX = 1U << 29,
} ENDPOINTS;

/* USB endpoint serial number */
#define ENDP_0 0x00
#define ENDP_1 0x01
#define ENDP_2 0x02
#define ENDP_3 0x03
#define ENDP_4 0x04
#define ENDP_5 0x05
#define ENDP_6 0x06
#define ENDP_7 0x07

#define ENDP_STATE_ACK 0x00
#define ENDP_STATE_NAK 0x02
#define ENDP_STATE_STALL 0x03

typedef enum
{
	Ack,
	Nak,
	Stall,
	Nyet,
	Nrdy, // USB3
	Erdy, // USB3
} TRANSACTION_STATUS;

typedef enum
{
	USB2_HIGHSPEED,
	USB2_FULLSPEED,
	USB2_LOWSPEED
} USB2_SPEED;

typedef enum USB_DEVICE_STATE
{
	DETACHED,
	ATTACHED, // USB device connected physically, this is a pre-requirement
	POWERED,
	DEFAULT,
	ADDRESS,
	CONFIGURED,
	SUSPENDED
} USB_DEVICE_STATE;

typedef enum USB_DEVICE_SPEED
{
	SPEED_NONE,
	SPEED_USB20,
	SPEED_USB30
} USB_DEVICE_SPEED;

// Standard feature selectors for CLEAR_FEATURE/SET_FEATURE

#define ENDPOINT_HALT 0
#define FUNCTION_SUSPEND 0
#define DEVICE_REMOTE_WAKEUP 1
#define TEST_MODE 2
#define B_HNP_ENABLE 3
#define A_HNP_SUPPORT 4
#define A_ALT_HNP_SU 5
#define WUSB_DEVICE 6
#define U1_ENABLE 48
#define U2_ENABLE 49
#define LTM_ENABLE 50
#define B3_NTF_HOST_REL 51
#define B3_RSP_ENABLE 52
#define LDM_ENABLE 53

/* USB endpoint direction */
#define OUT 0x00
#define IN 0x80

#endif
