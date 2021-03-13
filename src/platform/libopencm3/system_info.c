// SPDX-License-Identifier: GPL-3.0-only
/***********************************************************************
*                                                                      *
*                                                                      *
* Copyright 2021 svijsv                                                *
* This program is free software: you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation, version 3.                             *
*                                                                      *
* This program is distributed in the hope that it will be useful, but  *
* WITHOUT ANY WARRANTY; without even the implied warranty of           *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU    *
* General Public License for more details.                             *
*                                                                      *
* You should have received a copy of the GNU General Public License    *
* along with this program.  If not, see <http:// www.gnu.org/licenses/>.*
*                                                                      *
*                                                                      *
***********************************************************************/
// system_info.c
// Return information about the running system
// NOTES:
//

#ifdef __cplusplus
 extern "C" {
#endif

//#define DEBUG 1
/*
* Includes
*/
#include "system_info.h"
#include "system.h"
#include "adc.h"
#include "time.h"

#include <libopencm3/stm32/rcc.h>


/*
* Static values
*/
//
// These are all defined by the linker
// Note that it's the *address* of the identifier that's important, not it's
// value.
// https:// www.freertos.org/FreeRTOS_Support_Forum_Archive/March_2015/freertos_How_to_configure_the_Total_Heap_Size_5a94a34cj.html
//
// Start address for the initialization values of the .data section
extern char _sidata;
// Start address for the .data section
extern char _data;
// End address for the .data section
extern char _edata;
// Start address for the .bss section
//extern char _sbss;
// End address for the .bss section
extern char _ebss;



/*
* Types
*/


/*
* Variables
*/


/*
* Local function prototypes
*/


/*
* Interrupt handlers
*/
// __get_MSP() taken from CMSIS
static uint32_t __get_MSP(void) {
	//register uint32_t __regMainStackPointer __asm("msp");
	register uint32_t __regMainStackPointer __asm("sp");
	return (__regMainStackPointer);
}

/*
* Functions
*/
void print_platform_info(printf_putc_t printf_putc) {
	int bss_size, data_size, stack_size;
	const char *src;

	vvprintf(printf_putc, "libopencm3 backend\r\n");

	switch ((RCC_CFGR & RCC_CFGR_SW) >> RCC_CFGR_SW_SHIFT) {
	case RCC_CFGR_SW_SYSCLKSEL_HSICLK:
		src = "HSI";
		break;
	case RCC_CFGR_SW_SYSCLKSEL_HSECLK:
		src = "HSE";
		break;
	default:
		src = "PLL";
		break;
	}
	vvprintf(printf_putc, "SysCLK: %s, HSE: %s, HSI: %s, PLL: %s, LSE: %s, LSI: %s\r\n",
		src,
		rcc_is_osc_ready(RCC_HSE) ? "ON" : "OFF",
		rcc_is_osc_ready(RCC_HSI) ? "ON" : "OFF",
		rcc_is_osc_ready(RCC_PLL) ? "ON" : "OFF",
		rcc_is_osc_ready(RCC_LSE) ? "ON" : "OFF",
		rcc_is_osc_ready(RCC_LSI) ? "ON" : "OFF"
	);

	vvprintf(printf_putc, "HCLK: %uHz, PCLK1: %uHz, PCLK2: %uHz, ADC: %uHz\r\n",
		(uint )rcc_ahb_frequency,
		(uint )rcc_apb1_frequency,
		(uint )rcc_apb2_frequency,
#if USE_ADC
		(uint )G_freq_ADC
#else
		(uint )0
#endif
	);

	stack_size = (RAM_BASE + RAM_PRESENT) - __get_MSP();
	data_size = (&_edata) - (&_data);
	bss_size  = (&_ebss)  - (&_edata);

	vvprintf(printf_putc, "RAM used: %dB stack, %dB .data, %dB .bss\r\n", (int )stack_size, (int )(data_size), (int )(bss_size));

	return;
}


#ifdef __cplusplus
 }
#endif
