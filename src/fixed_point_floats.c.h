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
// fixed_point_floats.c
// Floating-point functions for use when fixed-points aren't worth it
// NOTES:
//   This file should only be included by fixed_point.c.
//
//   See fixed_point.c for any notes not specifically related to the floating-
//   point substitution.
//
//   I haven't extensively tested the various ways of calculating log() and
//   log10() but calling only log2() and then multiplying by the pre-computed
//   values seems to be marginally smaller than using the libmath log() and
//   log10() functions at the cost of being very slightly less accurate.
//
#include "debug.h"

#include <math.h>

#define LOGE_2  ((fixed_point_t )0.69314718056)
#define LOG10_2 ((fixed_point_t )0.301029995664)

#if FIXED_POINT_REPLACE_WITH_FLOAT == 1
# define LOG2(x)  log2f(x)
# define LOG10(x) log10f(x)
# define LOGE(x)  logf(x)
#else
# define LOG2(x)  log2(x)
# define LOG10(x) log10(x)
# define LOGE(x)  log(x)
#endif
//#define LOG10(x) (LOG2(x) * LOG10_2)
//#define LOGE(x)  (LOG2(x) * LOGE_2)

/*
fixed_point_t log10_fixed_point(fixed_point_t x) {
	ulib_assert(x > 0);

#if DO_FIXED_POINT_SAFETY_CHECKS
	if (x == 0) {
		return 0;
	}
#endif

	return LOG10(x);
}
*/
fixed_point_t log10_fixed_point(fixed_point_t x) {
	return log2_fixed_point(x) * LOG10_2;
}

/*
fixed_point_t log_fixed_point(fixed_point_t x) {
	ulib_assert(x > 0);

#if DO_FIXED_POINT_SAFETY_CHECKS
	if (x == 0) {
		return 0;
	}
#endif

	return LOGE(x);
}
*/
fixed_point_t log_fixed_point(fixed_point_t x) {
	return log2_fixed_point(x) * LOGE_2;
}

fixed_point_t log2_fixed_point(fixed_point_t x) {
	ulib_assert(x > 0);

#if DO_FIXED_POINT_SAFETY_CHECKS
	if (x == 0) {
		return 0;
	}
#endif

	return LOG2(x);
}
