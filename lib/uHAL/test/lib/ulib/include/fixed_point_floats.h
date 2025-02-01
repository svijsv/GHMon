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
// fixed_point_floats.h
// Floating-point functions for use when fixed-points aren't worth it
// NOTES:
//   This file should only be included by fixed_point.h.
//
//   See fixed_point.h for any notes not specifically related to the floating-
//   point substitution.
//
//

#include <float.h>

#if FIXED_POINT_REPLACE_WITH_FLOAT == 1
# define FIXED_POINT_MIN FLT_MIN
# define FIXED_POINT_MAX FLT_MAX
  typedef float fixed_point_t;

#else
# define FIXED_POINT_MIN DBL_MIN
# define FIXED_POINT_MAX DBL_MAX
 typedef double fixed_point_t;
#endif
typedef fixed_point_t fixed_point_math_t;

#define FIXED_POINT_1 ((fixed_point_t )1)

#define FIXED_POINT_FROM_INT(x) ((fixed_point_t )(x))
INLINE fixed_point_t fixed_point_from_int(fixed_point_t x) {
	return x;
}

#define FIXED_POINT_FROM_FLOAT(x) ((fixed_point_t )(x))
INLINE fixed_point_t fixed_point_from_float(float x) {
	return (fixed_point_t )(x);
}

#define FIXED_POINT_TO_INT(x) ((int )(x))
INLINE int fixed_point_to_int(fixed_point_t x) {
	return (int )x;
}
INLINE int fixed_point_to_int_rounded(fixed_point_t x) {
	return (int )(x + (fixed_point_t )0.5f);
}

INLINE float fixed_point_to_float(fixed_point_t x) {
	return (float )x;
}

#define FIXED_POINT_MUL(x, y) ((x) * (y))
INLINE fixed_point_t fixed_point_mul(fixed_point_math_t x, fixed_point_math_t y) {
	return (x * y);
}

#define FIXED_POINT_MUL_BY_INT(f, n) ((fixed_point_math_t )(f) * (fixed_point_t )(n))
INLINE fixed_point_t fixed_point_mul_by_int(fixed_point_t x, fixed_point_t y) {
	return (x * y);
}

#define FIXED_POINT_DIV(n, d) ((n) / (d))
INLINE fixed_point_t fixed_point_div(fixed_point_t n, fixed_point_t d) {
	return n / d;
}

#define FIXED_POINT_DIV_BY_INT(f, n) ((f) / (n))
INLINE fixed_point_t fixed_point_div_by_int(fixed_point_t f, fixed_point_t n) {
	return f / n;
}

fixed_point_t log_fixed_point(fixed_point_t x);
fixed_point_t log10_fixed_point(fixed_point_t x);
fixed_point_t log2_fixed_point(fixed_point_t x);
