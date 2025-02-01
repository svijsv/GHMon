// SPDX-License-Identifier: GPL-3.0-only
/***********************************************************************
*                                                                      *
*                                                                      *
* Copyright 2022, 2023, 2025 svijsv                                    *
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
* along with this program. If not, see <http:// www.gnu.org/licenses/>.*
*                                                                      *
*                                                                      *
***********************************************************************/
// math.c
// Math functions for use when libc isn't worth it
// NOTES:
//    https://github.com/torvalds/linux/blob/master/lib/math/div64.c
//    It's been adapted to my conventions, but that's only cosmetic.
//
//    https://www.bbcelite.com/deep_dives/shift-and-subtract_division.html
//    Used for div_u64_u64() because the Linux version truncates the divisor.
//
//
#include "math.h"
#if ULIB_ENABLE_MATH

#include "debug.h"
#include "util.h"

int64_t div_s64_s64(int64_t n, int64_t d) {
	int64_t res, t;

	ulib_assert(d != 0);

#if DO_MATH_SAFETY_CHECKS
	if (d == 0) {
		return n;
	}
#endif

	t = (n ^ d) >> 63;
	res = (int64_t )div_u64_u64((uint64_t )ABS(n), (uint64_t )ABS(d));

	return (res ^ t) - t;
}

// This is just long division with 16-bit 'digits'
uint64_t div_u64_u16(uint64_t n, uint16_t d) {
	uint32_t r32, d32;
	union {
		uint64_t i;
		struct {
#if HAVE_BIG_ENDIAN
			uint16_t d;
			uint16_t c;
			uint16_t b;
			uint16_t a;
#elif HAVE_LITTLE_ENDIAN
			uint16_t a;
			uint16_t b;
			uint16_t c;
			uint16_t d;
#else
# error "Unhandled endianness"
#endif
		} p;
	} nparts, rparts;

	ulib_assert(d != 0);

#if DO_MATH_SAFETY_CHECKS
	if (d == 0) {
		return n;
	}
#endif

	if (d > n) {
		return 0;
	}

	nparts.i = n;
	d32 = d;

	r32 = nparts.p.a;
	rparts.p.a = (uint16_t )(r32 / d32);

	r32 = (r32 % d) << 16;
	r32 |= nparts.p.b;
	rparts.p.b = (uint16_t )(r32 / d32);

	r32 = (r32 % d) << 16;
	r32 |= nparts.p.c;
	rparts.p.c = (uint16_t )(r32 / d32);

	r32 = (r32 % d) << 16;
	r32 |= nparts.p.d;
	rparts.p.d = (uint16_t )(r32 / d32);

	return rparts.i;
}

uint64_t div_u64_u32(uint64_t n, uint32_t d) {
	uint64_t rem, d64, c, res;
	uint32_t high;

	ulib_assert(d != 0);

#if DO_MATH_SAFETY_CHECKS
	if (d == 0) {
		return n;
	}
#endif

	rem = n;
	d64 = d;
	c = 1;
	high = (uint32_t )(rem >> 32);

	res = 0;
	if (high >= d) {
		high /= d;
		res = (uint64_t )high << 32;
		rem -= (uint64_t )(high * d) << 32;
	}

	while (((int64_t )d64 > 0) && (d64 < rem)) {
		d64 = d64 + d64;
		c = c + c;
	}

	do {
		if (rem >= d64) {
			rem -= d64;
			res += c;
		}
		d64 >>= 1;
		c >>= 1;
	} while (c);

	/*
	if (ret_rem != NULL) {
		*ret_rem = rem;
	}
	*/

	return res;
}

uint64_t div_u64_u64(uint64_t n, uint64_t d) {
	uint64_t high, res, rem;
	high = n >> 32;

	ulib_assert(d != 0);

#if DO_MATH_SAFETY_CHECKS
	if (d == 0) {
		return n;
	}
#endif

	if (d > n) {
		return 0;
	}

	if (high == 0) {
		res = div_u64_u32(n, (uint32_t )d);
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

#else
	// ISO C forbids empty translation units, this makes it happy.
	typedef int make_iso_compilers_happy;
#endif // ULIB_ENABLE_MATH
