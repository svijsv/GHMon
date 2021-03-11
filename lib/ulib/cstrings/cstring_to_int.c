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
// cstring_to_int.c
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
#if USE_CLIB_STDLIB
int cstring_to_int(const char *s) {
	assert(s != NULL);

	return atoi(s);
}

#else // !USE_CLIB_STDLIB
int cstring_to_int(const char *s) {
	cstrlen_t i;
	int value;

	assert(s != NULL);

	switch (s[0]) {
		case '-':
		case '+':
			i = 1;
			break;
		default:
			i = 0;
			break;
	}

	for (value = 0; s[i] != 0; ++i) {
		value *= 10;
		value += s[i] - '0';
	}

	if (s[0] == '-') {
		value = -value;
	}

	return value;
}
#endif // USE_CLIB_STDLIB


#endif // ! USE_MONOLITHIC_ULIB
#ifdef __cplusplus
 }
#endif
