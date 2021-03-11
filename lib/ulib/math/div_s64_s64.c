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
// div_s64_s64.c
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
int64_t div_s64_s64(int64_t n, int64_t d) {
	int64_t res, t;

	assert(d != 0);

	t = (n ^ d) >> 63;
	res = div_u64_u64(ABS(n), ABS(d));
	return (res ^ t) - t;
	return res;
}


#endif // ! USE_MONOLITHIC_ULIB
#ifdef __cplusplus
 }
#endif
