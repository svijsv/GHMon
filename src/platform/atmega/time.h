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
#ifndef _PLATFORM_ATMEGA_TIME_H
#define _PLATFORM_ATMEGA_TIME_H

/*
* Includes
*/
#include "common.h"


/*
* Static values
*/


/*
* Types
*/


/*
* Variable declarations
*/
extern volatile bool wakeup_alarm_is_set;


/*
* Function prototypes
*/
// Initialize the time-keeping peripherals
void time_init(void);

// Set the wake-up alarm
// This will return the number of ms the alarm was actually set for, which
// is probably less than what's desired due to hardware limitations. It's
// possible that a value may be returned without the alarm actually being set
// if the timer needed to do something like calibrate itself and took longer
// than the desired sleep period; in that case wakeup_alarm_is_set will be
// false
uint16_t set_wakeup_alarm(uint16_t ms);
void stop_wakeup_alarm(void);

// Enable and disable the systick timer
void enable_systick(void);
void disable_systick(void);

// Add time to the internal fake RTC
void add_RTC_millis(uint16_t ms);


/*
* Macros
*/


#endif // _PLATFORM_ATMEGA_TIME_H
#ifdef __cplusplus
 }
#endif
