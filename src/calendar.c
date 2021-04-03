// SPDX-License-Identifier: GPL-3.0-only
/***********************************************************************
*                                                                      *
*                                                                      *
* Copyright 2021 svijsv                                                *
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

#ifdef __cplusplus
 extern "C" {
#endif

//#define DEBUG 1
/*
* Includes
*/
#include "calendar.h"

#include "ulib/time.h"


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
err_t set_time(uint8_t hour, uint8_t minute, uint8_t second) {
	err_t res;
	uint32_t now;

	if ((hour > 24) || (minute > 59) || (second > 59)) {
		return EUSAGE;
	}

	// Conserve the date part of the RTC
	// Don't call get_RTC_seconds() from the macro SNAP_TO_FACTOR() or it will
	// be called twice.
	now = get_RTC_seconds();
	now = SNAP_TO_FACTOR(now, DAYS) + time_to_seconds(hour, minute, second);
	res = set_RTC_seconds(now);

	return res;
}
err_t set_date(uint8_t year, uint8_t month, uint8_t day) {
	err_t res;
	uint32_t now;

	if ((year > (0xFFFFFFFF/YEARS)) || (!IS_BETWEEN(month, 1, 12)) || (!IS_BETWEEN(day, 1, 31))) {
		return EUSAGE;
	}

	// Conserve the time part of the RTC
	now = (get_RTC_seconds() % DAYS) + date_to_seconds(year, month, day);
	res = set_RTC_seconds(now);

	return res;
}


#ifdef __cplusplus
 }
#endif
