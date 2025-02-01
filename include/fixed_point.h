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
// fixed_point.h
// Fixed-point functions for use when floats aren't worth it
// NOTES:
//
//
#ifndef _ULIB_FIXED_POINT_H
#define _ULIB_FIXED_POINT_H

#include "src/configify.h"
#if ULIB_ENABLE_FIXED_POINT

#include "types.h"
#include "util.h"

#if FIXED_POINT_REPLACE_WITH_FLOAT <= 0

// fixed_point_t is the type used to store representations.
// fixed_point_math_t is the type used for operations that may overflow fixed_point_t.
#if FIXED_POINT_BITS > 64
# error "FIXED_POINT_BITS is too big"
#elif FIXED_POINT_BITS == 64
# define FIXED_POINT_MIN INT64_MIN
# define FIXED_POINT_MAX INT64_MAX
  typedef int64_t fixed_point_t;
  // Maybe some day I'll be able to count on int128_t...
  //typedef __int128 fixed_point_math_t;
  typedef int64_t fixed_point_math_t;
#elif FIXED_POINT_BITS == 32
# define FIXED_POINT_MIN INT32_MIN
# define FIXED_POINT_MAX INT32_MAX
  typedef int32_t fixed_point_t;
  typedef int64_t fixed_point_math_t;
#elif FIXED_POINT_BITS == 16
# define FIXED_POINT_MIN INT16_MIN
# define FIXED_POINT_MAX INT16_MAX
  typedef int16_t fixed_point_t;
  typedef int32_t fixed_point_math_t;
#elif FIXED_POINT_BITS == 8
# define FIXED_POINT_MIN INT8_MIN
# define FIXED_POINT_MAX INT8_MAX
  typedef int8_t fixed_point_t;
  typedef int16_t fixed_point_math_t;
#else
# error "Unsupported FIXED_POINT_BITS size"
#endif

//
// '1' converted to our fixed-point notation.
#define FIXED_POINT_1 ((fixed_point_t )1 << FIXED_POINT_FRACT_BITS)
//
// The mask for the fraction bits of a fixed-point number
#define FIXED_POINT_FRACTION_MASK (FIXED_POINT_1 - 1)
//
// The mask for the whole-number bits of a fixed-point number
#define FIXED_POINT_WHOLE_MASK (~FIXED_POINT_FRACTION_MASK)

//
// Multiply two fixed-point numbers.
// (X*Sx) * (Y*Sy) = (X*Y) * (Sx*Sy)
// The scaling factor is squared, so one needs to be removed
// ((X*Y) * (S*S)) / S
#define FIXED_POINT_MUL(x, y) ((fixed_point_t )(((fixed_point_math_t )(x) * (fixed_point_math_t )(y)) >> FIXED_POINT_FRACT_BITS))
INLINE fixed_point_t fixed_point_mul(fixed_point_math_t x, fixed_point_math_t y) {
	return (fixed_point_t )((x * y) >> FIXED_POINT_FRACT_BITS);
}
//
// Multiply a fixed-point number by an integer.
// Integers are unscaled so no shift is necessary.
#define FIXED_POINT_MUL_BY_INT(f, n) ((fixed_point_math_t )(f) * (n))
INLINE fixed_point_t fixed_point_mul_by_int(fixed_point_t x, fixed_point_t y) {
	return (x * y);
}
//
// Divide two fixed-point numbers.
// https:// en.wikipedia.org/wiki/Fixed-point_arithmetic#Operations
// (X*Sx) / (Y*Sy) = (X/Y) * (Sy/Sx)
// The scaling factor cancels out, so it needs to be re-added
// ((X/Y) * (S/S)) * S
// Use an overridable macro for division because a certain platform doesn't
// have native support for 64 bit division and using libc adds a full Kb to
// the .data section of our binary...
#if !defined(_FIXED_POINT_DIV_PRIM)
# define _FIXED_POINT_DIV_PRIM(_n, _d) ((_n) / (_d))
#endif
#define FIXED_POINT_DIV(n, d) ((fixed_point_t )(_FIXED_POINT_DIV_PRIM(((fixed_point_math_t )(n) << FIXED_POINT_FRACT_BITS), (fixed_point_t )(d))))
INLINE fixed_point_t fixed_point_div(fixed_point_math_t n, fixed_point_t d) {
	return (fixed_point_t )_FIXED_POINT_DIV_PRIM((n << FIXED_POINT_FRACT_BITS), d);
}
//
// Divide a fixed-point number by an integer.
// Integers are unscaled so no shift is necessary.
#define FIXED_POINT_DIV_BY_INT(f, n) (_FIXED_POINT_DIV_PRIM((fixed_point_t )f, n))
INLINE fixed_point_t fixed_point_div_by_int(fixed_point_t f, fixed_point_t n) {
	return (fixed_point_t )_FIXED_POINT_DIV_PRIM(f, n);
}

//
// Calculate the natural logarithm of a fixed-point number.
fixed_point_t log_fixed_point(fixed_point_t x);
//
// Calculate the base 10 logarithm of a fixed-point number.
fixed_point_t log10_fixed_point(fixed_point_t x);
//
// Calculate the base 2 logarithm of a fixed-point number.
fixed_point_t log2_fixed_point(fixed_point_t x);

//
// Create a fixed-point number from an integer
#define FIXED_POINT_FROM_INT(x) ((fixed_point_t )(x) << FIXED_POINT_FRACT_BITS)
INLINE fixed_point_t fixed_point_from_int(fixed_point_t x) {
	return x << FIXED_POINT_FRACT_BITS;
}
//
// Create a fixed-point number from an integer numerator and denominator
#define FIXED_POINT_FROM_INT_FRACTION(n, d) (FIXED_POINT_DIV_BY_INT(FIXED_POINT_FROM_INT(n), d))
INLINE fixed_point_t fixed_point_from_int_fraction(fixed_point_t n, fixed_point_t d) {
	return fixed_point_div_by_int(fixed_point_from_int(n), d);
}
//
// Create a fixed-point number from a float
#define FIXED_POINT_FROM_FLOAT(x) ((x) * (float )FIXED_POINT_1)
INLINE fixed_point_t fixed_point_from_float(float x) {
	return (fixed_point_t )(x * (float )FIXED_POINT_1);
}
//
// Create an integer from a fixed-point number.
#define FIXED_POINT_TO_INT(x) ((fixed_point_t )(x) >> FIXED_POINT_FRACT_BITS)
INLINE fixed_point_t fixed_point_to_int(fixed_point_t x) {
	return x >> FIXED_POINT_FRACT_BITS;
}
//
// Create an integer from a fixed-point number, rounded.
#define FIXED_POINT_TO_INT_ROUNDED(x) (((fixed_point_t )x + (FIXED_POINT_1 / 2)) >> FIXED_POINT_FRACT_BITS)
INLINE fixed_point_t fixed_point_to_int_rounded(fixed_point_t x) {
	return (x + (FIXED_POINT_1 / 2)) >> FIXED_POINT_FRACT_BITS;
}
//
// Create a float from a fixed-point number.
INLINE float fixed_point_to_float(fixed_point_t x) {
	return (float )x / (float )FIXED_POINT_1;
}

#else // FIXED_POINT_REPLACE_WITH_FLOAT <= 0
# include "fixed_point_floats.h"
#endif // FIXED_POINT_REPLACE_WITH_FLOAT <= 0
#endif // ULIB_ENABLE_FIXED_POINT
#endif // _ULIB_FIXED_POINT_H
