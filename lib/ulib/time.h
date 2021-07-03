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
// util.h
// Small utility functions
// NOTES:
//

#ifdef __cplusplus
 extern "C" {
#endif
#ifndef _ULIB_TIME_H
#define _ULIB_TIME_H

/*
* Includes
*/
#include "common.h"

#include "types.h"
#include "util.h"
#include "fmem.h"


/*
* Static values
*/
// Seconds in a given time period
#define MINUTES (60U)
#define HOURS   (3600U)
#define DAYS    (86400U)
#define YEARS   (31536000U)

// Year 0 of the internal calendar
#define YEAR_0 2010


/*
* Types
*/


/*
* Variable declarations
*/
extern _FLASH const uint8_t days_per_month[];


/*
* Function prototypes
*/
// Convert between epoch-based times and calendar times
// year is the number of years after YEAR_0
utime_t date_to_seconds(uint8_t year, uint8_t month, uint8_t day);
utime_t time_to_seconds(uint8_t hour, uint8_t minute, uint8_t second);
void seconds_to_date(utime_t seconds, uint8_t *year, uint8_t *month, uint8_t *day);
void seconds_to_time(utime_t seconds, uint8_t *hour, uint8_t *minute, uint8_t *second);


/*
* Macros
*/
// Calculate when a timer started now would expire
#define SET_TIMEOUT(delay) ((SYSTICKS) + (delay))
// Check whether a timer set for some time has expired
// Depending on the system, SYSTICKS may not update continuously (such as
// during low-power modes)
#define TIMES_UP(timer) (UNLIKELY((SYSTICKS) >= (timer)))

// Like SET_TIMEOUT() and TIMES_UP() but for working with timeouts that may
// span periods when systicks are unavailable, like a low power mode.
#define SET_RTC_TIMEOUT(delay) ((RTCTICKS) + (delay))
#define RTC_TIMES_UP(timer) (UNLIKELY((RTCTICKS) >= (timer)))

// A dumb timeout that counts loop iterations
// Can't just check (--(timer) <= 0) because that will rollover if timer is 0
// and unsigned.
#define DUMB_TIMEOUT(timer) ((timer == 0) || (--(timer) == 0))

// Convert a frequency to a millisecond time period
//#define HZ_TO_MS(freq) (1000 / freq)
// Convert a frequency to a millisecond time period, rounded to the nearest
#define HZ_TO_MS(freq) ((1000 + (freq/2)) / freq)

// Convert a frequency to a microsecond time period
//#define HZ_TO_US(freq) (1000000 / freq)
// Convert a frequency to a microsecond time period, rounded to the nearest
#define HZ_TO_US(freq) ((1000000 + (freq/2)) / freq)


#endif // _LIB_TIME_H
#ifdef __cplusplus
 }
#endif
