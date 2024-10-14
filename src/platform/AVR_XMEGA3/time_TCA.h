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
// time_TCA.h
// Manage the time-keeping peripherals
//
#ifndef _uHAL_PLATFORM_XMEGA3_TIME_TCA_H
#define _uHAL_PLATFORM_XMEGA3_TIME_TCA_H

#include "time_private.h"


#if USE_SPLIT_TCA0
# define TCA0_MAX_CNT (0xFFU)
#else
# define TCA0_MAX_CNT (0xFFFFU)
#endif

#define TCAx_RESET(_TCAx_) do {(_TCAx_).SINGLE.CTRLA = 0; (_TCAx_).SINGLE.CTRLESET = TCA_SINGLE_CMD_RESET_gc; } while (0);
//
// Derive the current TCA0 clock from its base clock and prescaler
#define TCA0_FREQ (G_freq_TCACLK / TCA_prescalers[((uint8_t )(SELECT_BITS(TCA0.SINGLE.CTRLA, TCA_SINGLE_CLKSEL_gm)) >> TCA_SINGLE_CLKSEL_gp)])

#if USE_STDBY_TCA && defined(TCA_SINGLE_RUNSTDBY_bm)
# define TCA_RUNSTDBY_FLAG (TCA_SINGLE_RUNSTDBY_bm)
#else
# define TCA_RUNSTDBY_FLAG (0U)
#endif
#define TCA_CTRLA_FLAGS (TCA_RUNSTDBY_FLAG)


//
// TCA PWM settings
//
#if defined(TCA0_DUTY_CYCLE_SCALE) && TCA0_DUTY_CYCLE_SCALE > 0
# if !defined(TCA0_DUTY_CYCLE_ADJUST)
#  define TCA0_DUTY_CYCLE_ADJUST(_dc_) (_dc_)
# endif
# if TCA0_DUTY_CYCLE_SCALE > TCA0_MAX_CNT
#  error "TCA0_DUTY_CYCLE_SCALE > TCA0_MAX_CNT"
# endif
#else
# undef TCA0_DUTY_CYCLE_SCALE
# undef TCA0_DUTY_CYCLE_ADJUST
# if PWM_DUTY_CYCLE_SCALE <= TCA0_MAX_CNT
#  define TCA0_DUTY_CYCLE_SCALE PWM_DUTY_CYCLE_SCALE
#  define TCA0_DUTY_CYCLE_ADJUST(_dc_) (_dc_)
# elif PWM_DUTY_CYCLE_SCALE <= (TCA0_MAX_CNT*2)
#  define TCA0_DUTY_CYCLE_SCALE (PWM_DUTY_CYCLE_SCALE/2U)
#  define TCA0_DUTY_CYCLE_ADJUST(_dc_) SHIFT_DIV_2(_dc_)
# elif PWM_DUTY_CYCLE_SCALE <= (TCA0_MAX_CNT*4)
#  define TCA0_DUTY_CYCLE_SCALE (PWM_DUTY_CYCLE_SCALE/4U)
#  define TCA0_DUTY_CYCLE_ADJUST(_dc_) SHIFT_DIV_4(_dc_)
# elif PWM_DUTY_CYCLE_SCALE <= (TCA0_MAX_CNT*8)
#  define TCA0_DUTY_CYCLE_SCALE (PWM_DUTY_CYCLE_SCALE/8U)
#  define TCA0_DUTY_CYCLE_ADJUST(_dc_) SHIFT_DIV_8(_dc_)
# elif PWM_DUTY_CYCLE_SCALE <= (TCA0_MAX_CNT*16)
#  define TCA0_DUTY_CYCLE_SCALE (PWM_DUTY_CYCLE_SCALE/16U)
#  define TCA0_DUTY_CYCLE_ADJUST(_dc_) SHIFT_DIV_16(_dc_)
# elif PWM_DUTY_CYCLE_SCALE <= (TCA0_MAX_CNT*32)
#  define TCA0_DUTY_CYCLE_SCALE (PWM_DUTY_CYCLE_SCALE/32U)
#  define TCA0_DUTY_CYCLE_ADJUST(_dc_) SHIFT_DIV_32(_dc_)
# elif PWM_DUTY_CYCLE_SCALE <= (TCA0_MAX_CNT*64)
#  define TCA0_DUTY_CYCLE_SCALE (PWM_DUTY_CYCLE_SCALE/64U)
#  define TCA0_DUTY_CYCLE_ADJUST(_dc_) SHIFT_DIV_64(_dc_)
# elif PWM_DUTY_CYCLE_SCALE <= (TCA0_MAX_CNT*128)
#  define TCA0_DUTY_CYCLE_SCALE (PWM_DUTY_CYCLE_SCALE/128U)
#  define TCA0_DUTY_CYCLE_ADJUST(_dc_) SHIFT_DIV_128(_dc_)
# elif PWM_DUTY_CYCLE_SCALE <= (TCA0_MAX_CNT*256)
#  define TCA0_DUTY_CYCLE_SCALE (PWM_DUTY_CYCLE_SCALE/256U)
#  define TCA0_DUTY_CYCLE_ADJUST(_dc_) SHIFT_DIV_256(_dc_)
# else
#  define TCA0_DUTY_CYCLE_SCALE (TCA0_MAX_CNT)
#  define TCA0_DUTY_CYCLE_ADJUST(_dc_) (((uint32_t )(_dc_) * TCA0_MAX_CNT) / PWM_DUTY_CYCLE_SCALE)
#  warning "Using expensive calculations for TCA0 PWM duty cycles"
# endif
#endif
#if !TCA0_DUTY_CYCLE_SCALE
# error "Invalid TCA0_DUTY_CYCLE_SCALE"
#endif


//
// TCA systick settings
//
#if IS_SPLIT_TCA0(SYSTICK_TIMER)
# if SYSTICK_TIMER == TIMER_TCA0_HIGH
#  define SYSTICK_ISR TCA0_HUNF_vect
#  define CLEAR_SYSTICK_INTFLAG() (TCA0.SPLIT.INTFLAGS = TCA_SPLIT_HUNF_bm)
# elif SYSTICK_TIMER == TIMER_TCA0_LOW
#  define SYSTICK_ISR TCA0_LUNF_vect
#  define CLEAR_SYSTICK_INTFLAG() (TCA0.SPLIT.INTFLAGS = TCA_SPLIT_LUNF_bm)
# else
#  error "Unhandled SYSTICK_TIMER"
# endif

#elif IS_SINGLE_TCA0(SYSTICK_TIMER)
# define SYSTICK_ISR TCA0_OVF_vect
# define CLEAR_SYSTICK_INTFLAG() (TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm)
#endif


//
// TCA microsecond counter settings
//
// Comparing BEST_MOD with <= here would give the lowest divisor and possibly
// save some cycles when dividing the final value but would also lower the
// number of ticks the timer could give before rolling over
#if IS_TCA0(USCOUNTER_TIMER)
# define BEST_MOD (G_freq_TCACLK % 16000000U)
# define USCOUNTER_CLKDIV 16U
# if (G_freq_TCACLK % 8000000) < BEST_MOD
#  undef BEST_MOD
#  undef USCOUNTER_CLKDIV
#  define BEST_MOD (G_freq_TCACLK % 8000000U)
#  define USCOUNTER_CLKDIV 8U
# endif
# if (G_freq_TCACLK % 4000000) < BEST_MOD
#  undef BEST_MOD
#  undef USCOUNTER_CLKDIV
#  define BEST_MOD (G_freq_TCACLK % 4000000U)
#  define USCOUNTER_CLKDIV 4U
# endif
# if (G_freq_TCACLK % 2000000) < BEST_MOD
#  undef BEST_MOD
#  undef USCOUNTER_CLKDIV
#  define BEST_MOD (G_freq_TCACLK % 2000000U)
#  define USCOUNTER_CLKDIV 2U
# endif
# if (G_freq_TCACLK % 1000000) < BEST_MOD
#  undef BEST_MOD
#  undef USCOUNTER_CLKDIV
#  define BEST_MOD (G_freq_TCACLK % 1000000U)
#  define USCOUNTER_CLKDIV 1U
# endif
# undef BEST_MOD

# if USCOUNTER_CLKDIV == 16
#  define TCA_SPLIT_USCOUNTER_CLKSEL_DIV TCA_SPLIT_CLKSEL_DIV16_gc
# elif USCOUNTER_CLKDIV == 8
#  define TCA_SPLIT_USCOUNTER_CLKSEL_DIV TCA_SPLIT_CLKSEL_DIV8_gc
# elif USCOUNTER_CLKDIV == 4
#  define TCA_SPLIT_USCOUNTER_CLKSEL_DIV TCA_SPLIT_CLKSEL_DIV4_gc
# elif USCOUNTER_CLKDIV == 2
#  define TCA_SPLIT_USCOUNTER_CLKSEL_DIV TCA_SPLIT_CLKSEL_DIV2_gc
# else
#  define TCA_SPLIT_USCOUNTER_CLKSEL_DIV TCA_SPLIT_CLKSEL_DIV1_gc
# endif
# define TCA_SINGLE_USCOUNTER_CLKSEL_DIV TCA_SPLIT_USCOUNTER_CLKSEL_DIV

# if IS_SPLIT_TCA0(USCOUNTER_TIMER)
#  if USCOUNTER_TIMER == TIMER_TCA0_HIGH
#   define USCOUNTER_ISR TCA0_HUNF_vect
#   define CLEAR_USCOUNTER_INTFLAG() (TCA0.SPLIT.INTFLAGS = TCA_SPLIT_HUNF_bm)
#   define USCOUNTER_UNF_bm TCA_SPLIT_HUNF_bm
#   define USCOUNTER_CNT (TCA0.SPLIT.HCNT)
#   define USCOUNTER_PER  (TCA0.SPLIT.HPER)
#   define USCOUNTER_NPER (TCA0.SPLIT.LPER)

#  elif USCOUNTER_TIMER == TIMER_TCA0_LOW
#   define USCOUNTER_ISR TCA0_LUNF_vect
#   define CLEAR_USCOUNTER_INTFLAG() (TCA0.SPLIT.INTFLAGS = TCA_SPLIT_LUNF_bm)
#   define USCOUNTER_UNF_bm TCA_SPLIT_LUNF_bm
#   define USCOUNTER_CNT (TCA0.SPLIT.LCNT)
#   define USCOUNTER_PER  (TCA0.SPLIT.LPER)
#   define USCOUNTER_NPER (TCA0.SPLIT.HPER)

#  else
#   error "Unhandled USCOUNTER_TIMER"
#  endif
#  define USCOUNTER_MAX (0xFFU)
#  define USCOUNTER_IS_DOWNCOUNT 1

# else // IS_SPLIT_TCA0(USCOUNTER_TIMER)
#  define USCOUNTER_ISR TCA0_OVF_vect
#  define CLEAR_USCOUNTER_INTFLAG() (TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm)
#  define USCOUNTER_MAX (0xFFFFU)
#  define USCOUNTER_IS_DOWNCOUNT 0
# endif // IS_SPLIT_TCA0(USCOUNTER_TIMER)
#endif // IS_TCA0(USCOUNTER_TIMER)


#endif // _uHAL_PLATFORM_XMEGA3_TIME_TCA_H
