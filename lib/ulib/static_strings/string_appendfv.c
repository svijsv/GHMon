// SPDX-License-Identifier: GPL-3.0-only
/***********************************************************************
*                                                                      *
*                                                                      *
* Copyright 2021 svijsv
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
// string_appendfv.c
//
// NOTES:
//

#ifdef __cplusplus
 extern "C" {
#endif
#if ! USE_MONOLITHIC_ULIB

//#define DEBUG 1
/*
* Includes
*/
#include "private.h"
#include "../util.h"


/*
* Static values
*/


/*
* Types
*/


/*
* Variables
*/
// A pointer used by the non-reentrant string_putc() function
string_t *printf_string;


/*
* Local function prototypes
*/


/*
* Interrupt handlers
*/


/*
* Functions
*/
static void string_putc(int c) {
	assert(printf_string != NULL);

	string_append_from_char(printf_string, c);

	return;
}
string_t* string_appendfv(string_t *s, const char *format, va_list arp) {
	string_t *bs;

	ASSERT_STRING(s);
	ASSERT_CSTRING(format);

	bs = printf_string;
	printf_string = s;
	vaprintf(string_putc, format, arp);
	printf_string = bs;

	return s;
}


#endif // ! USE_MONOLITHIC_ULIB
#ifdef __cplusplus
 }
#endif
