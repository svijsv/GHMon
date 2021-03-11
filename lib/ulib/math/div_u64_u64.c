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
// div_u64_u64.c
//
// NOTES:
//   This doesn't use the version of the function from the linux source
//   because that truncates the divisor
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
// https:// www.bbcelite.com/deep_dives/shift-and-subtract_division.html
uint64_t div_u64_u64(uint64_t n, uint64_t d) {
	uint64_t high, res, rem;
	high = n >> 32;

	assert(d != 0);

	if (d > n) {
		return 0;
	}

	if (high == 0) {
		res = div_u64_u32(n, d);
	} else {
		uint64_t im = (uint64_t )1U << 63;
		res = 0;
		rem = 0;

		for (uiter_t i = 0; i < 64; ++i) {
			rem <<= 1;
			rem |= ((n & im) != 0);
			if (rem >= d) {
				rem -= d;
				res |= im;
			}
			im >>= 1;
		}
	}

	return res;
}

/*
// This is the linux version, which truncates the divisor:
uint64_t div_u64_u64(uint64_t n, uint64_t d) {
	uint64_t high, res;
	high = n >> 32;

	if (high == 0) {
		res = div_u64_u32(n, d);
	} else {
		int s;
		s = MSB32_IDX(high);
		res = div_u64_u32(n >> s, d >> s);

		if (res != 0) {
			res--;
		}
		if ((n - (res * d)) >= d) {
			res++;
		}
	}

	return res;
}
*/


#endif // ! USE_MONOLITHIC_ULIB
#ifdef __cplusplus
 }
#endif
