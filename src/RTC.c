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
// RTC.c
// Emulate an RTC for systems lacking one
//
// NOTES:
//

#include "common.h"

#if uHAL_USE_RTC

//
// Utility functions for handling the date and time on systems that use second-
// counter-based RTCs
// These aren't in any headers, the prototypes should be copied into the files that use them.
utime_t RTC_datetime_to_second_counter(const datetime_t *datetime, utime_t now) {
	utime_t new_now = 0;

	uHAL_assert(datetime != NULL);
	if (!uHAL_SKIP_INVALID_ARG_CHECKS && datetime == NULL) {
		return 0;
	}

	// Only change the date if it's set in the new structure
	if ((datetime->year | datetime->month | datetime->day) != 0) {
		uHAL_assert((IS_IN_RANGE(datetime->month, 1, 12)) && (IS_IN_RANGE(datetime->day, 1, 31)));
		if (!uHAL_SKIP_INVALID_ARG_CHECKS && ((!IS_IN_RANGE(datetime->month, 1, 12)) || (!IS_IN_RANGE(datetime->day, 1, 31)))) {
			return ERR_BADARG;
		}
		new_now = date_to_seconds(datetime);
	} else {
		new_now = SNAP_TO_FACTOR(now, SECONDS_PER_DAY);
	}

	// Only change the time if it's set in the new structure
	if ((datetime->hour | datetime->minute | datetime->second) != 0) {
		uHAL_assert((datetime->hour <= 23) || (datetime->minute <= 59) || (datetime->second <= 59));
		if (!uHAL_SKIP_INVALID_ARG_CHECKS && ((datetime->hour > 23) || (datetime->minute > 59) || (datetime->second > 59))) {
			return ERR_BADARG;
		}
		new_now += time_to_seconds(datetime);
	} else {
		new_now += now % SECONDS_PER_DAY;
	}

	// Use set_RTC_seconds() directly if you want to reset the clock.
	if (!uHAL_SKIP_OTHER_CHECKS && new_now == 0) {
		return 0;
	}
	return new_now;
}

#if uHAL_USE_RTC_EMULATION

#if ! SYSTICKS_PER_S
# undef SYSTICKS_PER_S
# define SYSTICKS_PER_S (1000U)
#endif

DEBUG_CPP_MACRO(SYSTICKS_PER_S)

// 'RTC' ticks, seconds
static utime_t RTC_ticks = 0;
static utime_t RTC_prev_systicks = 0;
// It would be easier in theory to add a millisecond to the RTC by subtracting
// one from RTC_prev_systicks but the time spent sleeping is so much greater
// than the time spent awake that doing so would roll the counter over rather
// quickly, so a separate counter is needed for that
static int32_t RTC_millis = 0;


//
// Manage the fake RTC system
//
utime_t get_RTC_seconds(void) {
	utime_t systicks, diff;

	systicks = NOW_MS();
	if (systicks > RTC_prev_systicks) {
		diff = systicks - RTC_prev_systicks;
		while (diff >= (int32_t )SYSTICKS_PER_S) {
			++RTC_ticks;
			diff -= (int32_t )SYSTICKS_PER_S;
		}
		// We adjust systicks so we don't lose any left over ticks when we update
		// RTC_prev_systicks below.
		systicks -= diff;
	}
	RTC_prev_systicks = systicks;

	// This should happen close enough to every second that repeated subtraction
	// will be faster than division
	while (RTC_millis >= 1000) {
		++RTC_ticks;
		RTC_millis -= 1000;
	}
	// Without this block, the time will simply pause until it's > SYSTICKS_PER_S
	// instead of reversing because reversing may cause issues with any code
	// that expects time to always move forward
	while (RTC_millis <= -1000) {
		if (RTC_ticks != 0) {
			--RTC_ticks;
		}
		RTC_millis += 1000;
	}

	return RTC_ticks;
}
err_t set_RTC_seconds(utime_t s) {
#if uHAL_USE_UPTIME_EMULATION
	fix_uptime(s, get_RTC_seconds());
#endif

	RTC_ticks = s;
	RTC_prev_systicks = NOW_MS();
	RTC_millis = 0;

	return ERR_OK;
}

err_t set_RTC_datetime(const datetime_t *datetime) {
	return set_RTC_seconds(RTC_datetime_to_second_counter(datetime, get_RTC_seconds()));
}
err_t get_RTC_datetime(datetime_t *datetime) {
	uHAL_assert(datetime != NULL);
	if (!uHAL_SKIP_INVALID_ARG_CHECKS && datetime == NULL) {
		return ERR_BADARG;
	}

	seconds_to_datetime(get_RTC_seconds(), datetime);
	return ERR_OK;
}

void add_RTC_millis(uint_fast16_t ms) {
	RTC_millis += ms;

	return;
}
void subtract_RTC_millis(uint_fast16_t ms) {
	if (ms < RTC_millis) {
		RTC_millis -= ms;
	} else {
		RTC_millis = 0;
	}

	return;
}

#endif // uHAL_USE_RTC_EMULATION
#endif // uHAL_USE_RTC
