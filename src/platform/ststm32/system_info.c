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


/*
* Static values
*/
// Save a few bytes by using variables for these so they can be deduplicated
static const char *ON  = "On";
static const char *OFF = "Off";

//
// These are all defined by the linker
// Note that it's the *address* of the identifier that's important, not it's
// value.
// https:// www.freertos.org/FreeRTOS_Support_Forum_Archive/March_2015/freertos_How_to_configure_the_Total_Heap_Size_5a94a34cj.html
//
// Start address for the initialization values of the .data section
extern char _sidata;
// Start address for the .data section
extern char _sdata;
// End address for the .data section
extern char _edata;
// Start address for the .bss section
extern char _sbss;
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


/*
* Functions
*/
void print_platform_info(printf_putc_t printf_putc) {
	int bss_size, data_size, stack_size;

	// Due to a bug in the hardware, the ID and revision are always 0
	vvprintf(printf_putc, "Hardware ID 0x%03X Rev 0x%04X, UID 0x%08X%08X%08X\r\n",
		(uint )GATHER_BITS(DBGMCU->IDCODE, 0x0FFF, DBGMCU_IDCODE_DEV_ID_Pos),
		(uint )GATHER_BITS(DBGMCU->IDCODE, 0xFFFF, DBGMCU_IDCODE_REV_ID_Pos),
		(uint )(((uint32_t *)UID_BASE)[0]),
		(uint )(((uint32_t *)UID_BASE)[1]),
		(uint )(((uint32_t *)UID_BASE)[2])
		);

	vvprintf(printf_putc, "CMSIS version 0x%08X\r\n",
		(uint )__STM32F1_CMSIS_VERSION
		);

	{
		uint32_t src = GATHER_BITS(RCC->CFGR, 0b11, RCC_CFGR_SWS_Pos);
		vvprintf(printf_putc, "SysCLK: %s, CSS: %s, HSE: %s, HSI: %s, PLL: %s, LSE: %s, LSI: %s\r\n",
			(src == 0b00) ? "HSI" : (src == 0b01) ? "HSE" : "PLL",
			(GATHER_BITS(RCC->CR,   0b1,  RCC_CR_CSSON_Pos  )) ? ON : OFF,
			(GATHER_BITS(RCC->CR,   0b1,  RCC_CR_HSEON_Pos  )) ? ON : OFF,
			(GATHER_BITS(RCC->CR,   0b1,  RCC_CR_HSION_Pos  )) ? ON : OFF,
			(GATHER_BITS(RCC->CR,   0b1,  RCC_CR_PLLON_Pos  )) ? ON : OFF,
			(GATHER_BITS(RCC->BDCR, 0b1,  RCC_BDCR_LSEON_Pos)) ? ON : OFF,
			(GATHER_BITS(RCC->CSR,  0b1,  RCC_CSR_LSION_Pos )) ? ON : OFF
		);
	}

	vvprintf(printf_putc, "HSI Calibration: 0x%01X, Trim: 0x%01X\r\n",
		(uint )GATHER_BITS(RCC->CR, 0xF,     RCC_CR_HSICAL_Pos),
		(uint )GATHER_BITS(RCC->CR, 0b11111, RCC_CR_HSITRIM_Pos)
	);

	vvprintf(printf_putc, "PLL Src: %s, Mult: 0x%01X\r\n",
		(GATHER_BITS(RCC->CFGR,  0b1,  RCC_CFGR_PLLSRC_Pos )) ?
			(GATHER_BITS(RCC->CFGR,  0b1,  RCC_CFGR_PLLXTPRE_Pos )) ? "HSE/2" : "HSE"
			: "HSI/2",
		// TODO: This is wrong if the multiplier is 0b1111
		(uint )(GATHER_BITS(RCC->CFGR, 0xF, RCC_CFGR_PLLMULL_Pos) + 2)
	);

	{
		uint32_t src, div;

		src = GATHER_BITS(RCC->BDCR, 0b11, RCC_BDCR_RTCSEL_Pos);
		READ_SPLITREG(div, RTC->PRLH, RTC->PRLL);
		vvprintf(printf_putc, "RTC Src: %s, Div: %u\r\n",
			(src == 0b00) ? "None" :
			(src == 0b01) ? "LSE"  :
			(src == 0b10) ? "LSI"  :
			"HSE/128",
			(uint )(div+1) // The divider is offset by one
		);
	}

	vvprintf(printf_putc, "HCLK: %uHz, PCLK1: %uHz, PCLK2: %uHz, ADC: %uHz\r\n",
		(uint )G_freq_HCLK,
		(uint )G_freq_PCLK1,
		(uint )G_freq_PCLK2,
#if USE_ADC
		(uint )G_freq_ADC
#else
		(uint )0
#endif
	);

	vvprintf(printf_putc, "Flash Latency: %u, Prefetch: %s, Half-cycle access: %s\r\n",
		(uint )GATHER_BITS(FLASH->ACR, 0b111, FLASH_ACR_LATENCY_Pos),
		GATHER_BITS(FLASH->ACR, 0b1, FLASH_ACR_PRFTBS_Pos) ? ON : OFF,
		GATHER_BITS(FLASH->ACR, 0b1, FLASH_ACR_HLFCYA_Pos) ? ON : OFF
	);

	stack_size = (RAM_BASE + RAM_PRESENT) - __get_MSP();
	data_size = (&_edata) - (&_sdata);
	bss_size  = (&_ebss)  - (&_sbss);

	vvprintf(printf_putc, "RAM used: %dB stack, %dB .data, %dB .bss\r\n", (int )stack_size, (int )(data_size), (int )(bss_size));

	return;
}


#ifdef __cplusplus
 }
#endif
