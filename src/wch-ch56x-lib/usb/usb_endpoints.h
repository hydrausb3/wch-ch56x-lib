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

#include "wch-ch56x-lib/usb/usb_types.h"

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

/**
 * Note : rx[0] is used for both tx and rx for EP0, when not in passthrough mode
 */
typedef struct usb_endpoints_t
{
	volatile USB_ENDPOINT tx[16];
	volatile USB_ENDPOINT rx[16];

	/**
   * @brief Process requests unhandled by the backend. Must return 0xffff if it
   * does not handle the request.
   */
	uint16_t (*endp0_user_handled_control_request)(USB_SETUP* request,
												   uint8_t** buffer);
	/**
   * @brief Called by the USB2 backend in passthrough mode after receiving a
   * SETUP request.
   * @return new state of endpoint response (ACK 0x00, NAK 0X02, STALL 0X03).
   * This will set the response for the next transfer (not this one).
   */
	uint8_t (*endp0_passthrough_setup_callback)(uint8_t* ptr, uint16_t size);

	/**
   * @brief Called by the backend when it has confirmed data has been sent
   * @param status The status of the transaction, sent by the host (ACK if all
   * went well)
   */
	void (*tx_complete[16])(TRANSACTION_STATUS status);

	/**
   * @brief rx_callback[0] is called by the USB2 backend in passthrough mode after receiving an
   * OUT request on ep0.
   */
	/**
   * @brief Called by the usb backend when data has been received on the
   * endpoint.
   * @arg ptr pointer to the buffer received for this endpoint
   * @arg size size of the buffer received
   * @return new state of endpoint response (ACK 0x00, NAK 0X02, STALL 0X03).
   * This will set the response for the next transfer (not this one).
   */
	uint8_t (*rx_callback[16])(uint8_t* const ptr, uint16_t size);

	void (*nak_callback)(uint8_t ep_num);

} usb_endpoints_t;

typedef struct usb2_endpoints_backend_handled_t
{
	/**
   * @brief Callback for the usb backend to execute when data is ready to be
   * sent for this endpoint. The backend can then set its registers accordingly.
   * @arg size Size of the packet to be sent.
   */
	void (*usb2_endp_tx_ready)(uint8_t endp_num, uint16_t size);

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
	void (*usb3_endp_tx_ready)(uint8_t endp_num, uint16_t size);

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

#ifdef __cplusplus
}
#endif

#endif
