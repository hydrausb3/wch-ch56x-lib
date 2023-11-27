/********************************** (C) COPYRIGHT
******************************* Copyright (c) 2021 Nanjing Qinheng
Microelectronics Co., Ltd. Copyright (c) 2022 Benjamin VERNOUX Copyright (c)
2023 Quarkslab

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

#ifndef USB30_UTILS_H
#define USB30_UTILS_H

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
#include "wch-ch56x-lib/USBDevice/usb_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @fn      USB30_switch_powermode
 *
 * @brief   Switch USB3.0 power mode
 *
 * @return   None
 */
__attribute__((always_inline)) static inline void
usb30_switch_powermode(uint8_t pwr_mode)
{
	while ((USBSS->LINK_STATUS & 4) != 0)
		;
	USBSS->LINK_CTRL &= 0xfffffffc;
	USBSS->LINK_CTRL |= pwr_mode;
}

/**
 * @fn     usb30_out_set
 *
 * @brief  Endpoint receive settings
 *
 * @param  endp - The endpoint number to be set
 *         nump - The number of packets remaining to be received by the endpoint
 *         status -  endpoint status 0-NRDY,1-ACK,2-STALL
 *
 * @return None
 */
__attribute__((always_inline)) static inline void
usb30_out_set(uint8_t endp, uint8_t status, uint8_t nump)
{
	vuint32_t* p = &USBSS->UEP0_RX_CTRL;
	p += endp * 4;
	*p = *p | ((nump) << 16) | (status << 26);
}

/**
 * @fn     usb30_device_set_address
 *
 * @brief  Set device address
 *
 * @param  address - address to be set
 *
 * @return None
 */
__attribute__((always_inline)) static inline void
usb30_device_set_address(uint32_t address)
{
	USBSS->USB_CONTROL &= 0x00ffffff;
	USBSS->USB_CONTROL |= address << 24;
}

/**
 * @fn     usb30_in_nump
 *
 * @brief  Get the number of packets remaining to be sent by the endpoint
 *
 * @param  endp - endpoint number
 *
 * @return Number of remaining packets to be sent
 */
__attribute__((always_inline)) static inline uint8_t
usb30_in_nump(uint8_t endp)
{
	vuint32_t* p = &USBSS->UEP0_TX_CTRL;
	p += endp * 4;
	return (((*p) >> 16) & 0x1f);
}
/**
 * @fn     usb30_in_set
 *
 * @brief  Endpoint send settings
 *
 * @param  endp - endpoint number
 *         lpf -  end of burst sign   1-enable 0-disable
 *         nump -  The number of packets the endpoint can send
 *         status -  endpoint status   0-NRDY,1-ACK,2-STALL
 *         TxLen - The data length of the last packet sent by the endpoint
 *
 * @return None
 */

__attribute__((always_inline)) static inline void
usb30_in_set(uint8_t endp, FunctionalState lpf, uint8_t status, uint8_t nump,
			 uint16_t TxLen)
{
	vuint32_t* p = &USBSS->UEP0_TX_CTRL;
	p += endp * 4;
	*p = *p | (nump << 16) | (status << 26) | (TxLen & 0x7ff) | (lpf << 28);
}

/**
 * @fn     usb30_send_erdy
 *
 * @brief  Endpoint flow control settings Send ERDY packets
 *
 * @param  endp - endpoint number   The highest bit table direction,
 *                                 the lower four bits are the endpoint number
 *         nump -  Number of packets received or sent by the endpoint
 *
 * @return None
 */
__attribute__((always_inline)) static inline void
usb30_send_erdy(uint8_t endp, uint8_t nump)
{
	uint32_t t = endp & 0xf;
	t = (t << 2) | ((uint32_t)nump << 6);
	if ((endp & 0x80) == 0)
	{
		USBSS->USB_FC_CTRL = t | (uint32_t)1;
		return;
	}
	else
	{
		USBSS->USB_FC_CTRL = t | (uint32_t)3;
		return;
	}
}

/**
 * @fn     usb30_out_clear_interrupt
 *
 * @brief  Clear the OUT transaction complete interrupt, keep only the packet
 * sequence number
 *
 * @param  endp - The endpoint number to be set
 *
 * @return None
 */
__attribute__((always_inline)) static inline void
usb30_out_clear_interrupt(uint8_t endp)
{
	vuint32_t* p = &USBSS->UEP0_RX_CTRL;
	p += endp * 4;
	*p &= 0x03e00000;
}

/**
 * @fn     usb30_out_clear_interrupt_all
 *
 * @brief  Clear the OUT transaction complete interrupt
 *
 * @param  endp - The endpoint number to be set
 *
 * @return None
 */
__attribute__((always_inline)) static inline void
usb30_out_clear_interrupt_all(uint8_t endp)
{
	vuint32_t* p = &USBSS->UEP0_RX_CTRL;
	p += endp * 4;
	*p &= 0x00000000;
}

/**
 * @fn     usb30_in_clear_interrupt_ALL
 *
 * @brief  Clear the IN transaction interrupt and the rest of the endpoint
 * state, keeping only the packet sequence number
 *
 * @param  endp - endpoint number
 *
 * @return None
 */
__attribute__((always_inline)) static inline void
usb30_in_clear_interrupt(uint8_t endp)
{
	vuint32_t* p = &USBSS->UEP0_TX_CTRL;
	p += endp * 4;
	*p &= 0x03e00000;
}

/**
 * @fn     usb30_in_clear_interrupt_ALL
 *
 * @brief  Clear the IN transaction interrupt and the rest of the endpoint
 * state,
 *
 * @param  endp - endpoint number
 *
 * @return None
 */
__attribute__((always_inline)) static inline void
usb30_in_clear_interrupt_all(uint8_t endp)
{
	vuint32_t* p = &USBSS->UEP0_TX_CTRL;
	p += endp * 4;
	*p &= 0x00000000;
}

/**
 * @fn     usb30_out_status
 *
 * @brief  Get endpoint received data length
 *
 * @param  endp - endpoint number
 *         nump - The number of packets remaining to be received by the endpoint
 *         len -  The length of the data received by the endpoint, for burst
 * transfers, the packet length of the last packet received by the endpoint
 *         status -  Indicates whether the host still has data packets to be
 * sent, 1-the end of the burst received a partial packet 0-the host still has
 * data packets to send
 *
 * @return None
 */
__attribute__((always_inline)) static inline void
usb30_out_status(uint8_t endp, uint8_t* nump, uint16_t* len, uint8_t* status)
{
	vuint32_t* p = &USBSS->UEP0_RX_CTRL;
	p += endp * 4;
	uint32_t t = *p;
	*len = t & 0xffff;
	*nump = (t >> 16) & 31;
	*status = (t >> 0x1d) & 1;
}

/**
 * @brief get register where address of RX data is to be written. Must be in
 * DMA.
 * @param endp endpoint number
 */
__attribute__((always_inline)) static inline vuint32_t*
usb30_get_rx_endpoint_addr_reg(uint8_t endp)
{
	switch (endp)
	{
	case ENDP_1:
		return &USBSS->UEP1_RX_DMA;
	case ENDP_2:
		return &USBSS->UEP2_RX_DMA;
	case ENDP_3:
		return &USBSS->UEP3_RX_DMA;
	case ENDP_4:
		return &USBSS->UEP4_RX_DMA;
	case ENDP_5:
		return &USBSS->UEP5_RX_DMA;
	case ENDP_6:
		return &USBSS->UEP6_RX_DMA;
	case ENDP_7:
		return &USBSS->UEP7_RX_DMA;
	default:
		return NULL;
	}
	return NULL;
}

/**
 * @brief get register where address of TX data will be read. Must be in DMA.
 * @param endp endpoint number
 * @return
 */
__attribute__((always_inline)) static inline vuint32_t*
usb30_get_tx_endpoint_addr_reg(uint8_t endp)
{
	switch (endp)
	{
	case ENDP_1:
		return &USBSS->UEP1_TX_DMA;
	case ENDP_2:
		return &USBSS->UEP2_TX_DMA;
	case ENDP_3:
		return &USBSS->UEP3_TX_DMA;
	case ENDP_4:
		return &USBSS->UEP4_TX_DMA;
	case ENDP_5:
		return &USBSS->UEP5_TX_DMA;
	case ENDP_6:
		return &USBSS->UEP6_TX_DMA;
	case ENDP_7:
		return &USBSS->UEP7_TX_DMA;
	default:
		return NULL;
	}
	return NULL;
}

#ifdef __cplusplus
}
#endif

#endif
