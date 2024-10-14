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
// cstrings.h
// Tools for dealing with C-style strings
// NOTES:
//
//
#ifndef _ULIB_CSTRINGS_H
#define _ULIB_CSTRINGS_H

#include "src/configify.h"
#if ULIB_ENABLE_CSTRINGS

#include "types.h"


//
// Check if two cstrings are the same
bool cstring_eq(const char *s1, const char *s2);
//
// Check if two cstrings are the same up to a given length.
bool cstring_eqn(const char *s1, const char *s2, uint_t n);
//
// Check if two cstrings are the same up to the length of the first string.
bool cstring_eqz(const char *s1, const char *s2);
//
// Return a pointer to the first non-whitespace character in s.
const char* cstring_eat_whitespace(const char *s);
//
// Return a pointer to the first non-sep character after the first sep
// character in s - that is to say, eat one token if present then find the
// next.
// Repeating separators are treated as a single one.
// Parsing is done when the returned string is empty.
const char* cstring_next_token(const char *cs, char sep);
//
// Return a pointer to the character after the first sep character. The first
// sep character is changed to a NUL so that input becomes a NUL-terminated
// string representing the first (possibly empty) token.
// Repeating separators are *NOT* treated as a single one.
// Parsing is done when the returned string is empty.
char *cstring_pop_token(char *input, char sep, uint_t *len);
//
// Like basename(), but accepting a const string.
// This normally returns a pointer inside s, but may return a statically-
// allocated string for ".".
// This differs from basename() in that paths ending in '/' will be returned
// as ".".
const char* cstring_basename(const char *s);
//
// Convert all the lowercase letters in s to uppercase.
char *cstring_to_upper(char *s);
//
// Convert all the uppercase letters in s to lowercase.
char *cstring_to_lower(char *s);
//
// Convert all occurances of one character to another.
char *cstring_tr(char *s, char old, char new);

//
// Convert an unsigned integer to a cstring of at most 'size' bytes including
// the trailing NUL byte
// If the string wouldn't fit the contents of 'dest' are undefined
// The base must be >= 2 and <= 16
// The return value is the number of bytes that would have been needed to
// store the whole string, including the trailing NUL byte
// Returns 0 on any error other than the buffer being too small
uint_t cstring_from_uint(char *dest, uint_t size, uint_t src, uint_t base);

//
// In-line conversion of a segment of a c-string (_s) to an integer (_i).
// This will increment _s and expects to deal only with characters 0-9, signs
// will have to be handled by the caller beforehand.
#define UINT_FROM_CSTRING_BASE10(_i, _s) \
	do { \
		(_i) = 0; \
		while (*(_s) >= '0' && *(_s) <= '9') { \
			(_i) = (_i) * 10; \
			(_i) += *(_s) - '0'; \
			++(_s); \
		} \
	} while (0);
#define UINT_FROM_CSTRING(_ii, _ss) UINT_FROM_CSTRING_BASE10(_ii, _ss)

//
// In-line replacement of one character with another in a string.
#define CSTRING_TR(_s, _old, _new) \
	do { \
		for (char *_c = (_s); *_c != 0; ++_c) { \
			if (*_c == _old) { \
				*_c = _new; \
			} \
		} \
	} while (0);

#endif // ULIB_ENABLE_CSTRINGS
#endif // _ULIB_CSTRINGS_H
