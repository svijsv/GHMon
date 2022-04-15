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
// cstring_strncpy.c
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
char* cstring_strncpy(char *dest, const char *src, cstrlen_t n) {
	assert (dest != NULL);
	assert (src  != NULL);

	return strncpy(dest, src, n);
}

#else // !USE_CLIB_STRING
char* cstring_strncpy(char *dest, const char *src, cstrlen_t n) {
	cstrlen_t i;

	for (i = 0; (i < n) && (src[i] != 0); ++i) {
		dest[i] = src[i];
	}
	// strncpy() writes zeroes to make sure 'n' bytes are written, but that's
	// probably not needed here
	/*
	for (; i < n; ++i) {
		dest[i] = 0;
	}
	*/
	if (i < n) {
		dest[i] = 0;
	}

	return dest;
}
#endif // USE_CLIB_STRING


#endif // ! USE_MONOLITHIC_ULIB
#ifdef __cplusplus
 }
#endif
