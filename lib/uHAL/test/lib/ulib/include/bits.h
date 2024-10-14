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
// bits.h
// Operations on bitfields
// NOTES:
//   Macros should only ever expand any given argument once if it can be
//   helped so that any arguments with side-effects (like registers that
//   reset themselves) don't cause problems. Of course some already don't.
//
//   The inline functions (defined in bits.c) are intended primarily for
//   situations where type checking the input is desired but also helps to
//   protect against double-expansion of arguments.
//
//   SET_BIT(), CLEAR_BIT(), TOGGLE_BIT(), and MODIFY_BITS() set the variable
//   given to them as the field, nothing else does.
//
//   http:// realtimecollisiondetection.net/blog/?p=78
//   http:// graphics.stanford.edu/~seander/bithacks.html
//   https:// gcc.gnu.org/onlinedocs/gcc/Other-Builtins.html
//
//
#ifndef _ULIB_BITS_H
#define _ULIB_BITS_H

#include "src/configify.h"
#if ULIB_ENABLE_BITOPS

#include "types.h"
#include "util.h"

#if ! defined(BITOP_FUNC_INLINE)
# if ULIB_BITOP_ENABLE_INLINED_FUNCTIONS
#  define BITOP_FUNC_INLINE INLINE
# else
#  define BITOP_FUNC_INLINE
# endif
#endif

// Set one or more bits in a field
#define _SET_BIT(field, bits) ((field) |= (bits))
BITOP_FUNC_INLINE uint set_bit_ui(uint field, uint bits);
BITOP_FUNC_INLINE uint8_t set_bit_u8(uint8_t field, uint8_t bits);
BITOP_FUNC_INLINE uint16_t set_bit_u16(uint16_t field, uint16_t bits);
BITOP_FUNC_INLINE uint32_t set_bit_u32(uint32_t field, uint32_t bits);
#if ULIB_BITOP_ENABLE_INLINED_64BIT_FUNCTIONS
BITOP_FUNC_INLINE uint64_t set_bit_u64(uint64_t field, uint64_t bits);
#endif

// Unset one or more bits in a field
#define _CLEAR_BIT(field, bits) ((field) &= ~(bits))
BITOP_FUNC_INLINE uint clear_bit_ui(uint field, uint bits);
BITOP_FUNC_INLINE uint8_t clear_bit_u8(uint8_t field, uint8_t bits);
BITOP_FUNC_INLINE uint16_t clear_bit_u16(uint16_t field, uint16_t bits);
BITOP_FUNC_INLINE uint32_t clear_bit_u32(uint32_t field, uint32_t bits);
#if ULIB_BITOP_ENABLE_INLINED_64BIT_FUNCTIONS
BITOP_FUNC_INLINE uint64_t clear_bit_u64(uint64_t field, uint64_t bits);
#endif

// Toggle one or more bits in a field
#define _TOGGLE_BIT(field, bits) ((field) ^= (bits))
BITOP_FUNC_INLINE uint toggle_bit_ui(uint field, uint bits);
BITOP_FUNC_INLINE uint8_t toggle_bit_u8(uint8_t field, uint8_t bits);
BITOP_FUNC_INLINE uint16_t toggle_bit_u16(uint16_t field, uint16_t bits);
BITOP_FUNC_INLINE uint32_t toggle_bit_u32(uint32_t field, uint32_t bits);
#if ULIB_BITOP_ENABLE_INLINED_64BIT_FUNCTIONS
BITOP_FUNC_INLINE uint64_t toggle_bit_u64(uint64_t field, uint64_t bits);
#endif

// Set or clear bits in a field limited by a mask
#define _MODIFY_BITS(field, mask, bits) ((field) = (((field) & ~(mask)) | ((bits) & (mask))))
BITOP_FUNC_INLINE uint modify_bits_ui(uint field, uint mask, uint bits);
BITOP_FUNC_INLINE uint8_t modify_bits_u8(uint8_t field, uint8_t mask, uint8_t bits);
BITOP_FUNC_INLINE uint16_t modify_bits_u16(uint16_t field, uint16_t mask, uint16_t bits);
BITOP_FUNC_INLINE uint32_t modify_bits_u32(uint32_t field, uint32_t mask, uint32_t bits);
#if ULIB_BITOP_ENABLE_INLINED_64BIT_FUNCTIONS
BITOP_FUNC_INLINE uint64_t modify_bits_u64(uint64_t field, uint64_t mask, uint64_t bits);
#endif

// Select every set bit in field that's also set in mask
#define SELECT_BITS(field, mask) ((field) & (mask))
BITOP_FUNC_INLINE uint select_bits_ui(uint field, uint mask);
BITOP_FUNC_INLINE uint8_t select_bits_u8(uint8_t field, uint8_t mask);
BITOP_FUNC_INLINE uint16_t select_bits_u16(uint16_t field, uint16_t mask);
BITOP_FUNC_INLINE uint32_t select_bits_u32(uint32_t field, uint32_t mask);
#if ULIB_BITOP_ENABLE_INLINED_64BIT_FUNCTIONS
BITOP_FUNC_INLINE uint64_t select_bits_u64(uint64_t field, uint64_t mask);
#endif

// Select every set bit in field except those set in mask
#define MASK_BITS(field, mask)   ((field) & ~(mask))
BITOP_FUNC_INLINE uint mask_bits_ui(uint field, uint mask);
BITOP_FUNC_INLINE uint8_t mask_bits_u8(uint8_t field, uint8_t mask);
BITOP_FUNC_INLINE uint16_t mask_bits_u16(uint16_t field, uint16_t mask);
BITOP_FUNC_INLINE uint32_t mask_bits_u32(uint32_t field, uint32_t mask);
#if ULIB_BITOP_ENABLE_INLINED_64BIT_FUNCTIONS
BITOP_FUNC_INLINE uint64_t mask_bits_u64(uint64_t field, uint64_t mask);
#endif

// Select every bit in a mask at some offset and then shift those bits to the
// LSB end
//#define GATHER_BITS(field, mask, offset) (SELECT_BITS((field), ((mask) << (offset))) >> (offset))
#define GATHER_BITS(field, mask, offset) (((field) >> (offset)) & (mask))
BITOP_FUNC_INLINE uint gather_bits_ui(uint field, uint mask, uint_fast8_t offset);
BITOP_FUNC_INLINE uint8_t gather_bits_u8(uint8_t field, uint8_t mask, uint_fast8_t offset);
BITOP_FUNC_INLINE uint16_t gather_bits_u16(uint16_t field, uint16_t mask, uint_fast8_t offset);
BITOP_FUNC_INLINE uint32_t gather_bits_u32(uint32_t field, uint32_t mask, uint_fast8_t offset);
#if ULIB_BITOP_ENABLE_INLINED_64BIT_FUNCTIONS
BITOP_FUNC_INLINE uint64_t gather_bits_u64(uint64_t field, uint64_t mask, uint_fast8_t offset);
#endif

// Check if any bit set in mask is set in field
#define BIT_IS_SET(field, mask)    (((field) & (mask)) != 0)
BITOP_FUNC_INLINE bool bit_is_set_ui(uint field, uint mask);
BITOP_FUNC_INLINE bool bit_is_set_u8(uint8_t field, uint8_t mask);
BITOP_FUNC_INLINE bool bit_is_set_u16(uint16_t field, uint16_t mask);
BITOP_FUNC_INLINE bool bit_is_set_u32(uint32_t field, uint32_t mask);
#if ULIB_BITOP_ENABLE_INLINED_64BIT_FUNCTIONS
BITOP_FUNC_INLINE bool bit_is_set_u64(uint64_t field, uint64_t mask);
#endif

// Check if every bit set in mask is set in field
// mask is expanded twice for convenience
#define BITS_ARE_SET(field, mask) (((field) & (mask)) == (mask))
BITOP_FUNC_INLINE bool bits_are_set_ui(uint field, uint mask);
BITOP_FUNC_INLINE bool bits_are_set_u8(uint8_t field, uint8_t mask);
BITOP_FUNC_INLINE bool bits_are_set_u16(uint16_t field, uint16_t mask);
BITOP_FUNC_INLINE bool bits_are_set_u32(uint32_t field, uint32_t mask);
#if ULIB_BITOP_ENABLE_INLINED_64BIT_FUNCTIONS
BITOP_FUNC_INLINE bool bits_are_set_u64(uint64_t field, uint64_t mask);
#endif

// Turn an integer into a bit mask
#define AS_BIT(n) (1U << (n))
BITOP_FUNC_INLINE uint as_bit_ui(uint_fast8_t n);
BITOP_FUNC_INLINE uint8_t as_bit_u8(uint_fast8_t n);
BITOP_FUNC_INLINE uint16_t as_bit_u16(uint_fast8_t n);
BITOP_FUNC_INLINE uint32_t as_bit_u32(uint_fast8_t n);
#if ULIB_BITOP_ENABLE_INLINED_64BIT_FUNCTIONS
BITOP_FUNC_INLINE uint64_t as_bit_u64(uint_fast8_t n);
#endif
/*
// The headers for the ATMega provide this. It probably shouldn't be used
// anywhere else since there's an equivalent above, but here it is anyway.
#ifndef _BV
# define _BV(n) (1U << (n))
#endif
*/

// Find the lowest set bit in a field
#define LOWEST_BIT(field) ((field) & -(field))
BITOP_FUNC_INLINE uint lowest_bit_ui(uint field);
BITOP_FUNC_INLINE uint8_t lowest_bit_u8(uint8_t field);
BITOP_FUNC_INLINE uint16_t lowest_bit_u16(uint16_t field);
BITOP_FUNC_INLINE uint32_t lowest_bit_u32(uint32_t field);
#if ULIB_BITOP_ENABLE_INLINED_64BIT_FUNCTIONS
BITOP_FUNC_INLINE uint64_t lowest_bit_u64(uint64_t field);
#endif
// Same, for unsigned ints, longs, and long longs
#define LSB(x)   (__builtin_ffs(x) - 1)
#define LSBL(x)  (__builtin_ffsl(x) - 1)
#define LSBLL(x) (__builtin_ffsll(x) - 1)

// Shift right so lowest set bit ends up at bit 0
#define SHIFT_LOWEST_BIT(field) ((field) / ((field) & -(field)))
BITOP_FUNC_INLINE uint shift_lowest_bit_ui(uint field);
BITOP_FUNC_INLINE uint8_t shift_lowest_bit_u8(uint8_t field);
BITOP_FUNC_INLINE uint16_t shift_lowest_bit_u16(uint16_t field);
BITOP_FUNC_INLINE uint32_t shift_lowest_bit_u32(uint32_t field);
#if ULIB_BITOP_ENABLE_INLINED_64BIT_FUNCTIONS
BITOP_FUNC_INLINE uint64_t shift_lowest_bit_u64(uint64_t field);
#endif

// Find the number of leading zeroes in unsigned ints, longs, and long longs
#define CLZ(x)   __builtin_clz(x)
#define CLZL(x)  __builtin_clzl(x)
#define CLZLL(x) __builtin_clzll(x)

//
// Generic function callers
//
#if ULIB_BITOP_ENABLE_GENERICS
//
// I would much prefer being able to just use the standard types directly but
// the way the compiler interprets these means that all of char, short, int, long,
// and long long need to be handled and the typedefs aren't defined the same
// for all platforms, so this really is the simplest method I"ve been able to
// find.
#include <limits.h>

#if UCHAR_MAX == 0xFF
# define _ULIB_BITS_DISPATCH_UC(_fni_) _fni_ ## _u8
#else
# error "Unable to determine size of unsigned char"
#endif

#if USHRT_MAX == 0xFFFF
# define _ULIB_BITS_DISPATCH_US(_fni_) _fni_ ## _u16
#else
# error "Unable to determine size of unsigned short"
#endif

#if UINT_MAX == 0xFFFF
# define _ULIB_BITS_DISPATCH_UI(_fni_) _fni_ ## _u16
#elif UINT_MAX == 0xFFFFFFFF
# define _ULIB_BITS_DISPATCH_UI(_fni_) _fni_ ## _u32
#else
# error "Unable to determine size of unsigned int"
#endif

#if ULONG_MAX == 0xFFFFFFFF
# define _ULIB_BITS_DISPATCH_UL(_fni_) _fni_ ## _u32
#elif ULONG_MAX == 0xFFFFFFFFFFFFFFFF
# define _ULIB_BITS_DISPATCH_UL(_fni_) _fni_ ## _u64
#else
# error "Unable to determine size of unsigned long"
#endif

#if ULLONG_MAX == 0xFFFFFFFFFFFFFFFF
# define _ULIB_BITS_DISPATCH_ULL(_fni_) _fni_ ## _u64
#else
# error "Unable to determine size of unsigned long long"
#endif

# define set_bit(_f_, _b_) _Generic((_f_), \
		unsigned char : _ULIB_BITS_DISPATCH_UC(set_bit),  \
		unsigned short: _ULIB_BITS_DISPATCH_US(set_bit), \
		unsigned long: _ULIB_BITS_DISPATCH_UL(set_bit), \
		unsigned long long: _ULIB_BITS_DISPATCH_ULL(set_bit), \
		unsigned int : _ULIB_BITS_DISPATCH_UI(set_bit)  \
	)((_f_), (_b_))
# define SET_BIT(_f_, _b_) ((_f_) = (set_bit((_f_), (_b_))))

# define clear_bit(_f_, _b_) _Generic((_f_), \
		unsigned char : _ULIB_BITS_DISPATCH_UC(clear_bit),  \
		unsigned short: _ULIB_BITS_DISPATCH_US(clear_bit), \
		unsigned long: _ULIB_BITS_DISPATCH_UL(clear_bit), \
		unsigned long long: _ULIB_BITS_DISPATCH_ULL(clear_bit), \
		unsigned int : _ULIB_BITS_DISPATCH_UI(clear_bit)  \
	)((_f_), (_b_))
# define CLEAR_BIT(_f_, _b_) ((_f_) = (clear_bit((_f_), (_b_))))

# define toggle_bit(_f_, _b_) _Generic((_f_), \
		unsigned char : _ULIB_BITS_DISPATCH_UC(toggle_bit),  \
		unsigned short: _ULIB_BITS_DISPATCH_US(toggle_bit), \
		unsigned long: _ULIB_BITS_DISPATCH_UL(toggle_bit), \
		unsigned long long: _ULIB_BITS_DISPATCH_ULL(toggle_bit), \
		unsigned int : _ULIB_BITS_DISPATCH_UI(toggle_bit)  \
	)((_f_), (_b_))
# define TOGGLE_BIT(_f_, _b_) ((_f_) = (toggle_bit((_f_), (_b_))))

# define modify_bits(_f_, _m_, _b_) _Generic((_f_), \
		unsigned char : _ULIB_BITS_DISPATCH_UC(modify_bits),  \
		unsigned short: _ULIB_BITS_DISPATCH_US(modify_bits), \
		unsigned long: _ULIB_BITS_DISPATCH_UL(modify_bits), \
		unsigned long long: _ULIB_BITS_DISPATCH_ULL(modify_bits), \
		unsigned int : _ULIB_BITS_DISPATCH_UI(modify_bits)  \
	)((_f_), (_m_), (_b_))
# define MODIFY_BITS(_f_, _m_, _b_) ((_f_) = (modify_bits((_f_), (_m_), (_b_))))

# define select_bits(_f_, _m_) _Generic((_f_), \
		unsigned char : _ULIB_BITS_DISPATCH_UC(select_bits),  \
		unsigned short: _ULIB_BITS_DISPATCH_US(select_bits), \
		unsigned long: _ULIB_BITS_DISPATCH_UL(select_bits), \
		unsigned long long: _ULIB_BITS_DISPATCH_ULL(select_bits), \
		unsigned int : _ULIB_BITS_DISPATCH_UI(select_bits)  \
	)((_f_), (_m_))

# define mask_bits(_f_, _m_) _Generic((_f_), \
		unsigned char : _ULIB_BITS_DISPATCH_UC(mask_bits),  \
		unsigned short: _ULIB_BITS_DISPATCH_US(mask_bits), \
		unsigned long: _ULIB_BITS_DISPATCH_UL(mask_bits), \
		unsigned long long: _ULIB_BITS_DISPATCH_ULL(mask_bits), \
		unsigned int : _ULIB_BITS_DISPATCH_UI(mask_bits)  \
	)((_f_), (_m_))

# define gather_bits(_f_, _m_, _o_) _Generic((_f_), \
		unsigned char : _ULIB_BITS_DISPATCH_UC(gather_bits),  \
		unsigned short: _ULIB_BITS_DISPATCH_US(gather_bits), \
		unsigned long: _ULIB_BITS_DISPATCH_UL(gather_bits), \
		unsigned long long: _ULIB_BITS_DISPATCH_ULL(gather_bits), \
		unsigned int : _ULIB_BITS_DISPATCH_UI(gather_bits)  \
	)((_f_), (_m_), (_o_))

# define bit_is_set(_f_, _m_) _Generic((_f_), \
		unsigned char : _ULIB_BITS_DISPATCH_UC(bit_is_set),  \
		unsigned short: _ULIB_BITS_DISPATCH_US(bit_is_set), \
		unsigned long: _ULIB_BITS_DISPATCH_UL(bit_is_set), \
		unsigned long long: _ULIB_BITS_DISPATCH_ULL(bit_is_set), \
		unsigned int : _ULIB_BITS_DISPATCH_UI(bit_is_set)  \
	)((_f_), (_m_))

# define bits_are_set(_f_, _m_) _Generic((_f_), \
		unsigned char : _ULIB_BITS_DISPATCH_UC(bits_are_set),  \
		unsigned short: _ULIB_BITS_DISPATCH_US(bits_are_set), \
		unsigned long: _ULIB_BITS_DISPATCH_UL(bits_are_set), \
		unsigned long long: _ULIB_BITS_DISPATCH_ULL(bits_are_set), \
		unsigned int : _ULIB_BITS_DISPATCH_UI(bits_are_set)  \
	)((_f_), (_m_))

# define as_bit(_n_) _Generic((_n_), \
		unsigned char : _ULIB_BITS_DISPATCH_UC(as_bit),  \
		unsigned short: _ULIB_BITS_DISPATCH_US(as_bit), \
		unsigned long: _ULIB_BITS_DISPATCH_UL(as_bit), \
		unsigned long long: _ULIB_BITS_DISPATCH_ULL(as_bit), \
		unsigned int : _ULIB_BITS_DISPATCH_UI(as_bit)  \
	)((_n_))

# define lowest_bit(_f_) _Generic((_f_), \
		unsigned char : LSB,  \
		unsigned short: LSB, \
		unsigned long: LSBL, \
		unsigned long long: LSBLL, \
		unsigned int : LSB  \
	)((_f_))

# define shift_lowest_bit(_f_) _Generic((_f_), \
		unsigned char : _ULIB_BITS_DISPATCH_UC(shift_lowest_bit),  \
		unsigned short: _ULIB_BITS_DISPATCH_US(shift_lowest_bit), \
		unsigned long: _ULIB_BITS_DISPATCH_UL(shift_lowest_bit), \
		unsigned long long: _ULIB_BITS_DISPATCH_ULL(shift_lowest_bit), \
		unsigned int : _ULIB_BITS_DISPATCH_UI(shift_lowest_bit)  \
	)((_f_))

# define clz(_f_) _Generic((_f_), \
		unsigned char : CLZ,  \
		unsigned short: CLZ, \
		unsigned long: CLZL, \
		unsigned long long: CLZLL, \
		unsigned int : CLZ  \
	)((_f_))

#else // ULIB_BITOP_ENABLE_GENERICS
# define set_bit(_f_, _b_) set_bit_ui((_f_), (_b_))
# define SET_BIT(_f_, _b_) (_SET_BIT((_f_), (_b_)))

# define clear_bit(_f_, _b_) clear_bit_ui((_f_), (_b_))
# define CLEAR_BIT(_f_, _b_) (_CLEAR_BIT((_f_), (_b_)))

# define toggle_bit(_f_, _b_) toggle_bit_ui((_f_), (_b_))
# define TOGGLE_BIT(_f_, _b_) (_TOGGLE_BIT((_f_), (_b_)))

# define modify_bits(_f_, _m_, _b_) modify_bits_ui((_f_), (_m_), (_b_))
# define MODIFY_BITS(_f_, _m_, _b_) (_MODIFY_BITS((_f_), (_m_), (_b_)))

# define select_bits(_f_, _m_) select_bit_ui((_f_), (_m_))
# define mask_bits(_f_, _m_) mask_bits_ui((_f_), (_m_))
# define gather_bits(_f_, _m_, _o_) gather_bits_ui((_f_), (_m_), (_o_))
# define bit_is_set(_f_, _m_) bit_is_set_ui((_f_), (_m_))
# define bits_are_set(_f_, _m_) bits_are_set_ui((_f_), (_m_))
# define as_bit(_n_) as_bit_ui((_n_))
# define lowest_bit(_f_) lowest_bit_ui((_f_))
# define shift_lowest_bit(_f_) shift_lowest_bit_ui((_f_))
# define clz(_f_) CLZ((_f_))
#endif // ULIB_BITOP_ENABLE_GENERICS


#if ULIB_BITOP_ENABLE_INLINED_FUNCTIONS
# include "bits_inlined.h"
#endif


#endif // ULIB_ENABLE_BITOPS
#endif // _ULIB__H
