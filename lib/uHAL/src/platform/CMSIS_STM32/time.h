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
// time.h
// Manage the time-keeping peripherals
// NOTES:
//   Prototypes for most of the related functions are in interface.h
//
//   PWM is supported only for timers 1-4 and only on the default pins for
//   the STM32F1 lines. Timer 2 may be used for other tasks and the others
//   may be absent depending on platform.
//
#ifndef _uHAL_PLATFORM_CMSIS_TIME_H
#define _uHAL_PLATFORM_CMSIS_TIME_H


#include "common.h"

#define TIMER_NONE 0
#define TIMER_1   1
#define TIMER_2   2
#define TIMER_3   3
#define TIMER_4   4
#define TIMER_5   5
#define TIMER_6   6
#define TIMER_7   7
#define TIMER_8   8
#define TIMER_9   9
#define TIMER_10 10
#define TIMER_11 11
#define TIMER_12 12
#define TIMER_13 13
#define TIMER_14 14
#define TIMER_CNT 14

// Initialize the time-keeping peripherals
void time_init(void);

#if uHAL_USE_HIBERNATE
// Set the sleep alarm
uint32_t set_sleep_alarm(uint32_t ms);
void stop_sleep_alarm(void);
bool sleep_alarm_is_set(void);
// Set the RTC alarm
// time is the number or seconds in the future when the alarm is triggered
void set_RTC_alarm(utime_t time);
void stop_RTC_alarm(void);
bool RTC_alarm_is_set(void);
#endif


#endif // _uHAL_PLATFORM_CMSIS_TIME_H
