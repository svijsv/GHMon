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
// string_strip_leading.c
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


/*
* Static values
*/


/*
* Types
*/


/*
* Variables
*/


/*
* Local function prototypes
*/


/*
* Interrupt handlers
*/


/*
* Functions
*/
string_t* string_strip_leading(string_t *s, const char c) {
	strlen_t i, j;

	ASSERT_STRING(s);
	ASSERT_CHAR(c);

	if (s->length == 0) {
		return s;
	}
	if (s->cstring[0] != c) {
		return s;
	}

	for (i = 0; s->cstring[i] == c; ++i) {};
	if (i == 0) {
		return s;
	}
	for (j = 0; i < s->length; ++i, ++j) {
		s->cstring[j] = s->cstring[i];
	}
	s->length = j;
	s->cstring[j] = 0;

	return s;
}


#endif // ! USE_MONOLITHIC_ULIB
#ifdef __cplusplus
 }
#endif
