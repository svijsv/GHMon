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
// cstring_basename.c
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
const char* cstring_basename(const char *s, char sep) {
	const char *bn;

	assert(s != NULL);
	assert(sep != 0);

	bn = strrchr(s, sep);
	return (bn != NULL) ? bn+1 : s;
}

#else // ! USE_CLIB_STRING
const char* cstring_basename(const char *s, char sep) {
	cstrlen_t i;

	assert(s != NULL);
	assert(sep != 0);

	i = cstring_len(s);
	if (i == 0) {
		return (char *)s;
	}

	for (; ((s[i] == sep) && (i > 0)); --i) {
		// Nothing to do here
	}
	for (; ((s[i] != sep) && (i > 0)); --i) {
		// Nothing to do here
	}
	if ((i == 0) && (s[0] != sep)) {
		return (char *)s;
	}

	return (char *)&s[i+1];
}
#endif // USE_CLIB_STRING


#endif // ! USE_MONOLITHIC_ULIB
#ifdef __cplusplus
 }
#endif
