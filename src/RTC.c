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

#if uHAL_USE_RTC && uHAL_USE_RTC_EMULATION


#if ! SYSTICKS_PER_S
# undef SYSTICKS_PER_S
# define SYSTICKS_PER_S (1000U)
#endif

DEBUG_CPP_MACRO(SYSTICKS_PER_S)

// 'RTC' ticks, seconds
static utime_t RTC_ticks = 0;
static utime_t RTC_prev_msticks = 0;
// It would be easier in theory to add a millisecond to the RTC by subtracting
// one from RTC_prev_msticks but the time spent sleeping is so much greater
// than the time spent awake that doing so would roll the counter over rather
// quickly, so a separate counter is needed for that
static int32_t RTC_millis = 0;


//
// Manage the fake RTC system
//
utime_t get_RTC_seconds(void) {
	utime_t msticks;

#if ! uHAL_SKIP_INIT_CHECKS
#endif
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
#endif

	msticks = NOW_MS();
	RTC_millis += (msticks - RTC_prev_msticks);
	RTC_prev_msticks = msticks;

	// This should happen close enough to every second that repeated subtraction
	// will be faster than division
	while (RTC_millis >= (int32_t )SYSTICKS_PER_S) {
		++RTC_ticks;
		RTC_millis -= (int32_t )SYSTICKS_PER_S;
	}
	// Without this block, the time will simply pause until it's > SYSTICKS_PER_S
	// instead of reversing because reversing may cause issues with any code
	// that expects time to always move forward
	while (RTC_millis <= -(int32_t )SYSTICKS_PER_S) {
		if (RTC_ticks != 0) {
			--RTC_ticks;
		}
		RTC_millis += SYSTICKS_PER_S;
	}

	return RTC_ticks;
}
err_t set_RTC_seconds(utime_t s) {
#if ! uHAL_SKIP_INIT_CHECKS
#endif
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
#endif

	RTC_ticks = s;
	RTC_prev_msticks = NOW_MS();
	RTC_millis = 0;

	return ERR_OK;
}
void add_RTC_millis(uint16_t ms) {
#if ! uHAL_SKIP_INIT_CHECKS
#endif
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
#endif

	RTC_millis += ms;

	return;
}
void subtract_RTC_millis(uint16_t ms) {
#if ! uHAL_SKIP_INIT_CHECKS
#endif
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
#endif

	if (ms < RTC_millis) {
		RTC_millis -= ms;
	} else {
		RTC_millis = 0;
	}

	return;
}

#endif // uHAL_USE_RTC && uHAL_USE_RTC_EMULATION
