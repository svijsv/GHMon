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
// cstring_len.c
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
#if USE_CLIB_STRING
cstrlen_t cstring_len(const char *c) {
	assert(c != NULL);

	return (cstrlen_t )strlen(c);
}

#else // !USE_CLIB_STRING
cstrlen_t cstring_len(const char *c) {
	cstrlen_t i;

	assert(c != NULL);

	for (i = 0; (c[i] != 0); ++i) {
		// Nothing to do in here
	}

	return i;
}
size_t strlen(const char *s) {
	return cstring_len(s);
}
#endif // USE_CLIB_STRING


#endif // ! USE_MONOLITHIC_ULIB
#ifdef __cplusplus
 }
#endif
