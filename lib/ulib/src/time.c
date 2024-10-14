// SPDX-License-Identifier: GPL-3.0-only
/***********************************************************************
*                                                                      *
*                                                                      *
* Copyright 2022, 2023 svijsv                                          *
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
* along with this program. If not, see <http:// www.gnu.org/licenses/>.*
*                                                                      *
*                                                                      *
***********************************************************************/
// time.c
// Tools for dealing with time
//
// NOTES:
//
#include "time.h"
#if ULIB_ENABLE_TIME

#include "debug.h"


#define MAX_YEARS (((utime_t )-1) / SECONDS_PER_YEAR)


FMEM_STORAGE const uint8_t days_per_month[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

// This will return the number of matching years between TIME_YEAR_0 and year
// excluding TIME_YEAR_0 and year themselves
uint8_t GET_LEAPS(uint8_t year, uint16_t factor) {
	uint16_t check;
	uint8_t count;

	check = factor - (TIME_YEAR_0 % factor);
	for (count = 0; check < year; check += factor) {
		++count;
	}

	return count;
}
bool IS_LEAP_YEAR(uint16_t year) {
	uint16_t check = TIME_YEAR_0 + year;

	return (((check % 4) == 0) && ((check % 100) != 0)) || ((check % 400) == 0);
}

utime_t date_to_seconds(uint8_t year, uint8_t month, uint8_t day) {
	utime_t days;
	uint8_t year_4s, year_100s, year_400s, leap_days;

	// This is always true because year is only 8 bits
	/*
	ulib_assert(year <= MAX_YEARS);
	*/
	ulib_assert(IS_IN_RANGE_INCL(month, 1, 12));
	ulib_assert(IS_IN_RANGE_INCL(day, 1, 31));

#if DO_TIME_SAFETY_CHECKS
	// This is always false because year is only 8 bits
	/*
	if (year > MAX_YEARS) {
		year = 0;
	}
	*/
	if (!IS_IN_RANGE_INCL(month, 1, 12)) {
		month = 1;
	}
	if (!IS_IN_RANGE_INCL(day, 1, 31)) {
		day = 1;
	}
#endif

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
	leap_days = (uint8_t )(year_4s - year_100s) + year_400s;
	if ((year != 0) && IS_LEAP_YEAR(0)) {
		++leap_days;
	}
	if (IS_LEAP_YEAR(year)) {
		if (month > 2) {
			++leap_days;
		}
	}
	days += leap_days;

	return (utime_t )((utime_t )year * (utime_t )SECONDS_PER_YEAR) + (utime_t )((utime_t )days * (utime_t )SECONDS_PER_DAY);
}
void seconds_to_date(utime_t seconds, uint8_t *restrict ret_year, uint8_t *restrict ret_month, uint8_t *restrict ret_day) {
	uint8_t tmp_month;
	uint16_t tmp_year, tmp_day, leap_days = 0;

	ulib_assert(ret_year != NULL);
	ulib_assert(ret_month != NULL);
	ulib_assert(ret_day != NULL);

	tmp_year = (uint16_t )(seconds / SECONDS_PER_YEAR);
	tmp_day = (uint16_t )((seconds % SECONDS_PER_YEAR) / SECONDS_PER_DAY);

	// Leap years are years that are divisible by 4 but not by 100, or years
	// that are divisible by 400
	// Exclude the current year from the calculation to simplify things later
	// on
	if (tmp_year > 0) {
		uint8_t year_4s, year_100s, year_400s;

		year_4s   = GET_LEAPS((uint8_t)(tmp_year-1), 4);
		year_100s = GET_LEAPS((uint8_t)(tmp_year-1), 100);
		year_400s = GET_LEAPS((uint8_t)(tmp_year-1), 400);
		leap_days = (uint8_t )(year_4s - year_100s) + year_400s;
	}
	if ((tmp_year != 0) && (IS_LEAP_YEAR(0))) {
		++leap_days;
	}
	// FIXME: Handle leap_days > 365?
	if (leap_days > tmp_day) {
		// FIXME: Handle tmp_year == 0? Don't know how we'd get that with so
		// many leap days though
		--tmp_year;
		// Day 1 of one year is the same as day 366 of the previous year
		// We're still 0-indexed so we use 365 instead of 366
		tmp_day = 365 - (uint16_t )(leap_days - tmp_day);
	} else {
		tmp_day -= leap_days;
	}

	// Months and days are 1-indexed
	tmp_month = 1;
	++tmp_day;
	for (uiter_t i = 0; (days_per_month[i] < tmp_day); ++i) {
		tmp_day -= days_per_month[i];
		++tmp_month;
	}

	if (IS_LEAP_YEAR(tmp_year) && (tmp_month > 2)) {
		--tmp_day;

		if (tmp_day == 0) {
			--tmp_month;

			if (tmp_month == 2) {
				tmp_day = 29;
			} else {
				//tmp_day = days_per_month[tmp_month-1];
				tmp_day = 31;
			}
		}
	}

#if DO_TIME_SAFETY_CHECKS
	if (ret_year != NULL) {
		*ret_year  = (uint8_t )tmp_year;
	}
	if (ret_month != NULL) {
		*ret_month = (uint8_t )tmp_month;
	}
	if (ret_day != NULL) {
		*ret_day = (uint8_t )tmp_day;
	}
#else
	*ret_year  = (uint8_t )tmp_year;
	*ret_month = (uint8_t )tmp_month;
	*ret_day   = (uint8_t )tmp_day;
#endif

	return;
}

void seconds_to_time(utime_t seconds, uint8_t *restrict ret_hour, uint8_t *restrict ret_minute, uint8_t *restrict ret_second) {
	ulib_assert(ret_hour != NULL);
	ulib_assert(ret_minute != NULL);
	ulib_assert(ret_second != NULL);

#if DO_TIME_SAFETY_CHECKS
	if (ret_hour != NULL) {
		*ret_hour   = (uint8_t )((seconds % SECONDS_PER_DAY) / SECONDS_PER_HOUR);
	}
	if (ret_minute != NULL) {
		*ret_minute = (uint8_t )((seconds % SECONDS_PER_HOUR) / SECONDS_PER_MINUTE);
	}
	if (ret_second != NULL) {
		*ret_second = (uint8_t )((seconds % SECONDS_PER_MINUTE));
	}
#else
	*ret_hour   = (uint8_t )((seconds % SECONDS_PER_DAY) / SECONDS_PER_HOUR);
	*ret_minute = (uint8_t )((seconds % SECONDS_PER_HOUR) / SECONDS_PER_MINUTE);
	*ret_second = (uint8_t )((seconds % SECONDS_PER_MINUTE));
#endif

	return;
}
utime_t time_to_seconds(uint8_t hour, uint8_t minute, uint8_t second) {
	ulib_assert(hour < 24);
	ulib_assert(minute < 60);
	ulib_assert(second < 60);

#if DO_TIME_SAFETY_CHECKS
	if (hour >= 24) {
		hour = 0;
	}
	if (minute >= 60) {
		minute = 0;
	}
	if (second >= 60) {
		second = 0;
	}
#endif

	return ((utime_t )hour * (utime_t )SECONDS_PER_HOUR) + ((utime_t )minute * (utime_t )SECONDS_PER_MINUTE) + (utime_t )second;
}


#else
	// ISO C forbids empty translation units, this makes it happy.
	typedef int make_iso_compilers_happy;
#endif // ULIB_ENABLE_TIME
