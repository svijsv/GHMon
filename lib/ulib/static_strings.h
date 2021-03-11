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
// static_strings.h
// A library for working with statically-allocated strings
// NOTES:
//   Integer overflows are a real possibility here when strlen_t is 8 bits,
//   so they need to be checked for ALWAYS.
//
//   Iterations should be performed in such a way that reaching STRING_MAX
//   won't cause problems when it's the max size of strlen_t.
//

#ifdef __cplusplus
 extern "C" {
#endif
#ifndef _ULIB_STATIC_STRINGS_H
#define _ULIB_STATIC_STRINGS_H

/*
* Includes
*/
#include "common.h"

#include "types.h"
#include "cstrings.h"

#include <stdarg.h>


/*
* Static values
*/
// Max length of the string, excluding the terminal null. Keep at 255 or less
// if the total length including null needs to fit in a uint8_t.
#define STRING_MAX 80
//#define STRING_MAX 255

// A static empty string to make initializing variables easier.
#define STRING_EMPTY { 0, "" }


/*
* Types
*/
typedef uint8_t strlen_t;
// typedef cstrlen_t strlen_t;

typedef struct {
	strlen_t length;
	char     cstring[STRING_MAX+1];
} string_t;


/*
* Variable declarations
*/


/*
* Function prototypes
*/
// Clear an existing string; currently the same as string_init()
string_t* string_clear(string_t *s);
// Initialize a new string; currently the same as string_clear()
string_t* string_init(string_t *s);

// Setting functions will copy in as much as will fit, and truncate the
// string without notice if they have to.
string_t* string_set_from_char(string_t *s, const char c);
string_t* string_set_from_cstring(string_t *s, const char *c, strlen_t len);
string_t* string_set_from_string(string_t *s, const string_t *src);
// *_printf() functions aren't thread safe
string_t* string_printf(string_t *s, const char *format, ...)
	__attribute__ ((format(printf, 2, 3)));

// Appending functions will copy in as much as will fit, and truncate the
// string without notice if they have to.
string_t* string_append_from_char(string_t *s, const char c);
string_t* string_append_from_cstring(string_t *s, const char *c, strlen_t len);
string_t* string_append_from_string(string_t *s, const string_t *src);
// *_printf() functions aren't thread-safe
string_t* string_appendfv(string_t *s, const char *format, va_list arp);
string_t* string_appendf(string_t *s, const char *format, ...)
	__attribute__ ((format(printf, 2, 3)));
// n is the number to append
// width is the minimum width to allow excluding '-', 0 to disable
// pad is the char to pad to width with
string_t* string_append_from_int(string_t *s, int n, uint8_t width, char pad);
// n is the number to append
// d is the number to divide n by; the remainder is printed after a decimal point
// No fancy padding with this one.
string_t* string_append_from_int_div(string_t *s, int n, int d);
// Append c to string until it's size characters long
string_t* string_pad_from_char(string_t *s, const char c, strlen_t size);

// Test whether a string is empty
bool string_is_empty(const string_t *s);
// Test whether a string is the same a cstring
bool string_equals_cstring(const string_t *l, const char *r);
// Test whether a string is the same another string
bool string_equals_string(const string_t *l, const string_t *r);

// Strip a string's basename, leaving just the directory
// If there is no directory component, the string is returned as '.'
string_t* string_dirname(string_t *s, const char sep);
// Strip a string's directory, leaving just the basename
// Will return sep if that's the only path element
string_t* string_basename(string_t *s, const char sep);
// Add c to the end of a string if it doesn't already end in it
// Replaces the last character if the string can't get any bigger
string_t* string_prove_trailing(string_t *s, const char c);
// Remove all leading and trailing instances of c from string
string_t* string_trim_char(string_t *s, const char c);
// Remove all trailing instances of c from string
string_t* string_strip_trailing(string_t *s, const char c);
// Remove all leading instances of c from string
string_t* string_strip_leading(string_t *s, const char c);
// Truncate a string to l characters
string_t* string_truncate(string_t *s, const strlen_t l);

#endif // _LIB_STATIC_STRINGS_H
#ifdef __cplusplus
 }
#endif
