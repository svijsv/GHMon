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

#include <avr/io.h>


/*
* Static values
*/
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
	int stack_size, bss_size, data_size;

	vvprintf(printf_putc, F("Device signature: 0x%02X%02X%02X\r\n"),
		(uint )SIGNATURE_0,
		(uint )SIGNATURE_1,
		(uint )SIGNATURE_2
		);

	vvprintf(printf_putc, F("Osc: %luHz, Core: %luHz, ADC: %luHz\r\n"),
		(uint32_t )G_freq_OSC,
		(uint32_t )G_freq_CORECLK,
		(uint32_t )G_freq_ADC
		);

	stack_size  = SPL;
	stack_size |= (uint16_t )SPH << 8;
	stack_size = RAMEND - stack_size;
	data_size = (int )(&__data_end) - (int )(&__data_start);
	bss_size  = (int )(&__bss_end ) - (int )(&__bss_start );
	vvprintf(printf_putc, F("RAM used: %dB stack, %dB .data, %dB .bss\r\n"), (int )stack_size, (int )data_size, (int )bss_size);

	return;
}


#ifdef __cplusplus
 }
#endif
