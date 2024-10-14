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
// time_RTT.h
// Manage the time-keeping peripherals
//
#ifndef _uHAL_PLATFORM_XMEGA3_TIME_RTT_H
#define _uHAL_PLATFORM_XMEGA3_TIME_RTT_H

#include "time_private.h"


#if SYSTICK_TIMER == TIMER_RTT
# define SYSTICK_ISR RTC_PIT_vect
# define CLEAR_SYSTICK_INTFLAG() (RTC.PITINTFLAGS = RTC_PI_bm)
#endif


#endif // _uHAL_PLATFORM_XMEGA3_TIME_RTT_H
