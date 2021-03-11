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
// msb32.c
// Find the most-significant set bit in a 32-bit integer
// NOTES:
//   https:// rosettacode.org/wiki/Find_first_and_last_set_bit_of_a_long_integer#C
//      I can't find a license there, don't know how to handle that
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
#define step(x) if (n >= ((uint32_t)1) << x) b <<= x, n >>= x
uint32_t msb32(uint32_t n) {
	uint32_t b = 1;

	step(16); step(8); step(4); step(2); step(1);
	return b;
}
#undef step


#endif // ! USE_MONOLITHIC_ULIB
#ifdef __cplusplus
 }
#endif
