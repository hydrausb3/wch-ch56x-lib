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
#include "wch-ch56x-lib/usb/usb_device.h"
#include "wch-ch56x-lib/usb/usb_types.h"

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

#define UsbSetupBuf ((USB_SETUP*)usb3_backend_current_device->endpoints.rx[0].buffer) // endpoint 0
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

extern usb_device_t* usb3_backend_current_device;
extern volatile uint16_t SetupLen;
extern volatile uint8_t SetupReqCode;
extern uint8_t* volatile pDescr;

// total length received
extern volatile uint16_t ep1_rx_total_length;
extern volatile uint16_t ep2_rx_total_length;
extern volatile uint16_t ep3_rx_total_length;
extern volatile uint16_t ep4_rx_total_length;
extern volatile uint16_t ep5_rx_total_length;
extern volatile uint16_t ep6_rx_total_length;
extern volatile uint16_t ep7_rx_total_length;

// how many burst the host was told it could send at most
extern volatile uint8_t ep1_rx_previously_set_max_burst;
extern volatile uint8_t ep2_rx_previously_set_max_burst;
extern volatile uint8_t ep3_rx_previously_set_max_burst;
extern volatile uint8_t ep4_rx_previously_set_max_burst;
extern volatile uint8_t ep5_rx_previously_set_max_burst;
extern volatile uint8_t ep6_rx_previously_set_max_burst;
extern volatile uint8_t ep7_rx_previously_set_max_burst;

extern volatile uint16_t ep1_tx_remaining_length;
extern volatile uint16_t ep2_tx_remaining_length;
extern volatile uint16_t ep3_tx_remaining_length;
extern volatile uint16_t ep4_tx_remaining_length;
extern volatile uint16_t ep5_tx_remaining_length;
extern volatile uint16_t ep6_tx_remaining_length;
extern volatile uint16_t ep7_tx_remaining_length;
extern volatile uint8_t ep1_tx_total_bursts;
extern volatile uint8_t ep2_tx_total_bursts;
extern volatile uint8_t ep3_tx_total_bursts;
extern volatile uint8_t ep4_tx_total_bursts;
extern volatile uint8_t ep5_tx_total_bursts;
extern volatile uint8_t ep6_tx_total_bursts;
extern volatile uint8_t ep7_tx_total_bursts;

/**
 * @brief Get the current number of bytes received on endpoint
 * @param endp endpoint number
 * @return
 */
__attribute__((always_inline)) static inline volatile uint16_t*
usb30_get_rx_endpoint_total_length(uint8_t endp)
{
	switch (endp)
	{
	case ENDP_1:
		return &ep1_rx_total_length;
		break;
	case ENDP_2:
		return &ep2_rx_total_length;
		break;
	case ENDP_3:
		return &ep3_rx_total_length;
		break;
	case ENDP_4:
		return &ep4_rx_total_length;
		break;
	case ENDP_5:
		return &ep5_rx_total_length;
		break;
	case ENDP_6:
		return &ep6_rx_total_length;
		break;
	case ENDP_7:
		return &ep7_rx_total_length;
		break;
	default:
		return NULL;
	}
}

/**
 * @brief In an OUT transfer, the device doesn't know what size data from the
 * host will be. Since the device sets the max number of bursts it can receive,
 * by having the max number of bursts, the number of actually received bursts
 * can be computed. The host can send any number of packets between 1 and
 * prev_set_max_burst, and we only get the number of remaining bursts, and the
 * size of the last packet, so we need prev_set_max_burst to compute the number
 * of full packets actually received. num_packets_received = prev_set_max_burst
 * - remaining_number_of_packets
 * @param endp endpoint number
 */
__attribute__((always_inline)) static inline volatile uint8_t*
usb30_get_rx_endpoint_prev_set_max_burst(uint8_t endp)
{
	switch (endp)
	{
	case ENDP_1:
		return &ep1_rx_previously_set_max_burst;
		break;
	case ENDP_2:
		return &ep2_rx_previously_set_max_burst;
		break;
	case ENDP_3:
		return &ep3_rx_previously_set_max_burst;
		break;
	case ENDP_4:
		return &ep4_rx_previously_set_max_burst;
		break;
	case ENDP_5:
		return &ep5_rx_previously_set_max_burst;
		break;
	case ENDP_6:
		return &ep6_rx_previously_set_max_burst;
		break;
	case ENDP_7:
		return &ep7_rx_previously_set_max_burst;
		break;
	default:
		return NULL;
	}
}

/**
 * @brief Get the remaining bytes to be sent for endp
 * @param endp endpoint number
 */
__attribute__((always_inline)) static inline volatile uint16_t*
usb30_get_tx_endpoint_remaining_length(uint8_t endp)
{
	switch (endp)
	{
	case ENDP_1:
		return &ep1_tx_remaining_length;
		break;
	case ENDP_2:
		return &ep2_tx_remaining_length;
		break;
	case ENDP_3:
		return &ep3_tx_remaining_length;
		break;
	case ENDP_4:
		return &ep4_tx_remaining_length;
		break;
	case ENDP_5:
		return &ep5_tx_remaining_length;
		break;
	case ENDP_6:
		return &ep6_tx_remaining_length;
		break;
	case ENDP_7:
		return &ep7_tx_remaining_length;
		break;
	default:
		return NULL;
	}
}

/**
 * @brief Get the number of remaining packets to be sent for endp (including a
 * packet of size less than max_packet_size)
 */
__attribute__((always_inline)) static inline volatile uint8_t*
usb30_get_tx_endpoint_total_bursts(uint8_t endp)
{
	switch (endp)
	{
	case ENDP_1:
		return &ep1_tx_total_bursts;
		break;
	case ENDP_2:
		return &ep2_tx_total_bursts;
		break;
	case ENDP_3:
		return &ep3_tx_total_bursts;
		break;
	case ENDP_4:
		return &ep4_tx_total_bursts;
		break;
	case ENDP_5:
		return &ep5_tx_total_bursts;
		break;
	case ENDP_6:
		return &ep6_tx_total_bursts;
		break;
	case ENDP_7:
		return &ep7_tx_total_bursts;
		break;
	default:
		return NULL;
	}
}

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
void usb3_endp_tx_ready(uint8_t endp_num, uint16_t size);

__attribute__((interrupt("WCH-Interrupt-fast"))) void LINK_IRQHandler(void);
__attribute__((interrupt("WCH-Interrupt-fast"))) void USBSS_IRQHandler(void);
#ifdef __cplusplus
}
#endif
#endif