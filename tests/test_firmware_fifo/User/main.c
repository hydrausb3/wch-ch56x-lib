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

// Disable warnings in bsp arising from -pedantic -Wall -Wconversion
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic ignored "-Wvariadic-macros"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#include "CH56x_common.h"
#pragma GCC diagnostic pop
#pragma GCC diagnostic pop
#pragma GCC diagnostic pop

#include "wch-ch56x-lib/logging/logging.h"
#include "wch-ch56x-lib/memory/fifo.h"

#undef FREQ_SYS
/* System clock / MCU frequency in Hz (lowest possible speed 15MHz) */
#define FREQ_SYS (120000000)

#define FIFO_SIZE 1000

#define MAX_COUNT 1000

#define TMR_CLOCK_CYCLES 100000

#define RATIO 10

HYDRA_FIFO_DEF(fifo, uint16_t, FIFO_SIZE);

volatile uint16_t count = 0;
/*********************************************************************
 * @fn      main
 *
 * @brief   Main program.
 *
 * @return  none
 */
int main()
{
	bsp_gpio_init();

	bsp_init(FREQ_SYS);

	LOG_INIT(FREQ_SYS);

	LOG("FIFO stress test\r\n");

	TMR0_TimerInit(TMR_CLOCK_CYCLES);
	R8_TMR0_INTER_EN = RB_TMR_IE_CYC_END;
	PFIC_EnableIRQ(TMR0_IRQn);

	uint16_t buffer[FIFO_SIZE];
	while (1)
	{
		uint16_t read = fifo_read(&fifo, buffer);

		if (read > 0)
		{
			LOG("Total read %d \r\n", read);
			for (uint16_t i = 0; i < read; ++i)
			{
				LOG("Read %d\r\n", buffer[i]);
				if (buffer[i] >= MAX_COUNT)
				{
					goto end;
				}
			}
		}
		// bsp_wait_nb_cycles(TMR_CLOCK_CYCLES / RATIO);
	}
end:
	return 0;
}

/*******************************************************************************
 * @fn     TMR0_IRQHandler
 * @return None
 */
__attribute__((interrupt("WCH-Interrupt-fast"))) void TMR0_IRQHandler(void);
__attribute__((interrupt("WCH-Interrupt-fast"))) void TMR0_IRQHandler(void)
{
	R8_TMR0_INT_FLAG = RB_TMR_IF_CYC_END;
	LOG("clock \r\n");
	if (count < MAX_COUNT)
	{
		++count;
		uint16_t temp = count;
		fifo_write(&fifo, &temp, 1);
		LOG("writing %d \r\n", count);
		TMR0_TimerInit(TMR_CLOCK_CYCLES);
	}
	else
	{
		R8_TMR0_INTER_EN = 0;
		PFIC_DisableIRQ(TMR0_IRQn);
		R8_TMR0_CTRL_MOD = RB_TMR_ALL_CLEAR;
	}
	return;
}

__attribute__((interrupt("WCH-Interrupt-fast"))) void WDOG_IRQHandler(void);
__attribute__((interrupt("WCH-Interrupt-fast"))) void WDOG_IRQHandler(void)
{
	LOG_DUMP();

	LOG_IF_LEVEL(LOG_LEVEL_CRITICAL,
				 "WDOG_IRQHandler\r\n"
				 " SP=0x%08X\r\n"
				 " MIE=0x%08X\r\n"
				 " MSTATUS=0x%08X\r\n"
				 " MCAUSE=0x%08X\r\n"
				 " MVENDORID=0x%08X\r\n"
				 " MARCHID=0x%08X\r\n"
				 " MISA=0x%08X\r\n"
				 " MIMPID=0x%08X\r\n"
				 " MHARTID=0x%08X\r\n"
				 " MEPC=0x%08X\r\n"
				 " MSCRATCH=0x%08X\r\n"
				 " MTVEC=0x%08X\r\n",
				 __get_SP(), __get_MIE(), __get_MSTATUS(), __get_MCAUSE(),
				 __get_MVENDORID(), __get_MARCHID(), __get_MISA(), __get_MIMPID(),
				 __get_MHARTID(), __get_MEPC(), __get_MSCRATCH(), __get_MTVEC());

	LOG_DUMP();

	bsp_wait_ms_delay(100000000);
}

/*********************************************************************
 * @fn      HardFault_Handler
 *
 * @brief   Example of basic HardFault Handler called if an exception occurs
 *
 * @return  none
 */
__attribute__((interrupt("WCH-Interrupt-fast"))) void HardFault_Handler(void);
__attribute__((interrupt("WCH-Interrupt-fast"))) void HardFault_Handler(void)
{
	LOG_DUMP();

	// asm("ebreak"); to trigger a breakpoint and test hardfault_handler
	LOG_IF_LEVEL(LOG_LEVEL_CRITICAL,
				 "HardFault_Handler\r\n"
				 " SP=0x%08X\r\n"
				 " MIE=0x%08X\r\n"
				 " MSTATUS=0x%08X\r\n"
				 " MCAUSE=0x%08X\r\n"
				 " MVENDORID=0x%08X\r\n"
				 " MARCHID=0x%08X\r\n"
				 " MISA=0x%08X\r\n"
				 " MIMPID=0x%08X\r\n"
				 " MHARTID=0x%08X\r\n"
				 " MEPC=0x%08X\r\n"
				 " MSCRATCH=0x%08X\r\n"
				 " MTVEC=0x%08X\r\n",
				 __get_SP(), __get_MIE(), __get_MSTATUS(), __get_MCAUSE(),
				 __get_MVENDORID(), __get_MARCHID(), __get_MISA(), __get_MIMPID(),
				 __get_MHARTID(), __get_MEPC(), __get_MSCRATCH(), __get_MTVEC());

	LOG_DUMP();

	bsp_wait_ms_delay(100000000);
}
