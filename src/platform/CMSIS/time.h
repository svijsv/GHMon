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
#define TIM_MS_TICKS 1

// The EXTI line used for the RTC alarm; 17 on STM32F103
#define RTC_ALARM_EXTI_LINE_Pos 17
#define RTC_ALARM_EXTI_LINE (1 << RTC_ALARM_EXTI_LINE_Pos)

//
// Timers
// Timers must be on APB1 (alarms 2-7 or 12-14) because that's the bus I
// check when setting up the prescalers
//
// The actual timers available vary by MCU, only 2 and 3 seem to be universal:
//    STM32F103x6    has timers 1-3
//    STM32F103x[B8] has timers 1-4
//    STM32F103x[EG] has timers 1-8
// Other STM32 MCUs may have more or less than these
// Only the clocks for timers 1-4 are currently initialized in timers_init(),
// if any others are used they need to be added there.
//
// Sleep alarm timer
// If this changes, uncomment the code for timer 2 PWM and comment out the
// PWM code for the new timer in time.c
#define SLEEP_ALARM_TIM       TIM2
#define SLEEP_ALARM_CLOCKEN   RCC_PERIPH_TIM2
#define SLEEP_ALARM_IRQn      TIM2_IRQn
#define SleepAlarm_IRQHandler TIM2_IRQHandler
//
// Micro-second counter timer
// USCOUNTER_TIM is #defined in platform.h for convenience
//#define USCOUNTER_TIM   TIM3
#define USCOUNTER_CLOCKEN RCC_PERIPH_TIM3
//
// Timers used for PWM output
// The timers used for the sleep alarm and the microsecond counter can't be
// used for PWM
//
// Some timer-to-pin mappings are specified in platform.h. All such mappings
// for a given MCU should be in the datasheet (NOT the reference manual, which
// as far as I can tell only lists the alternate pin mappings) in the Pinouts
// and Pin Description section.
//
// Timers 1-4 are the 'primary' PWM timers, they control 4 pins each and
// correspond to the labeled pins on bluepill/compatible board schematics so
// when additional timers are present 1-4 should be reserved for PWM
//
// Timer 1 conflicts with UART1 and USB
// Timer 2 conflicts with the ADC and UART2
// Timer 3 conflicts with the ADC and SPI1
// Timer 4 conflicts with I2C1 and remapped UART1
// As long as a specific pin is never used with both PWM and the conflicting
// peripheral at the same time it shouldn't cause any problems.
#define USE_TIMER1_PWM 1
#define USE_TIMER2_PWM 0
#define USE_TIMER3_PWM 0
#define USE_TIMER4_PWM 1

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
