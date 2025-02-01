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
/// @note
///    This file should only be included by interface.h.
///

#include "ulib/include/time.h"


#if __HAVE_DOXYGEN__
///
/// Get the system time enumerated in seconds.
///
/// This updates even during sleep.
///
/// @attention This may return either the system time or the system uptime
///  depending on configuration.
///
/// @attention This may be lower for a later read if the system time was changed
///  in the meantime.
# define NOW()
///
/// Read the systick counter.
///
/// This always increases or stays the same from one read to the next.
///
/// @attention
/// This will not increase during sleep.
# define NOW_MS()
#else
# if ! defined(NOW)
#  if uHAL_USE_RTC
#   define NOW() (get_RTC_seconds())
#  elif uHAL_USE_UPTIME
#   define NOW() (get_uptime())
#  endif
# endif
///
/// An alias for @c NOW_MS() required by @c ulib.
# define GET_SYSTICKS_MS() NOW_MS()
///
/// An alias for @c NOW() required by @c ulib.
# define RTCTICKS NOW()
#endif


///
/// @name SYSTICK Interface.
/// @{
//
///
/// Enable the system millisecond tick counter.
void enable_systick(void);
///
/// Disable the system millisecond tick counter.
void disable_systick(void);
///
/// Check if the systick counter is enabled.
///
/// @retval true if turned on.
/// @retval false if turned off.
bool systick_is_enabled(void);
/// @}


#if uHAL_USE_RTC || __HAVE_DOXYGEN__
///
/// @name Real-Time Clock Interface.
/// @{
//
///
/// Set system date/time
///
/// @param datetime The new system date/time. Must not be NULL.
///
/// @note If the year, month, and day are all @c 0, the date isn't set.
/// @note If the hour, minute, and second are all @c 0, the time isn't set.
///
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t set_RTC_datetime(const datetime_t *datetime);

///
/// Read the system date/time
///
/// @param datetime The object used to return the current date/time. Must not
/// be NULL.
///
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t get_RTC_datetime(datetime_t *datetime);

///
/// Set the system time enumerated in seconds.
///
/// @note
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
/// @note
/// This only impacts emulated RTCs.
///
/// @param ms The number of milliseconds to add to the RTC.
void add_RTC_millis(uint_fast16_t ms);

///
/// Subtract milliseconds from the RTC.
///
/// @note
/// This only impacts emulated RTCs.
///
/// @param ms The number of milliseconds to add to the RTC.
void subtract_RTC_millis(uint_fast16_t ms);
#endif // uHAL_USE_RTC_EMULATION
/// @}
#endif // uHAL_USE_RTC

#if uHAL_USE_UPTIME || __HAVE_DOXYGEN__
///
/// @name Uptime Counter Interface.
/// @{
//
/// Initialize the uptime counter.
///
/// This will normally be called by the platform initialization code.
///
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t init_uptime(void);

///
/// Set the uptime counter.
///
/// @param uptime_seconds The number of seconds to set the counter to.
///
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t set_uptime(utime_t uptime_seconds);

///
/// Adjust the uptime counter.
///
/// @param adjustment_seconds The number of seconds to adjust the counter by.
///
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t adj_uptime(itime_t adjustment_seconds);

#if uHAL_USE_UPTIME_EMULATION || __HAVE_DOXYGEN__
///
/// Fix the uptime counter after an RTC update.
///
/// @note This is only used for emulated uptime counters.
///
/// @param new_now The new value of the RTC in seconds.
/// @param old_now The previous value of the RTC in seconds.
///
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t fix_uptime(utime_t new_now, utime_t old_now);
#endif

///
/// Return the system uptime in seconds.
///
/// @returns The number of seconds which have elapsed since the system booted.
utime_t get_uptime(void);
/// @}
#endif // uHAL_USE_UPTIME

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
/// @note
/// Don't expect this to be very accurate.
///
/// @param ms The number of milliseconds to pause.
void dumb_delay_ms(utime_t ms);

///
/// A 'dumber' delay that doesn't even know about milliseconds.
///
/// @note
/// Don't expect this to be very accurate.
///
/// @param cycles The number of cycles to pause.
void dumb_delay_cycles(uint_fast32_t cycles);
/// @}
