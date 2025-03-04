// SPDX-License-Identifier: GPL-3.0-only
/***********************************************************************
*                                                                      *
*                                                                      *
* Copyright 2022, 2023 svijsv                                          *
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
// util.h
// Utility functions
// NOTES:
//
//
#ifndef _ULIB_UTIL_H
#define _ULIB_UTIL_H

#include "src/configify.h"
#if ULIB_ENABLE_UTIL

#include "types.h"


//
// Initialize a block of memory
void mem_init(void *mem, uint8_t value, size_t size);

//
// Determine the system endianess
#if (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__) || defined(__BIG_ENDIAN) || defined(_BIG_ENDIAN)
# define HAVE_BIG_ENDIAN 1
# define HAVE_LITTLE_ENDIAN 0
#elif (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__) || defined(__LITTLE_ENDIAN) || defined(_LITTLE_ENDIAN)
# define HAVE_BIG_ENDIAN 0
# define HAVE_LITTLE_ENDIAN 1
#endif

//
// Check if a value is between two others.
#define IS_IN_RANGE_INCL(x, min, max) ((x >= (min)) && (x <= (max)))
#define IS_IN_RANGE_EXCL(x, min, max) ((x >  (min)) && (x <  (max)))
#define IS_IN_RANGE(x, min, max) IS_IN_RANGE_INCL(x, min, max)
//
// Find the smaller of two values.
#ifndef MIN
# define MIN(x, y) (((x) < (y)) ? (x) : (y))
#endif
//
// Find the larger of two values.
#ifndef MAX
# define MAX(x, y) (((x) < (y)) ? (y) : (x))
#endif
//
// Find the nearest multiple of some factor which isn't larger than some value
#define SNAP_TO_FACTOR(max, factor) (((factor) != 0) ? ((max) - ((max) % (factor))) : 0)
//
// Add two unsigned numbers without going over some value, which may be the
// maximum the type can hold.
#define CLIP_UADD(a, b, max) ((((max) - (a)) > (b)) ? ((a) + (b)) : (max))
//
// Multiply two unsigned numbers without going over some value, which may be
// the maximum the type can hold.
//#define CLIP_UMUL(a, b, max) ((((a) == 0 || (b) == 0) || ((a) < ((max) / (b)))) ? ((a) * (b)) : (max))
#define CLIP_UMUL(a, b, max) (((a) == 0 || (b) == 0) ? 0 : ((a) < (max) / (b)) ? ((a) * (b)) : (max))

//
// Read an integer composed of smaller integers
#define READ_SPLIT16(combined, high, low) \
	do { \
		((combined) = (uint16_t )((((uint16_t )(high)) << 8U) | ((uint16_t )(low)))); \
	} while (0);
#define READ_SPLIT32(combined, high, low) \
	do { \
		((combined) = (uint32_t )((((uint32_t )(high)) << 16U) | ((uint32_t )(low)))); \
	} while (0);
//
// Read an integer composed of smaller integers where the low half may overflow
// into the high half during the read, e.g. for asynchronous timers
#ifndef READ_SPLIT16_ASYNC
# define READ_SPLIT16_ASYNC(combined, high, low) \
	do { \
		((combined) = (uint16_t )((((uint16_t )(high)) << 8U) | ((uint16_t )(low)))); \
	} while ((uint8_t )(((combined) >> 8U) & 0xFFU) != (uint8_t )(high))
#endif
#ifndef READ_SPLIT32_ASYNC
# define READ_SPLIT32_ASYNC(combined, high, low) \
	do { \
		((combined) = (uint32_t )((((uint32_t )(high)) << 16U) | ((uint32_t )(low)))); \
	} while ((uint16_t )(((combined) >> 16U) & 0xFFFFU) != (uint16_t )(high))
#endif

//
// Write an integer composed of smaller integers
#define WRITE_SPLIT16(high, low, combined) \
	do { \
		(low ) = (uint16_t )(combined) & 0xFFU; \
		(high) = (uint16_t )(combined) >> 8U; \
	} while (0);
#define WRITE_SPLIT32(high, low, combined) \
	do { \
		(low ) = (uint32_t )(combined) & 0xFFFFU; \
		(high) = (uint32_t )(combined) >> 16U; \
	} while (0);
//
// Write an integer composed of smaller integers where the low half may overflow
// into the high half during the write
// This should probably never be used because an actually asynchronous register
// probably won't be updated right away, meaning this will get stuck in the loop
// waiting for the high half to match the new value.
#ifndef WRITE_SPLIT16_ASYNC
# define WRITE_SPLIT16_ASYNC(high, low, combined) \
	do { \
		(low ) = (uint16_t )(combined) & 0xFFU; \
		(high) = (uint16_t )(combined) >> 8U; \
	} while ((uint8_t )(((combined) >> 8U) & 0xFFU) != (uint8_t )(high))
#endif
#ifndef WRITE_SPLIT32_ASYNC
# define WRITE_SPLIT32_ASYNC(high, low, combined) \
	do { \
		(low ) = (uint32_t )(combined) & 0xFFFFU; \
		(high) = (uint32_t )(combined) >> 16U; \
	} while ((uint16_t )(((combined) >> 16U) & 0xFFFFU) != (uint16_t )(high))
#endif

//
// Write a volatile variable without atomic access; this doesn't protect
// anything trying to read it during the write
#ifndef WRITE_VOLATILE
# define WRITE_VOLATILE(set, get) \
	do { \
		(set) = (get); \
	} while ((set) != (get));
#endif
//
// Read a volatile variable without atomic access
// For now it's identical to WRITE_VOLATILE() except the volatile variable
// is 'get' instead of 'set'.
#ifndef READ_VOLATILE
# define READ_VOLATILE(set, get) \
	do { \
		(set) = (get); \
	} while ((set) != (get));
#endif

//
// Optimize a specific function independent of compiler flags.
#if !defined(DEBUG) || !DEBUG
# define OPTIMIZE_FUNCTION __attribute__((optimize("O2")))
#else
# define OPTIMIZE_FUNCTION
#endif
#define DONT_OPTIMIZE_FUNCTION __attribute__((optimize("O0")))

//
// Strongly suggest that the compiler inline a function.
#define INLINE static inline
#define ALWAYS_INLINE static inline __attribute__((always_inline))

//
// Inform the compiler that some code path is likely or unlikely.
#define LIKELY(x)    (__builtin_expect((x),1))
#define UNLIKELY(x)  (__builtin_expect((x),0))

//
// Tell the compiler to not emit warnings for an unused value.
#define UNUSED(x) ((void )(x))

//
// Calculate the number of entities in an array (as opposed to the number of
// bytes used by it).
#define SIZEOF_ARRAY(a) (sizeof((a)) / sizeof((a)[0]))

//
// Determine if a pointer is valid. This may include additional checks in
// the future.
#define POINTER_IS_VALID(x) ((x) != NULL)


#endif // ULIB_ENABLE_UTIL
#endif // _ULIB_UTIL_H
