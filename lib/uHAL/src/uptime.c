// SPDX-License-Identifier: GPL-3.0-only
/***********************************************************************
*                                                                      *
*                                                                      *
* Copyright 2025 svijsv                                                *
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
// uptime.c
// Emulate an uptime counter for systems lacking one
//
// NOTES:
//

#include "common.h"

#if uHAL_USE_UPTIME

#define PRINT_ALL_FIELDS 0

const char* print_uptime(utime_t seconds, char *buf, uint_fast8_t buf_size) {
	uint_fast8_t i = buf_size;
	uint_fast32_t tmp;

	uHAL_assert(buf != NULL);
	// Make sure string is large enough to hold 'DdHHhMMmSSs' + NUL byte.
	uHAL_assert(buf_size >= 12);
	//assert(buf_size > 0);

	if (!uHAL_SKIP_INVALID_ARG_CHECKS) {
		if (buf == NULL || buf_size < 12) {
			return "";
		}
	}

	// Prefix the decrement because we start 'i' just past the end of the array.
	// This also means buf[i] is always our string start.
	buf[--i] = 0;

	tmp = seconds % SECONDS_PER_MINUTE;
	buf[--i] = 's';
	buf[--i] = (tmp % 10) + '0';
	buf[--i] = (tmp / 10) + '0';

	tmp = seconds / SECONDS_PER_MINUTE;
	if (!PRINT_ALL_FIELDS && tmp == 0) {
		goto END;
	}
	tmp %= MINUTES_PER_HOUR;
	buf[--i] = 'm';
	buf[--i] = (tmp % 10) + '0';
	buf[--i] = (tmp / 10) + '0';

	tmp = seconds / SECONDS_PER_HOUR;
	if (!PRINT_ALL_FIELDS && tmp == 0) {
		goto END;
	}
	tmp %= (MINUTES_PER_DAY / MINUTES_PER_HOUR);
	buf[--i] = 'h';
	buf[--i] = (tmp % 10) + '0';
	buf[--i] = (tmp / 10) + '0';

	tmp = seconds / SECONDS_PER_DAY;
	if (!PRINT_ALL_FIELDS && tmp == 0) {
		goto END;
	}
	buf[--i] = 'd';
	do {
		buf[--i] = (tmp % 10) + '0';
		tmp /= 10;
	} while (tmp > 0 && i > 0);

END:
	return &buf[i];
}


#if uHAL_USE_UPTIME_EMULATION

// For simplicity, the uptime origin may be considered to be prior to device
// startup so we need a signed type.
static itime_t uptime_origin = 0;

err_t init_uptime(void) {
	uptime_origin = get_RTC_seconds();

	return ERR_OK;
}

err_t set_uptime(utime_t uptime_seconds) {
	uptime_origin = (itime_t )get_RTC_seconds() - (itime_t )uptime_seconds;

	return ERR_OK;
}

err_t adj_uptime(itime_t adjustment_seconds) {
	// Adjusting the uptime downwards shifts the origin forward and adjusting
	// upwards shifts the origin backwards.
	uptime_origin -= adjustment_seconds;

	return ERR_OK;
}

err_t fix_uptime(utime_t new_now, utime_t old_now) {
	uptime_origin += (itime_t )new_now - (itime_t )old_now;

	return ERR_OK;
}

utime_t get_uptime(void) {
	itime_t now = get_RTC_seconds();

	uHAL_assert(now >= uptime_origin);

	return (now > uptime_origin) ? now - uptime_origin : 0;
}

#endif // uHAL_USE_UPTIME_EMULATION
#endif // uHAL_USE_UPTIME
