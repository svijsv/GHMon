// SPDX-License-Identifier: GPL-3.0-only
/***********************************************************************
*                                                                      *
*                                                                      *
* Copyright 2021, 2024 svijsv                                          *
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
// calendar.c
// NOTES:
//

#include "common.h"

#if uHAL_USE_RTC

#include "ulib/include/time.h"


err_t set_time(uint8_t hour, uint8_t minute, uint8_t second) {
	err_t res;
	uint32_t now;

	assert((hour <= 23) || (minute <= 59) || (second <= 59));
#if ! uHAL_SKIP_INIT_CHECKS
#endif
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if ((hour > 23) || (minute > 59) || (second > 59)) {
		return ERR_BADARG;
	}
#endif

	// Conserve the date part of the RTC
	// Don't call get_RTC_seconds() from the macro SNAP_TO_FACTOR() or it will
	// be called twice.
	now = get_RTC_seconds();
	now = SNAP_TO_FACTOR(now, SECONDS_PER_DAY) + time_to_seconds(hour, minute, second);
	LOGGER("Setting time to %02u:%02u:%02u\r\n", (uint_t )hour, (uint_t )minute, (uint_t )second);
	res = set_RTC_seconds(now);

	return res;
}
err_t set_date(uint8_t year, uint8_t month, uint8_t day) {
	err_t res;
	uint32_t now;

	//assert((year <= (0xFFFFFFFF/SECONDS_PER_YEAR)) && (IS_IN_RANGE(month, 1, 12)) && (IS_IN_RANGE(day, 1, 31)));
	assert((IS_IN_RANGE(month, 1, 12)) && (IS_IN_RANGE(day, 1, 31)));
#if ! uHAL_SKIP_INIT_CHECKS
#endif
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	//if ((year > (0xFFFFFFFF/SECONDS_PER_YEAR)) || (!IS_IN_RANGE(month, 1, 12)) || (!IS_IN_RANGE(day, 1, 31))) {
	if ((!IS_IN_RANGE(month, 1, 12)) || (!IS_IN_RANGE(day, 1, 31))) {
		return ERR_BADARG;
	}
#endif

	// Conserve the time part of the RTC
	now = (get_RTC_seconds() % SECONDS_PER_DAY) + date_to_seconds(year, month, day);
	LOGGER("Setting date to %02u.%02u.%02u\r\n", (uint_t )(TIME_YEAR_0+(uint_t )year), (uint_t )month, (uint_t )day);
	res = set_RTC_seconds(now);

	return res;
}

#endif // uHAL_USE_RTC
