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

#include "wch-ch56x-lib/USBDevice/usb20.h"
#include "wch-ch56x-lib/USBDevice/usb30.h"
#include "wch-ch56x-lib/USBDevice/usb_device.h"
#include "wch-ch56x-lib/USBDevice/usb_endpoints.h"

#define USB2_UNSUPPORTED_ENDPOINTS                                        \
	((ENDPOINT_8_TX | ENDPOINT_8_RX | ENDPOINT_9_TX | ENDPOINT_9_RX |     \
	  ENDPOINT_10_TX | ENDPOINT_10_RX | ENDPOINT_11_TX | ENDPOINT_11_RX | \
	  ENDPOINT_12_TX | ENDPOINT_12_RX | ENDPOINT_13_TX | ENDPOINT_13_RX | \
	  ENDPOINT_14_TX | ENDPOINT_14_RX | ENDPOINT_15_TX | ENDPOINT_15_RX))
#define USB2_ENDP0_MAX_PACKET_SIZE ((uint8_t)(64))

// the default device is a regular USB3 with the mandatory USB2 compatibility.
// however, it is possible to separate the USB2 and USB3 devices so that they use different devices.
usb_device_t* usb2_backend_current_device = &usb_device_0;

static volatile puint8_t desc_head = NULL;
static volatile uint16_t endp0_remaining_bytes = 0;
static volatile uint16_t endp0_tx_remaining_bytes = 0;
static volatile uint16_t endp1_tx_remaining_bytes = 0;
static volatile uint16_t endp2_tx_remaining_bytes = 0;
static volatile uint16_t endp3_tx_remaining_bytes = 0;
static volatile uint16_t endp4_tx_remaining_bytes = 0;
static volatile uint16_t endp5_tx_remaining_bytes = 0;
static volatile uint16_t endp6_tx_remaining_bytes = 0;
static volatile uint16_t endp7_tx_remaining_bytes = 0;
static volatile USB_SETUP usb_setup_req;

static volatile bool ep0_passthrough_enabled = false;

void _default_usb2_device_handle_bus_reset(void);
void _default_usb2_device_handle_bus_reset(void) {}

usb2_user_handled_t usb2_user_handled = {
	.usb2_device_handle_bus_reset = _default_usb2_device_handle_bus_reset
};

usb2_endpoints_backend_handled_t usb2_endpoints_backend_handled = {
	.usb2_endp_tx_ready = usb2_endp_tx_ready,
	.usb2_endp_rx_set_state_callback = usb2_endp_rx_set_state_callback,
	.usb2_endp_tx_set_state_callback = usb2_endp_tx_set_state_callback,
};

void usb2_device_init()
{
	PFIC_EnableIRQ(USBHS_IRQn);
	LOG_IF(LOG_LEVEL_DEBUG, LOG_ID_USB2, "Enabling USB2 \r\n");

	switch (usb2_backend_current_device->speed)
	{
	case USB2_HIGHSPEED:
		R8_USB_CTRL = UCST_HS;
		break;
	case USB2_FULLSPEED:
		R8_USB_CTRL = UCST_FS;
		break;
	case USB2_LOWSPEED:
		R8_USB_CTRL = UCST_LS;
		break;
	case SPEED_NONE:
	case USB30_SUPERSPEED:
	default:
		usb2_device_deinit();
		return;
	}
	R8_USB_CTRL |= RB_DEV_PU_EN | RB_USB_INT_BUSY | RB_USB_DMA_EN;
	R8_USB_INT_EN = RB_USB_IE_SETUPACT | RB_USB_IE_TRANS | RB_USB_IE_SUSPEND |
					RB_USB_IE_BUSRST;

	// init device's state
	usb2_backend_current_device->addr = 0;
	usb2_backend_current_device->state = POWERED;

	usb2_setup_endpoints();
}

void usb2_device_deinit(void)
{
	LOG_IF(LOG_LEVEL_DEBUG, LOG_ID_USB2, "Disabling USB2 \r\n");
	PFIC_DisableIRQ(USBHS_IRQn);
	R8_USB_CTRL = RB_USB_CLR_ALL | RB_USB_RESET_SIE;
}

void usb2_setup_endpoints(void)
{
	R32_UEP0_RT_DMA = (uint32_t)(uint8_t*)usb2_backend_current_device->endpoints.rx[0].buffer;
	R16_UEP0_MAX_LEN = 64; // configure EP0 max buffer length
	R16_UEP0_T_LEN = 0;
	R8_UEP0_TX_CTRL = UEP_T_RES_NAK | RB_UEP_T_TOG_0;
	R8_UEP0_RX_CTRL = UEP_R_RES_ACK | RB_UEP_R_TOG_0;

	if (usb2_backend_current_device->endpoint_mask & ENDPOINT_1_TX)
	{
		R8_UEP4_1_MOD |= RB_UEP1_TX_EN;
		R32_UEP1_TX_DMA = (uint32_t)(uint8_t*)usb2_backend_current_device->endpoints.tx[1].buffer;
		R16_UEP1_T_LEN = 0;
		R8_UEP1_TX_CTRL = UEP_T_RES_NAK | RB_UEP_T_TOG_0;
		usb2_backend_current_device->endpoints.tx[1].state = ENDP_STATE_NAK;
	}
	if (usb2_backend_current_device->endpoint_mask & ENDPOINT_1_RX)
	{
		R8_UEP4_1_MOD |= RB_UEP1_RX_EN;
		R32_UEP1_RX_DMA = (uint32_t)(uint8_t*)usb2_backend_current_device->endpoints.rx[1].buffer;
		R16_UEP1_MAX_LEN = usb2_backend_current_device->endpoints.rx[1].max_packet_size;
		R8_UEP1_RX_CTRL = UEP_R_RES_ACK | RB_UEP_R_TOG_0;
		usb2_backend_current_device->endpoints.rx[1].state = ENDP_STATE_ACK;
	}

	if (usb2_backend_current_device->endpoint_mask & ENDPOINT_2_TX)
	{
		R8_UEP2_3_MOD |= RB_UEP2_TX_EN;
		R32_UEP2_TX_DMA = (uint32_t)(uint8_t*)usb2_backend_current_device->endpoints.tx[2].buffer;
		R16_UEP2_T_LEN = 0;
		R8_UEP2_TX_CTRL = UEP_T_RES_NAK | RB_UEP_T_TOG_0;
		usb2_backend_current_device->endpoints.tx[2].state = ENDP_STATE_NAK;
	}
	if (usb2_backend_current_device->endpoint_mask & ENDPOINT_2_RX)
	{
		R8_UEP2_3_MOD |= RB_UEP2_RX_EN;
		R32_UEP2_RX_DMA = (uint32_t)(uint8_t*)usb2_backend_current_device->endpoints.rx[2].buffer;
		R16_UEP2_MAX_LEN = usb2_backend_current_device->endpoints.rx[2].max_packet_size;
		R8_UEP2_RX_CTRL = UEP_R_RES_ACK | RB_UEP_R_TOG_0;
		usb2_backend_current_device->endpoints.rx[2].state = ENDP_STATE_ACK;
	}

	if (usb2_backend_current_device->endpoint_mask & ENDPOINT_3_TX)
	{
		R8_UEP2_3_MOD |= RB_UEP3_TX_EN;
		R32_UEP3_TX_DMA = (uint32_t)(uint8_t*)usb2_backend_current_device->endpoints.tx[3].buffer;
		R16_UEP3_T_LEN = 0;
		R8_UEP3_TX_CTRL = UEP_T_RES_NAK | RB_UEP_T_TOG_0;
		usb2_backend_current_device->endpoints.tx[3].state = ENDP_STATE_NAK;
	}
	if (usb2_backend_current_device->endpoint_mask & ENDPOINT_3_RX)
	{
		R8_UEP2_3_MOD |= RB_UEP3_RX_EN;
		R32_UEP3_RX_DMA = (uint32_t)(uint8_t*)usb2_backend_current_device->endpoints.rx[3].buffer;
		R16_UEP3_MAX_LEN = usb2_backend_current_device->endpoints.rx[3].max_packet_size;
		R8_UEP3_RX_CTRL = UEP_R_RES_ACK | RB_UEP_R_TOG_0;
		usb2_backend_current_device->endpoints.rx[3].state = ENDP_STATE_ACK;
	}

	if (usb2_backend_current_device->endpoint_mask & ENDPOINT_4_TX)
	{
		R8_UEP4_1_MOD |= RB_UEP4_TX_EN;
		R32_UEP4_TX_DMA = (uint32_t)(uint8_t*)usb2_backend_current_device->endpoints.tx[4].buffer;
		R16_UEP4_T_LEN = 0;
		R8_UEP4_TX_CTRL = UEP_T_RES_NAK | RB_UEP_T_TOG_0;
		usb2_backend_current_device->endpoints.tx[4].state = ENDP_STATE_NAK;
	}
	if (usb2_backend_current_device->endpoint_mask & ENDPOINT_4_RX)
	{
		R8_UEP4_1_MOD |= RB_UEP4_RX_EN;
		R32_UEP4_RX_DMA = (uint32_t)(uint8_t*)usb2_backend_current_device->endpoints.rx[4].buffer;
		R16_UEP4_MAX_LEN = usb2_backend_current_device->endpoints.rx[4].max_packet_size;
		R8_UEP4_RX_CTRL = UEP_R_RES_ACK | RB_UEP_R_TOG_0;
		usb2_backend_current_device->endpoints.rx[4].state = ENDP_STATE_ACK;
	}

	if (usb2_backend_current_device->endpoint_mask & ENDPOINT_5_TX)
	{
		R8_UEP5_6_MOD |= RB_UEP5_TX_EN;
		R32_UEP5_TX_DMA = (uint32_t)(uint8_t*)usb2_backend_current_device->endpoints.tx[5].buffer;
		R16_UEP5_T_LEN = 0;
		R8_UEP5_TX_CTRL = UEP_T_RES_NAK | RB_UEP_T_TOG_0;
		usb2_backend_current_device->endpoints.tx[5].state = ENDP_STATE_NAK;
	}
	if (usb2_backend_current_device->endpoint_mask & ENDPOINT_5_RX)
	{
		R8_UEP5_6_MOD |= RB_UEP5_RX_EN;
		R32_UEP5_RX_DMA = (uint32_t)(uint8_t*)usb2_backend_current_device->endpoints.rx[5].buffer;
		R16_UEP5_MAX_LEN = usb2_backend_current_device->endpoints.rx[5].max_packet_size;
		R8_UEP5_RX_CTRL = UEP_R_RES_ACK | RB_UEP_R_TOG_0;
		usb2_backend_current_device->endpoints.rx[5].state = ENDP_STATE_ACK;
	}

	if (usb2_backend_current_device->endpoint_mask & ENDPOINT_6_TX)
	{
		R8_UEP5_6_MOD |= RB_UEP6_TX_EN;
		R32_UEP6_TX_DMA = (uint32_t)(uint8_t*)usb2_backend_current_device->endpoints.tx[6].buffer;
		R16_UEP6_T_LEN = 0;
		R8_UEP6_TX_CTRL = UEP_T_RES_NAK | RB_UEP_T_TOG_0;
		usb2_backend_current_device->endpoints.tx[6].state = ENDP_STATE_NAK;
	}
	if (usb2_backend_current_device->endpoint_mask & ENDPOINT_6_RX)
	{
		R8_UEP5_6_MOD |= RB_UEP6_RX_EN;
		R32_UEP6_RX_DMA = (uint32_t)(uint8_t*)usb2_backend_current_device->endpoints.rx[6].buffer;
		R16_UEP6_MAX_LEN = usb2_backend_current_device->endpoints.rx[6].max_packet_size;
		R8_UEP6_RX_CTRL = UEP_R_RES_ACK | RB_UEP_R_TOG_0;
		usb2_backend_current_device->endpoints.rx[6].state = ENDP_STATE_ACK;
	}

	if (usb2_backend_current_device->endpoint_mask & ENDPOINT_7_TX)
	{
		R8_UEP7_MOD |= RB_UEP7_TX_EN;
		R32_UEP7_TX_DMA = (uint32_t)(uint8_t*)usb2_backend_current_device->endpoints.tx[7].buffer;
		R16_UEP7_T_LEN = 0;
		R8_UEP7_TX_CTRL = UEP_T_RES_NAK | RB_UEP_T_TOG_0;
		usb2_backend_current_device->endpoints.tx[7].state = ENDP_STATE_NAK;
	}
	if (usb2_backend_current_device->endpoint_mask & ENDPOINT_7_RX)
	{
		R8_UEP7_MOD |= RB_UEP7_RX_EN;
		R32_UEP7_RX_DMA = (uint32_t)(uint8_t*)usb2_backend_current_device->endpoints.rx[7].buffer;
		R16_UEP7_MAX_LEN = usb2_backend_current_device->endpoints.rx[7].max_packet_size;
		R8_UEP7_RX_CTRL = UEP_R_RES_ACK | RB_UEP_R_TOG_0;
		usb2_backend_current_device->endpoints.rx[7].state = ENDP_STATE_ACK;
	}

	if (usb2_backend_current_device->endpoint_mask & (USB2_UNSUPPORTED_ENDPOINTS))
	{
		LOG_IF(LOG_LEVEL_DEBUG, LOG_ID_USB2, "Unsupported endpoints \r\n");
	}
}

void usb2_reset_endpoints(void) { usb2_setup_endpoints(); }

void usb2_ep0_passthrough_enabled(bool enable)
{
	ep0_passthrough_enabled = enable ? true : false;
}

/**
 * @fn USP_ENDP0_Set_Configuration_Callback
 * @brief After the SET_CONFIGURATION standard request by the host, all
 *endpoints associated to the configuration must be set to their default state,
 *and all endpoints toggles must be at DATA0.
 **/
__attribute__((always_inline)) inline void
usb2_ep0_set_configuration_callback(void)
{
	if (usb2_backend_current_device->current_config == 0)
	{
		LOG_IF(LOG_LEVEL_DEBUG, LOG_ID_USB2, "ERROR : config should be > 0 \r\n");
		return;
	}

	usb2_reset_endpoints();
}

/**
 * @fn usb2_ep0_next_data_packet_size
 * @brief Get the size of the next data sent in the DATA stage
 **/
__attribute__((always_inline)) static inline uint16_t
usb2_ep0_next_data_packet_size(void)
{
	return endp0_remaining_bytes >= USB2_ENDP0_MAX_PACKET_SIZE
			   ? USB2_ENDP0_MAX_PACKET_SIZE
			   : endp0_remaining_bytes;
}

/**
 * @fn usb2_ep0_request_handler
 * @brief Process the request sent by the host in the SETUP stage of the Control
 *transfer and prepare for the data stage if there is one
 * @return the number of bytes to send for the first packet
 **/
__attribute__((always_inline)) static inline uint16_t
usb2_ep0_request_handler(void)
{
	uint16_t len = 0;

	LOG_IF(LOG_LEVEL_DEBUG, LOG_ID_USB2,
		   "Received SETUP stage of CTRL transfer \r\n");
	if (usb_setup_req.bRequest == USB_GET_DESCRIPTOR)
	{
		LOG_IF(LOG_LEVEL_DEBUG, LOG_ID_USB2,
			   "Received GET_DESCRIPTOR standard request, descriptor %d \r\n",
			   usb_setup_req.wValue.bw.bb0);
		switch (usb_setup_req.wValue.bw.bb0)
		{
		case USB_DESCR_TYP_DEVICE:
			LOG_IF(LOG_LEVEL_DEBUG, LOG_ID_USB2, "DEVICE DESCRIPTOR \r\n");
			if (usb2_backend_current_device->usb_descriptors.usb2_device_descr != NULL)
			{
				desc_head = (puint8_t)usb2_backend_current_device->usb_descriptors.usb2_device_descr;
				endp0_remaining_bytes =
					usb2_backend_current_device->usb_descriptors.usb2_device_descr->bLength >
							usb_setup_req.wLength
						? usb_setup_req.wLength
						: usb2_backend_current_device->usb_descriptors.usb2_device_descr->bLength;
				LOG_IF(LOG_LEVEL_DEBUG, LOG_ID_USB2, "size of device descr %d \r\n",
					   endp0_remaining_bytes);
				len = usb2_ep0_next_data_packet_size();
				memcpy(usb2_backend_current_device->endpoints.rx[0].buffer, desc_head, len);
			}
			break;
		case USB_DESCR_TYP_CONFIG:
			LOG_IF(LOG_LEVEL_DEBUG, LOG_ID_USB2, "CONFIG DESCRIPTOR \r\n");
			if (usb2_backend_current_device->usb_descriptors.usb2_device_config_descrs[0] != NULL)
			{
				uint16_t usb_config_descr_total_size =
					((USB_CFG_DESCR*)(usb2_backend_current_device->usb_descriptors
										  .usb2_device_config_descrs[0]))
						->wTotalLength;
				desc_head =
					(puint8_t)usb2_backend_current_device->usb_descriptors.usb2_device_config_descrs[0];
				endp0_remaining_bytes =
					usb_config_descr_total_size > usb_setup_req.wLength
						? usb_setup_req.wLength
						: usb_config_descr_total_size;
				len = usb2_ep0_next_data_packet_size();
				memcpy(usb2_backend_current_device->endpoints.rx[0].buffer, desc_head, len);
			}
			break;
		case USB_DESCR_TYP_STRING:
			LOG_IF(LOG_LEVEL_DEBUG, LOG_ID_USB2, "STRING DESCRIPTOR \r\n");
			if (usb2_backend_current_device->usb_descriptors.usb_string_descrs != NULL)
			{
				uint8_t str_descr_index = usb_setup_req.wValue.bw.bb1;
				uint16_t usb_str_descr_total_size =
					((USB_STRING_DESCR*)(usb2_backend_current_device->usb_descriptors
											 .usb_string_descrs[str_descr_index]))
						->bLength;
				desc_head =
					(puint8_t)
						usb2_backend_current_device->usb_descriptors.usb_string_descrs[str_descr_index];
				endp0_remaining_bytes = usb_str_descr_total_size;
				len = usb2_ep0_next_data_packet_size();
				memcpy(usb2_backend_current_device->endpoints.rx[0].buffer, desc_head, len);
			}
			break;
		default:
			LOG_IF(LOG_LEVEL_DEBUG, LOG_ID_USB2,
				   "Descriptor not currently supported \r\n");
			len = 0xffff;
			break;
		}
	}
	else if (usb_setup_req.bRequest == USB_SET_ADDRESS)
	{
		usb2_backend_current_device->addr = usb_setup_req.wValue.bw.bb1;
		endp0_remaining_bytes = usb_setup_req.wLength;
	}
	else if (usb_setup_req.bRequest == USB_SET_CONFIGURATION)
	{
		LOG_IF(LOG_LEVEL_DEBUG, LOG_ID_USB2,
			   "Received SET_CONFIGURATION standard request, config %d \r\n",
			   usb_setup_req.wValue.bw.bb1);
		usb2_backend_current_device->current_config = usb_setup_req.wValue.bw.bb1;
		usb2_ep0_set_configuration_callback();
		if (usb_setup_req.wValue.bw.bb1 != 0)
			usb2_backend_current_device->state = CONFIGURED;
	}
	else if (usb_setup_req.bRequest == USB_SET_FEATURE)
	{
		if (usb_setup_req.wValue.w ==
			ENDPOINT_HALT) // reset endpoint toggle to DATA0
		{
			usb2_reset_endpoints();
		}
	}
	else
	{
		uint8_t* buffer = NULL;
		len = usb2_backend_current_device->endpoints.endp0_user_handled_control_request(
			(USB_SETUP*)&usb_setup_req, &buffer);

		if (len > 0)
		{
			if (buffer == NULL)
			{
				return 0xffff;
			}
			desc_head = (puint8_t)buffer;
			endp0_remaining_bytes = len;
			len = usb2_ep0_next_data_packet_size();
			memcpy(usb2_backend_current_device->endpoints.rx[0].buffer, desc_head, len);
		}
	}

	return len;
}

/**
 * @fn usb2_ep0_in_prepare_next
 * @brief Called by usb2_ep0_in_handler. This marks the end of an IN transfer
 *(data from device to host), and returns the length of data left to be sent.
 **/
__attribute__((always_inline)) static inline uint16_t
usb2_ep0_in_prepare_next(void)
{
	switch (usb_setup_req.bRequest)
	{
	case USB_GET_DESCRIPTOR: {
		uint16_t len = usb2_ep0_next_data_packet_size();
		endp0_remaining_bytes -= len;
		desc_head += len;
		LOG_IF(LOG_LEVEL_DEBUG, LOG_ID_USB2,
			   "ENDP0 IN Callback remaining_bytes : %d\r\n", endp0_remaining_bytes);
	}
		return usb2_ep0_next_data_packet_size();
		break;
	case USB_SET_ADDRESS:
		// Even though the address is received in the SETUP stage, the STATUS stage
		// must be finished with address 0 and new address set after that.
		if (usb2_set_device_address(usb2_backend_current_device->addr))
		{
			LOG_IF(LOG_LEVEL_DEBUG, LOG_ID_USB2, "Set address to %d \r\n",
				   usb2_backend_current_device->addr);
			usb2_backend_current_device->state = ADDRESS;
		}
		else
		{
			LOG_IF(LOG_LEVEL_DEBUG, LOG_ID_USB2, "Failed to set address to %d \r\n",
				   usb2_backend_current_device->addr);
		}
		break;
	default:
		break;
	}
	return 0;
}

/**
 * @fn usb2_ep0_setup_stage_handler
 * @brief Called when the RB_USB_IE_SETUPACT bit is set in R8_USB_INT_FG
 *(interrupt at the end of the SETUP stage of control transfer)
 **/
__attribute__((always_inline)) static inline void
usb2_ep0_setup_stage_handler(void)
{
	uint16_t len = usb2_ep0_request_handler();
	if (len != 0xffff)
	{
		// either sends data, or ZLP (zero length packet)(in case there is no data
		// packet, like for Set_Address)
		R16_UEP0_T_LEN = len;
		R8_UEP0_TX_CTRL =
			UEP_T_RES_ACK |
			RB_UEP_T_TOG_1; // endp0 is in fact constituted of two endpoints : ENDP0
		// IN and ENDP0 OUT. Therefore, they both have their own
		// toggle bit
		R8_UEP0_RX_CTRL =
			UEP_R_RES_ACK |
			RB_UEP_R_TOG_1; // here we set both toggle bits to DATA1, for either the
		// DATA IN or DATA OUT stage
		LOG_IF(LOG_LEVEL_DEBUG, LOG_ID_USB2, "R8_UEP0_TX_CTRL ");
		// LOG_8_BITS_REGISTER((puint8_t)&R8_UEP0_TX_CTRL);
		LOG_IF(LOG_LEVEL_DEBUG, LOG_ID_USB2, "R8_UEP0_RX_CTRL ");
		// LOG_8_BITS_REGISTER((puint8_t)&R8_UEP0_RX_CTRL);
	}
	else
	{
		// Unsupported request
		LOG_IF(LOG_LEVEL_DEBUG, LOG_ID_USB2, "stall \r\n");
		R16_UEP0_T_LEN = 0;
		R8_UEP0_TX_CTRL = UEP_T_RES_STALL;
		R8_UEP0_RX_CTRL = UEP_R_RES_STALL;
	}
}

/**
 * @fn usb2_ep0_in_handler
 * @brief Called when the RB_USB_IE_TRANS bit is set in R8_USB_INT_FG, and the
 *PID is IN.
 **/
__attribute__((always_inline)) static inline void usb2_ep0_in_handler(void)
{
	uint16_t len = usb2_ep0_in_prepare_next();
	// transmission complete
	if (len == 0)
	{
		if (usb_setup_req.bRequestType & USB_REQ_TYP_IN)
		{
			LOG_IF(LOG_LEVEL_DEBUG, LOG_ID_USB2, "PID IN transmission complete \r\n");
			R16_UEP0_T_LEN = 0;
			R8_UEP0_TX_CTRL = UEP_T_RES_STALL;
			R8_UEP0_RX_CTRL = UEP_R_RES_ACK |
							  RB_UEP_R_TOG_1; // prepare for Status stage, with DATA
			// OUT sending ZLP (zero-length packet)
			desc_head = 0;
		}
		else
		{
			LOG_IF(LOG_LEVEL_DEBUG, LOG_ID_USB2,
				   "Received STATUS for OUT transaction \r\n");
			R16_UEP0_T_LEN = 0;
			R8_UEP0_TX_CTRL = UEP_T_RES_STALL; // nothing to transmit
			R8_UEP0_RX_CTRL = UEP_R_RES_ACK | RB_UEP_R_TOG_0; // keep listening
		}
	}
	else
	{
		LOG_IF(LOG_LEVEL_DEBUG, LOG_ID_USB2, "Sending next %d bytes endp0 \r\n",
			   len);
		memcpy(usb2_backend_current_device->endpoints.rx[0].buffer, desc_head, len);
		R16_UEP0_T_LEN = len;
		R8_UEP0_TX_CTRL ^= RB_UEP_T_TOG_1; // alternate between DATA0 and DATA1
		R8_UEP0_TX_CTRL =
			(R8_UEP0_TX_CTRL & ~(uint8_t)RB_UEP_TRES_MASK) | (uint8_t)UEP_T_RES_ACK;
	}
}

/**
 * @fn usb2_ep0_out_handler
 * @brief Called when the RB_USB_IE_TRANS bit is set in R8_USB_INT_FG, and the
 *PID is OUT.
 **/
__attribute__((always_inline)) static inline void usb2_ep0_out_handler(void)
{
	if (usb_setup_req.bRequestType & USB_REQ_TYP_IN)
	{
		LOG_IF(LOG_LEVEL_DEBUG, LOG_ID_USB2,
			   "Received STATUS for IN transaction \r\n");
		// LOG_USB_SETUP_REQ((USB_SETUP*)&usb_setup_req);

		// process Status stage
		if (usb2_ep0_next_data_packet_size() > 0)
		{
			LOG_IF(LOG_LEVEL_DEBUG, LOG_ID_USB2, "setup command not finished\r\n");
			R8_UEP0_RX_CTRL = UEP_R_RES_NAK | RB_UEP_R_TOG_1;
		}
		else
		{
			LOG_IF(LOG_LEVEL_DEBUG, LOG_ID_USB2, "setup command finished\r\n");
			R8_UEP0_RX_CTRL = UEP_T_RES_STALL;
			R16_UEP0_T_LEN = 0;
			R8_UEP0_TX_CTRL = 0;
			usb2_backend_current_device->endpoints.tx_complete[0](Ack);
		}
	}
	else
	{
		LOG_IF(LOG_LEVEL_DEBUG, LOG_ID_USB2,
			   "PID OUT transmission complete, preparing for IN STATUS \r\n");
		R16_UEP0_T_LEN =
			0; // sending a ZLP for the DATA IN packet of the STATUS stage
		R8_UEP0_RX_CTRL = UEP_R_RES_STALL;
		R8_UEP0_TX_CTRL = UEP_T_RES_ACK | RB_UEP_T_TOG_1;
	}
}

/**
 * @brief Get the remaining number of bytes left to send
 * @param endp_num endpoint number
 * @return
 */
__attribute__((always_inline)) static inline vuint16_t*
usb2_get_tx_endpoint_remaining_len(uint8_t endp_num)
{
	switch (endp_num)
	{
	case ENDP_0:
		return &endp0_tx_remaining_bytes;
	case ENDP_1:
		return &endp1_tx_remaining_bytes;
		break;
	case ENDP_2:
		return &endp2_tx_remaining_bytes;
		break;
	case ENDP_3:
		return &endp3_tx_remaining_bytes;
		break;
	case ENDP_4:
		return &endp4_tx_remaining_bytes;
		break;
	case ENDP_5:
		return &endp5_tx_remaining_bytes;
		break;
	case ENDP_6:
		return &endp6_tx_remaining_bytes;
		break;
	case ENDP_7:
		return &endp7_tx_remaining_bytes;
		break;
	default:
		return NULL;
		break;
	}
	return NULL;
}

/**
 * @fn usb2_in_transfer_handler
 * @brief Called when the RB_USB_IE_TRANS bit is set in R8_USB_INT_FG, the PID
 *is IN and endpoint is 1.
 **/
__attribute__((always_inline)) static inline void
usb2_in_transfer_handler(uint8_t endp_num)
{
	volatile USB_ENDPOINT* endp = &usb2_backend_current_device->endpoints.tx[endp_num];
	vuint8_t* TX_CTRL = usb2_get_tx_endpoint_ctrl_reg(endp_num);
	vuint16_t* T_Len = usb2_get_tx_endpoint_len_reg(endp_num);
	vuint16_t* tx_remaining_bytes = usb2_get_tx_endpoint_remaining_len(endp_num);

#ifdef DEBUG
	if (endp == NULL || TX_CTRL == NULL || T_Len == NULL ||
		tx_remaining_bytes == NULL)
		return;
#endif

	if (*usb2_get_tx_endpoint_addr_reg(endp_num) != 0)
	{
		uint16_t len = *tx_remaining_bytes > endp->max_packet_size
						   ? endp->max_packet_size
						   : *tx_remaining_bytes;
		*tx_remaining_bytes -= len;

		if (*tx_remaining_bytes == 0)
		{
			LOG_IF(LOG_LEVEL_DEBUG, LOG_ID_USB2,
				   "End of IN transfer for ENDP %d \r\n", endp_num);
			*T_Len = 0;
			*usb2_get_tx_endpoint_addr_reg(endp_num) = (uint32_t)endp->buffer;
			if (endp_num == 0)
			{
				*usb2_get_rx_endpoint_addr_reg(0) = (uint32_t)usb2_backend_current_device->endpoints.rx[0].buffer;
			}
			*TX_CTRL = (*TX_CTRL & ~RB_UEP_TRES_MASK) | UEP_T_RES_NAK;
			usb2_backend_current_device->endpoints.tx_complete[endp_num](Ack);
		}
		else
		{
			*TX_CTRL = (*TX_CTRL & ~RB_UEP_TRES_MASK) | UEP_T_RES_ACK;
		}
		LOG_IF(LOG_LEVEL_DEBUG, LOG_ID_USB2, "ENDP %d TX Remaining bytes : %d \r\n",
			   endp_num, *tx_remaining_bytes);
	}
	else
	{
		*TX_CTRL = (*TX_CTRL & ~RB_UEP_TRES_MASK) | UEP_T_RES_NAK;
	}
	*TX_CTRL ^= RB_UEP_T_TOG_1; // switch between DATA0/DATA1 toggle
}

/**
 * @fn usb2_out_transfer_handler
 * @brief Called when the RB_USB_IE_TRANS bit is set in R8_USB_INT_FG, the PID
 *is OUT and ep is 1.
 **/
__attribute__((always_inline)) static inline void
usb2_out_transfer_handler(uint8_t endp_num)
{
	volatile USB_ENDPOINT* endp = &usb2_backend_current_device->endpoints.rx[endp_num];
	vuint8_t* RX_CTRL = usb2_get_rx_endpoint_ctrl_reg(endp_num);

#ifdef DEBUG
	if (endp == NULL || RX_CTRL == NULL)
		return;
#endif

	LOG_IF(LOG_LEVEL_DEBUG, LOG_ID_USB2, "R8_UEP1_TX_CTRL ");
	// LOG_8_BITS_REGISTER((uint8_t*)&R8_UEP1_TX_CTRL);
	if (endp->buffer != NULL)
	{
		LOG_IF(LOG_LEVEL_DEBUG, LOG_ID_USB2, "Received %d bytes on ep %d\r\n",
			   R16_USB_RX_LEN, endp_num);
		endp->state = usb2_backend_current_device->endpoints.rx_callback[endp_num](endp->buffer, R16_USB_RX_LEN);

		*usb2_get_rx_endpoint_addr_reg(endp_num) = (uint32_t)endp->buffer;
		*RX_CTRL = (*RX_CTRL & ~RB_UEP_TRES_MASK) | endp->state;
	}
	else
	{
		*RX_CTRL = (*RX_CTRL & ~RB_UEP_TRES_MASK) | UEP_T_RES_NAK;
	}
	*RX_CTRL ^= RB_UEP_T_TOG_1; // switch between DATA0/DATA1 toggle
}

void usb2_endp_rx_set_state_callback(uint8_t endp_num)
{
	volatile USB_ENDPOINT* endp = &usb2_backend_current_device->endpoints.rx[endp_num];

	if (endp == NULL)
		return;

	vuint8_t* RX_CTRL = usb2_get_rx_endpoint_ctrl_reg(endp_num);
	*RX_CTRL = (*RX_CTRL & ~RB_UEP_TRES_MASK) | endp->state;
}

void usb2_endp_tx_set_state_callback(uint8_t endp_num)
{
	volatile USB_ENDPOINT* endp = &usb2_backend_current_device->endpoints.tx[endp_num];

	if (endp == NULL)
		return;

	vuint8_t* TX_CTRL = usb2_get_tx_endpoint_ctrl_reg(endp_num);
	*TX_CTRL = (*TX_CTRL & ~RB_UEP_TRES_MASK) | endp->state;
}

void usb2_endp_tx_ready(uint8_t endp_num, uint16_t size)
{
	volatile USB_ENDPOINT* endp = &usb2_backend_current_device->endpoints.tx[endp_num];
	vuint8_t* TX_CTRL = usb2_get_tx_endpoint_ctrl_reg(endp_num);
	vuint16_t* T_Len = usb2_get_tx_endpoint_len_reg(endp_num);
	vuint16_t* tx_remaining_bytes = usb2_get_tx_endpoint_remaining_len(endp_num);

#ifdef DEBUG
	if (endp == NULL || TX_CTRL == NULL || T_Len == NULL ||
		tx_remaining_bytes == NULL)
		return;
#endif

	if (*tx_remaining_bytes > 0)
	{
		LOG_IF(
			LOG_LEVEL_DEBUG, LOG_ID_USB2,
			"WARNING : endp has not finished transferring its current buffer \r\n");
	}
	*tx_remaining_bytes = size;
	*usb2_get_tx_endpoint_addr_reg(endp_num) = (uint32_t)endp->buffer;
	*T_Len = *tx_remaining_bytes > endp->max_packet_size ? endp->max_packet_size
														 : *tx_remaining_bytes;
	*TX_CTRL = (*TX_CTRL & ~RB_UEP_TRES_MASK) |
			   UEP_T_RES_ACK; // endpoint was NAK after last IN transfer
}

__attribute__((interrupt("WCH-Interrupt-fast"))) void USBHS_IRQHandler(void)
{
	uint8_t usb_dev_endp = (R8_USB_INT_ST & RB_DEV_ENDP_MASK) & 0xf;
	uint8_t usb_pid = (R8_USB_INT_ST & RB_DEV_TOKEN_MASK) >> 4;

	LOG_IF(LOG_LEVEL_TRACE, LOG_ID_TRACE, "USBHS_IRQHandler-start\r\n");
	if (R8_USB_INT_FG & RB_USB_IF_SETUOACT && usb2_backend_current_device->state != POWERED)
	{
		usb_setup_req = *(USB_SETUP*)usb2_backend_current_device->endpoints.rx[0].buffer;

		if (ep0_passthrough_enabled)
		{
			volatile USB_ENDPOINT* endp0 = &usb2_backend_current_device->endpoints.rx[0];

			endp0->state = usb2_backend_current_device->endpoints.endp0_passthrough_setup_callback(
				usb2_backend_current_device->endpoints.rx[0].buffer, sizeof(USB_SETUP));
			*usb2_get_rx_endpoint_addr_reg(0) = (uint32_t)endp0->buffer;
			R8_UEP0_TX_CTRL = UEP_T_RES_NAK | RB_UEP_T_TOG_1;
			R8_UEP0_RX_CTRL = endp0->state | RB_UEP_T_TOG_1;
		}
		else
		{
			usb2_ep0_setup_stage_handler();
		}

		R8_USB_INT_FG = RB_USB_IF_SETUOACT; // Clear interrupt flag
	}
	else if ((R8_USB_INT_FG & RB_USB_IF_TRANSFER) &&
			 usb2_backend_current_device->state != POWERED)
	{
#ifdef LOG
		if (!(R8_USB_INT_ST & RB_USB_ST_TOGOK))
		{
			// LOG(" TOG MATCH FAIL : ENDP %x pid %x \n", usb_dev_endp, usb_pid);
		}
#endif
		switch (usb_dev_endp)
		{
		case 0:
			if (usb_pid == PID_IN)
			{
				if (ep0_passthrough_enabled)
				{
					usb2_in_transfer_handler(usb_dev_endp);
				}
				else
				{
					usb2_ep0_in_handler();
				}
			}
			else if (usb_pid == PID_OUT)
			{
				if (ep0_passthrough_enabled)
				{
					usb2_out_transfer_handler(usb_dev_endp);
				}
				else
				{
					usb2_ep0_out_handler();
				}
			}
			break;
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
			if (usb_pid == PID_IN)
			{
				usb2_in_transfer_handler(usb_dev_endp);
			}
			else if (usb_pid == PID_OUT)
			{
				usb2_out_transfer_handler(usb_dev_endp);
			}
			break;
		default:
			break;
		}

		R8_USB_INT_FG = RB_USB_IF_TRANSFER; // Clear interrupt flag
	}
	else if (R8_USB_INT_FG & RB_USB_IF_SUSPEND) // wakeup event or bus suspend
	{
		R8_USB_INT_FG = RB_USB_IF_SUSPEND;
	}
	else if (R8_USB_INT_FG & RB_USB_IF_BUSRST)
	{
		usb2_user_handled.usb2_device_handle_bus_reset();
		usb2_set_device_address(0);
		usb2_backend_current_device->addr = 0;
		usb2_setup_endpoints();
		usb2_backend_current_device->state = DEFAULT;
		R8_USB_INT_FG = RB_USB_IF_BUSRST;
	}

	LOG_IF(LOG_LEVEL_TRACE, LOG_ID_TRACE, "USBHS_IRQHandler-end\r\n");
}
