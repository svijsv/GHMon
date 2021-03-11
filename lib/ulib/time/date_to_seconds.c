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
// date_to_seconds.c
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
utime_t date_to_seconds(uint8_t year, uint8_t month, uint8_t day) {
	utime_t days;
	uint8_t year_4s, year_100s, year_400s, leap_days;

	assert(year <= (0xFFFFFFFF/YEARS));
	assert(IS_BETWEEN(month, 1, 12));
	assert(IS_BETWEEN(day, 1, 31));

	// Dates are 1-indexed, but they're counted in a 0-indexed manner
	days = day - 1;

	// Find the number of days in all previous months
	for (uiter_t i = 1; i < month; ++i) {
		days += days_per_month[i-1];
	}

	// Leap years are years that are divisible by 4 but not by 100, or years
	// that are divisible by 400
	year_4s   = GET_LEAPS(year, 4);
	year_100s = GET_LEAPS(year, 100);
	year_400s = GET_LEAPS(year, 400);
	leap_days = year_4s - year_100s + year_400s;
	if (IS_LEAP_YEAR(0)) {
		++leap_days;
	}
	if (IS_LEAP_YEAR(year)) {
		if (month < 3) {
			--leap_days;
		}
	}
	days += leap_days;

	return ((utime_t )year * YEARS) + ((utime_t )days * DAYS);
}


#endif // ! USE_MONOLITHIC_ULIB
#ifdef __cplusplus
 }
#endif
