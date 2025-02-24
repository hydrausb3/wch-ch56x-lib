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

#include "wch-ch56x-lib/usb/usb30.h"
#include "wch-ch56x-lib/usb/usb20.h"
#include "wch-ch56x-lib/usb/usb30_utils.h"
#include "wch-ch56x-lib/usb/usb_descriptors.h"
#include "wch-ch56x-lib/usb/usb_endpoints.h"

#define ENDP0_MAX_PACKET_SIZE 512

usb3_endpoints_backend_handled_t usb3_endpoints_backend_handled = {
	.usb3_endp_tx_ready = usb3_endp_tx_ready
};

// the default device is a regular USB3 with the mandatory USB2 compatibility.
// however, it is possible to separate the USB2 and USB3 devices so that they use different devices.
usb_device_t* usb3_backend_current_device = &usb_device_0;

/* Global define */
/* Global Variable */
static volatile uint8_t tx_lmp_port = 0;
static volatile uint8_t link_sta = 0;
volatile uint16_t SetupLen = 0;
volatile uint8_t SetupReqCode = 0;
uint8_t* volatile pDescr;
static volatile bool usb2_fallback_enabled = false;

// total length received
volatile uint16_t ep1_rx_total_length = 0;
volatile uint16_t ep2_rx_total_length = 0;
volatile uint16_t ep3_rx_total_length = 0;
volatile uint16_t ep4_rx_total_length = 0;
volatile uint16_t ep5_rx_total_length = 0;
volatile uint16_t ep6_rx_total_length = 0;
volatile uint16_t ep7_rx_total_length = 0;

// how many burst the host was told it could send at most
volatile uint8_t ep1_rx_previously_set_max_burst = 0;
volatile uint8_t ep2_rx_previously_set_max_burst = 0;
volatile uint8_t ep3_rx_previously_set_max_burst = 0;
volatile uint8_t ep4_rx_previously_set_max_burst = 0;
volatile uint8_t ep5_rx_previously_set_max_burst = 0;
volatile uint8_t ep6_rx_previously_set_max_burst = 0;
volatile uint8_t ep7_rx_previously_set_max_burst = 0;

volatile uint16_t ep1_tx_remaining_length = 0;
volatile uint16_t ep2_tx_remaining_length = 0;
volatile uint16_t ep3_tx_remaining_length = 0;
volatile uint16_t ep4_tx_remaining_length = 0;
volatile uint16_t ep5_tx_remaining_length = 0;
volatile uint16_t ep6_tx_remaining_length = 0;
volatile uint16_t ep7_tx_remaining_length = 0;
volatile uint8_t ep1_tx_total_bursts = 0;
volatile uint8_t ep2_tx_total_bursts = 0;
volatile uint8_t ep3_tx_total_bursts = 0;
volatile uint8_t ep4_tx_total_bursts = 0;
volatile uint8_t ep5_tx_total_bursts = 0;
volatile uint8_t ep6_tx_total_bursts = 0;
volatile uint8_t ep7_tx_total_bursts = 0;

/***************************************************************************
 * @fn     USB3_force
 *
 * @brief  Switch to USB3 or do a fallback to USB2 if not available
 *
 * @return None
 */
void usb30_device_init(bool enable_usb2_fallback)
{
	usb2_fallback_enabled = enable_usb2_fallback;
	PFIC_EnableIRQ(USBSS_IRQn);
	PFIC_EnableIRQ(LINK_IRQn);

	// Enable USB
	USBSS->LINK_CFG = 0x140;
	USBSS->LINK_CTRL = 0x12;
	uint32_t t = 0x4c4b41;
	while (USBSS->LINK_STATUS & 4)
	{
		t--;
		if (t == 0)
		{
			while (1)
			{
			}
		}
	}
	for (int i = 0; i < 8; i++)
	{
		SS_TX_CONTRL(i) = 0;
		SS_RX_CONTRL(i) = 0;
	}
	USBSS->USB_STATUS = 0x13;
	USBSS->USB_CONTROL = 0x30021;
	USBSS->UEP_CFG = 0;
	USBSS->LINK_CFG |= 2;
	USBSS->LINK_INT_CTRL = 0x10bc7d;
	USBSS->LINK_CTRL = 2;

	usb30_init_endpoints();

	LOG_IF(LOG_LEVEL_DEBUG, LOG_ID_USB3,
		   "Finished initializing USB3 device \r\n");
}

void usb30_device_deinit(void)
{
	LOG_IF(LOG_LEVEL_DEBUG, LOG_ID_USB3, "Disabling USB3 device\r\n");
	PFIC_DisableIRQ(USBSS_IRQn);
	PFIC_DisableIRQ(LINK_IRQn);
	// Disable USB
	usb30_switch_powermode(POWER_MODE_2);
	USBSS->LINK_CFG = PIPE_RESET | LFPS_RX_PD;
	USBSS->LINK_CTRL = GO_DISABLED | POWER_MODE_3;
	USBSS->LINK_INT_CTRL = 0;
	USBSS->USB_CONTROL = USB_FORCE_RST | USB_ALL_CLR;
}

/*******************************************************************************
 * @fn     USB30_bus_reset
 *
 * @brief  USB3.0 bus reset, select the reset method in the code,
 *         it is recommended to use the method of only resetting the USB,
 *         if there is a problem with the connection,
 *         you can use the method of resetting the MCU
 *
 * @return None
 */
__attribute__((always_inline)) static inline void USB30_BusReset(void);
__attribute__((always_inline)) static inline void USB30_BusReset(void)
{
	usb30_device_deinit(); // USB3.0 initialization
	bsp_wait_ms_delay(20);
	usb30_device_init(usb2_fallback_enabled); // USB3.0 initialization
}

/*******************************************************************************
 * @fn     USB30D_init
 *
 * @brief  USB3.0 device initialization
 *
 * @return None
 */

void usb30_init_endpoints(void)
{
	USBSS->UEP_CFG = EP0_R_EN | EP0_T_EN; // set end point rx/tx enable
	USBSS->UEP0_DMA = (uint32_t)(uint8_t*)usb3_backend_current_device->endpoints.rx[0].buffer;

	if (usb3_backend_current_device->endpoint_mask & ENDPOINT_1_TX)
	{
		USBSS->UEP_CFG |= EP1_T_EN;
		USBSS->UEP1_TX_DMA = (uint32_t)(uint8_t*)usb3_backend_current_device->endpoints.tx[1].buffer;
		usb30_in_set(ENDP_1, DISABLE, NRDY, 0, 0);
		usb3_backend_current_device->endpoints.tx[1].state = ENDP_STATE_NAK;
	}
	if (usb3_backend_current_device->endpoint_mask & ENDPOINT_1_RX)
	{
		USBSS->UEP_CFG |= EP1_R_EN;
		USBSS->UEP1_RX_DMA = (uint32_t)(uint8_t*)usb3_backend_current_device->endpoints.rx[1].buffer;
		usb30_out_set(ENDP_1, ACK, usb3_backend_current_device->endpoints.rx[1].max_burst);
		ep1_rx_previously_set_max_burst = usb3_backend_current_device->endpoints.rx[1].max_burst;
		usb3_backend_current_device->endpoints.rx[1].state = ENDP_STATE_ACK;
	}

	if (usb3_backend_current_device->endpoint_mask & ENDPOINT_2_TX)
	{
		USBSS->UEP_CFG |= EP2_T_EN;
		USBSS->UEP2_TX_DMA = (uint32_t)(uint8_t*)usb3_backend_current_device->endpoints.tx[2].buffer;
		usb30_in_set(ENDP_2, DISABLE, NRDY, 0, 0);
		usb3_backend_current_device->endpoints.tx[2].state = ENDP_STATE_NAK;
	}
	if (usb3_backend_current_device->endpoint_mask & ENDPOINT_2_RX)
	{
		USBSS->UEP_CFG |= EP2_R_EN;
		USBSS->UEP2_RX_DMA = (uint32_t)(uint8_t*)usb3_backend_current_device->endpoints.rx[2].buffer;
		usb30_out_set(ENDP_2, ACK, usb3_backend_current_device->endpoints.rx[2].max_burst);
		ep2_rx_previously_set_max_burst = usb3_backend_current_device->endpoints.rx[2].max_burst;
		usb3_backend_current_device->endpoints.rx[2].state = ENDP_STATE_ACK;
	}

	if (usb3_backend_current_device->endpoint_mask & ENDPOINT_3_TX)
	{
		USBSS->UEP_CFG |= EP3_T_EN;
		USBSS->UEP3_TX_DMA = (uint32_t)(uint8_t*)usb3_backend_current_device->endpoints.tx[3].buffer;
		usb30_in_set(ENDP_3, DISABLE, NRDY, 0, 0);
		usb3_backend_current_device->endpoints.tx[3].state = ENDP_STATE_NAK;
	}
	if (usb3_backend_current_device->endpoint_mask & ENDPOINT_3_RX)
	{
		USBSS->UEP_CFG |= EP3_R_EN;
		USBSS->UEP3_RX_DMA = (uint32_t)(uint8_t*)usb3_backend_current_device->endpoints.rx[3].buffer;
		usb30_out_set(ENDP_3, ACK, usb3_backend_current_device->endpoints.rx[3].max_burst);
		ep3_rx_previously_set_max_burst = usb3_backend_current_device->endpoints.rx[3].max_burst;
		usb3_backend_current_device->endpoints.rx[3].state = ENDP_STATE_ACK;
	}

	if (usb3_backend_current_device->endpoint_mask & ENDPOINT_4_TX)
	{
		USBSS->UEP_CFG |= EP4_T_EN;
		USBSS->UEP4_TX_DMA = (uint32_t)(uint8_t*)usb3_backend_current_device->endpoints.tx[4].buffer;
		usb30_in_set(ENDP_4, DISABLE, NRDY, 0, 0);
		usb3_backend_current_device->endpoints.tx[4].state = ENDP_STATE_NAK;
	}
	if (usb3_backend_current_device->endpoint_mask & ENDPOINT_4_RX)
	{
		USBSS->UEP_CFG |= EP4_R_EN;
		USBSS->UEP4_RX_DMA = (uint32_t)(uint8_t*)usb3_backend_current_device->endpoints.rx[4].buffer;
		usb30_out_set(ENDP_4, ACK, usb3_backend_current_device->endpoints.rx[4].max_burst);
		ep4_rx_previously_set_max_burst = usb3_backend_current_device->endpoints.rx[4].max_burst;
		usb3_backend_current_device->endpoints.rx[4].state = ENDP_STATE_ACK;
	}

	if (usb3_backend_current_device->endpoint_mask & ENDPOINT_5_TX)
	{
		USBSS->UEP_CFG |= EP5_T_EN;
		USBSS->UEP5_TX_DMA = (uint32_t)(uint8_t*)usb3_backend_current_device->endpoints.tx[5].buffer;
		usb30_in_set(ENDP_5, DISABLE, NRDY, 0, 0);
		usb3_backend_current_device->endpoints.tx[5].state = ENDP_STATE_NAK;
	}
	if (usb3_backend_current_device->endpoint_mask & ENDPOINT_5_RX)
	{
		USBSS->UEP_CFG |= EP5_R_EN;
		USBSS->UEP5_RX_DMA = (uint32_t)(uint8_t*)usb3_backend_current_device->endpoints.rx[5].buffer;
		usb30_out_set(ENDP_5, ACK, usb3_backend_current_device->endpoints.rx[5].max_burst);
		ep5_rx_previously_set_max_burst = usb3_backend_current_device->endpoints.rx[5].max_burst;
		usb3_backend_current_device->endpoints.rx[5].state = ENDP_STATE_ACK;
	}

	if (usb3_backend_current_device->endpoint_mask & ENDPOINT_6_TX)
	{
		USBSS->UEP_CFG |= EP6_T_EN;
		USBSS->UEP6_TX_DMA = (uint32_t)(uint8_t*)usb3_backend_current_device->endpoints.tx[6].buffer;
		usb30_in_set(ENDP_6, DISABLE, NRDY, 0, 0);
		usb3_backend_current_device->endpoints.tx[6].state = ENDP_STATE_NAK;
	}
	if (usb3_backend_current_device->endpoint_mask & ENDPOINT_6_RX)
	{
		USBSS->UEP_CFG |= EP6_R_EN;
		USBSS->UEP6_RX_DMA = (uint32_t)(uint8_t*)usb3_backend_current_device->endpoints.rx[6].buffer;
		usb30_out_set(ENDP_6, ACK, usb3_backend_current_device->endpoints.rx[6].max_burst);
		ep6_rx_previously_set_max_burst = usb3_backend_current_device->endpoints.rx[6].max_burst;
		usb3_backend_current_device->endpoints.rx[6].state = ENDP_STATE_ACK;
	}

	if (usb3_backend_current_device->endpoint_mask & ENDPOINT_7_TX)
	{
		USBSS->UEP_CFG |= EP7_T_EN;
		USBSS->UEP7_TX_DMA = (uint32_t)(uint8_t*)usb3_backend_current_device->endpoints.tx[7].buffer;
		usb30_in_set(ENDP_7, DISABLE, NRDY, 0, 0);
		usb3_backend_current_device->endpoints.tx[7].state = ENDP_STATE_NAK;
	}
	if (usb3_backend_current_device->endpoint_mask & ENDPOINT_7_RX)
	{
		USBSS->UEP_CFG |= EP7_R_EN;
		USBSS->UEP7_RX_DMA = (uint32_t)(uint8_t*)usb3_backend_current_device->endpoints.rx[7].buffer;
		usb30_out_set(ENDP_7, ACK, usb3_backend_current_device->endpoints.rx[7].max_burst);
		ep7_rx_previously_set_max_burst = usb3_backend_current_device->endpoints.rx[7].max_burst;
		usb3_backend_current_device->endpoints.rx[7].state = ENDP_STATE_ACK;
	}

	if (usb3_backend_current_device->endpoint_mask & (UNSUPPORTED_ENDPOINTS))
	{
		LOG_IF(LOG_LEVEL_DEBUG, LOG_ID_USB3, "Unsupported endpoints \r\n");
	}
}

void usb30_reinit_endpoints(void)
{
	usb30_in_clear_interrupt_all(ENDP_1);
	usb30_out_clear_interrupt_all(ENDP_1);
	usb30_in_clear_interrupt_all(ENDP_2);
	usb30_out_clear_interrupt_all(ENDP_2);
	usb30_in_clear_interrupt_all(ENDP_3);
	usb30_out_clear_interrupt_all(ENDP_3);
	usb30_in_clear_interrupt_all(ENDP_4);
	usb30_out_clear_interrupt_all(ENDP_4);
	usb30_in_clear_interrupt_all(ENDP_5);
	usb30_out_clear_interrupt_all(ENDP_5);
	usb30_in_clear_interrupt_all(ENDP_6);
	usb30_out_clear_interrupt_all(ENDP_6);
	usb30_in_clear_interrupt_all(ENDP_7);
	usb30_out_clear_interrupt_all(ENDP_7);

	usb30_init_endpoints();
}

/**
 * @fn      usb30_standard_req_handler
 * @brief  USB Device Mode Standard Request Command Handling
 * @return   The length of data that the host requests the device to send
 */
__attribute__((always_inline)) static inline uint16_t
usb30_standard_req_handler(void)
{
	SetupReqCode = UsbSetupBuf->bRequest;
	SetupLen = UsbSetupBuf->wLength;
	uint16_t len = 0;

	switch (SetupReqCode)
	{
	case USB_GET_DESCRIPTOR:
		switch (UsbSetupBuf->wValue.bw.bb0)
		{
		case USB_DESCR_TYP_DEVICE: /* Get device descriptor */
			if (usb3_backend_current_device->usb_descriptors.usb3_device_descr == 0)
			{
				LOG_IF(LOG_LEVEL_DEBUG, LOG_ID_USB3,
					   "ERROR : device descriptor not set \r\n");
			}
			else
			{
				if (SetupLen > sizeof(*usb3_backend_current_device->usb_descriptors.usb3_device_descr))
					SetupLen = sizeof(*usb3_backend_current_device->usb_descriptors.usb3_device_descr);
				pDescr = (uint8_t*)usb3_backend_current_device->usb_descriptors.usb3_device_descr;
			}
			break;
		case USB_DESCR_TYP_CONFIG: /* Get configuration descriptor */
			if (usb3_backend_current_device->usb_descriptors.usb3_device_config_descrs == 0)
			{
				LOG_IF(LOG_LEVEL_DEBUG, LOG_ID_USB3,
					   "ERROR : config descriptor not set \r\n");
			}
			else
			{
				// when requesting the config descriptor, the host expect all
				// subordinate descriptors to be sent along with
				uint16_t usb_config_descr_total_size =
					((USB_CFG_DESCR*)(usb3_backend_current_device->usb_descriptors
										  .usb3_device_config_descrs[0]))
						->wTotalLength;
				if (SetupLen > usb_config_descr_total_size)
					SetupLen = usb_config_descr_total_size;
				pDescr =
					(uint8_t*)usb3_backend_current_device->usb_descriptors.usb3_device_config_descrs[0];
			}
			break;
		case USB_DESCR_TYP_BOS: /* Get BOS Descriptor */
			if (usb3_backend_current_device->usb_descriptors.usb_bos_descr == 0)
			{
				LOG_IF(LOG_LEVEL_DEBUG, LOG_ID_USB3,
					   "ERROR : BOS descriptor not set \r\n");
			}
			else
			{
				// when requesting the config descriptor, the host expect all
				// subordinate descriptors to be sent along with
				uint16_t usb_bos_descr_total_size =
					((USB_BOS_DESCR*)(usb3_backend_current_device->usb_descriptors.usb_bos_descr))
						->wTotalLength;
				if (SetupLen > usb_bos_descr_total_size)
					SetupLen = usb_bos_descr_total_size;
				pDescr = (uint8_t*)usb3_backend_current_device->usb_descriptors.usb_bos_descr;
			}
			break;
		case USB_DESCR_TYP_STRING:
			if (usb3_backend_current_device->usb_descriptors.usb_string_descrs == 0)
			{
				LOG_IF(LOG_LEVEL_DEBUG, LOG_ID_USB3,
					   "ERROR : str descriptor not set \r\n");
			}
			else
			{
				uint8_t str_descr_index = UsbSetupBuf->wValue.bw.bb1;
				LOG_IF(LOG_LEVEL_DEBUG, LOG_ID_USB3, "str descr index %d \r\n",
					   str_descr_index);
				// when requesting the config descriptor, the host expect all
				// subordinate descriptors to be sent along with
				uint16_t usb_str_descr_total_size =
					((USB_STRING_DESCR*)(usb3_backend_current_device->usb_descriptors
											 .usb_string_descrs[str_descr_index]))
						->bLength;
				if (SetupLen > usb_str_descr_total_size)
					SetupLen = usb_str_descr_total_size;
				pDescr =
					(uint8_t*)
						usb3_backend_current_device->usb_descriptors.usb_string_descrs[str_descr_index];
			}
			break;
		default:
			len = USB_DESCR_UNSUPPORTED; // unsupported descriptor
			SetupReqCode = INVALID_REQ_CODE;
			break;
		}
		if (len != USB_DESCR_UNSUPPORTED)
		{
			len = SetupLen >= ENDP0_MAX_PACKET_SIZE
					  ? ENDP0_MAX_PACKET_SIZE
					  : SetupLen; // The length of this transmission
			memcpy(usb3_backend_current_device->endpoints.rx[0].buffer, pDescr, len); // device  /* load upload data */
			SetupLen -= len;
			pDescr += len;
		}
		break;
	case USB_SET_ADDRESS: /* Set Address */
		SetupLen = UsbSetupBuf->wValue.bw
					   .bb1; // Temporarily store the address of the USB device
		break;
	case USB_GET_CONFIGURATION: // Get configuration value
		usb3_backend_current_device->endpoints.rx[0].buffer[0] = usb3_backend_current_device->current_config;
		SetupLen = 1;
		break;
	case USB_SET_CONFIGURATION:
		usb30_reinit_endpoints();
		usb3_backend_current_device->current_config = UsbSetupBuf->wValue.bw.bb1;
		if (UsbSetupBuf->wValue.bw.bb1 != 0)
			usb3_backend_current_device->state = CONFIGURED;
		break;
	case USB_CLEAR_FEATURE: /* TODO usb30_standard_req_handler() code
                             USB_CLEAR_FEATURE */
		break;
	case USB_SET_FEATURE: /* TODO usb30_standard_req_handler() code
                           USB_SET_FEATURE */
		break;
	case USB_GET_INTERFACE: /* TODO usb30_standard_req_handler() code
                             USB_GET_INTERFACE */
		break;
	case USB_SET_INTERFACE: /* TODO usb30_standard_req_handler() code
                             USB_SET_INTERFACE */
		break;
	case USB_GET_STATUS: /* TODO usb30_standard_req_handler() better code
                          USB_GET_STATUS */
		len = 2;
		usb3_backend_current_device->endpoints.rx[0].buffer[0] = 0x01;
		usb3_backend_current_device->endpoints.rx[0].buffer[1] = 0x00;
		SetupLen = 0;
		break;
	case 0x30: // Not documented TODO usb30_standard_req_handler() 0x30 remove or
		// document it
		break;
	case 0x31: // Not documented TODO usb30_standard_req_handler() 0x31 remove or
		// document it
		SetupLen = UsbSetupBuf->wValue.bw
					   .bb1; // Temporarily store the address of the USB device
		break;
	default:
		len = USB_DESCR_UNSUPPORTED; // return stall, unsupported command
		SetupReqCode = INVALID_REQ_CODE;
		break;
	}
	return len;
}

/**
 * @fn      ep0_status_stage_handler
 * @brief   Control Transfer Status stage handler
 * @return   None
 */
__attribute__((always_inline)) static inline void
ep0_status_stage_handler(void)
{
	switch (SetupReqCode)
	{
	case USB_SET_ADDRESS:
		usb3_backend_current_device->speed = USB30_SUPERSPEED;
		usb3_backend_current_device->state = ADDRESS;
		usb30_device_set_address(SetupLen); // SET ADDRESS
		break;
	}
}

/**
 * @fn      ep0_in_handler
 * @brief  endpoint0 IN Transfer completion handler function
 * @return The length of data sent in an IN transfer response
 */
__attribute__((always_inline)) static inline uint16_t ep0_in_handler(void)
{
	uint16_t len = 0;

	if (pDescr != NULL)
	{
		len = SetupLen >= ENDP0_MAX_PACKET_SIZE ? ENDP0_MAX_PACKET_SIZE : SetupLen;
		memcpy(usb3_backend_current_device->endpoints.rx[0].buffer, pDescr, len);
		SetupLen -= len;
		pDescr += len;
		/* Note USB_SET_ADDRESS is done in ep0_status_stage_handler() */
	}
	return len;
}

/**
 * @fn      ep0_out_handler
 * @brief  endpoint0 OUT Transfer completion handler function
 * @return   None
 */
__attribute__((always_inline)) static inline uint16_t ep0_out_handler(void);
__attribute__((always_inline)) static inline uint16_t ep0_out_handler(void)
{
	uint16_t len = 0;
	return len;
}

/***************Endpoint IN Transaction Processing*******************/

/**
 * @fn      usb30_ep_in_handler
 * @brief   IN transactions handler
 * @return   None
 */
__attribute__((always_inline)) static inline void
usb30_ep_in_handler(uint8_t endp_num)
{
	uint8_t nump;
	volatile uint8_t* tx_ep_total_bursts =
		usb30_get_tx_endpoint_total_bursts(endp_num);
	volatile USB_ENDPOINT* endp = &usb3_backend_current_device->endpoints.tx[endp_num];

#ifdef DEBUG
	if (endp == NULL || tx_ep_total_bursts == NULL)
		return;
#endif

	nump = usb30_in_nump(endp_num);
	uint8_t num_packet_sent = *tx_ep_total_bursts - nump;

	LOG_IF(LOG_LEVEL_DEBUG, LOG_ID_USB3,
		   "IN transfer on ep %d, nump_packet_sent %d, total burst %d, remaining "
		   "length %d, nump %d \r\n",
		   endp_num, num_packet_sent,
		   *usb30_get_tx_endpoint_total_bursts(endp_num),
		   *usb30_get_tx_endpoint_remaining_length(endp_num), nump);

	for (int i = 0; i < num_packet_sent; ++i)
	{
		if (*usb30_get_tx_endpoint_remaining_length(endp_num) >=
			endp->max_packet_size)
		{
			*usb30_get_tx_endpoint_remaining_length(endp_num) -=
				endp->max_packet_size;
		}
		else
		{
			*usb30_get_tx_endpoint_remaining_length(endp_num) = 0;
		}
	}

	if (*usb30_get_tx_endpoint_remaining_length(endp_num) == 0)
	{
		LOG_IF(LOG_LEVEL_DEBUG, LOG_ID_USB3,
			   "Finished IN transfer on ep %d, remaining length %d\r\n", endp_num,
			   *usb30_get_tx_endpoint_remaining_length(endp_num));

		usb30_in_clear_interrupt(
			endp_num); // Clear endpoint state Keep only packet sequence number
		usb3_backend_current_device->endpoints.tx_complete[endp_num](Ack); // set new data before we're ready for more
		if (usb30_in_nump(endp_num) ==
			0)
		{ // do not send NRDY if new data has already been set
			usb30_in_set(endp_num | IN, DISABLE, NRDY, 0, 0);
			*usb30_get_tx_endpoint_addr_reg(endp_num) =
				(uint32_t)(uint8_t*)0; // Burst transfer DMA address offset Need to reset
		}
	}
	else
	{
		if (nump > endp->max_burst)
			nump = endp->max_burst;

		// There is still nump packet left to be sent; during the burst process, the
		// host may not be able to take all the data packets at one time. Therefore,
		// it is necessary to determine the current number of remaining packets and
		// notify the host of how many packets are left to be taken.
		usb30_in_clear_interrupt(
			endp_num); // Clear endpoint state Keep only packet sequence number
		// Full packets are sent first, then an eventual partial packet if there are
		// remaining bytes
		usb30_in_set(endp_num, ENABLE, ACK, nump,
					 *usb30_get_tx_endpoint_remaining_length(endp_num) >=
							 nump * endp->max_packet_size
						 ? endp->max_packet_size
						 : *usb30_get_tx_endpoint_remaining_length(endp_num) -
							   (nump - 1) * endp->max_packet_size);

		// Set NumP again to the remaining number of packets that can be sent, in
		// case the host reset it to bMaxBurst Note that the NumP sent with ERDY can
		// be ignored by the host, so it's mostly informative
		usb30_send_erdy(endp_num | IN, nump);
		*tx_ep_total_bursts -= num_packet_sent;
	}
}

/*************** Endpointn OUT Transaction Processing *******************/

/**
 * @fn      usb30_ep_out_handler
 * @brief   OUT transactions handler
 * @return   None
 */
__attribute__((always_inline)) static inline void
usb30_ep_out_handler(uint8_t endp_num)
{
	uint16_t rx_len;
	uint8_t nump;
	uint8_t nump_sent;
	uint8_t status;
	volatile uint8_t* rx_ep_previously_set_max_burst =
		usb30_get_rx_endpoint_prev_set_max_burst(endp_num);
	volatile USB_ENDPOINT* endp = &usb3_backend_current_device->endpoints.rx[endp_num];

#ifdef DEBUG
	if (endp == NULL || rx_ep_previously_set_max_burst == NULL)
		return;
#endif

	usb30_out_status(endp_num, &nump, &rx_len,
					 &status); // Get the number of received packets rxlen is the
	// packet length of the last packet

	// assuming the backend was configured to received
	// Rx_Previously_Set_Max_Burst, the number of sent packets can be computed
	nump_sent = *rx_ep_previously_set_max_burst - nump;

	// status == 1 means the last packet was incomplete (nump-sent - 1 full
	// packets + one packet of size rx_len) status == 0 means the last packet
	// received was complete (at least this seems to be the case)
	if (status != 1)
	{
		*usb30_get_rx_endpoint_total_length(endp_num) +=
			endp->max_packet_size * (nump_sent);
	}
	else
	{
		*usb30_get_rx_endpoint_total_length(endp_num) +=
			endp->max_packet_size * (nump_sent - 1) + rx_len;
	}

#if (defined LOG_ID_USB3)
	LOG_IF(
		LOG_LEVEL_DEBUG, LOG_ID_USB3,
		"endp_num %d, nump %d, true nump %d, buffer %p, max_burst %d,  "
		"previously set max burst %d,  rx_len %d, status %d, total length %d\r\n",
		endp_num, nump, nump_sent, endp->buffer, endp->max_burst,
		*usb30_get_rx_endpoint_prev_set_max_burst(endp_num), rx_len, status,
		*usb30_get_rx_endpoint_total_length(endp_num));
#endif

	if (status & 0x01 || nump == 0 || status == 0)
	{
		// All received

		// Set the endpoint to not ready while processing

		usb30_out_set(endp_num, NRDY, 0);

		usb3_backend_current_device->endpoints.rx_callback[endp_num](endp->buffer,
																	 *usb30_get_rx_endpoint_total_length(
																		 endp_num)); // process data before receiving more

		// Prepare for next packets
		usb30_out_clear_interrupt(endp_num); // Clear all state of the endpoint Keep
			// only the packet sequence

		// Reset OUT transaction state
		*usb30_get_rx_endpoint_addr_reg(endp_num) =
			(uint32_t)(uint8_t*)
				endp->buffer; // In burst mode, the address needs to be reset due to
		// automatic address offset.
		*rx_ep_previously_set_max_burst = endp->max_burst;
		*usb30_get_rx_endpoint_total_length(endp_num) = 0;

		// Set the endpoint as ready
		usb30_out_set(endp_num, ACK,
					  endp->max_burst); // Able to send endp_rx.max_burst packets
		usb30_send_erdy(
			endp_num | OUT,
			endp->max_burst); // Notify the host to take endp_rx.max_burst packets
	}
	else
	{
		if (nump > endp->max_burst)
			nump = endp->max_burst;

		usb30_out_clear_interrupt(endp_num); // Clear all state of the endpoint Keep
			// only the packet sequence
		*rx_ep_previously_set_max_burst = nump;
		usb30_out_set(endp_num, ACK, nump); // Able to receive nump packet
		usb30_send_erdy(endp_num | OUT, nump);
	}
}

/*******************************************************************************
 * @fn     usb30_itp_callback
 *
 * @brief  USB3.0 Isochronous Timestamp Packet (ITP) Callback
 *
 * @return None
 */
void usb30_itp_callback(uint32_t ITPCounter) {}

void usb3_endp_tx_ready(uint8_t endp_num, uint16_t size)
{
	volatile USB_ENDPOINT* endp = &usb3_backend_current_device->endpoints.tx[endp_num];
	vuint32_t* tx_ep_addr_reg = usb30_get_tx_endpoint_addr_reg(endp_num);

#ifdef DEBUG
	if (endp == NULL || tx_ep_addr_reg == NULL)
		return;
#endif

	uint16_t max_full_packets = size / endp->max_packet_size;
	uint16_t remainder = size % endp->max_packet_size;
	uint16_t last_packet_size;

	if (max_full_packets > 0 && remainder == 0)
	{
		last_packet_size = endp->max_packet_size;
	}
	else
	{
		last_packet_size = remainder;
	}
	uint8_t total_num_packets =
		max_full_packets + (last_packet_size != endp->max_packet_size ? 1 : 0);

	*tx_ep_addr_reg =
		(uint32_t)(uint8_t*)
			endp->buffer; // Burst transfer DMA address offset Need to reset
	// prepare state of IN transaction
	*usb30_get_tx_endpoint_remaining_length(endp_num) = size;
	*usb30_get_tx_endpoint_total_bursts(endp_num) = total_num_packets;
	usb30_in_set(
		endp_num, ENABLE, ACK, total_num_packets,
		last_packet_size); // Set the endpoint to be able to send 4 packets
	LOG_IF(LOG_LEVEL_DEBUG, LOG_ID_USB3, "IN bursts %d last packet size %d \r\n",
		   total_num_packets, last_packet_size);

	usb30_send_erdy(endp_num | IN,
					total_num_packets); // Notify the host that this endpoint is
	// ready for burst transfer
}

/*******************************************************************************
 * @fn     LINK_IRQHandler
 *
 * @brief  USB3.0 Link Interrupt Handler.
 *
 * @return None
 */
__attribute__((interrupt("WCH-Interrupt-fast"))) void LINK_IRQHandler(void)
{
	if (USBSS->LINK_INT_FLAG & LINK_Ux_EXIT_FLAG) // device enter U2
	{
		USBSS->LINK_CFG = CFG_EQ_EN | DEEMPH_CFG | TERM_EN;
		usb30_switch_powermode(POWER_MODE_0);
		USBSS->LINK_INT_FLAG = LINK_Ux_EXIT_FLAG;
	}
	if (USBSS->LINK_INT_FLAG & LINK_RDY_FLAG) // POLLING SHAKE DONE
	{
		USBSS->LINK_INT_FLAG = LINK_RDY_FLAG;
		if (tx_lmp_port) // LMP, TX PORT_CAP & RX PORT_CAP
		{
			USBSS->LMP_TX_DATA0 = LINK_SPEED | PORT_CAP | LMP_HP;
			USBSS->LMP_TX_DATA1 = UP_STREAM | NUM_HP_BUF;
			USBSS->LMP_TX_DATA2 = 0x0;
			tx_lmp_port = 0;
			usb3_backend_current_device->state = DEFAULT;
			usb3_backend_current_device->speed = USB30_SUPERSPEED;
		}
		// Successful USB3.0 communication
		link_sta = 3;
	}

	if (USBSS->LINK_INT_FLAG & LINK_INACT_FLAG)
	{
		USBSS->LINK_INT_FLAG = LINK_INACT_FLAG;
		usb30_switch_powermode(POWER_MODE_2);
	}
	if (USBSS->LINK_INT_FLAG &
		LINK_DISABLE_FLAG) // GO DISABLED. Happens either because there is no
	// cable, or USB3.x is not supported
	{
		USBSS->LINK_INT_FLAG = LINK_DISABLE_FLAG;
		usb3_backend_current_device->state = DETACHED;
		link_sta = 1;

		usb30_device_deinit();

		if (!usb2_fallback_enabled)
		{ // try to USB3 init in a loop, up until a USB3
			// link is detected
			bsp_wait_ms_delay(10);
			usb30_device_init(usb2_fallback_enabled);
			return;
		}
		PFIC_DisableIRQ(USBSS_IRQn);
		PFIC_DisableIRQ(LINK_IRQn);
		usb2_device_init();
	}
	if (USBSS->LINK_INT_FLAG & LINK_RX_DET_FLAG)
	{
		USBSS->LINK_INT_FLAG = LINK_RX_DET_FLAG;
		usb30_switch_powermode(POWER_MODE_2);
	}
	if (USBSS->LINK_INT_FLAG & TERM_PRESENT_FLAG) // term present , begin POLLING
	{
		USBSS->LINK_INT_FLAG = TERM_PRESENT_FLAG;
		if (USBSS->LINK_STATUS & LINK_PRESENT)
		{
			usb30_switch_powermode(POWER_MODE_2);
			USBSS->LINK_CTRL |= POLLING_EN;
		}
		else
		{
			USBSS->LINK_INT_CTRL = 0;
			bsp_wait_us_delay(2000);
			USB30_BusReset();
			usb3_backend_current_device->state = DEFAULT;
		}
	}
	if (USBSS->LINK_INT_FLAG & LINK_TXEQ_FLAG) // POLLING SHAKE DONE
	{
		tx_lmp_port = 1;
		USBSS->LINK_INT_FLAG = LINK_TXEQ_FLAG;
		usb30_switch_powermode(POWER_MODE_0);
	}
	if (USBSS->LINK_INT_FLAG & WARM_RESET_FLAG)
	{
		USBSS->LINK_INT_FLAG = WARM_RESET_FLAG;
		usb30_switch_powermode(POWER_MODE_2);
		USB30_BusReset();
		usb30_device_set_address(0);
		USBSS->LINK_CTRL |= TX_WARM_RESET;
		while (USBSS->LINK_STATUS & RX_WARM_RESET)
			;
		USBSS->LINK_CTRL &= ~TX_WARM_RESET;
		bsp_wait_us_delay(2);
	}
	if (USBSS->LINK_INT_FLAG &
		HOT_RESET_FLAG) // The host may issue a hot reset, pay attention to the
	// configuration of the endpoint
	{
		USBSS->USB_CONTROL |= ((uint32_t)1) << 31;
		USBSS->LINK_INT_FLAG = HOT_RESET_FLAG; // HOT RESET begin
		USBSS->UEP0_TX_CTRL = 0;

		if (usb3_backend_current_device->endpoint_mask & ENDPOINT_1_TX)
		{
			usb30_in_set(ENDP_1, DISABLE, NRDY, 0, 0);
		}
		if (usb3_backend_current_device->endpoint_mask & ENDPOINT_1_RX)
		{
			usb30_out_set(ENDP_1, NRDY, 0);
		}

		if (usb3_backend_current_device->endpoint_mask & ENDPOINT_2_TX)
		{
			usb30_in_set(ENDP_2, DISABLE, NRDY, 0, 0);
		}
		if (usb3_backend_current_device->endpoint_mask & ENDPOINT_2_RX)
		{
			usb30_out_set(ENDP_2, NRDY, 0);
		}

		if (usb3_backend_current_device->endpoint_mask & ENDPOINT_3_TX)
		{
			usb30_in_set(ENDP_3, DISABLE, NRDY, 0, 0);
		}
		if (usb3_backend_current_device->endpoint_mask & ENDPOINT_3_RX)
		{
			usb30_out_set(ENDP_3, NRDY, 0);
		}

		if (usb3_backend_current_device->endpoint_mask & ENDPOINT_4_TX)
		{
			usb30_in_set(ENDP_4, DISABLE, NRDY, 0, 0);
		}
		if (usb3_backend_current_device->endpoint_mask & ENDPOINT_4_RX)
		{
			usb30_out_set(ENDP_4, NRDY, 0);
		}

		if (usb3_backend_current_device->endpoint_mask & ENDPOINT_5_TX)
		{
			usb30_in_set(ENDP_5, DISABLE, NRDY, 0, 0);
		}
		if (usb3_backend_current_device->endpoint_mask & ENDPOINT_5_RX)
		{
			usb30_out_set(ENDP_5, NRDY, 0);
		}

		if (usb3_backend_current_device->endpoint_mask & ENDPOINT_6_TX)
		{
			usb30_in_set(ENDP_6, DISABLE, NRDY, 0, 0);
		}
		if (usb3_backend_current_device->endpoint_mask & ENDPOINT_6_RX)
		{
			usb30_out_set(ENDP_6, NRDY, 0);
		}

		if (usb3_backend_current_device->endpoint_mask & ENDPOINT_7_TX)
		{
			usb30_in_set(ENDP_7, DISABLE, NRDY, 0, 0);
		}
		if (usb3_backend_current_device->endpoint_mask & ENDPOINT_7_RX)
		{
			usb30_out_set(ENDP_7, NRDY, 0);
		}

		usb30_device_set_address(0);
		USBSS->LINK_CTRL &= ~TX_HOT_RESET; // HOT RESET end
	}
	if (USBSS->LINK_INT_FLAG & LINK_GO_U1_FLAG) // device enter U1
	{
		usb30_switch_powermode(POWER_MODE_1);
		USBSS->LINK_INT_FLAG = LINK_GO_U1_FLAG;
	}
	if (USBSS->LINK_INT_FLAG & LINK_GO_U2_FLAG) // device enter U2
	{
		usb30_switch_powermode(POWER_MODE_2);
		USBSS->LINK_INT_FLAG = LINK_GO_U2_FLAG;
	}
	if (USBSS->LINK_INT_FLAG & LINK_GO_U3_FLAG) // device enter U2
	{
		usb30_switch_powermode(POWER_MODE_2);
		USBSS->LINK_INT_FLAG = LINK_GO_U3_FLAG;
	}
	return;
}

/*******************************************************************************
 * @fn     USBSS_IRQHandler
 *
 * @brief  USB3.0 Interrupt Handler.
 *
 * @return None
 */
__attribute__((interrupt("WCH-Interrupt-fast"))) void USBSS_IRQHandler(void)
{
	static uint32_t count = 0;

	vuint32_t usb_status = USBSS->USB_STATUS;

	if ((usb_status & 1) == 0)
	{
		if ((usb_status & 2) != 0)
		{
			if ((USBSS->LMP_RX_DATA0 & 0x1e0) == 0xa0) // 0b111100000(LMP subtype) -> 0xa0 = 0b101(Port configuration subtype) 00000(LMP packet type)
			{
				USBSS->LMP_TX_DATA0 = 0x2c0; // 0b1(accept link speed setting) 0110(Port Configuration Response subtype) 00000(LMP packet type)
				USBSS->LMP_TX_DATA1 = 0;
				USBSS->LMP_TX_DATA2 = 0;
			}
			USBSS->USB_STATUS = 2;
			return;
		}
		if ((usb_status & 8) == 0)
		{
			return;
		}
		usb30_itp_callback(USBSS->USB_ITP);
		USBSS->USB_STATUS = 8;
		return;
	}

	// USB3 EP1 to EP7 management
	uint8_t ep = (usb_status >> 8) & 7;
	uint32_t ep_in = (usb_status >> 0xc) & 1;
	if (ep != 0)
	{
		if (ep_in == 0)
		{
			switch (ep)
			{
			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
				usb30_ep_out_handler(ep);
				break;
			}
			return;
		}
		switch (ep)
		{
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
			usb30_ep_in_handler(ep);
			break;
		}
		return;
	}

	// USB3 EP0 management
	uint16_t req_len;
	if (ep_in != 0)
	{
		req_len = ep0_in_handler();
		count = count + 1;
		if (req_len == 0)
		{
			count = 0;
			USBSS->USB_FC_CTRL = 0x41; // USB30_send_ERDY(ENDP_0 | OUT, 1);
			USBSS->UEP0_RX_CTRL = 0x4010000; // USBSS->UEP0_RX_CTRL = 0x4010000;
				// //USB30_OUT_set(0, ACK, 1);
			USBSS->UEP0_TX_CTRL = 0x14010000; // USB30_IN_set(0, ENABLE, ACK, 1, 0);
			return;
		}
		USBSS->USB_FC_CTRL = 0x41; // USB30_send_ERDY(ENDP_0 | OUT, 1);
		USBSS->UEP0_RX_CTRL = 0;
		// 0x14010000 ~= USB30_IN_set(0, ENABLE, STALL, 1, ACK)
		USBSS->UEP0_TX_CTRL = (USBSS->UEP0_TX_CTRL & 0xec1ffc00) |
							  (count * 0x200000) | req_len | 0x14010000;
		return;
	}

	uint32_t uep0_rx_ctrl = USBSS->UEP0_RX_CTRL;
	if (-1 < ((int32_t)(uep0_rx_ctrl << 1)))
	{
		if (((int32_t)(uep0_rx_ctrl << 2)) < 0)
		{
			ep0_status_stage_handler();
			USBSS->UEP0_RX_CTRL = 0;
			USBSS->UEP0_TX_CTRL = 0;
			return;
		}
		ep0_out_handler();
		USBSS->USB_FC_CTRL = 0x41; // USB30_send_ERDY(ENDP_0 | OUT, 1);
		USBSS->UEP0_RX_CTRL = 0x4010000;
		return;
	}
	USBSS->UEP0_RX_CTRL = 0;
	uint8_t data_req = *((uint8_t*)usb3_backend_current_device->endpoints.rx[0].buffer);
	if ((data_req & 0x60) == 0)
	{
		req_len = usb30_standard_req_handler();
	}
	else
	{
		// this branch is manually inlined because call to user implemented func
		// prevents automatic inlining
		SetupReqCode = UsbSetupBuf->bRequest;
		SetupLen = UsbSetupBuf->wLength;
		uint8_t endp_dir = UsbSetupBuf->bRequestType & 0x80;
		uint8_t* buffer = NULL;

		req_len = usb3_backend_current_device->endpoints.endp0_user_handled_control_request(
			UsbSetupBuf, &buffer);

		SetupLen = req_len;
		// handle IN non standard requests
		if (req_len != USB_DESCR_UNSUPPORTED && endp_dir)
		{
			if (buffer == NULL)
			{
				req_len = USB_DESCR_UNSUPPORTED;
			}
			else
			{
				pDescr = buffer;
				uint16_t len = SetupLen >= ENDP0_MAX_PACKET_SIZE
								   ? ENDP0_MAX_PACKET_SIZE
								   : SetupLen; // The length of this transmission
				memcpy(usb3_backend_current_device->endpoints.rx[0].buffer, pDescr, len); // device  /* load )upload data */
				pDescr += len;
				SetupLen -= len;
				req_len = len;
			}
		}
		else if (req_len != USB_DESCR_UNSUPPORTED && !endp_dir)
		{
			req_len = 0; // does not work if != 0 ?
		}
	}

	if ((char)data_req < '\0')
	{
		if (req_len == 0xffff)
		{
			USBSS->USB_FC_CTRL = 0x41; // USB30_send_ERDY(ENDP_0 | OUT, 1);
			USBSS->UEP0_TX_CTRL = 0x8010000; // USB30_IN_set(0, DISABLE, STALL, 1, 0);
			return;
		}
		else
		{
			if (0x200 < req_len)
			{
				return;
			}
			USBSS->USB_FC_CTRL = 0x41; // USB30_send_ERDY(ENDP_0 | OUT, 1);
			USBSS->UEP0_TX_CTRL =
				req_len | 0x14010000; // USB30_IN_set(0, ENABLE, ACK, 1, req_len);
			return;
		}
	}
	else
	{
		if (req_len == 0xffff)
		{
			USBSS->USB_FC_CTRL = 0x41; // USB30_send_ERDY(ENDP_0 | OUT, 1);
			USBSS->UEP0_RX_CTRL = 0x8010000; // USB30_IN_set(0, DISABLE, STALL, 1, 0);
			return;
		}
		else
		{
			if (0x200 < req_len)
			{
				return;
			}
			USBSS->USB_FC_CTRL = 0x41;
			USBSS->UEP0_RX_CTRL = 0x4010000; // USB30_OUT_set(0, ACK, 1);
			return;
		}
	}
	USBSS->USB_FC_CTRL = 0x41; // USB30_send_ERDY(ENDP_0 | OUT, 1);
	USBSS->UEP0_RX_CTRL = 0x4010000; // USB30_OUT_set(0, ACK, 1);
	return;
}
