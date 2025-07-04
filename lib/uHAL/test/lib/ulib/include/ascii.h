// SPDX-License-Identifier: GPL-3.0-only
/***********************************************************************
*                                                                      *
*                                                                      *
* Copyright 2024 svijsv                                                *
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
// ascii.h
// Tools for dealing with 7-bit ASCII characters
//
// NOTES:
//    Most of this is re-implementations of functions available in ctype.h
//    and should conform to the expected behavor
//
//
#ifndef _ULIB_ASCII_H
#define _ULIB_ASCII_H

#include "src/configify.h"
#if ULIB_ENABLE_ASCII

#include "types.h"

#if ! ASCII_SUBSTITUTE_WITH_CTYPE
//
// Confirm an unsigned int is a valid 7-bit ascii code
bool ascii_is_valid(uint_t c);
//
// Check if 'c' is a control code
bool ascii_is_cntrl(uint8_t c);
//
// Check if 'c' is a printable character, including space
bool ascii_is_print(uint8_t c);
//
// Check if 'c' is a printable character, excluding space
bool ascii_is_graph(uint8_t c);
//
// Check if 'c' is a space, formfeed, newline, carriage return, tab, or
// vertical tab
bool ascii_is_space(uint8_t c);
//
// Check if 'c' is a space or tab
bool ascii_is_blank(uint8_t c);
//
// Check if 'c' is a digit ('0'-'9')
bool ascii_is_digit(uint8_t c);
//
// Check if 'c' is hexadecimal digit ('0'-'9', 'a'-'f', or 'A'-'F')
bool ascii_is_xdigit(uint8_t c);
//
// Check if 'c' is alphabetical ('a'-'z' or 'A'-'Z')
bool ascii_is_alpha(uint8_t c);
//
// Check if 'c' is alphanumeric ('0'-'9', 'a'-'z', or 'A'-'Z')
bool ascii_is_alnum(uint8_t c);
//
// Check if 'c' is a printable character other than a digit, letter, or space
bool ascii_is_punct(uint8_t c);
//
// Check if 'c' is in the range of 'a'-'z'
bool ascii_is_lower(uint8_t c);
//
// Check if 'c' is in the range of 'A'-'Z'
bool ascii_is_upper(uint8_t c);

//
// Convert 'c' from  the range 'a'-'z' to 'A' - 'Z'
uint8_t ascii_to_upper(uint8_t c);
//
// Convert 'c' from  the range 'A'-'Z' to 'a' - 'z'
uint8_t ascii_to_lower(uint8_t c);

#if ASCII_SUBSTITUTE_FOR_CTYPE
#define isascii(_c_) ascii_is_valid(_c_)
#define iscntrl(_c_) ascii_is_cntrl(_c_)
#define isprint(_c_) ascii_is_print(_c_)
#define isgraph(_c_) ascii_is_graph(_c_)
#define isspace(_c_) ascii_is_space(_c_)
#define isblank(_c_) ascii_is_blank(_c_)
#define isdigit(_c_) ascii_is_digit(_c_)
#define isxdigit(_c_) ascii_is_xdigit(_c_)
#define isalpha(_c_) ascii_is_alpha(_c_)
#define isalnum(_c_) ascii_is_alnum(_c_)
#define ispunct(_c_) ascii_is_punct(_c_)
#define islower(_c_) ascii_is_lower(_c_)
#define isupper(_c_) ascii_is_upper(_c_)

#define toupper(_c_) ascii_to_upper(_c_)
#define tolower(_c_) ascii_to_lower(_c_)
#endif // ASCII_SUBSTITUTE_FOR_CTYPE

#else // ! ASCII_SUBSTITUTE_WITH_CTYPE
#include <ctype.h>

#define ascii_is_valid(_c_) isascii(_c_)
#define ascii_is_cntrl(_c_) iscntrl(_c_)
#define ascii_is_print(_c_) isprint(_c_)
#define ascii_is_graph(_c_) isgraph(_c_)
#define ascii_is_space(_c_) isspace(_c_)
#define ascii_is_blank(_c_) isblank(_c_)
#define ascii_is_digit(_c_) isdigit(_c_)
#define ascii_is_xdigit(_c_) isxdigit(_c_)
#define ascii_is_alpha(_c_) isalpha(_c_)
#define ascii_is_alnum(_c_) isalnum(_c_)
#define ascii_is_punct(_c_) ispunct(_c_)
#define ascii_is_lower(_c_) islower(_c_)
#define ascii_is_upper(_c_) isupper(_c_)

#define ascii_to_upper(_c_) toupper(_c_)
#define ascii_to_lower(_c_) tolower(_c_)
#endif // ! ASCII_SUBSTITUTE_WITH_CTYPE

//
// These don't have ctype equivalents
//
// Convert 'c' from the range '0'-'9' to 0-9
uint8_t ascii_to_digit(uint8_t c);
//
// Convert 'c' from the range 0-9 to '0'-'9'
uint8_t ascii_from_digit(uint8_t c);
//
// Convert 'c' from the range '0'-'F' (or '0'-'f') to 0x0-0xF
uint8_t ascii_to_xdigit(uint8_t c);
//
// Convert 'c' from the range 0x0-0xF to '0'-'F'
uint8_t ascii_from_xdigit(uint8_t c);

#define ASCII_IS_VALID(_c_) ((_c_) < 0x80U)
#define ASCII_IS_CNTRL(_c_) ((_c_) < 0x20U || (_c_) == 0x7FU)
#define ASCII_IS_PRINT(_c_) ((_c_) >= 0x20U && (_c_) != 0x7FU)
#define ASCII_IS_GRAPH(_c_) ((_c_) > 0x20U && (_c_) != 0x7FU)
#define ASCII_IS_BLANK(_c_) ((_c_) == ' ' || (_c_) == '\t')
#define ASCII_IS_DIGIT(_c_) ((_c_) >= '0' && (_c_) <= '9')
#define ASCII_IS_LOWER(_c_) ((_c_) >= 'a' && (_c_) <= 'z')
#define ASCII_IS_UPPER(_c_) ((_c_) >= 'A' && (_c_) <= 'Z')

#define ASCII_TO_UPPER(_c_) ((_c_) ^ 0x20U)
#define ASCII_TO_LOWER(_c_) ((_c_) | 0x20U)
#define ASCII_TO_DIGIT(_c_)   ((_c_) ^ 0x30U)
#define ASCII_FROM_DIGIT(_c_) ((_c_) | 0x30U)
#define ASCII_TO_XDIGIT(_c_)   (((_c_) > 9) ? (0x0AU + (((_c_) | 0x20U) - 'a')) : ((_c_) ^ 0x30U))
#define ASCII_FROM_XDIGIT(_c_) (((_c_) > 9) ? (('A' - 0x0AU) + (_c_)) : ((_c_) | 0x30U))

#endif // ULIB_ENABLE_ASCII
#endif // _ULIB_ASCII_H
