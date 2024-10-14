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
// system.h
// General platform initialization
// NOTES:
//   Prototypes for some of the related functions are in interface.h
//
#ifndef _uHAL_PLATFORM_XMEGA3_SYSTEM_H
#define _uHAL_PLATFORM_XMEGA3_SYSTEM_H


#include "common.h"

// Needed for cli() and sei()
#include <avr/interrupt.h>


// Enable/disable interrupts while preserving original state
#define ENABLE_INTERRUPTS(sreg)  do { sreg = SREG; sei(); } while (0);
#define DISABLE_INTERRUPTS(sreg) do { sreg = SREG; cli(); } while (0);
#define RESTORE_INTERRUPTS(sreg) do { SREG = sreg; } while (0);
#define SAVE_INTERRUPTS(sreg)    do { sreg = SREG; } while (0);


// As explained in the 'Accessing 16-bit Registers' section of the manual,
// the high and low bytes are connected to the 8-bit bus through a temporary
// register and the right way to access the register is to disable interrupts,
// access the low byte, access the high byte, then restore interrupts
//
// I'm not sure this is normally needed as long as GCC always handles the low
// byte first, unless there's a chance of ISRs accessing the same peripheral
INLINE uint16_t _read_reg16(register8_t *low, register8_t *high) {
	uint8_t sreg;
	uint16_t w;

	DISABLE_INTERRUPTS(sreg);
	w = *low;
	w |= (uint16_t )(*high) << 8U;
	RESTORE_INTERRUPTS(sreg);

	return w;
}
INLINE void _write_reg16(register8_t *low, register8_t *high, uint16_t value) {
	uint8_t sreg;

	DISABLE_INTERRUPTS(sreg);
	*low = (value & 0x00FFU);
	*high = (value & 0xFF00U) >> 8U;
	RESTORE_INTERRUPTS(sreg);

	return;
}
#if ! uHAL_SKIP_OTHER_CHECKS
# define read_reg16(_reg_) _read_reg16(_reg_ ## L, _reg_ ## H)
# define write_reg16(_reg_, _value_) _write_reg16(_reg_ ## L, _reg_ ## H, _value_)
#else
# define read_reg16(_reg_) (*(_reg_))
# define write_reg16(_reg_, _value_) (*(_reg_) = (_value_))
#endif


#endif // _uHAL_PLATFORM_XMEGA3_SYSTEM_H
