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
// time_TCB.h
// Manage the time-keeping peripherals
//
#ifndef _uHAL_PLATFORM_XMEGA3_TIME_TCB_H
#define _uHAL_PLATFORM_XMEGA3_TIME_TCB_H

#include "time_private.h"


//
// TCB PWM settings
//
// Using both PWM and either the systick or micro-second counter at the same
// time won't work for TCBx because there's no way to set the output low before
// the timer reloads
#if SYSTICK_TIMER == TIMER_TCB0 || USCOUNTER_TIMER == TIMER_TCB0 || !HAVE_TCB0
# define DISABLE_TCB0_PWM 1
#endif
#if SYSTICK_TIMER == TIMER_TCB1 || USCOUNTER_TIMER == TIMER_TCB1 || !HAVE_TCB1
# define DISABLE_TCB1_PWM 1
#endif
#if SYSTICK_TIMER == TIMER_TCB2 || USCOUNTER_TIMER == TIMER_TCB2 || !HAVE_TCB2
# define DISABLE_TCB2_PWM 1
#endif
#if SYSTICK_TIMER == TIMER_TCB3 || USCOUNTER_TIMER == TIMER_TCB3 || !HAVE_TCB3
# define DISABLE_TCB3_PWM 1
#endif
#ifndef DISABLE_TCB0_PWM
# define DISABLE_TCB0_PWM 0
#endif
#ifndef DISABLE_TCB1_PWM
# define DISABLE_TCB1_PWM 0
#endif
#ifndef DISABLE_TCB2_PWM
# define DISABLE_TCB2_PWM 0
#endif
#ifndef DISABLE_TCB3_PWM
# define DISABLE_TCB3_PWM 0
#endif


//
// TCB systick settings
//
#if IS_TCB(SYSTICK_TIMER)
# if SYSTICK_TIMER == TIMER_TCB0 && HAVE_TCB0
#  define SYSTICK_ISR TCB0_INT_vect
#  define CLEAR_SYSTICK_INTFLAG() (TCB0.INTFLAGS = TCB_CAPT_bm)
#  define SYSTICK_TCBx TCB0
# elif SYSTICK_TIMER == TIMER_TCB1 && HAVE_TCB1
#  define SYSTICK_ISR TCB1_INT_vect
#  define CLEAR_SYSTICK_INTFLAG() (TCB1.INTFLAGS = TCB_CAPT_bm)
#  define SYSTICK_TCBx TCB1
# elif SYSTICK_TIMER == TIMER_TCB2 && HAVE_TCB2
#  define SYSTICK_ISR TCB2_INT_vect
#  define CLEAR_SYSTICK_INTFLAG() (TCB2.INTFLAGS = TCB_CAPT_bm)
#  define SYSTICK_TCBx TCB2
# elif SYSTICK_TIMER == TIMER_TCB3 && HAVE_TCB3
#  define SYSTICK_ISR TCB3_INT_vect
#  define CLEAR_SYSTICK_INTFLAG() (TCB3.INTFLAGS = TCB_CAPT_bm)
#  define SYSTICK_TCBx TCB3
# else
#  error "Unhandled SYSTICK_TIMER"
# endif
#endif // IS_TCB(SYSTICK_TIMER)


//
// TCB microsecond counter settings
//
#if IS_TCB(USCOUNTER_TIMER)
# if USCOUNTER_TIMER == TIMER_TCB0 && HAVE_TCB0
#  define USCOUNTER_ISR TCB0_INT_vect
#  define CLEAR_USCOUNTER_INTFLAG() (TCB0.INTFLAGS = TCB_CAPT_bm)
#  define USCOUNTER_TCBx TCB0
# elif USCOUNTER_TIMER == TIMER_TCB1 && HAVE_TCB1
#  define USCOUNTER_ISR TCB1_INT_vect
#  define CLEAR_USCOUNTER_INTFLAG() (TCB1.INTFLAGS = TCB_CAPT_bm)
#  define USCOUNTER_TCBx TCB1
# elif USCOUNTER_TIMER == TIMER_TCB2 && HAVE_TCB2
#  define USCOUNTER_ISR TCB2_INT_vect
#  define CLEAR_USCOUNTER_INTFLAG() (TCB2.INTFLAGS = TCB_CAPT_bm)
#  define USCOUNTER_TCBx TCB2
# elif USCOUNTER_TIMER == TIMER_TCB3 && HAVE_TCB3
#  define USCOUNTER_ISR TCB3_INT_vect
#  define CLEAR_USCOUNTER_INTFLAG() (TCB3.INTFLAGS = TCB_CAPT_bm)
#  define USCOUNTER_TCBx TCB3
# else
#  error "Unhandled USCOUNTER_TIMER"
# endif

# define USCOUNTER_MAX (0xFFFFU)
# define USCOUNTER_IS_DOWNCOUNT 0
# define USCOUNTER_CLKDIV 2U
#endif // IS_TCB(USCOUNTER_TIMER)


#endif // _uHAL_PLATFORM_XMEGA3_TIME_TCB_H
