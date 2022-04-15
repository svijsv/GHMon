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
// cstrings.h
// A library for working with C-style string
// NOTES:
//

#ifdef __cplusplus
 extern "C" {
#endif
#ifndef _ULIB_CSTRINGS_H
#define _ULIB_CSTRINGS_H

/*
* Includes
*/
#include "common.h"

#include "types.h"

/*
* Types
*/
typedef uint16_t cstrlen_t;

/*
* Static values
*/

/*
* Variable declarations
*/

/*
* Function prototypes
*/
// Determine the length of a string excluding the terminal NUL
cstrlen_t cstring_len(const char *c);

// Compare the first non-matching character of s1 and s2, interpretted as
// uint8_t
// Returns:
//   '-1' if s1 < s2
//   '0'  if s1 and s2 are the same
//   '1'  if s1 > s2
int cstring_cmp(const char *s1, const char *s2);
int cstring_ncmp(const char *s1, const char *s2, cstrlen_t n);

// Interpret a string as an integer
// s is assumed to be a valid number in base 10 and not larger than can fit
// in an int.
// The only non-numerals allowed are a (possible) leading '-' or '+';
// conversion stops when anything else is encountered
int cstring_to_int(const char *s);
// Interpret an integer as a string
// width is the minimal width of the string
// pad is the character used to left-pad up to width
// Returns a pointer to the trailing NUL byte of the string
char* cstring_from_int(char *s, int n, int8_t width, char pad);

// Return a pointer to the file name part of a file's path
const char* cstring_basename(const char *s, char sep);

// Return a pointer to the first non-whitespace character in s
const char* cstring_eat_whitespace(const char *s);
// Return a pointer to the first non-delim character after the first delim
// character in s - that is to say, eat one token if present then find the
// next
// Repeating delimiters are treated as a single one.
const char* cstring_next_token(const char *cs, char delim);

// Copy up to 'n' bytes from 'src' to 'dest'.
// If USE_CLIB_STRING isn't set, this differs from strncpy() in that if the
// length of 'src' is less than 'n' the extra space is not overwritten with
// 0.
char* cstring_strncpy(char *dest, const char *src, cstrlen_t n);

/*
* Macros
*/

#endif // _LIB_CSTRINGS_H
#ifdef __cplusplus
 }
#endif
