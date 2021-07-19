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
	uint hclk_psc = 0;

	// Due to a bug in the hardware, the ID and revision are always 0 on the
	// STM32F1
	vvprintf(printf_putc, "Hardware ID 0x%03X Rev 0x%04X, UID 0x%08X%08X%08X\r\n",
		(uint )GATHER_BITS(DBGMCU->IDCODE, 0x0FFF, DBGMCU_IDCODE_DEV_ID_Pos),
		(uint )GATHER_BITS(DBGMCU->IDCODE, 0xFFFF, DBGMCU_IDCODE_REV_ID_Pos),
		(uint )(((uint32_t *)UID_BASE)[0]),
		(uint )(((uint32_t *)UID_BASE)[1]),
		(uint )(((uint32_t *)UID_BASE)[2])
		);
	vvprintf(printf_putc, "CMSIS %s version 0x%08X\r\n",
		CMSIS_NAME,
		(uint )CMSIS_VERSION
		);

	{
		uint hclk = 0, pclk1 = 0, pclk2 = 0, adcclk = 0;

		switch (SELECT_BITS(RCC->CFGR, HCLK_PRESCALER_Msk)) {
		case HCLK_PRESCALER_1:
			hclk = G_freq_OSC;
			hclk_psc = 1;
			break;
		case HCLK_PRESCALER_2:
			hclk = G_freq_OSC / 2;
			hclk_psc = 2;
			break;
		case HCLK_PRESCALER_4:
			hclk = G_freq_OSC / 4;
			hclk_psc = 4;
			break;
		case HCLK_PRESCALER_8:
			hclk = G_freq_OSC / 8;
			hclk_psc = 8;
			break;
		case HCLK_PRESCALER_16:
			hclk = G_freq_OSC / 16;
			hclk_psc = 16;
			break;
		case HCLK_PRESCALER_64:
			hclk = G_freq_OSC / 64;
			hclk_psc = 64;
			break;
		case HCLK_PRESCALER_128:
			hclk = G_freq_OSC / 128;
			hclk_psc = 128;
			break;
		case HCLK_PRESCALER_256:
			hclk = G_freq_OSC / 256;
			hclk_psc = 256;
			break;
		case HCLK_PRESCALER_512:
			hclk = G_freq_OSC / 512;
			hclk_psc = 512;
			break;
		}
		switch (SELECT_BITS(RCC->CFGR, PCLK1_PRESCALER_Msk)) {
		case PCLK1_PRESCALER_1:
			pclk1 = hclk;
			break;
		case PCLK1_PRESCALER_2:
			pclk1 = hclk / 2;
			break;
		case PCLK1_PRESCALER_4:
			pclk1 = hclk / 4;
			break;
		case PCLK1_PRESCALER_8:
			pclk1 = hclk / 8;
			break;
		case PCLK1_PRESCALER_16:
			pclk1 = hclk / 16;
			break;
		}
		switch (SELECT_BITS(RCC->CFGR, PCLK2_PRESCALER_Msk)) {
		case PCLK2_PRESCALER_1:
			pclk2 = hclk;
			break;
		case PCLK2_PRESCALER_2:
			pclk2 = hclk / 2;
			break;
		case PCLK2_PRESCALER_4:
			pclk2 = hclk / 4;
			break;
		case PCLK2_PRESCALER_8:
			pclk2 = hclk / 8;
			break;
		case PCLK2_PRESCALER_16:
			pclk2 = hclk / 16;
			break;
		}
#if USE_ADC
		clock_enable(ADCx_CLOCKEN);
		switch (SELECT_BITS(ADC_PRESCALER_REG, ADC_PRESCALER_Msk)) {
		case ADC_PRESCALER_2:
			adcclk = ADCx_BUSFREQ / 2;
			break;
		case ADC_PRESCALER_4:
			adcclk = ADCx_BUSFREQ / 4;
			break;
		case ADC_PRESCALER_6:
			adcclk = ADCx_BUSFREQ / 6;
			break;
		case ADC_PRESCALER_8:
			adcclk = ADCx_BUSFREQ / 8;
			break;
		}
		clock_disable(ADCx_CLOCKEN);
#endif
		vvprintf(printf_putc, "HCLK: %uHz, PCLK1: %uHz, PCLK2: %uHz, ADC: %uHz\r\n",
		(uint )hclk,
		(uint )pclk1,
		(uint )pclk2,
		(uint )adcclk
		);
	}

	vvprintf(printf_putc, "Should be:\r\nHCLK: %uHz, PCLK1: %uHz, PCLK2: %uHz, ADC: %uHz\r\n",
		(uint )G_freq_HCLK,
		(uint )G_freq_PCLK1,
		(uint )G_freq_PCLK2,
#if USE_ADC
		(uint )G_freq_ADC
#else
		(uint )0
#endif
	);

	{
		uint32_t src = GATHER_BITS(RCC->CFGR, 0b11, RCC_CFGR_SWS_Pos);
		vvprintf(printf_putc, "SysCLK: %s/%u, CSS: %s, HSE: %s, HSI: %s, PLL: %s, LSE: %s, LSI: %s\r\n",
			(src == 0b00) ? "HSI" : (src == 0b01) ? "HSE" : "PLL",
			(uint )hclk_psc,
			(BIT_IS_SET(RCC->CR,   RCC_CR_CSSON  )) ? ON : OFF,
			(BIT_IS_SET(RCC->CR,   RCC_CR_HSEON  )) ? ON : OFF,
			(BIT_IS_SET(RCC->CR,   RCC_CR_HSION  )) ? ON : OFF,
			(BIT_IS_SET(RCC->CR,   RCC_CR_PLLON  )) ? ON : OFF,
			(BIT_IS_SET(RCC->BDCR, RCC_BDCR_LSEON)) ? ON : OFF,
			(BIT_IS_SET(RCC->CSR,  RCC_CSR_LSION )) ? ON : OFF
		);
	}

	vvprintf(printf_putc, "HSI Calibration: 0x%01X, Trim: 0x%01X\r\n",
		(uint )GATHER_BITS(RCC->CR, 0xF,     RCC_CR_HSICAL_Pos),
		(uint )GATHER_BITS(RCC->CR, 0b11111, RCC_CR_HSITRIM_Pos)
	);

#if USE_STM32F1_PLL
	vvprintf(printf_putc, "PLL Src: %s, Mult: %u\r\n",
		(BIT_IS_SET(RCC->CFGR, RCC_CFGR_PLLSRC)) ?
			(BIT_IS_SET(RCC->CFGR, RCC_CFGR_PLLXTPRE)) ? "HSE/2" : "HSE"
			: "HSI/2",
		// TODO: This is wrong if the multiplier is 0b1111
		(uint )(GATHER_BITS(RCC->CFGR, 0xF, RCC_CFGR_PLLMULL_Pos) + 2)
	);
#else
	vvprintf(printf_putc, "PLL Src: %s, DivM: %u, MulN: %u, DivP: %u \r\n",
		(BIT_IS_SET(RCC->PLLCFGR, RCC_PLLCFGR_PLLSRC)) ? "HSE" : "HSI",
		(uint )( GATHER_BITS(RCC->PLLCFGR, 0x3F,  RCC_PLLCFGR_PLLM_Pos)),
		(uint )( GATHER_BITS(RCC->PLLCFGR, 0x1FF, RCC_PLLCFGR_PLLN_Pos)),
		(uint )((GATHER_BITS(RCC->PLLCFGR, 0x3,   RCC_PLLCFGR_PLLP_Pos) + 1) * 2)
	);
#endif

#if USE_STM32F1_RTC
	{
		uint32_t src, div;

		src = GATHER_BITS(RCC->BDCR, 0b11, RCC_BDCR_RTCSEL_Pos);
		READ_SPLITREG(div, RTC->PRLH, RTC->PRLL);
		vvprintf(printf_putc, "RTC Src: %s/%u\r\n",
			(src == 0b00) ? "None" :
			(src == 0b01) ? "LSE"  :
			(src == 0b10) ? "LSI"  :
			"(HSE/128)",
			(uint )(div+1) // The divider is offset by one
		);
	}
#else
	{
		uint32_t src;

		src = GATHER_BITS(RCC->BDCR, 0b11, RCC_BDCR_RTCSEL_Pos);
		vvprintf(printf_putc, "RTC Src: %s, DivA: %u, DivS: %u\r\n",
			(src == 0b00) ? "None" :
			(src == 0b01) ? "LSE"  :
			(src == 0b10) ? "LSI"  :
			"HSE",
			(uint )GATHER_BITS(RTC->PRER, 0x7F,   RTC_PRER_PREDIV_A_Pos),
			(uint )GATHER_BITS(RTC->PRER, 0x7FFF, RTC_PRER_PREDIV_S_Pos)
		);
	}
#endif

#if defined(STM32F1)
	vvprintf(printf_putc, "Flash Latency: %u, Prefetch: %s, Half-cycle access: %s\r\n",
		(uint )GATHER_BITS(FLASH->ACR, 0b111, FLASH_ACR_LATENCY_Pos),
		BIT_IS_SET(FLASH->ACR, FLASH_ACR_PRFTBE) ? ON : OFF,
		BIT_IS_SET(FLASH->ACR, FLASH_ACR_HLFCYA) ? ON : OFF
	);
#else
	vvprintf(printf_putc, "Flash Latency: %u, Prefetch: %s\r\n",
		(uint )GATHER_BITS(FLASH->ACR, 0b1111, FLASH_ACR_LATENCY_Pos),
		BIT_IS_SET(FLASH->ACR, FLASH_ACR_PRFTEN) ? ON : OFF
	);
#endif

	stack_size = (RAM_BASE + RAM_PRESENT) - __get_MSP();
	data_size = (&_edata) - (&_sdata);
	bss_size  = (&_ebss)  - (&_sbss);

	vvprintf(printf_putc, "RAM used: %dB stack, %dB .data, %dB .bss\r\n", (int )stack_size, (int )(data_size), (int )(bss_size));

	return;
}

#ifdef __cplusplus
 }
#endif
