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
// time.c
// Manage the time-keeping peripherals
//
// NOTES:
//

#include "time_private.h"

#if IS_TCA0(SYSTICK_TIMER)
# include "time_TCA.h"
#elif IS_TCB(SYSTICK_TIMER)
# include "time_TCB.h"
#elif SYSTICK_TIMER == TIMER_RTT
# include "time_RTT.h"
#endif

DEBUG_CPP_MACRO(SYSTICK_TIMER)


// System ticks, milliseconds
volatile utime_t G_sys_msticks = 0;

ISR(SYSTICK_ISR) {
	CLEAR_SYSTICK_INTFLAG();
	++G_sys_msticks;
}
