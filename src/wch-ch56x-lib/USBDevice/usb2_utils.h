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

#ifndef USB2_UTILS_H
#define USB2_UTILS_H

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
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * @fn usb2_is_address_valid
 * @brief USB address should be a number between 0 and 127
 **/
__attribute__((always_inline)) static inline uint8_t
usb2_is_address_valid(uint8_t address)
{
	return address <= 0x7f;
}

__attribute__((always_inline)) static inline bool
usb2_set_device_address(uint8_t address)
{
	if (usb2_is_address_valid(address))
	{
		R8_USB_DEV_AD = address;
		return true;
	}
	return false;
}

/**
 * @brief Get RX control register address
 * @param endp_num endpoint number
 * @return
 */
__attribute__((always_inline)) static inline vuint8_t*
usb2_get_rx_endpoint_ctrl_reg(uint8_t endp_num)
{
	switch (endp_num)
	{
	case ENDP_0:
		return &R8_UEP0_RX_CTRL;
		break;
	case ENDP_1:
		return &R8_UEP1_RX_CTRL;
		break;
	case ENDP_2:
		return &R8_UEP2_RX_CTRL;
		break;
	case ENDP_3:
		return &R8_UEP3_RX_CTRL;
		break;
	case ENDP_4:
		return &R8_UEP4_RX_CTRL;
		break;
	case ENDP_5:
		return &R8_UEP5_RX_CTRL;
		break;
	case ENDP_6:
		return &R8_UEP6_RX_CTRL;
		break;
	case ENDP_7:
		return &R8_UEP7_RX_CTRL;
		break;
	default:
		return NULL;
		break;
	}
	return NULL;
}

/**
 * @brief Get TX endpoint control register address
 * @param endp_num endpoint number
 * @return
 */
__attribute__((always_inline)) static inline vuint8_t*
usb2_get_tx_endpoint_ctrl_reg(uint8_t endp_num)
{
	switch (endp_num)
	{
	case ENDP_0:
		return &R8_UEP0_TX_CTRL;
		break;
	case ENDP_1:
		return &R8_UEP1_TX_CTRL;
		break;
	case ENDP_2:
		return &R8_UEP2_TX_CTRL;
		break;
	case ENDP_3:
		return &R8_UEP3_TX_CTRL;
		break;
	case ENDP_4:
		return &R8_UEP4_TX_CTRL;
		break;
	case ENDP_5:
		return &R8_UEP5_TX_CTRL;
		break;
	case ENDP_6:
		return &R8_UEP6_TX_CTRL;
		break;
	case ENDP_7:
		return &R8_UEP7_TX_CTRL;
		break;
	default:
		return NULL;
		break;
	}
	return NULL;
}

/**
 * @brief Get TX endpoint address register address
 * @param endp_num endpoint number
 * @return
 */
__attribute__((always_inline)) static inline vpuint32_t
usb2_get_tx_endpoint_addr_reg(uint8_t endp_num)
{
	switch (endp_num)
	{
	case ENDP_0:
		return &R32_UEP0_RT_DMA;
		break;
	case ENDP_1:
		return &R32_UEP1_TX_DMA;
		break;
	case ENDP_2:
		return &R32_UEP2_TX_DMA;
		break;
	case ENDP_3:
		return &R32_UEP3_TX_DMA;
		break;
	case ENDP_4:
		return &R32_UEP4_TX_DMA;
		break;
	case ENDP_5:
		return &R32_UEP5_TX_DMA;
		break;
	case ENDP_6:
		return &R32_UEP6_TX_DMA;
		break;
	case ENDP_7:
		return &R32_UEP7_TX_DMA;
		break;
	default:
		return NULL;
		break;
	}
	return NULL;
}

/**
 * @brief Get RX endpoint address register address
 * @param endp_num endpoint number
 * @return
 */
__attribute__((always_inline)) static inline vpuint32_t
usb2_get_rx_endpoint_addr_reg(uint8_t endp_num)
{
	switch (endp_num)
	{
	case ENDP_0:
		return &R32_UEP0_RT_DMA;
		break;
	case ENDP_1:
		return &R32_UEP1_RX_DMA;
		break;
	case ENDP_2:
		return &R32_UEP2_RX_DMA;
		break;
	case ENDP_3:
		return &R32_UEP3_RX_DMA;
		break;
	case ENDP_4:
		return &R32_UEP4_RX_DMA;
		break;
	case ENDP_5:
		return &R32_UEP5_RX_DMA;
		break;
	case ENDP_6:
		return &R32_UEP6_RX_DMA;
		break;
	case ENDP_7:
		return &R32_UEP7_RX_DMA;
		break;
	default:
		return NULL;
		break;
	}
	return NULL;
}

/**
 * @brief Get TX endpoint address of register holding len of data to be
 * transmitted.
 * @param endp_num endpoint number
 * @return
 */
__attribute__((always_inline)) static inline vuint16_t*
usb2_get_tx_endpoint_len_reg(uint8_t endp_num)
{
	switch (endp_num)
	{
	case ENDP_0:
		return &R16_UEP0_T_LEN;
		break;
	case ENDP_1:
		return &R16_UEP1_T_LEN;
		break;
	case ENDP_2:
		return &R16_UEP2_T_LEN;
		break;
	case ENDP_3:
		return &R16_UEP3_T_LEN;
		break;
	case ENDP_4:
		return &R16_UEP4_T_LEN;
		break;
	case ENDP_5:
		return &R16_UEP5_T_LEN;
		break;
	case ENDP_6:
		return &R16_UEP6_T_LEN;
		break;
	case ENDP_7:
		return &R16_UEP7_T_LEN;
		break;
	default:
		return NULL;
		break;
	}
	return NULL;
}

#ifdef __cplusplus
}
#endif

#endif
