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

#ifndef USB_ENDPOINT_H
#define USB_ENDPOINT_H

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "wch-ch56x-lib/USBDevice/usb_device.h"
#include "wch-ch56x-lib/USBDevice/usb_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct USB_ENDPOINT_T
{
	uint8_t* volatile buffer;
	volatile uint8_t max_burst;
	volatile uint16_t max_packet_size;
	volatile uint16_t max_packet_size_with_burst;
	volatile uint8_t state; // 0x00 ACK, 0x02 NAK, 0x03 STALL
} USB_ENDPOINT;

extern uint8_t endp0_buffer[512]; // 512 = max size of endp0 buffer for USB3.
extern volatile USB_ENDPOINT endp0;
extern volatile USB_ENDPOINT endp1_tx;
extern volatile USB_ENDPOINT endp1_rx;
extern volatile USB_ENDPOINT endp2_tx;
extern volatile USB_ENDPOINT endp2_rx;
extern volatile USB_ENDPOINT endp3_tx;
extern volatile USB_ENDPOINT endp3_rx;
extern volatile USB_ENDPOINT endp4_tx;
extern volatile USB_ENDPOINT endp4_rx;
extern volatile USB_ENDPOINT endp5_tx;
extern volatile USB_ENDPOINT endp5_rx;
extern volatile USB_ENDPOINT endp6_tx;
extern volatile USB_ENDPOINT endp6_rx;
extern volatile USB_ENDPOINT endp7_tx;
extern volatile USB_ENDPOINT endp7_rx;

typedef struct usb_endpoints_user_handled_t
{
	/**
   * @brief Process requests unhandled by the backend. Must return 0xffff if it
   * does not handle the request.
   */
	uint16_t (*endp0_user_handled_control_request)(USB_SETUP* request,
												   uint8_t** buffer);
	/**
   * @brief Called by the USB2 backend in passthrough mode after receiving a
   * SETUP request.
   */
	void (*endp0_passthrough_setup_callback)(uint8_t* ptr, uint16_t size);
	/**
   * @brief Called by the USB2 backend in passthrough mode after receiving an
   * OUT request on ep0.
   */
	uint8_t (*endp0_rx_callback)(uint8_t* const ptr, uint16_t size);

	/**
   * @brief Called by the backend when it has confirmed data has been sent
   * @param status The status of the transaction, sent by the host (ACK if all
   * went well)
   */
	void (*endp0_tx_complete)(TRANSACTION_STATUS status);
	void (*endp1_tx_complete)(TRANSACTION_STATUS status);
	void (*endp2_tx_complete)(TRANSACTION_STATUS status);
	void (*endp3_tx_complete)(TRANSACTION_STATUS status);
	void (*endp4_tx_complete)(TRANSACTION_STATUS status);
	void (*endp5_tx_complete)(TRANSACTION_STATUS status);
	void (*endp6_tx_complete)(TRANSACTION_STATUS status);
	void (*endp7_tx_complete)(TRANSACTION_STATUS status);

	/**
   * @brief Called by the usb backend when data has been received on the
   * endpoint.
   * @arg ptr pointer to the buffer received for this endpoint
   * @arg size size of the buffer received
   * @return new state of endpoint response (ACK 0x00, NAK 0X02, STALL 0X03).
   * This will set the response for the next transfer (not this one).
   */
	uint8_t (*endp1_rx_callback)(uint8_t* const ptr, uint16_t size);
	uint8_t (*endp2_rx_callback)(uint8_t* const ptr, uint16_t size);
	uint8_t (*endp3_rx_callback)(uint8_t* const ptr, uint16_t size);
	uint8_t (*endp4_rx_callback)(uint8_t* const ptr, uint16_t size);
	uint8_t (*endp5_rx_callback)(uint8_t* const ptr, uint16_t size);
	uint8_t (*endp6_rx_callback)(uint8_t* const ptr, uint16_t size);
	uint8_t (*endp7_rx_callback)(uint8_t* const ptr, uint16_t size);

} usb_endpoints_user_handled_t;

extern usb_endpoints_user_handled_t usb_endpoints_user_handled;

typedef struct usb2_endpoints_backend_handled_t
{
	/**
   * @brief Callback for the usb backend to execute when data is ready to be
   * sent for this endpoint. The backend can then set its registers accordingly.
   * @arg size Size of the packet to be sent.
   */
	void (*usb2_endp0_tx_ready)(uint16_t size);
	void (*usb2_endp1_tx_ready)(uint16_t size);
	void (*usb2_endp2_tx_ready)(uint16_t size);
	void (*usb2_endp3_tx_ready)(uint16_t size);
	void (*usb2_endp4_tx_ready)(uint16_t size);
	void (*usb2_endp5_tx_ready)(uint16_t size);
	void (*usb2_endp6_tx_ready)(uint16_t size);
	void (*usb2_endp7_tx_ready)(uint16_t size);

	/**
   * @brief Callbacks implemented by the USB2 backends
   * @param endp_num
   */
	void (*usb2_endp_rx_set_state_callback)(uint8_t endp_num);

	/**
   * @brief Callbacks implemented by the USB2 backends
   * @param endp_num
   */
	void (*usb2_endp_tx_set_state_callback)(uint8_t endp_num);
} usb2_endpoints_backend_handled_t;

extern usb2_endpoints_backend_handled_t usb2_endpoints_backend_handled;

typedef struct usb3_endpoints_backend_handled_t
{
	/**
   * @brief Callback for the usb backend to execute when data is ready to be
   * sent for this endpoint. The backend can then set its registers accordingly.
   * @arg size Size of the packet to be sent.
   */
	void (*usb3_endp0_tx_ready)(uint16_t size);
	void (*usb3_endp1_tx_ready)(uint16_t size);
	void (*usb3_endp2_tx_ready)(uint16_t size);
	void (*usb3_endp3_tx_ready)(uint16_t size);
	void (*usb3_endp4_tx_ready)(uint16_t size);
	void (*usb3_endp5_tx_ready)(uint16_t size);
	void (*usb3_endp6_tx_ready)(uint16_t size);
	void (*usb3_endp7_tx_ready)(uint16_t size);

	/**
   * @brief Callbacks implemented by the USB3 backends
   * @param endp_num
   */
	void (*usb3_endp_rx_set_state_callback)(uint8_t endp_num);

	/**
   * @brief Callbacks implemented by the USB3 backends
   * @param endp_num
   */
	void (*usb3_endp_tx_set_state_callback)(uint8_t endp_num);
} usb3_endpoints_backend_handled_t;

extern usb3_endpoints_backend_handled_t usb3_endpoints_backend_handled;

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
endp0_tx_set_new_buffer(uint8_t* const ptr, uint16_t size)
{
	if (size > endp0.max_packet_size_with_burst || ptr == 0)
	{
		return false;
	}
	memcpy(endp0_buffer, ptr, size);
	if (usb_device.speed == SPEED_USB30)
		usb3_endpoints_backend_handled.usb3_endp0_tx_ready(size);
	else
		usb2_endpoints_backend_handled.usb2_endp0_tx_ready(size);

	return true;
}

__attribute__((always_inline)) inline static bool
endp1_tx_set_new_buffer(uint8_t* const ptr, uint16_t size)
{
	if (size > endp1_tx.max_packet_size_with_burst || ptr == 0)
	{
		return false;
	}
	endp1_tx.buffer = ptr;

	if (usb_device.speed == SPEED_USB30)
		usb3_endpoints_backend_handled.usb3_endp1_tx_ready(size);
	else
		usb2_endpoints_backend_handled.usb2_endp1_tx_ready(size);

	return true;
}

__attribute__((always_inline)) inline static bool
endp2_tx_set_new_buffer(uint8_t* const ptr, uint16_t size)
{
	if (size > endp2_tx.max_packet_size_with_burst || ptr == 0)
	{
		return false;
	}
	endp2_tx.buffer = ptr;

	if (usb_device.speed == SPEED_USB30)
		usb3_endpoints_backend_handled.usb3_endp2_tx_ready(size);
	else
		usb2_endpoints_backend_handled.usb2_endp2_tx_ready(size);

	return true;
}

__attribute__((always_inline)) inline static bool
endp3_tx_set_new_buffer(uint8_t* const ptr, uint16_t size)
{
	if (size > endp3_tx.max_packet_size_with_burst || ptr == 0)
	{
		return false;
	}
	endp3_tx.buffer = ptr;

	if (usb_device.speed == SPEED_USB30)
		usb3_endpoints_backend_handled.usb3_endp3_tx_ready(size);
	else
		usb2_endpoints_backend_handled.usb2_endp3_tx_ready(size);

	return true;
}

__attribute__((always_inline)) inline static bool
endp4_tx_set_new_buffer(uint8_t* const ptr, uint16_t size)
{
	if (size > endp4_tx.max_packet_size_with_burst || ptr == 0)
	{
		return false;
	}
	endp4_tx.buffer = ptr;

	if (usb_device.speed == SPEED_USB30)
		usb3_endpoints_backend_handled.usb3_endp4_tx_ready(size);
	else
		usb2_endpoints_backend_handled.usb2_endp4_tx_ready(size);

	return true;
}

__attribute__((always_inline)) inline static bool
endp5_tx_set_new_buffer(uint8_t* const ptr, uint16_t size)
{
	if (size > endp5_tx.max_packet_size_with_burst || ptr == 0)
	{
		return false;
	}
	endp5_tx.buffer = ptr;

	if (usb_device.speed == SPEED_USB30)
		usb3_endpoints_backend_handled.usb3_endp5_tx_ready(size);
	else
		usb2_endpoints_backend_handled.usb2_endp5_tx_ready(size);

	return true;
}

__attribute__((always_inline)) inline static bool
endp6_tx_set_new_buffer(uint8_t* const ptr, uint16_t size)
{
	if (size > endp6_tx.max_packet_size_with_burst || ptr == 0)
	{
		return false;
	}
	endp6_tx.buffer = ptr;

	if (usb_device.speed == SPEED_USB30)
		usb3_endpoints_backend_handled.usb3_endp6_tx_ready(size);
	else
		usb2_endpoints_backend_handled.usb2_endp6_tx_ready(size);

	return true;
}

__attribute__((always_inline)) inline static bool
endp7_tx_set_new_buffer(uint8_t* const ptr, uint16_t size)
{
	if (size > endp7_tx.max_packet_size_with_burst || ptr == 0)
	{
		return false;
	}
	endp7_tx.buffer = ptr;

	if (usb_device.speed == SPEED_USB30)
		usb3_endpoints_backend_handled.usb3_endp7_tx_ready(size);
	else
		usb2_endpoints_backend_handled.usb2_endp7_tx_ready(size);

	return true;
}

__attribute__((always_inline)) inline static bool
endp_tx_set_new_buffer(uint8_t endp, uint8_t* const ptr, uint16_t size)
{
	switch (endp)
	{
	case 1:
		return endp1_tx_set_new_buffer(ptr, size);
		break;
	case 2:
		return endp2_tx_set_new_buffer(ptr, size);
		break;
	case 3:
		return endp3_tx_set_new_buffer(ptr, size);
		break;
	case 4:
		return endp4_tx_set_new_buffer(ptr, size);
		break;
	case 5:
		return endp5_tx_set_new_buffer(ptr, size);
		break;
	case 6:
		return endp6_tx_set_new_buffer(ptr, size);
		break;
	case 7:
		return endp7_tx_set_new_buffer(ptr, size);
		break;
	}
	return false;
}

__attribute__((always_inline)) inline static void
endp_tx_complete(uint8_t endp, TRANSACTION_STATUS status)
{
	switch (endp)
	{
	case 0:
		usb_endpoints_user_handled.endp0_tx_complete(status);
		break;
	case 1:
		usb_endpoints_user_handled.endp1_tx_complete(status);
		break;
	case 2:
		usb_endpoints_user_handled.endp2_tx_complete(status);
		break;
	case 3:
		usb_endpoints_user_handled.endp3_tx_complete(status);
		break;
	case 4:
		usb_endpoints_user_handled.endp4_tx_complete(status);
		break;
	case 5:
		usb_endpoints_user_handled.endp5_tx_complete(status);
		break;
	case 6:
		usb_endpoints_user_handled.endp6_tx_complete(status);
		break;
	case 7:
		usb_endpoints_user_handled.endp7_tx_complete(status);
		break;
	default:
		break;
	}
}

__attribute__((always_inline)) inline static uint8_t
endp_rx_callback(uint8_t endp, uint8_t* const ptr, uint16_t size)
{
	switch (endp)
	{
	case 0:
		return usb_endpoints_user_handled.endp0_rx_callback(ptr, size);
		break;
	case 1:
		return usb_endpoints_user_handled.endp1_rx_callback(ptr, size);
		break;
	case 2:
		return usb_endpoints_user_handled.endp2_rx_callback(ptr, size);
		break;
	case 3:
		return usb_endpoints_user_handled.endp3_rx_callback(ptr, size);
		break;
	case 4:
		return usb_endpoints_user_handled.endp4_rx_callback(ptr, size);
		break;
	case 5:
		return usb_endpoints_user_handled.endp5_rx_callback(ptr, size);
		break;
	case 6:
		return usb_endpoints_user_handled.endp6_rx_callback(ptr, size);
		break;
	case 7:
		return usb_endpoints_user_handled.endp7_rx_callback(ptr, size);
		break;
	default:
		break;
	}
	return ENDP_STATE_NAK;
}

/**
 * @brief Set the current state of the RX endpoint. This will affect the next
 * received OUT request.
 * @param endp_num endpoint number
 * @param state either ENDP_STATE_ACK, ENDP_STATE_NAK or ENDP_STATE_STALL
 */
void endp_rx_set_state(uint8_t endp_num, uint8_t state);

/**
 * @brief Set the current state of the TX endpoint. This will affect the next
 * received IN request.
 * @param endp_num endpoint number
 * @param state either ENDP_STATE_ACK, ENDP_STATE_NAK or ENDP_STATE_STALL
 */
void endp_tx_set_state(uint8_t endp_num, uint8_t state);

/**
 * @brief Get OUT endpoint struct from endpoint number
 * @param endp endpoint number
 */
__attribute__((always_inline)) inline static volatile USB_ENDPOINT*
usb_get_rx_endp(uint8_t endp)
{
	switch (endp)
	{
	case ENDP_0:
		return &endp0;
	case ENDP_1:
		return &endp1_rx;
	case ENDP_2:
		return &endp2_rx;
	case ENDP_3:
		return &endp3_rx;
	case ENDP_4:
		return &endp4_rx;
	case ENDP_5:
		return &endp5_rx;
	case ENDP_6:
		return &endp6_rx;
	case ENDP_7:
		return &endp7_rx;
	default:
		return NULL;
	}
	return NULL;
}

/**
 * @brief Get IN endpoint struct from endpoint number
 * @param endp endpoint number
 * TODO: force inlining this makes the USB3 peripheral not enumerate ... Not
 * sure why, maybe Link_IRQHandler gets too big ?
 */
volatile USB_ENDPOINT* usb_get_tx_endp(uint8_t endp);

#ifdef __cplusplus
}
#endif

#endif
