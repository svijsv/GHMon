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
/// @file
/// @brief Time Management Interface
/// @attention
///    This file should only be included by interface.h.
///

///
/// @name SYSTICK Interface.
/// @{
//
///
/// Enable the system millisecond tick.
void enable_systick(void);
///
/// Disable the system millisecond tick.
void disable_systick(void);
///
/// Check if the systick is enabled.
///
/// @retval true if turned on.
/// @retval false if turned off.
bool systick_is_enabled(void);

#if __HAVE_DOXYGEN__
///
/// Read the systick counter.
///
/// This always increases or stays the same from one read to the next.
///
/// @attention
/// This will not increase during sleep.
# define NOW_MS()
#endif
///
/// An alias for @c NOW_MS() required by @c ulib.
#define GET_SYSTICKS_MS() NOW_MS()
/// @}


///
/// @name Real-Time Clock Interface.
/// @{
//
#if uHAL_USE_RTC || __HAVE_DOXYGEN__
///
/// Set system time
///
/// @param hour The hour part of the new system time, in the range 0-23.
/// @param minute The minute part of the new system time.
/// @param second The second part of the new system time.
///
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t set_time(uint8_t hour, uint8_t minute, uint8_t second);

///
/// Set system date
///
/// @param year The last 2 digits of the year part of the new system date.
/// @param month The month part of the new system date.
/// @param day The day part of the new system date.
///
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t set_date(uint8_t year, uint8_t month, uint8_t day);

///
/// Set the system time enumerated in seconds.
///
/// @attention
/// This has the effect of setting both the date and the time.
///
/// @param s The new system time.
///
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t set_RTC_seconds(utime_t s);

///
/// Get the system time enumerated in seconds.
///
/// @returns The system time.
utime_t get_RTC_seconds(void);

#if uHAL_USE_RTC_EMULATION || __HAVE_DOXYGEN__
///
/// Add milliseconds to the RTC.
///
/// @attention
/// This only impacts emulated RTCs.
///
/// @param ms The number of milliseconds to add to the RTC.
void add_RTC_millis(uint_fast16_t ms);

///
/// Subtract milliseconds from the RTC.
///
/// @attention
/// This only impacts emulated RTCs.
///
/// @param ms The number of milliseconds to add to the RTC.
void subtract_RTC_millis(uint_fast16_t ms);
#endif // uHAL_USE_RTC_EMULATION

#ifndef NOW
///
/// Get the system time enumerated in seconds.
///
/// This updates even during sleep.
///
/// @attention This may be lower for a later read if the system time was set
/// in the meantime.
# define NOW() (get_RTC_seconds())
#endif
///
/// An alias for @c NOW() required by @c ulib.
#define RTCTICKS NOW()

#endif // uHAL_USE_RTC
/// @}


///
/// @name Microsecond Counter Interface.
/// @{
//
#if uHAL_USE_USCOUNTER || __HAVE_DOXYGEN__
///
/// Enable the microsecond counter.
///
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t uscounter_on(void);
///
/// Disable the microsecond counter.
///
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t uscounter_off(void);

///
/// Start the microsecond counter.
///
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t uscounter_start(void);

///
/// Stop the microsecond counter.
///
/// @returns The number of microseconds since the counter was started..
uint_fast32_t uscounter_stop(void);

///
/// Read the current value of the microsecond counter.
///
/// @returns The number of microseconds since the counter was started..
uint_fast32_t uscounter_read(void);
/// @}
#endif // uHAL_USE_USCOUNTER

///
/// @name System Delay Interface.
/// @{
//
///
/// Don't do anything for a bit.
///
/// @attention
/// The systick must be enabled or this will hang indefinitely.
///
/// @param ms The number of milliseconds to pause.
void delay_ms(utime_t ms);

///
/// A 'dumb' delay that doesn't know about ticks.
///
/// @attention
/// Don't expect this to be very accurate.
///
/// @param ms The number of milliseconds to pause.
void dumb_delay_ms(utime_t ms);

///
/// A 'dumber' delay that doesn't even know about milliseconds.
///
/// @attention
/// Don't expect this to be very accurate.
///
/// @param cycles The number of cycles to pause.
void dumb_delay_cycles(uint_fast32_t cycles);
/// @}
