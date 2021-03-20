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
// math.h
// Math functions for use when libc isn't worth it
// NOTES:
//

#ifdef __cplusplus
 extern "C" {
#endif
#ifndef _ULIB_MATH_H
#define _ULIB_MATH_H

/*
* Includes
*/
#include "common.h"

#include "types.h"
#include "util.h"


/*
* Static values
*/
// Shifts needed to adjust a value by a given power of 2
#define P2_2_SHIFTS    (1)
#define P2_4_SHIFTS    (2)
#define P2_8_SHIFTS    (3)
#define P2_16_SHIFTS   (4)
#define P2_32_SHIFTS   (5)
#define P2_64_SHIFTS   (6)
#define P2_128_SHIFTS  (7)
#define P2_256_SHIFTS  (8)
#define P2_512_SHIFTS  (9)
#define P2_1024_SHIFTS (10)

//
// User-overrideable values
//
// Type used to store fixed-point numbers; may need to be signed, depending
// on operations
#ifndef FIXEDP_ITYPE
# define FIXEDP_ITYPE int32_t
#endif
// Type used for math operations which may overflow; should be signed if
// FIXEDP_ITYPE is
#ifndef FIXEDP_MTYPE
# define FIXEDP_MTYPE int64_t
#endif
// Number of bits reserved for the fraction portion
#ifndef FIXEDP_FRACT_BITS
# define FIXEDP_FRACT_BITS 8
#endif

//
// Constants
//
// '1' converted to our fixed-point notation
#define FIXEDP_1 ((FIXEDP_ITYPE )1 << FIXEDP_FRACT_BITS)


/*
* Types
*/


/*
* Variable declarations
*/


/*
* Function prototypes
*/
//
// Software 64-bit math operations for systems where it's not natively
// supported and the use of libc isn't desired
//
// Divide n by d
uint64_t div_u64_u16(uint64_t n, uint16_t d);
uint64_t div_u64_u32(uint64_t n, uint32_t d);
uint64_t div_u64_u64(uint64_t n, uint64_t d);
int64_t  div_s64_s64( int64_t n,  int64_t d);

//
// Fixed-point operations
//
// Calculate the natural logarithm of a fixed-point number
FIXEDP_ITYPE log_fixedp(FIXEDP_ITYPE x);
// Calculate the base 2 logarithm of a fixed-point number
FIXEDP_ITYPE log2_fixedp(FIXEDP_ITYPE x);
// Calculate the base 10 logarithm of a fixed-point number
FIXEDP_ITYPE log10_fixedp(FIXEDP_ITYPE x);

/*
* Macros
*/
// Absolute value of an integer, naive-but-simple approach
#define ABS(x) (((x) > 0) ? (x) : -(x))

// Quick power-of-2 division for unsigned ints
#define SHIFT_DIV_2(x)    ((x) >>  1)
#define SHIFT_DIV_4(x)    ((x) >>  2)
#define SHIFT_DIV_8(x)    ((x) >>  3)
#define SHIFT_DIV_16(x)   ((x) >>  4)
#define SHIFT_DIV_32(x)   ((x) >>  5)
#define SHIFT_DIV_64(x)   ((x) >>  6)
#define SHIFT_DIV_128(x)  ((x) >>  7)
#define SHIFT_DIV_256(x)  ((x) >>  8)
#define SHIFT_DIV_512(x)  ((x) >>  9)
#define SHIFT_DIV_1024(x) ((x) >> 10)

// Quick power-of-2 multiplication for unsigned ints
#define SHIFT_MUL_2(x)    ((x) <<  1)
#define SHIFT_MUL_4(x)    ((x) <<  2)
#define SHIFT_MUL_8(x)    ((x) <<  3)
#define SHIFT_MUL_16(x)   ((x) <<  4)
#define SHIFT_MUL_32(x)   ((x) <<  5)
#define SHIFT_MUL_64(x)   ((x) <<  6)
#define SHIFT_MUL_128(x)  ((x) <<  7)
#define SHIFT_MUL_256(x)  ((x) <<  8)
#define SHIFT_MUL_512(x)  ((x) <<  9)
#define SHIFT_MUL_1024(x) ((x) << 10)


//
// Fixed-point operations
//
// Use an overridable macro for division because a certain platform doesn't
// have native support for 64 bit division and using libc adds a full Kb to
// the .data section of our binary...
#ifndef FIXEDP_DIVI
# define FIXEDP_DIVI(x, y) ((FIXEDP_ITYPE )(x) / (FIXEDP_ITYPE )(y))
#endif

// Create a fixed-point number from an integer
#define FIXEDP_FROM(x) ((FIXEDP_ITYPE )(x) << FIXEDP_FRACT_BITS)
// Create an integer from a fixed-point number
#define FIXEDP_AWAY(x) ((FIXEDP_ITYPE )(x) >> FIXEDP_FRACT_BITS)

// (X*Sx) * (Y*Sy) = (X*Y) * (Sx*Sy)
// The scaling factor is squared, so one needs to be removed
// ((X*Y) * (S*S)) / S
#define FIXEDP_MUL(x, y) (((FIXEDP_MTYPE )(x) * (FIXEDP_MTYPE )(y)) >> FIXEDP_FRACT_BITS)
// https:// en.wikipedia.org/wiki/Fixed-point_arithmetic#Operations
// (X*Sx) / (Y*Sy) = (X/Y) * (Sy/Sx)
// The scaling factor cancels out, so it needs to be re-added
// ((X/Y) * (S/S)) * S
#define FIXEDP_DIV(x, y) (FIXEDP_DIVI(((FIXEDP_MTYPE )(x) << FIXEDP_FRACT_BITS), (FIXEDP_ITYPE )(y)))


#endif // _LIB_MATH_H
#ifdef __cplusplus
 }
#endif
