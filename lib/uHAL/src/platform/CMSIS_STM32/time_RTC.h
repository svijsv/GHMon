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
// time_RTC.h
// Manage the RTC
// NOTES:
//
#ifndef _uHAL_PLATFORM_CMSIS_TIME_RTC_H
#define _uHAL_PLATFORM_CMSIS_TIME_RTC_H

#include "common.h"

// The EXTI line used for the RTC alarm
#define RTC_ALARM_EXTI_LINE_Pos 17U
#define RTC_ALARM_EXTI_LINE (1U << RTC_ALARM_EXTI_LINE_Pos)

#if uHAL_USE_INTERNAL_LS_OSC
# define G_freq_RTC G_freq_LSI
#else
# define G_freq_RTC G_freq_LSE
#endif

void set_RTC_prediv(uint32_t psc);


#endif // _uHAL_PLATFORM_CMSIS_TIME_RTC_H
