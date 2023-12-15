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

#ifndef USB30_H
#define USB30_H

#ifdef __cplusplus
extern "C" {
#endif

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

#include "wch-ch56x-lib/logging/logging.h"
#include "wch-ch56x-lib/USBDevice/usb_device.h"
#include "wch-ch56x-lib/USBDevice/usb_types.h"

// link CFG
#define TERM_EN (1 << 1)
#define PIPE_RESET (1 << 3)
#define LFPS_RX_PD (1 << 5)
#define CFG_EQ_EN (1 << 6)
#define DEEMPH_CFG (1 << 8)

#define POWER_MODE_0 ((uint32_t)0x00000000)
#define POWER_MODE_1 ((uint32_t)0x00000001)
#define POWER_MODE_2 ((uint32_t)0x00000002)
#define POWER_MODE_3 ((uint32_t)0x00000003)

#define LINK_PRESENT (1 << 0)
#define RX_WARM_RESET ((uint32_t)1 << 1)

#define LINK_TXEQ (1 << 6)
#define GO_DISABLED (1 << 4)
#define POLLING_EN (1 << 12)

#define TX_HOT_RESET ((uint32_t)1 << 16)
#define RX_HOT_RESET ((uint32_t)1 << 24)

#define TX_WARM_RESET ((uint32_t)1 << 8)
#define TX_Ux_EXIT ((uint32_t)1 << 9)
// link int flag
#define LINK_RDY_FLAG (1 << 0)
#define LINK_RECOV_FLAG (1 << 1)
#define LINK_INACT_FLAG (1 << 2)
#define LINK_DISABLE_FLAG (1 << 3)
#define LINK_GO_U3_FLAG (1 << 4)
#define LINK_GO_U2_FLAG (1 << 5)
#define LINK_GO_U1_FLAG (1 << 6)
#define LINK_GO_U0_FLAG (1 << 7)
#define LINK_U3_WAKE_FLAG (1 << 8)
#define LINK_Ux_REJECT_FLAG (1 << 9)
#define TERM_PRESENT_FLAG (1 << 10)
#define LINK_TXEQ_FLAG (1 << 11)
#define LINK_Ux_EXIT_FLAG (1 << 12)
#define WARM_RESET_FLAG (1 << 13)
#define U3_WAKEUP_FLAG (1 << 14)
#define HOT_RESET_FLAG (1 << 15)
#define LINK_RX_DET_FLAG (1 << 20)

#define EP0_R_EN (1 << 0)
#define EP1_R_EN (1 << 1)
#define EP2_R_EN (1 << 2)
#define EP3_R_EN (1 << 3)
#define EP4_R_EN (1 << 4)
#define EP5_R_EN (1 << 5)
#define EP6_R_EN (1 << 6)
#define EP7_R_EN (1 << 7)

#define EP0_T_EN (1 << 8)
#define EP1_T_EN (1 << 9)
#define EP2_T_EN (1 << 10)
#define EP3_T_EN (1 << 11)
#define EP4_T_EN (1 << 12)
#define EP5_T_EN (1 << 13)
#define EP6_T_EN (1 << 14)
#define EP7_T_EN (1 << 15)

#define USB_FORCE_RST (1 << 2)
#define USB_ALL_CLR (1 << 1)
// LMP
#define LMP_HP 0
#define LMP_SUBTYPE_MASK (0xf << 5)
#define SET_LINK_FUNC (0x1 << 5)
#define U2_INACT_TOUT (0x2 << 5)
#define VENDOR_TEST (0x3 << 5)
#define PORT_CAP (0x4 << 5)
#define PORT_CFG (0x5 << 5)
#define PORT_CFG_RES (0x6 << 5)

#define LINK_SPEED (1 << 9)

#define NUM_HP_BUF (4 << 0)
#define DOWN_STREAM (1 << 16)
#define UP_STREAM (2 << 16)
#define TIE_BRK (1 << 20)

#define UsbSetupBuf ((USB_SETUP*)endp0_buffer) // endpoint 0
#define ENDP0_MAXPACK 512

// status response
#define NRDY 0
#define ACK 0x01
#define STALL 0x02
#define INVALID 0x03

// number of NUMP
#define NUMP_0 0x00
#define NUMP_1 0x01
#define NUMP_2 0x02
#define NUMP_3 0x03
#define NUMP_4 0x04
#define NUMP_5 0x05
#define NUMP_6 0x06

#define SS_RX_CONTRL(ep) (&USBSS->UEP0_RX_CTRL)[ep * 4]
#define SS_TX_CONTRL(ep) (&USBSS->UEP0_TX_CTRL)[ep * 4]

#define UNSUPPORTED_ENDPOINTS                                             \
	((ENDPOINT_8_TX | ENDPOINT_8_RX | ENDPOINT_9_TX | ENDPOINT_9_RX |     \
	  ENDPOINT_10_TX | ENDPOINT_10_RX | ENDPOINT_11_TX | ENDPOINT_11_RX | \
	  ENDPOINT_12_TX | ENDPOINT_12_RX | ENDPOINT_13_TX | ENDPOINT_13_RX | \
	  ENDPOINT_14_TX | ENDPOINT_14_RX | ENDPOINT_15_TX | ENDPOINT_15_RX))

/**
 * @brief Enable USB30 device but without downgrade to USB2
 * @param enable_usb2_fallback If enabled, the connection will activate USB2 if
 * USB3 is unavailable or if the USB cable is unplugged (unfortunately)
 */
void usb30_device_init(bool enable_usb2_fallback);

void usb30_device_deinit(void);

/**
 * @brief Initialize all endpoints (OUT endpoints : ACK / IN endpoints  : NRDY)
 * @param
 */
void usb30_init_endpoints(void);

/**
 * @brief Clear all control registers and reinitialize endpoints state with
 * usb30_init_endpoints
 * @param
 */
void usb30_reinit_endpoints(void);

/**
 * @fn      usb30_itp_callback
 * @brief   ITP callback function
 * @return   None
 */
void usb30_itp_callback(uint32_t ITPCounter);

/**
 * @brief Called by the USB abstraction layer when new data has been set for the
 * corresponding endpoint
 * @param size Size of the new data
 */
void usb3_endp1_tx_ready(uint16_t size);
void usb3_endp2_tx_ready(uint16_t size);
void usb3_endp3_tx_ready(uint16_t size);
void usb3_endp4_tx_ready(uint16_t size);
void usb3_endp5_tx_ready(uint16_t size);
void usb3_endp6_tx_ready(uint16_t size);
void usb3_endp7_tx_ready(uint16_t size);

#ifdef __cplusplus
}
#endif
#endif