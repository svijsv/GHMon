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
// clz32.c
// Find the number of leading zeroes in a 32-bit integer
// NOTES:
//   Use __builtin_clz() if that's available
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
int clz32(uint32_t x) {
	uint i;

	for (i = 32; x != 0; --i, x >>= 1) {
		// Nothing to do here
	}
	return i;
}

/*
// https:// stackoverflow.com/questions/54661131/log2-approximation-in-fixed-point
int32_t clz32 (uint32_t x) {
	uint32_t n, y;

	n = 31 + (!x);
	if ((y = (x & 0xffff0000U))) { n -= 16;  x = y; }
	if ((y = (x & 0xff00ff00U))) { n -=  8;  x = y; }
	if ((y = (x & 0xf0f0f0f0U))) { n -=  4;  x = y; }
	if ((y = (x & 0xccccccccU))) { n -=  2;  x = y; }
	if ((    (x & 0xaaaaaaaaU))) { n -=  1;         }
	return n;
}
*/


#endif // ! USE_MONOLITHIC_ULIB
#ifdef __cplusplus
 }
#endif
