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
// math.h
// Math functions for use when libc isn't worth it
// NOTES:
//
//
#ifndef _ULIB_MATH_H
#define _ULIB_MATH_H

#include "src/configify.h"
#if ULIB_ENABLE_MATH

#include "types.h"


/*
* Integer operations
*/
// Absolute value of an integer, naive-but-simple approach.
#ifndef ABS
# define ABS(x) (((x) > 0) ? (x) : -(x))
#endif
//
// Like ABS(), but negative.
#ifndef NABS
# define NABS(x) (((x) > 0) ? -(x) : (x))
#endif
//
// Round a division instead of truncating.
#define DIV_ROUNDED(n, d) (((n) + ((d)/2)) / (d))
//
// Test for truncation in a division.
#define DIV_IS_TRUNCATED(n, d) ((((n) / (d)) * (d)) != (n))


/*
* Power-of-two operations
*/
// Shifts needed to adjust a value by a given power of 2
#define POW2_2_SHIFTS    (1U)
#define POW2_4_SHIFTS    (2U)
#define POW2_8_SHIFTS    (3U)
#define POW2_16_SHIFTS   (4U)
#define POW2_32_SHIFTS   (5U)
#define POW2_64_SHIFTS   (6U)
#define POW2_128_SHIFTS  (7U)
#define POW2_256_SHIFTS  (8U)
#define POW2_512_SHIFTS  (9U)
#define POW2_1024_SHIFTS (10U)
//
// Quick power-of-2 division for unsigned ints
#define SHIFT_DIV_2(x)    ((x) >>  1U)
#define SHIFT_DIV_4(x)    ((x) >>  2U)
#define SHIFT_DIV_8(x)    ((x) >>  3U)
#define SHIFT_DIV_16(x)   ((x) >>  4U)
#define SHIFT_DIV_32(x)   ((x) >>  5U)
#define SHIFT_DIV_64(x)   ((x) >>  6U)
#define SHIFT_DIV_128(x)  ((x) >>  7U)
#define SHIFT_DIV_256(x)  ((x) >>  8U)
#define SHIFT_DIV_512(x)  ((x) >>  9U)
#define SHIFT_DIV_1024(x) ((x) >> 10U)
//
// Quick power-of-2 multiplication for unsigned ints
#define SHIFT_MUL_2(x)    ((x) <<  1U)
#define SHIFT_MUL_4(x)    ((x) <<  2U)
#define SHIFT_MUL_8(x)    ((x) <<  3U)
#define SHIFT_MUL_16(x)   ((x) <<  4U)
#define SHIFT_MUL_32(x)   ((x) <<  5U)
#define SHIFT_MUL_64(x)   ((x) <<  6U)
#define SHIFT_MUL_128(x)  ((x) <<  7U)
#define SHIFT_MUL_256(x)  ((x) <<  8U)
#define SHIFT_MUL_512(x)  ((x) <<  9U)
#define SHIFT_MUL_1024(x) ((x) << 10U)


/*
* Software 64-bit math operations
*/
// Divide n by d
uint64_t div_u64_u16(uint64_t n, uint16_t d);
uint64_t div_u64_u32(uint64_t n, uint32_t d);
uint64_t div_u64_u64(uint64_t n, uint64_t d);
int64_t  div_s64_s64( int64_t n,  int64_t d);


#endif // ULIB_ENABLE_MATH
#endif // _ULIB_MATH_H
