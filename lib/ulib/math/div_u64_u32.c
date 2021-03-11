// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2003 Bernardo Innocenti <bernie@develer.com>
 *
 * Based on former do_div() implementation from asm-parisc/div64.h:
 *	Copyright (C) 1999 Hewlett-Packard Co
 *	Copyright (C) 1999 David Mosberger-Tang <davidm@hpl.hp.com>
 *
 *
 * Generic C version of 64bit/32bit division and modulo, with
 * 64bit result and 32bit remainder.
 *
 * The fast case for (n>>32 == 0) is handled inline by do_div().
 *
 * Code generated for this function might be very inefficient
 * for some CPUs. __div64_32() can be overridden by linking arch-specific
 * assembly versions such as arch/ppc/lib/div64.S and arch/sh/lib/div64.S
 * or by defining a preprocessor macro in arch/include/asm/div64.h.
 */
// div_u64_u32.c
//
// NOTES:
//   https:// github.com/torvalds/linux/blob/master/lib/math/div64.c
//   It's been adapted to my conventions, but that's only cosmetic
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
uint64_t div_u64_u32(uint64_t n, uint32_t d) {
	uint64_t rem, d64, c, res;
	uint32_t high;

	assert(d != 0);

	rem = n;
	d64 = d;
	c = 1;
	high = rem >> 32;

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

// 	if (r != NULL) {
// 		*r = rem;
// 	}

	return res;
}

#endif // ! USE_MONOLITHIC_ULIB
#ifdef __cplusplus
 }
#endif
