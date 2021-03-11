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
// string_append_from_int.c
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
string_t* string_append_from_int(string_t *s, int n, uint8_t width, char pad) {
	int m, r;

	assert((pad != 0) || (width == 0));
	ASSERT_STRING(s);

	if (n < 0) {
		string_append_from_char(s, '-');
		n = -n;
	}

	// Find the maximum number of needed characters
	for (m = 1, r = 0; ((n / m) != 0); m *= 10, ++r) {
		// Nothing to do in here
	}
	if (r == 0) {
		if (width > 0) {
			--width;
			for (; r < width; ++r) {
				string_append_from_char(s, pad);
			}
		}
		return string_append_from_char(s, '0');
	}

	for (; width > r; ++r) {
		string_append_from_char(s, pad);
	}
	// Without this first m /= 10 we'd have a leading 0.
	for (m /= 10; m > 0; m /= 10) {
		string_append_from_char(s, '0' + ((n/m) % 10));
	}

	return s;
}


#endif // ! USE_MONOLITHIC_ULIB
#ifdef __cplusplus
 }
#endif
