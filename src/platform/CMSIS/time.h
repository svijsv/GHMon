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
// time.h
// Manage the time-keeping peripherals
// NOTES:
//   Prototypes for most of the related functions are in interface.h
//

#ifdef __cplusplus
 extern "C" {
#endif
#ifndef _PLATFORM_CMSIS_TIME_H
#define _PLATFORM_CMSIS_TIME_H

/*
* Includes
*/
#include "common.h"


/*
* Static values
*/
// Set the number of ticks per millisecond for 16 bit timers
// At PCLK1 speeds >~65MHz with a PCLK1 prescaler of 1 or PCLK1 speeds >~32MHz
// with any other PCLK1 prescaler the 16 bit timer prescaler would overflow if
// the ratio were kept at 1.
// Increasing this decreases the maximum duration proportionately.
// Must be at least 1.
#define TIM_MS_PERIOD 1

// The EXTI line used for the RTC alarm; 17 on STM32F103
#define RTC_ALARM_EXTI_LINE_Pos 17
#define RTC_ALARM_EXTI_LINE (1 << RTC_ALARM_EXTI_LINE_Pos)

//
// Timers
// SLEEP_ALARM_TIM must be on APB1 (alarms 2-7 or 12-14)
#define SLEEP_ALARM_TIM TIM2
#define SLEEP_ALARM_CLOCKEN_Pos RCC_APB1ENR_TIM2EN_Pos
#define SLEEP_ALARM_CLOCKEN     (1 << SLEEP_ALARM_CLOCKEN_Pos)
#define SLEEP_ALARM_IRQn TIM2_IRQn
#define SleepAlarm_IRQHandler TIM2_IRQHandler

/*
* Types
*/


/*
* Variable declarations (defined in time.c)
*/


/*
* Function prototypes (defined in time.c)
*/
// Initialize the time-keeping peripherals
void time_init(void);

// Set the sleep alarm
void set_sleep_alarm(uint16_t ms);
void stop_sleep_alarm(void);
// Set the RTC alarm
// time is the number or seconds in the future when the alarm is triggered
void set_RTC_alarm(utime_t time);
void stop_RTC_alarm(void);


/*
* Macros
*/


#endif // _PLATFORM_CMSIS_TIME_H
#ifdef __cplusplus
 }
#endif
