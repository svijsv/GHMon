// SPDX-License-Identifier: GPL-3.0-only
/***********************************************************************
*                                                                      *
*                                                                      *
* Copyright 2021 svijsv
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
// div_u64_u16.c
//
// NOTES:
//

#ifdef __cplusplus
 extern "C" {
#endif
#if ! USE_MONOLITHIC_ULIB

//#define DEBUG 1
//#define NDEBUG 1
/*
* Includes
*/
#include "private.h"


/*
* Static values
*/


/*
* Types
*/


/*
* Variables
*/


/*
* Function prototypes
*/


/*
* Macros
*/


/*
* Interrupt handlers
*/


/*
* Functions
*/
// This is just long division with 16-bit 'digits'
uint64_t div_u64_u16(uint64_t n, uint16_t d) {
	uint32_t r32, d32;
	union {
		uint64_t i;
		struct {
#if __BYTE_ORDER == __BIG_ENDIAN
			uint16_t d;
			uint16_t c;
			uint16_t b;
			uint16_t a;
#elif __BYTE_ORDER == __LITTLE_ENDIAN
			uint16_t a;
			uint16_t b;
			uint16_t c;
			uint16_t d;
#else
# error "Unhandled endianness"
#endif
		} p;
	} nparts, rparts;

	assert(d != 0);

	if (d > n) {
		return 0;
	}

	nparts.i = n;
	d32 = d;

	r32 = nparts.p.a;
	rparts.p.a = (r32 / d32);

	r32 = (r32 % d) << 16;
	r32 |= nparts.p.b;
	rparts.p.b = (r32 / d32);

	r32 = (r32 % d) << 16;
	r32 |= nparts.p.c;
	rparts.p.c = (r32 / d32);

	r32 = (r32 % d) << 16;
	r32 |= nparts.p.d;
	rparts.p.d = (r32 / d32);

	return rparts.i;
}


#endif // ! USE_MONOLITHIC_ULIB
#ifdef __cplusplus
 }
#endif
