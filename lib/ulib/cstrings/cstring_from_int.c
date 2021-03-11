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
// cstring_from_int.c
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
char* cstring_from_int(char *s, int n, int8_t width, char pad) {
	cstrlen_t i;

	assert(s != NULL);
	assert((pad != 0) || (width <= 0));

	i = 0;
	if (n < 0) {
		s[i++] = '-';
		n = -n;
	}

	// There's always at least one digit printed
	--width;
	if (n < 10) {
		for (; width > 0; --width) {
			s[i++] = pad;
		}
		s[i++] = '0' + (n);
	} else {
		int p;
		int8_t r;

		for (p = 1, r = 0; (n / p) > 0; p *= 10, ++r) {
			// Nothing to do here
		}
		for (; width > r; --width) {
			s[i++] = pad;
		}
		// Without the p /= 10 we'd have a leading 0.
		for (p /= 10; p > 0; p /= 10) {
			s[i++] = '0' + (n / p);
			n %= p;
		}
	}
	s[i] = 0;

	return &s[i];
}


#endif // ! USE_MONOLITHIC_ULIB
#ifdef __cplusplus
 }
#endif
