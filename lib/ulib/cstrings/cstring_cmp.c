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
// cstring_cmp.c
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
int cstring_cmp(const char *s1, const char *s2) {
	assert (s1 != NULL);
	assert (s2 != NULL);

	return strcmp(s1, s2);
}

#else // !USE_CLIB_STRING
int cstring_cmp(const char *s1, const char *s2) {
	const uint8_t *us1;
	const uint8_t *us2;
	cstrlen_t i;

	assert (s1 != NULL);
	assert (s2 != NULL);

	us1 = (uint8_t *)s1;
	us2 = (uint8_t *)s2;

	for (i = 0; ((us1[i] == us2[i]) && (us1[i] != 0)); ++i) {
		// Nothing to do in here
	}

	if (us1[i] > us2[i]) {
		return 1;
	}
	if (us1[i] < us2[i]) {
		return -1;
	}
	return 0;
}
#endif // USE_CLIB_STRING


#endif // ! USE_MONOLITHIC_ULIB
#ifdef __cplusplus
 }
#endif
