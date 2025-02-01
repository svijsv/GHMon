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


#define MAX_YEARS ((UTIME_MAX / SECONDS_PER_YEAR))

FMEM_STORAGE const uint_fast8_t days_per_month[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

/*
static bool IS_LEAP_YEAR(time_year_t year) {
	time_year_t check = year;

	return (((year % 4) == 0) && ((year % 100) != 0)) || ((year % 400) == 0);
}
*/
#define IS_LEAP_YEAR(_year_) (((((_year_) % 4) == 0) && (((_year_) % 100) != 0)) || (((_year_) % 400) == 0))

// Count the number of leap years between TIME_YEAR_0 and end_year, inclusive
static uint_fast8_t count_leap_years(time_year_t end_year) {
	uint_fast8_t count = 0;

	// Leap years are years that are divisible by 4 but not by 100, or years
	// that are divisible by 400
	if (end_year >= TIME_YEAR_0) {
		// Count from the first leap year not following year 0
		// If I understand correctly, (X/Y)*Y can only be optimized into the expected
		// outcome from the normal division operations, not to X.
		time_year_t period = (time_year_t )(end_year - ((TIME_YEAR_0 / 4) * 4));
		count = (uint_fast8_t )(period / 4);

		period = (time_year_t )(end_year - ((TIME_YEAR_0 / 100) * 100));
		count -= (uint_fast8_t )(period / 100);

		period = (time_year_t )(end_year - ((TIME_YEAR_0 / 400) * 400));
		count += (uint_fast8_t )(period / 400);

		if (IS_LEAP_YEAR(TIME_YEAR_0)) {
			++count;
		}
	}

	return count;
}

utime_t date_to_seconds(const datetime_t *datetime) {
	uint_fast16_t days;
	uint_fast8_t leap_days;
	time_year_t year;

	ulib_assert(datetime != NULL);
	//ulib_assert(year >= TIME_YEAR_0 && (year - TIME_YEAR_0) <= MAX_YEARS);
	ulib_assert(IS_IN_RANGE_INCL(datetime->month, 1, 12));
	ulib_assert(IS_IN_RANGE_INCL(datetime->day, 1, 31));

	// Handle un-set RTCs
	year = (datetime->year < TIME_YEAR_0) ? datetime->year + TIME_YEAR_0 : datetime->year;

#if DO_TIME_SAFETY_CHECKS
	if (datetime == NULL) {
		return 0;
	}
	if (year < TIME_YEAR_0 || (year - TIME_YEAR_0) > MAX_YEARS) {
		return 0;
	}
	if (!IS_IN_RANGE_INCL(datetime->month, 1, 12)) {
		return 0;
	}
	if (!IS_IN_RANGE_INCL(datetime->day, 1, 31)) {
		return 0;
	}
#endif

	// Dates are 1-indexed, but they're counted in a 0-indexed manner
	days = datetime->day - 1;

	// Find the number of days in all previous months
	for (uiter_t i = 1; i < datetime->month; ++i) {
		days += days_per_month[i-1];
	}

	leap_days = count_leap_years(year);
	if (IS_LEAP_YEAR(year) && (datetime->month <= 2)) {
		--leap_days;
	}
	days += leap_days;

	return (utime_t )((utime_t )(year - TIME_YEAR_0) * (utime_t )SECONDS_PER_YEAR) + (utime_t )((utime_t )days * (utime_t )SECONDS_PER_DAY);
}

void seconds_to_date(utime_t seconds, datetime_t *ret_datetime) {
	uint_fast8_t tmp_month, leap_days = 0;
	uint_fast16_t tmp_day;
	time_year_t tmp_year;

	ulib_assert(ret_datetime != NULL);
#if DO_TIME_SAFETY_CHECKS
	if (ret_datetime == NULL) {
		return;
	}
#endif

	tmp_year = (time_year_t )(seconds / SECONDS_PER_YEAR) + TIME_YEAR_0;
	tmp_day = (uint_fast16_t )((seconds % SECONDS_PER_YEAR) / SECONDS_PER_DAY);

	// Don't count the current year to make it easier to handle the stuff below
	leap_days = count_leap_years(tmp_year-1);

	// FIXME: Handle leap_days > 365?
	if (leap_days > tmp_day) {
		// FIXME: Handle tmp_year == 0? Don't know how we'd ever get that though.
		--tmp_year;
		// Day 1 of one year is the same as day 366 of the previous year
		// We're still 0-indexed so we use 365 instead of 366
		tmp_day = 365 - (uint_fast16_t )(leap_days - tmp_day);
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
				tmp_day = days_per_month[tmp_month-1];
			}
		}
	}

	ret_datetime->year  = tmp_year;
	ret_datetime->month = tmp_month;
	ret_datetime->day   = (uint_fast8_t )tmp_day;

	return;
}

utime_t time_to_seconds(const datetime_t *datetime) {
	uint_fast8_t hour, minute, second;

	ulib_assert(datetime != NULL);
	ulib_assert(datetime->hour < 24);
	ulib_assert(datetime->minute < 60);
	ulib_assert(datetime->second < 60);

#if DO_TIME_SAFETY_CHECKS
	if (datetime == NULL) {
		return 0;
	}
#endif

	hour = datetime->hour;
	minute = datetime->minute;
	second = datetime->second;

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

	return (hour * SECONDS_PER_HOUR) + (minute * SECONDS_PER_MINUTE) + second;
}
void seconds_to_time(utime_t seconds, datetime_t *ret_datetime) {
	ulib_assert(ret_datetime != NULL);

#if DO_TIME_SAFETY_CHECKS
	if (ret_datetime == NULL) {
		return;
	}
#endif
	ret_datetime->hour   = (uint8_t )((seconds % SECONDS_PER_DAY) / SECONDS_PER_HOUR);
	ret_datetime->minute = (uint8_t )((seconds % SECONDS_PER_HOUR) / SECONDS_PER_MINUTE);
	ret_datetime->second = (uint8_t )((seconds % SECONDS_PER_MINUTE));

	return;
}

utime_t datetime_to_seconds(const datetime_t *datetime) {
	return date_to_seconds(datetime) + time_to_seconds(datetime);
}
void seconds_to_datetime(utime_t seconds, datetime_t *ret_datetime) {
	seconds_to_date(seconds, ret_datetime);
	seconds_to_time(seconds, ret_datetime);

	return;
}

#else
	// ISO C forbids empty translation units, this makes it happy.
	typedef int make_iso_compilers_happy;
#endif // ULIB_ENABLE_TIME
