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
// string_dirname.c
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
string_t* string_dirname(string_t *s, const char sep) {
	strlen_t i;

	ASSERT_STRING(s);
	ASSERT_CHAR(sep);

	if (s->length == 0) {
		return s;
	}

	for (i = s->length - 1; ((i > 0) && (s->cstring[i] == sep)); --i) {};
	for (; i > 0; --i) {
		if (s->cstring[i] == sep) {
			for (; s->cstring[i] == sep; --i) {};
			++i;
			s->cstring[i] = 0;
			s->length = i;
			return s;
		}
	}
	if ((i == 0) && (s->cstring[0] == sep)) {
		s->cstring[1] = 0;
		s->length = 1;
	} else {
		s->cstring[0] = '.';
		s->cstring[1] = 0;
		s->length = 1;
	}

	return s;
}


#endif // ! USE_MONOLITHIC_ULIB
#ifdef __cplusplus
 }
#endif
