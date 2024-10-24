// SPDX-License-Identifier: GPL-3.0-only
/***********************************************************************
*                                                                      *
*                                                                      *
* Copyright 2021, 2024 svijsv                                          *
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
#include "common.h"
#include "system.h"
#include "adc.h"
#include "time.h"

#include "ulib/include/printf.h"


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


void _print_platform_info(void (*printf_putc)(uint_fast8_t c)) {
	int bss_size, data_size, stack_size;
	uint psc;

	// Due to a bug in the hardware, the ID and revision are always 0 on the
	// STM32F1
	printf_vv(printf_putc, "Hardware ID 0x%03X Rev 0x%04X, UID 0x%08X%08X%08X\r\n",
		(uint )GATHER_BITS(DBGMCU->IDCODE, 0x0FFFU, DBGMCU_IDCODE_DEV_ID_Pos),
		(uint )GATHER_BITS(DBGMCU->IDCODE, 0xFFFFU, DBGMCU_IDCODE_REV_ID_Pos),
		(uint )(((uint32_t *)UID_BASE)[0]),
		(uint )(((uint32_t *)UID_BASE)[1]),
		(uint )(((uint32_t *)UID_BASE)[2])
		);
	printf_vv(printf_putc, "CMSIS %s version 0x%08X\r\n",
		CMSIS_NAME,
		(uint )CMSIS_VERSION
		);

	{
		uint hclk = 0, pclk1 = 0, pclk2 = 0;

		switch (SELECT_BITS(RCC->CFGR, RCC_CFGR_HPRE_Msk)) {
		/*
		case RCC_CFGR_HPRE_DIV1:
			psc = 1;
			break;
		*/
		case RCC_CFGR_HPRE_DIV2:
			psc = 2;
			break;
		case RCC_CFGR_HPRE_DIV4:
			psc = 4;
			break;
		case RCC_CFGR_HPRE_DIV8:
			psc = 8;
			break;
		case RCC_CFGR_HPRE_DIV16:
			psc = 16;
			break;
		case RCC_CFGR_HPRE_DIV64:
			psc = 64;
			break;
		case RCC_CFGR_HPRE_DIV128:
			psc = 128;
			break;
		case RCC_CFGR_HPRE_DIV256:
			psc = 256;
			break;
		case RCC_CFGR_HPRE_DIV512:
			psc = 512;
			break;
		default:
			psc = 1;
			break;
		}
		hclk = G_freq_CORE / psc;

		switch (SELECT_BITS(RCC->CFGR, RCC_CFGR_PPRE1_Msk)) {
		/*
		case RCC_CFGR_PPRE1_DIV1:
			pclk1 = hclk;
			break;
		*/
		case RCC_CFGR_PPRE1_DIV2:
			psc = 2U;
			break;
		case RCC_CFGR_PPRE1_DIV4:
			psc = 4U;
			break;
		case RCC_CFGR_PPRE1_DIV8:
			psc = 8U;
			break;
		case RCC_CFGR_PPRE1_DIV16:
			psc = 16U;
			break;
		default:
			psc = 1;
			break;
		}
		pclk1 = hclk / psc;

		switch (SELECT_BITS(RCC->CFGR, RCC_CFGR_PPRE2_Msk)) {
		/*
		case RCC_CFGR_PPRE2_DIV1:
			pclk2 = hclk;
			break;
		*/
		case RCC_CFGR_PPRE2_DIV2:
			psc = 2U;
			break;
		case RCC_CFGR_PPRE2_DIV4:
			psc = 4U;
			break;
		case RCC_CFGR_PPRE2_DIV8:
			psc = 8U;
			break;
		case RCC_CFGR_PPRE2_DIV16:
			psc = 16U;
			break;
		default:
			psc = 1;
			break;
		}
		pclk2 = hclk / psc;

#if uHAL_USE_ADC
		uint adcclk = 0, adc_sps = 0, adc_bits = 0;
		bool redisable = !clock_is_enabled(RCC_PERIPH_ADC1);

		if (redisable) {
			clock_enable(RCC_PERIPH_ADC1);
		}
		switch (SELECT_BITS(ADC_PRESCALER_REG, ADC_PRESCALER_Msk)) {
		/*
		case ADC_PRESCALER_2:
			psc = 2;
			break;
		*/
		case ADC_PRESCALER_4:
			psc = 4;
			break;
		case ADC_PRESCALER_6:
			psc = 6;
			break;
		case ADC_PRESCALER_8:
			psc = 8;
			break;
		default:
			psc = 2;
			break;
		}
		adcclk = G_freq_PCLK2 / psc;

# if HAVE_STM32F1_ADC
		adc_bits = 12;
# else
		switch (SELECT_BITS(ADC1->CR1, ADC_CR1_RES)) {
		case ADC_CR1_RES_12:
			adc_bits = 12;
			break;
		case ADC_CR1_RES_10:
			adc_bits = 10;
			break;
		case ADC_CR1_RES_8:
			adc_bits = 8;
			break;
		//case ADC_CR1_RES_6:
		default:
			adc_bits = 6;
			break;
		}
# endif

		if (redisable) {
			clock_disable(RCC_PERIPH_ADC1);
		}
		adc_sps = ADC_SAMPLES_PER_S;

		printf_vv(printf_putc, "ADC: %uHz, %u bits, %u samples/S\r\n",
		(uint )adcclk,
		(uint )adc_bits,
		(uint )adc_sps
		);
#endif

		printf_vv(printf_putc, "HCLK: %uHz, PCLK1: %uHz, PCLK2: %uHz\r\n",
		(uint )hclk,
		(uint )pclk1,
		(uint )pclk2
		);
	}
	printf_vv(printf_putc, "Should be:\r\nHCLK: %uHz, PCLK1: %uHz, PCLK2: %uHz\r\n",
		(uint )G_freq_HCLK,
		(uint )G_freq_PCLK1,
		(uint )G_freq_PCLK2
	);

	{
		uint32_t src = GATHER_BITS(RCC->CFGR, 0b11U, RCC_CFGR_SWS_Pos);
		printf_vv(printf_putc, "SysCLK: %s, CSS: %s, HSE: %s, HSI: %s, PLL: %s, LSE: %s, LSI: %s\r\n",
			(src == 0b00U) ? "HSI" : (src == 0b01U) ? "HSE" : "PLL",
			(BIT_IS_SET(RCC->CR,   RCC_CR_CSSON  )) ? ON : OFF,
			(BIT_IS_SET(RCC->CR,   RCC_CR_HSEON  )) ? ON : OFF,
			(BIT_IS_SET(RCC->CR,   RCC_CR_HSION  )) ? ON : OFF,
			(BIT_IS_SET(RCC->CR,   RCC_CR_PLLON  )) ? ON : OFF,
			(BIT_IS_SET(RCC->BDCR, RCC_BDCR_LSEON)) ? ON : OFF,
			(BIT_IS_SET(RCC->CSR,  RCC_CSR_LSION )) ? ON : OFF
		);
	}

	printf_vv(printf_putc, "HSI Calibration: 0x%01X, Trim: 0x%01X\r\n",
		(uint )GATHER_BITS(RCC->CR, 0xFU,     RCC_CR_HSICAL_Pos),
		(uint )GATHER_BITS(RCC->CR, 0b11111U, RCC_CR_HSITRIM_Pos)
	);

#if HAVE_STM32F1_PLL
	printf_vv(printf_putc, "PLL Src: %s, Mult: %u\r\n",
		(BIT_IS_SET(RCC->CFGR, RCC_CFGR_PLLSRC)) ?
			(BIT_IS_SET(RCC->CFGR, RCC_CFGR_PLLXTPRE)) ? "HSE/2" : "HSE"
			: "HSI/2",
		// FIXME: This is wrong if the multiplier is 0b1111
		(uint )(GATHER_BITS(RCC->CFGR, 0xFU, RCC_CFGR_PLLMULL_Pos) + 2U)
	);
#else
	printf_vv(printf_putc, "PLL Src: %s, DivM: %u, MulN: %u, DivP: %u \r\n",
		(BIT_IS_SET(RCC->PLLCFGR, RCC_PLLCFGR_PLLSRC)) ? "HSE" : "HSI",
		(uint )( GATHER_BITS(RCC->PLLCFGR, 0x3FU,  RCC_PLLCFGR_PLLM_Pos)),
		(uint )( GATHER_BITS(RCC->PLLCFGR, 0x1FFU, RCC_PLLCFGR_PLLN_Pos)),
		(uint )((GATHER_BITS(RCC->PLLCFGR, 0x3U,   RCC_PLLCFGR_PLLP_Pos) + 1U) * 2U)
	);
#endif

#if HAVE_STM32F1_RTC
	{
		uint32_t src, div;

		src = GATHER_BITS(RCC->BDCR, 0b11U, RCC_BDCR_RTCSEL_Pos);
		READ_SPLITREG32(div, RTC->PRLH, RTC->PRLL);
		printf_vv(printf_putc, "RTC Src: %s/%u\r\n",
			(src == 0b00U) ? "None" :
			(src == 0b01U) ? "LSE"  :
			(src == 0b10U) ? "LSI"  :
			"(HSE/128)",
			(uint )(div + 1U) // The divider is offset by one
		);
	}
#else
	{
		uint32_t src;

		src = GATHER_BITS(RCC->BDCR, 0b11U, RCC_BDCR_RTCSEL_Pos);
		printf_vv(printf_putc, "RTC Src: %s, DivA: %u, DivS: %u\r\n",
			(src == 0b00U) ? "None" :
			(src == 0b01U) ? "LSE"  :
			(src == 0b10U) ? "LSI"  :
			"HSE",
			(uint )GATHER_BITS(RTC->PRER, 0x7FU,   RTC_PRER_PREDIV_A_Pos),
			(uint )GATHER_BITS(RTC->PRER, 0x7FFFU, RTC_PRER_PREDIV_S_Pos)
		);
	}
#endif

	const uint flash_size_kb = *(uint16_t *)FLASHSIZE_BASE;
#if HAVE_STM32F1_FLASH
	printf_vv(printf_putc, "Flash Size: %uKB, Latency: %u, Prefetch: %s, Half-cycle access: %s\r\n",
		(uint )flash_size_kb,
		(uint )GATHER_BITS(FLASH->ACR, 0b111U, FLASH_ACR_LATENCY_Pos),
		BIT_IS_SET(FLASH->ACR, FLASH_ACR_PRFTBE) ? ON : OFF,
		BIT_IS_SET(FLASH->ACR, FLASH_ACR_HLFCYA) ? ON : OFF
	);
#else
	printf_vv(printf_putc, "Flash Size: %uKB, Latency: %u, Prefetch: %s\r\n",
		(uint )flash_size_kb,
		(uint )GATHER_BITS(FLASH->ACR, 0b1111U, FLASH_ACR_LATENCY_Pos),
		BIT_IS_SET(FLASH->ACR, FLASH_ACR_PRFTEN) ? ON : OFF
	);
#endif

	stack_size = (RAM_BASE + RAM_PRESENT) - __get_MSP();
	data_size = (uint )(&_edata) - (uint )(&_sdata);
	bss_size  = (uint )(&_ebss)  - (uint )(&_sbss);
	printf_vv(printf_putc, "RAM used: %dB/%uB stack, %dB .data, %dB .bss\r\n", (int )stack_size, (uint )RAM_PRESENT, (int )(data_size), (int )(bss_size));

	return;
}
