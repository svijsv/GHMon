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
#include "ulib/include/halloc.h"

#include <avr/io.h>

//
// These are all defined by the linker
// Note that it's the *address* of the identifier that's important, not it's
// value.
//
extern char __data_start;
extern char __data_end;
extern char __bss_start;
extern char __bss_end;
extern char __heap_start;


void _print_platform_info(void (*printf_putc)(uint_fast8_t c)) {
	int stack_size, bss_size, data_size, heap_size = 0;
	uint8_t *signature = (uint8_t *)(SIGNATURES_START);

	uHAL_assert(printf_putc != NULL);
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (printf_putc == NULL) {
		return;
	}
#endif

	printf_vv(printf_putc, F("Firmware signature: 0x%02X%02X%02X\r\n"),
		(uint )SIGNATURE_0,
		(uint )SIGNATURE_1,
		(uint )SIGNATURE_2
		);
	printf_vv(printf_putc, F("Device signature: 0x%02X%02X%02X\r\n"),
		(uint )signature[0],
		(uint )signature[1],
		(uint )signature[2]
		);

	const char *clksrc;
	switch (SELECT_BITS(CLKCTRL.MCLKCTRLA, CLKCTRL_CLKSEL_gm)) {
	case CLKCTRL_CLKSEL_OSC20M_gc:
		clksrc = "16MHz/20MHz Internal";
		break;
	case CLKCTRL_CLKSEL_OSCULP32K_gc:
		clksrc = "32KHz Internal";
		break;
	case CLKCTRL_CLKSEL_XOSC32K_gc:
		clksrc = "32.768KHz External";
		break;
	case CLKCTRL_CLKSEL_EXTCLK_gc:
		clksrc = "External";
		break;
	default:
		clksrc = "Unknown";
		break;
	}

	uint8_t clkdiv = 1;
	if (BIT_IS_SET(CLKCTRL.MCLKCTRLB, CLKCTRL_PEN_bm)) {
		switch (SELECT_BITS(CLKCTRL.MCLKCTRLB, CLKCTRL_PDIV_gm)) {
		case CLKCTRL_PDIV_2X_gc:
			clkdiv = 2;
			break;
		case CLKCTRL_PDIV_4X_gc:
			clkdiv = 4;
			break;
		case CLKCTRL_PDIV_8X_gc:
			clkdiv = 8;
			break;
		case CLKCTRL_PDIV_16X_gc:
			clkdiv = 16;
			break;
		case CLKCTRL_PDIV_32X_gc:
			clkdiv = 32;
			break;
		case CLKCTRL_PDIV_64X_gc:
			clkdiv = 64;
			break;

		case CLKCTRL_PDIV_6X_gc:
			clkdiv = 6;
			break;
		case CLKCTRL_PDIV_10X_gc:
			clkdiv = 10;
			break;
		case CLKCTRL_PDIV_12X_gc:
			clkdiv = 12;
			break;
		case CLKCTRL_PDIV_24X_gc:
			clkdiv = 24;
			break;
		case CLKCTRL_PDIV_48X_gc:
			clkdiv = 48;
			break;

		default:
			clkdiv = 1;
			break;
		}
	}
	uint32_t coreclk = F_OSC / clkdiv;

#if uHAL_USE_ADC
	uint32_t adcclk = coreclk >> (GATHER_BITS(ADC0.CTRLC, 0x07U, ADC_PRESC_gp) + 1U);
#else
	uint32_t adcclk = 0;
#endif

	printf_vv(printf_putc, F("Osc: %luHz, Core: %luHz, Src: %s, ADC: %luHz\r\n"),
		(uint32_t )F_OSC,
		(uint32_t )coreclk,
		clksrc,
		(uint32_t )adcclk
		);

	stack_size  = SPL;
	stack_size |= (uint16_t )SPH << 8;
	stack_size = RAMEND - stack_size;
	data_size = (int )(&__data_end) - (int )(&__data_start);
	bss_size  = (int )(&__bss_end ) - (int )(&__bss_start );
#if ULIB_ENABLE_HALLOC
	heap_size = (int )halloc_total_allocated();
#endif
	printf_vv(printf_putc, F("RAM used: %dB stack, %dB heap, %dB .data, %dB .bss\r\n"), (int )stack_size, (int )heap_size, (int )data_size, (int )bss_size);

	return;
}
