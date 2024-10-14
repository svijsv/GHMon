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
// time_private.h
// Manage the time-keeping peripherals
//
#ifndef _uHAL_PLATFORM_XMEGA3_TIME_PRIVATE_H
#define _uHAL_PLATFORM_XMEGA3_TIME_PRIVATE_H

#include "common.h"
#include "time.h"
#include "system.h"

#include "ulib/include/math.h"

#include <avr/io.h>
#include <avr/interrupt.h>


// This is handled in platform.h for practical reasons.
/*
#define TIMER_NONE      0
#define TIMER_TCA0      1
#define TIMER_TCA0_HIGH 2
#define TIMER_TCA0_LOW  3
#define TIMER_TCB0      4
#define TIMER_TCB1      5
#define TIMER_TCB2      6
#define TIMER_TCB3      7
#define TIMER_RTT       8
*/

#ifdef TCA0
# define HAVE_TCA0 1
#else
# define HAVE_TCA0 0
#endif
#ifdef TCB0
# define HAVE_TCB0 1
#else
# define HAVE_TCB0 0
#endif
#ifdef TCB1
# define HAVE_TCB1 1
#else
# define HAVE_TCB1 0
#endif
#ifdef TCB2
# define HAVE_TCB2 1
#else
# define HAVE_TCB2 0
#endif
#ifdef TCB3
# define HAVE_TCB3 1
#else
# define HAVE_TCB3 0
#endif

#define IS_TCA0(_TIM) (((_TIM) == TIMER_TCA0_HIGH) || ((_TIM) == TIMER_TCA0_LOW) || ((_TIM) == TIMER_TCA0))
#define IS_SPLIT_TCA0(_TIM) (((_TIM) == TIMER_TCA0_HIGH) || ((_TIM) == TIMER_TCA0_LOW))
#define IS_SINGLE_TCA0(_TIM) ((_TIM) == TIMER_TCA0)
#define IS_TCB(_TIM) (((_TIM) == TIMER_TCB0) || ((_TIM) == TIMER_TCB1) || ((_TIM) == TIMER_TCB2) || ((_TIM) == TIMER_TCB3))

#ifndef USE_SPLIT_TCA0
# if IS_SINGLE_TCA0(SYSTICK_TIMER) || IS_SINGLE_TCA0(USCOUNTER_TIMER)
#  define USE_SPLIT_TCA0 0
# else
#  define USE_SPLIT_TCA0 1
# endif
#endif

// This is handled in platform.h for practical reasons (and now defaults to just
// the RTT).
/*
#ifndef SYSTICK_TIMER
// The TCA timers can be used for both (limited) PWM and the systick at the same
// time and have a broader range of prescalers but TCB timers have fewer outputs
// and maintaining the resolution of a PWM output can be important, so on balance
// I guess let's default to TCA for the systick
# if HAVE_TCA0
#  if USE_SPLIT_TCA0
#   define SYSTICK_TIMER TIMER_TCA0_HIGH
#  else
#   define SYSTICK_TIMER TIMER_TCA0
#  endif
# elif HAVE_TCB0
#  define SYSTICK_TIMER TIMER_TCB0
# elif HAVE_TCB1
#  define SYSTICK_TIMER TIMER_TCB1
# elif HAVE_TCB2
#  define SYSTICK_TIMER TIMER_TCB2
# elif HAVE_TCB3
#  define SYSTICK_TIMER TIMER_TCB3
# else
#  error "Unable to find SYSTICK timer"
# endif
#endif
*/

#if ! uHAL_USE_USCOUNTER
# undef USCOUNTER_TIMER
# define USCOUNTER_TIMER TIMER_NONE
#else
# if ! USCOUNTER_TIMER
#  undef USCOUNTER_TIMER
# endif
#endif
#ifndef USCOUNTER_TIMER
# if SYSTICK_TIMER != TIMER_TCB0
#  define USCOUNTER_TIMER TIMER_TCB0
# elif ! USE_SPLIT_TCA0
#  define USCOUNTER_TIMER TIMER_TCA0
# else
#  define USCOUNTER_TIMER TIMER_TCA0_HIGH
# endif
#endif

#ifndef RTT_RECALIBRATE_INTERVAL_S
# define RTT_RECALIBRATE_INTERVAL_S (60U*30U)
#endif

void systick_init(void);

void RTT_init(void);
void wakeup_alarm_init(void);

void uscounter_init(void);
void uscounter_start_timer(void);
uint16_t uscounter_stop_timer(void);
void uscounter_resume_timer(void);
uint16_t uscounter_pause_timer(void);

void TCA_PWM_init(TCA_t *TCAx);
err_t TCA_pwm_off(TCA_t *TCAx, uint8_t wo_bm);
err_t TCA_pwm_set(TCA_t *TCAx, uint8_t wo_bm, uint16_t duty_cycle);
err_t TCA_pwm_on(TCA_t *TCAx, uint8_t wo_bm, uint16_t duty_cycle);


void TCB_PWM_init(TCB_t *TCBx);
err_t TCB_pwm_off(TCB_t *TCBx);
err_t TCB_pwm_set(TCB_t *TCBx, uint16_t duty_cycle);
err_t TCB_pwm_on(TCB_t *TCBx, uint16_t duty_cycle);


#endif // _uHAL_PLATFORM_XMEGA3_TIME_PRIVATE_H
