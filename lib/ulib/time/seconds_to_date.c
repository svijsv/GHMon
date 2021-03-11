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
// seconds_to_date.c
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
void seconds_to_date(utime_t seconds, uint8_t *year, uint8_t *month, uint8_t *day) {
	int8_t tmonth;
	int16_t tyear, tday, leap_days;
	uint8_t year_4s, year_100s, year_400s;

	assert(year != NULL);
	assert(month != NULL);
	assert(day != NULL);

	tyear = seconds / YEARS;
	tday = ((seconds % YEARS) / DAYS);

	// Leap years are years that are divisible by 4 but not by 100, or years
	// that are divisible by 400
	// Exclude the current year from the calculation to simplify things later
	// on
	year_4s   = GET_LEAPS(tyear-1, 4);
	year_100s = GET_LEAPS(tyear-1, 100);
	year_400s = GET_LEAPS(tyear-1, 400);
	leap_days = year_4s - year_100s + year_400s;
	if ((IS_LEAP_YEAR(0)) && (tyear != 0)) {
		++leap_days;
	}
	// TODO: Handle leap_days > 365?
	if (leap_days > tday) {
		// TODO: Handle tyear == 0
		--tyear;
		// Day 0 of one year is the same as day 366 of the previous year
		tday = 366 - (leap_days - tday);
	} else {
		tday -= leap_days;
	}

	// Months and days are 1-indexed
	tmonth = 1;
	++tday;
	for (uiter_t i = 0; (days_per_month[i] < tday); ++i) {
		tday -= days_per_month[i];
		++tmonth;
	}

	if (IS_LEAP_YEAR(tyear) && (tmonth > 2)) {
		--tday;

		if (tday == 0) {
			--tmonth;

			if (tmonth == 2) {
				tday = 29;
			} else {
				tday = days_per_month[tmonth-1];
			}
		}
	}

	*year  = tyear;
	*month = tmonth;
	*day   = tday;

	return;
}


#endif // ! USE_MONOLITHIC_ULIB
#ifdef __cplusplus
 }
#endif
