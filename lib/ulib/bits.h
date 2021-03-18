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
// bits.h
// Bitwise operation macros
// NOTES:
//   Macros should only ever expand any given argument once if it can be
//   helped so that any arguments with side-effects (like registers that
//   reset themselves) don't cause problems. Of course some already don't.
//
//   http:// realtimecollisiondetection.net/blog/?p=78
//   http:// graphics.stanford.edu/~seander/bithacks.html
//   https:// gcc.gnu.org/onlinedocs/gcc/Other-Builtins.html
//
// TODO:
// something to get/set flags in array bitfields like uint8_t flags[(FLAG_COUNT/8)+1]
//
//

#ifdef __cplusplus
 extern "C" {
#endif
#ifndef _ULIB_BITS_H
#define _ULIB_BITS_H

/*
* Includes
*/
#include "common.h"

#include "types.h"

/*
* Static values
*/

/*
* Types
*/

/*
* Variable declarations (defined in bits.c)
*/

/*
* Function prototypes (defined in bits.c)
*/
// Find the number of leading zeroes in a 32-bit integer
// Use CLZ32() rather than this directly.
int clz32(uint32_t x);
// Find the mask of most-significant set bit in a 32-bit integer
// Use MSB32() rather than this directly.
uint32_t msb32(uint32_t n);
// Find the index of the most-significant set bit in a 32-bit integer
// Use MSB32_IDX() rather than this directly.
int msb32_idx(uint32_t n);


/*
* Macros
*/
// Set one or more bits in a field
#define SET_BIT(field, bits) ((field) |= (bits))
// Unset one or more bits in a field
#define CLEAR_BIT(field, bits) ((field) &= ~(bits))
// Toggle one or more bits in a field
#define TOGGLE_BIT(field, bits) ((field) ^= (bits))
// Read one or more bits in a field
#degine GET_BIT(field, bits) ((field) & (bits))
// Set or clear bits in a field limited by a mask
//#define MODIFY_BITS(field, mask, bits) ((field) = (((field) & ~(mask)) | (bits)))
#define MODIFY_BITS(field, mask, bits) ((field) = (((field) & ~(mask)) | ((bits) & (mask))))

// Select every set bit in field that's also set in mask
#define SELECT_BITS(field, mask) ((field) & (mask))
// Select every set bit in field except those set in mask
#define MASK_BITS(field, mask)   ((field) & ~(mask))
// Select every bit in a mask at some offset and then shift those bits to the
// LSB end
// offset is expanded twice for convenience
#define GATHER_BITS(field, mask, offset) (SELECT_BITS((field), ((mask) << (offset))) >> (offset))

// Check if any bit set in mask is set in field
//#define BIT_IS_SET(field, mask)    ((field) & (mask))
#define BIT_IS_SET(field, mask)    (((field) & (mask)) != 0)
// Check if every bit set in mask is set in field
// mask is expanded twice for convenience
#define BITS_ARE_SET(field, mask) (((field) & (mask)) == (mask))

// Turn an integer into a bit mask
#define AS_BIT(n) (1U << (n))

// Find the lowest set bit in a field
#define LOWEST_BIT(field) ((field) & -(field))
// Shift right so lowest set bit ends up at bit 0
#define SHIFT_LOWEST_BIT(field) ((field) / ((field) & -(field)))

// Find the number of leading zeroes in a 32-bit integer
#define CLZ32(x) (((x) == 0) ? 32 : clz32((uint32_t )x))
// Same, for unsigned ints, longs, and long longs
#define CLZ(x)   __builtin_clz(x)
#define CLZL(x)  __builtin_clzl(x)
#define CLZLL(x) __builtin_clzll(x)

// Find the mask of most-significant set bit in a 32-bit integer
#define MSB32(x) (((x) == 0) ? 0 : msb32((uint32_t )x))
// Find the index of the most-significant set bit in a 32-bit integer
#define MSB32_IDX(x) (((x) == 0) ? 0 : msb32_idx((uint32_t )x))

// Find the mask of the least-significant set bit in a 32-bit integer
#define LSB32(x) ((uint32_t )(x) & -(int32_t )(x))
// Same, for unsigned ints, longs, and long longs
#define LSB(x)   (__builtin_ffs(x) - 1)
#define LSBL(x)  (__builtin_ffsl(x) - 1)
#define LSBLL(x) (__builtin_ffsll(x) - 1)

#endif // _LIB_BITS_H
#ifdef __cplusplus
 }
#endif
