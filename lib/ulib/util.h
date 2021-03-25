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
// util.h
// Small utility functions
// NOTES:
//

#ifdef __cplusplus
 extern "C" {
#endif
#ifndef _ULIB_UTIL_H
#define _ULIB_UTIL_H

/*
* Includes
*/
#include "common.h"
#include "types.h"

#include <stdarg.h>


/*
* Static values
*/


/*
* Types
*/
// A putc() function compatible with vvprintf() and vaprintf() below
typedef void (*printf_putc_t)(int c);


/*
* Variable declarations
*/


/*
* Function prototypes
*/
// Initialize a block of memory
void mem_init(void *mem, uint8_t value, uint32_t size);

// Two printf() shims
void vvprintf(printf_putc_t printf_putc, const char *fmt, ...)
	__attribute__ ((format(printf, 2, 3)));
void vaprintf(printf_putc_t printf_putc, const char *fmt, va_list arp);


/*
* Macros
*/
#define LIKELY(x)    __builtin_expect((x),1)
#define UNLIKELY(x)  __builtin_expect((x),0)

#define UNUSED(x) ((void )(x))

#define IS_BETWEEN(x, min, max) ((x >= (min)) && (x <= (max)))

// Find the nearest multiple of some factor which isn't larger than some value
#define SNAP_TO_FACTOR(value, factor) (((factor) != 0) ? ((value) - ((value) % (factor))) : 0)

// Find absolute value of an integer; naive approach is good enough
//#define ABS(x) (((x) > 0) ? (x) : -(x))

// Copy a split register while making sure the low half doesn't overflow into
// high in the process
#define READ_SPLITREG(combined, high, low) \
	do { \
		((combined) = (((uint32_t )(high)) << 16) | (((uint32_t )(low)) & 0xFFFF)); \
	} while (((combined) & 0xFFFF0000) != (((uint32_t )(high)) << 16));
#define READ_SPLITREG16(combined, high, low) \
	do { \
		((combined) = (((uint16_t )(high)) << 8) | (((uint16_t )(low)) & 0xFF)); \
	} while (((combined) & 0xFF00) != (((uint16_t )(high)) << 8));

// Write to a split register while making sure the low half doesn't overflow
// into high in the process
#define WRITE_SPLITREG(combined, high, low) \
	do { \
		(high) = (uint16_t )((combined) >> 16); \
		(low)  = (uint16_t )((combined) & 0xFFFF); \
	} while (((combined) & 0xFFFF0000) != (((uint32_t )(high)) << 16));
#define WRITE_SPLITREG16(combined, high, low) \
	do { \
		(high) = (uint8_t )((combined) >> 8); \
		(low)  = (uint8_t )((combined) & 0xFF); \
	} while (((combined) & 0xFF00) != (((uint16_t )(high)) << 8));

// Write a volatile variable without atomic access; this doesn't protect
// anything trying to read it during the write
#define WRITE_VOLATILE(set, get) \
	do { \
		(set) = (get); \
	} while ((set) != (get));
// Read a volatile variable without atomic access
// For now, it's identical to WRITE_VOLATILE()
#define READ_VOLATILE(set, get) \
	do { \
		(set) = (get); \
	} while ((set) != (get));

#if !DEBUG
# define OPTIMIZE_FUNCTION __attribute__((optimize("O2")))
# define DONT_OPTIMIZE_FUNCTION __attribute__((optimize("O0")))
#else
# define OPTIMIZE_FUNCTION
# define DONT_OPTIMIZE_FUNCTION
#endif

// Use these to show the value of a macro at compile-time
// https://stackoverflow.com/questions/1562074/how-do-i-show-the-value-of-a-define-at-compile-time
#define XTRINGIZE(x) STRINGIZE(x)
#define STRINGIZE(x) #x
//#pragma message "The value of ABC: " XTRINGIZE(ABC)


#endif // _LIB_UTIL_H
#ifdef __cplusplus
 }
#endif
