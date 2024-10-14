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
#ifndef _uHAL_PLATFORM_CMSIS_TIME_PRIVATE_H
#define _uHAL_PLATFORM_CMSIS_TIME_PRIVATE_H


#include "time.h"
#include "time_reconcile_IRQs.h"
#include "time_find_active.h"


// Timers 2 through 5 have 16 bit counters on the STM32F1s and 32 bit counters
// in other lines
#if ! HAVE_STM32F1_TIMERS
# define TIM2_5_MAX_CNT 0xFFFFFFFFUL
#else
# define TIM2_5_MAX_CNT 0xFFFFUL
#endif
#define TIM_MAX_CNT 0xFFFFUL
#define TIM_MAX_PSC 0xFFFFUL

// If any prescaler other than 1 is used on a timer's bus clock, the timer's
// frequency is doubled
#if G_freq_PCLK1 == G_freq_HCLK
# define TIM_APB1_MAX_HZ G_freq_PCLK1
#else
# define TIM_APB1_MAX_HZ (G_freq_PCLK1 * 2UL)
#endif
#if G_freq_PCLK2 == G_freq_HCLK
# define TIM_APB2_MAX_HZ G_freq_PCLK2
#else
# define TIM_APB2_MAX_HZ (G_freq_PCLK2 * 2UL)
#endif

//#define QUICK_PSC(_TIMID_, _HZ_) (is_apb1_tim(_TIMID_) ? TIM_APB1_MAX_HZ/(_HZ_) :  TIM_APB2_MAX_HZ/(_HZ_)) - 1UL;
#define QUICK_PSC(_TIMID_, _HZ_) (IS_APB1_TIM(_TIMID_) ? TIM_APB1_MAX_HZ/(_HZ_) :  TIM_APB2_MAX_HZ/(_HZ_)) - 1UL;
#define IS_APB1_TIM(_TIMID_) ((_TIMID_) != TIMER_1 && (_TIMID_) != TIMER_8 && (_TIMID_) != TIMER_9 && (_TIMID_) != TIMER_10 && (_TIMID_) != TIMER_11)


//
// When breaking the time files into separate modules (they were formally all #included
// into one main file), the non-debug test programs grew in size by around 200-300
// bytes. This isn't a ton for STM32s but was unexpected. The only thing I could find
// that made any significant difference in reducing that growth was by using inlined
// helper functions here, which cut the growth in half. I organized it so it's easy
// to test further in the future because the results may change when more timers are
// in use.
//
#define HELPER_STORAGE INLINE
#include "time_helper.h"
/*
#define HELPER_STORAGE
#ifdef INCLUDED_BY_TIME_C
# include "time_helper.h"
#else
 HELPER_STORAGE uint16_t calculate_TIM_prescaler(rcc_periph_t tim, uint32_t hz);
 //HELPER_STORAGE uint16_t calculate_TIM_prescaler(uint_fast8_t timid, uint32_t hz);

 HELPER_STORAGE TIM_TypeDef* get_tim_from_id(uint_fast8_t tim_id);
 HELPER_STORAGE rcc_periph_t get_rcc_from_id(uint_fast8_t tim_id);
 HELPER_STORAGE uint_fast8_t get_af_from_id(uint_fast8_t tim_id);
 HELPER_STORAGE bool is_apb1_tim(uint_fast8_t tim_id);
 HELPER_STORAGE bool is_pwm_tim(uint_fast8_t tim_id);
#endif
*/

void systick_init(void);
void pwm_init(void);
void sleep_alarm_timer_init(void);
void uscounter_timer_init(void);
void RTC_init(void);


#endif // _uHAL_PLATFORM_CMSIS_TIME_PRIVATE_H
