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

#ifndef USB20_H
#define USB20_H

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

#include "usb_device.h"
#include "wch-ch56x-lib/logging/logging.h"
#include "wch-ch56x-lib/usb/usb2_utils.h"
#include "wch-ch56x-lib/usb/usb_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern usb_device_t* usb2_backend_current_device;

// USB20 Data structures

// PID according to WCH569 doc, does not correspond to USB packets PIDs
enum
{
	PID_OUT = 0,
	PID_SOF = 1,
	PID_IN = 2
};

typedef struct usb2_user_handled_t
{
	/**
   * @brief Called from the USB2 backend when a bus reset occurs
   */
	void (*usb2_device_handle_bus_reset)(void);
} usb2_user_handled_t;

extern usb2_user_handled_t usb2_user_handled;
extern volatile uint16_t endp_tx_remaining_bytes[16];
extern volatile USB_SETUP current_req;
extern volatile uint16_t current_req_size;
extern uint16_t usb2_endp0_max_packet_size;

/**
 * @fn     usb2_device_init
 *
 * @brief  USB2.0 device initialization
 * @return None
 */
void usb2_device_init(void);

/**
 * @fn     usb2_device_deinit
 *
 * @brief  USB2.0 device de-initialization
 * @return None
 */
void usb2_device_deinit(void);

/**
 * @fn usb2_setup_endpoints_in_mask
 * @brief Setup endpoints set in bitmask mask
 * @param mask Bitmask, endpoints to activate
 **/
void usb2_setup_endpoints_in_mask(uint32_t mask);

/**
 * @fn usb2_setup_endpoints
 * @brief Setup the registers containing the address of the endpoint buffers.
 *ENDP0 has a unique buffer for both the IN and the OUT endpoints. Resets the
 *control registers of the endpoints
 **/
void usb2_setup_endpoints(void);

/**
 * @fn usb2_reset_endpoints
 * @brief Set endpoints (except ENDP0 because it is already in use at this
 *point) to their default state, with toggle at DATA0
 **/
void usb2_reset_endpoints(void);

/**
 * @brief If set to true, enabled ep0 passthrough mode : SETUP requests will be
 * passed to endp0_passthrough_setup_callback, and other IN/OUT requests will be
 * transferred like any other endpoint. In this mode, all control requests are
 * handled by the user.
 * @param enable set to true to enable, false to disable
 */
void usb2_ep0_passthrough_enabled(bool enable);

/**
 * @brief Called by the USB abstraction layer when new data has been set for the
 * corresponding endpoint
 * @param endp_num Endpoint number
 * @param size Size of the new data
 */
void usb2_endp_tx_ready(uint8_t endp_num, uint16_t size);

/**
 * @brief Called by the USB abstraction layer when the user sets a new state for
 * the endp_num IN endpoint
 * @param endp_num
 */
void usb2_endp_tx_set_state_callback(uint8_t endp_num);

/**
 * @brief Called by the USB abstraction layer when the user sets a new state for
 * the endp_num OUT endpoint
 * @param endp_num
 */
void usb2_endp_rx_set_state_callback(uint8_t endp_num);

/**
 * @brief Enable NAK status
 * @param endp_num
 */
void usb2_enable_nak(bool enable);

__attribute__((interrupt("WCH-Interrupt-fast"))) void USBHS_IRQHandler(void);

#ifdef __cplusplus
}
#endif

#endif
