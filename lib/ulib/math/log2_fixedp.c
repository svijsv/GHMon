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
// log2fixedp.c
//
// NOTES:
//

#ifdef __cplusplus
 extern "C" {
#endif
#if ! USE_MONOLITHIC_ULIB

//#define DEBUG 1
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
* Local function prototypes
*/


/*
* Interrupt handlers
*/


/*
* Functions
*/
FIXEDP_ITYPE log2_fixedp(FIXEDP_ITYPE x) {
	FIXEDP_ITYPE log2_x, b;
	FIXEDP_MTYPE z;

	assert(x > 0);

	log2_x = 0;
	b = 1U << (FIXEDP_FRACT_BITS - 1);

	// https:// stackoverflow.com/questions/4657468/fast-fixed-point-pow-log-exp-and-sqrt
	//   (Second answer)
	//    references http:// www.claysturner.com/dsp/BinaryLogarithm.pdf
	// https:// github.com/dmoulding/log2fix
	while (x < FIXEDP_1) {
		x <<= 1;
		log2_x -= FIXEDP_1;
	}
	while (x >= (2 << FIXEDP_FRACT_BITS)) {
		x >>= 1;
		log2_x += 1 << FIXEDP_FRACT_BITS;
	}
	z = x;
	for (uiter_t i = 0; i < FIXEDP_FRACT_BITS; ++i) {
		z = (z * z) >> FIXEDP_FRACT_BITS;
		if (z >= (2U << FIXEDP_FRACT_BITS)) {
			z >>= 1;
			log2_x += b;
		}
		b >>= 1;
	}

	return log2_x;
}

/*
// https:// stackoverflow.com/questions/54661131/log2-approximation-in-fixed-point
#define FRAC_BITS_OUT (FIXEDP_FRACT_BITS)
#define INT_BITS_OUT  (32-FIXEDP_FRACT_BITS)
#define FRAC_BITS_IN  (FIXEDP_FRACT_BITS)
#define INT_BITS_IN   (32-FIXEDP_FRACT_BITS)
#define RND_SHIFT     (31 - FRAC_BITS_OUT)
#define RND_CONST     ((1 << RND_SHIFT) / 2)
#define RND_ADJUST    (0x10d) // established heuristically
#define LOG2_TBL_SIZE (6)
#define TBL_SIZE      ((1 << LOG2_TBL_SIZE) + 2)
int32_t log2_fp(int32_t x) {
	// for i = [0,65]: log2(1 + i/64) * (1 << 31)
	static _FLASH const uint32_t log2Tab [TBL_SIZE] = {
		0x00000000, 0x02dcf2d1, 0x05aeb4dd, 0x08759c50, 
		0x0b31fb7d, 0x0de42120, 0x108c588d, 0x132ae9e2, 
		0x15c01a3a, 0x184c2bd0, 0x1acf5e2e, 0x1d49ee4c, 
		0x1fbc16b9, 0x22260fb6, 0x24880f56, 0x26e2499d, 
		0x2934f098, 0x2b803474, 0x2dc4439b, 0x30014ac6, 
		0x32377512, 0x3466ec15, 0x368fd7ee, 0x38b25f5a, 
		0x3acea7c0, 0x3ce4d544, 0x3ef50ad2, 0x40ff6a2e, 
		0x43041403, 0x450327eb, 0x46fcc47a, 0x48f10751, 
		0x4ae00d1d, 0x4cc9f1ab, 0x4eaecfeb, 0x508ec1fa, 
		0x5269e12f, 0x5440461c, 0x5612089a, 0x57df3fd0, 
		0x59a80239, 0x5b6c65aa, 0x5d2c7f59, 0x5ee863e5, 
		0x60a02757, 0x6253dd2c, 0x64039858, 0x65af6b4b, 
		0x675767f5, 0x68fb9fce, 0x6a9c23d6, 0x6c39049b, 
		0x6dd2523d, 0x6f681c73, 0x70fa728c, 0x72896373, 
		0x7414fdb5, 0x759d4f81, 0x772266ad, 0x78a450b8, 
		0x7a231ace, 0x7b9ed1c7, 0x7d17822f, 0x7e8d3846, 
		0x80000000, 0x816fe50b
	};
	int32_t f1, f2, dx, a, b, approx, lz, i, idx;
	uint32_t t;

	// x = 2**i * (1 + f), 0 <= f < 1. Find i
	lz = clz (x);
	i = INT_BITS_IN - lz;
	// normalize f
	t = (uint32_t)x << (lz + 1);
	// index table of log2 values using LOG2_TBL_SIZE msbs of fraction
	idx = t >> (32 - LOG2_TBL_SIZE);
	// difference between argument and smallest sampling point
	dx = t - (idx << (32 - LOG2_TBL_SIZE));
	// fit parabola through closest three sampling points; find coeffs a, b
	f1 = (log2Tab[idx+1] - log2Tab[idx]);
	f2 = (log2Tab[idx+2] - log2Tab[idx]);
	a = f2 - (f1 << 1);
	b = (f1 << 1) - a;
	// find function value for argument by computing ((a*dx+b)*dx)
	approx = (int32_t)((((int64_t)a)*dx) >> (32 - LOG2_TBL_SIZE)) + b;
	approx = (int32_t)((((int64_t)approx)*dx) >> (32 - LOG2_TBL_SIZE + 1));
	approx = log2Tab[idx] + approx;
	// round fractional part of result
	approx = (((uint32_t)approx) + RND_CONST + RND_ADJUST) >> RND_SHIFT;
	// combine integer and fractional parts of result
	return (i << FRAC_BITS_OUT) + approx;
}
*/


#endif // ! USE_MONOLITHIC_ULIB
#ifdef __cplusplus
 }
#endif
