// SPDX-License-Identifier: GPL-3.0-only
/***********************************************************************
*                                                                      *
*                                                                      *
* Copyright 2023 svijsv                                                *
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
// printf.h
// Formatted printing support
//
// NOTES:
//
#ifndef _ULIB_PRINTF_H
#define _ULIB_PRINTF_H

#include "src/configify.h"
#if ULIB_ENABLE_PRINTF

#include "types.h"

#include <stdarg.h>


// Print printf-formatted strings one character at a time using pputc().
// printf_vv() is comparable to printf() and printf_va() to vprintf().
// printf_va() is defined weakly so that it can easily be replaced by another
// implementation.
//
// Each printf() format string takes the form:
//    %[flags][width][.precision][length modifier]conversion
//
// Of the standard flags, this implementation supports:
//    #  : Alternate forms for binary, octal, and hex output.
//    0  : Pad integers with leading 0s instead of spaces.
//    -  : Left-justify fields when a width is given.
//    ' ': Put a blank space before positive numbers in signed conversions.
//    +  : Put a '+' before positive numbers in signed conversions.
//    '  : For decimal integers, print a separator at each thousands place.
//
// Of the standard length modifiers, this implementation supports:
//    hh : char
//    h  : short
//    l  : long
//    ll : long long
//    j  : intmax_t
//    z  : size_t
//    t  : ptrdiff_t
// The specific length modifiers which can be handled properly will depend on
// the value of PRINTF_MAX_INT_BYTES.
//
// Of the standard conversions, this implementation supports:
//    d,i: Signed decimal int.
//    u  : Unsigned decimal int.
//    b  : Unsigned binary int.
//    o  : Unsigned octal int.
//    x,X: Unsigned hexadecimal int.
//    c  : Unsigned char (passed as an int).
//    s  : String.
//    %  : A literal '%'.
//
// These functions differ from the standard printf() in the following ways:
//    The 'l' flags are not supported for %c or %s.
//
//    The '%m$' notation for accessing arguments by index isn't supported.
//
//    The number of characters written is not returned.
//
//    At least one digit is always printed for integers instead of printing
//    nothing for '0' when the precision is 0.
//
void printf_vv(void (*pputc)(uint_fast8_t c), const char *restrict fmt, ...)
	__attribute__ ((format(printf, 2, 3)));
void printf_va(void (*pputc)(uint_fast8_t c), const char *restrict fmt, va_list arp);

#endif // ULIB_ENABLE_PRINTF
#endif // _ULIB_PRINTF_H
