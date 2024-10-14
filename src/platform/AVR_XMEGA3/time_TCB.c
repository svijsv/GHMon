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

#include "time_TCB.h"

#define TCB_SYSTICK_MAX_CNT 0xFFFFU
#define TCB_PWM_MAX_CNT     0xFFU

#if DISABLE_TCB0_PWM && HAVE_TCB0
# warning "Disabling PWM from TCB0"
#endif
#if DISABLE_TCB1_PWM && HAVE_TCB1
# warning "Disabling PWM from TCB1"
#endif
#if DISABLE_TCB2_PWM && HAVE_TCB2
# warning "Disabling PWM from TCB2"
#endif
#if DISABLE_TCB3_PWM && HAVE_TCB3
# warning "Disabling PWM from TCB3"
#endif

#if (SYSTICK_TIMER == TIMER_TCB0 || USCOUNTER_TIMER == TIMER_TCB0) && !HAVE_TCB0
# error "SYSTICK or USCOUNTER is set to non-existant timer TCB1"
#endif
#if (SYSTICK_TIMER == TIMER_TCB1 || USCOUNTER_TIMER == TIMER_TCB1) && !HAVE_TCB1
# error "SYSTICK or USCOUNTER is set to non-existant timer TCB1"
#endif
#if (SYSTICK_TIMER == TIMER_TCB2 || USCOUNTER_TIMER == TIMER_TCB2) && !HAVE_TCB2
# error "SYSTICK or USCOUNTER is set to non-existant timer TCB2"
#endif
#if (SYSTICK_TIMER == TIMER_TCB3 || USCOUNTER_TIMER == TIMER_TCB3) && !HAVE_TCB3
# error "SYSTICK or USCOUNTER is set to non-existant timer TCB3"
#endif

#if defined(TCB_DUTY_CYCLE_SCALE) && TCB_DUTY_CYCLE_SCALE > 0
# if !defined(TCB_DUTY_CYCLE_ADJUST)
#  define TCB_DUTY_CYCLE_ADJUST(_dc_) (_dc_)
# endif
# if TCB_DUTY_CYCLE_SCALE > TCB_PWM_MAX_CNT
#  error "TCB_DUTY_CYCLE_SCALE > TCB_PWM_MAX_CNT"
# endif
#else
# undef TCB_DUTY_CYCLE_SCALE
# undef TCB_DUTY_CYCLE_ADJUST
# if PWM_DUTY_CYCLE_SCALE <= TCB_PWM_MAX_CNT
#  define TCB_DUTY_CYCLE_SCALE PWM_DUTY_CYCLE_SCALE
#  define TCB_DUTY_CYCLE_ADJUST(_dc_) (_dc_)
# elif PWM_DUTY_CYCLE_SCALE <= (TCB_PWM_MAX_CNT*2)
#  define TCB_DUTY_CYCLE_SCALE (PWM_DUTY_CYCLE_SCALE/2U)
#  define TCB_DUTY_CYCLE_ADJUST(_dc_) SHIFT_DIV_2(_dc_)
# elif PWM_DUTY_CYCLE_SCALE <= (TCB_PWM_MAX_CNT*4)
#  define TCB_DUTY_CYCLE_SCALE (PWM_DUTY_CYCLE_SCALE/4U)
#  define TCB_DUTY_CYCLE_ADJUST(_dc_) SHIFT_DIV_4(_dc_)
# elif PWM_DUTY_CYCLE_SCALE <= (TCB_PWM_MAX_CNT*8)
#  define TCB_DUTY_CYCLE_SCALE (PWM_DUTY_CYCLE_SCALE/8U)
#  define TCB_DUTY_CYCLE_ADJUST(_dc_) SHIFT_DIV_8(_dc_)
# elif PWM_DUTY_CYCLE_SCALE <= (TCB_PWM_MAX_CNT*16)
#  define TCB_DUTY_CYCLE_SCALE (PWM_DUTY_CYCLE_SCALE/16U)
#  define TCB_DUTY_CYCLE_ADJUST(_dc_) SHIFT_DIV_16(_dc_)
# elif PWM_DUTY_CYCLE_SCALE <= (TCB_PWM_MAX_CNT*32)
#  define TCB_DUTY_CYCLE_SCALE (PWM_DUTY_CYCLE_SCALE/32U)
#  define TCB_DUTY_CYCLE_ADJUST(_dc_) SHIFT_DIV_32(_dc_)
# elif PWM_DUTY_CYCLE_SCALE <= (TCB_PWM_MAX_CNT*64)
#  define TCB_DUTY_CYCLE_SCALE (PWM_DUTY_CYCLE_SCALE/64U)
#  define TCB_DUTY_CYCLE_ADJUST(_dc_) SHIFT_DIV_64(_dc_)
# elif PWM_DUTY_CYCLE_SCALE <= (TCB_PWM_MAX_CNT*128)
#  define TCB_DUTY_CYCLE_SCALE (PWM_DUTY_CYCLE_SCALE/128U)
#  define TCB_DUTY_CYCLE_ADJUST(_dc_) SHIFT_DIV_128(_dc_)
# elif PWM_DUTY_CYCLE_SCALE <= (TCB_PWM_MAX_CNT*256)
#  define TCB_DUTY_CYCLE_SCALE (PWM_DUTY_CYCLE_SCALE/256U)
#  define TCB_DUTY_CYCLE_ADJUST(_dc_) SHIFT_DIV_256(_dc_)
# else
#  define TCB_DUTY_CYCLE_SCALE (TCB_PWM_MAX_CNT)
#  define TCB_DUTY_CYCLE_ADJUST(_dc_) (((uint32_t )(_dc_) * TCB_PWM_MAX_CNT) / PWM_DUTY_CYCLE_SCALE)
#  warning "Using expensive calculations for TCB PWM duty cycles"
# endif
#endif
#if !TCB_DUTY_CYCLE_SCALE
# error "Invalid TCB_DUTY_CYCLE_SCALE"
#endif

DEBUG_CPP_MACRO(TCB_DUTY_CYCLE_SCALE)

#define NEED_TCB_PWM (uHAL_USE_PWM) && (!DISABLE_TCB0_PWM || !DISABLE_TCB1_PWM || !DISABLE_TCB2_PWM || !DISABLE_TCB3_PWM)

#ifndef TCB_OVF_bm
# define TCB_OVF_bm 0U
#endif
#define CLEAR_TCB_INTERRUPTS(_TCBx_) ((_TCBx_).INTFLAGS = TCB_CAPT_bm | TCB_OVF_bm)

#if USE_STDBY_TCB
# define TCB_RUNSTDBY_FLAG (TCB_RUNSTDBY_bm)
#else
# define TCB_RUNSTDBY_FLAG (0U)
#endif
#define TCB_CTRLA_FLAGS (TCB_RUNSTDBY_FLAG)

//
// SYSTICK
//
#if IS_TCB(SYSTICK_TIMER)
# define TIMER_CLOCK G_freq_TCBCLK
# define COUNTER_MAX TCB_SYSTICK_MAX_CNT
# define TIMER_HZ 1000U
# include "time_TCB_calc_exact.h"
  DEBUG_CPP_MACRO(TIMER_TOP)
  DEBUG_CPP_MACRO(TIMER_SRC)
  DEBUG_CPP_MACRO(TIMER_ERROR)

void systick_init(void) {
	SYSTICK_TCBx.EVCTRL = 0;
	SYSTICK_TCBx.INTCTRL = 0;
	SYSTICK_TCBx.CTRLA = TIMER_SRC|TCB_CTRLA_FLAGS;
	SYSTICK_TCBx.CTRLB = TCB_CNTMODE_INT_gc;
	write_reg16(&SYSTICK_TCBx.CNT, 0);
	write_reg16(&SYSTICK_TCBx.CCMP, TIMER_TOP);
	// The manual recommends clearing the interrupt flags after configuration,
	// which is done by writing a '1' to them
	CLEAR_TCB_INTERRUPTS(SYSTICK_TCBx);

	return;
}
void enable_systick(void) {
	SET_BIT(SYSTICK_TCBx.INTCTRL, TCB_CAPT_bm);
	SET_BIT(SYSTICK_TCBx.CTRLA, TCB_ENABLE_bm);

	return;
}
void disable_systick(void) {
	CLEAR_BIT(SYSTICK_TCBx.INTCTRL, TCB_CAPT_bm);
	CLEAR_BIT(SYSTICK_TCBx.CTRLA, TCB_ENABLE_bm);

	return;
}
bool systick_is_enabled(void) {
	return BIT_IS_SET(SYSTICK_TCBx.CTRLA, TCB_ENABLE_bm);
}
#endif // IS_TCB(SYSTICK_TIMER)

//
// USCOUNTER
//
#if IS_TCB(USCOUNTER_TIMER)
void uscounter_init(void) {
	USCOUNTER_TCBx.EVCTRL = 0;
	USCOUNTER_TCBx.INTCTRL = 0;
	USCOUNTER_TCBx.CTRLA = TCB_CLKSEL_CLKDIV2_gc|TCB_CTRLA_FLAGS;
	USCOUNTER_TCBx.CTRLB = TCB_CNTMODE_INT_gc;
	write_reg16(&USCOUNTER_TCBx.CNT, 0);
	write_reg16(&USCOUNTER_TCBx.CCMP, USCOUNTER_MAX);
	// The manual recommends clearing the interrupt flags after configuration,
	// which is done by writing a '1' to them
	CLEAR_TCB_INTERRUPTS(USCOUNTER_TCBx);

	return;
}

void uscounter_start_timer(void) {
	write_reg16(&USCOUNTER_TCBx.CNT, 0);
	USCOUNTER_TCBx.INTFLAGS = TCB_CAPT_bm;
	SET_BIT(USCOUNTER_TCBx.INTCTRL, TCB_CAPT_bm);
	SET_BIT(USCOUNTER_TCBx.CTRLA, TCB_ENABLE_bm);

	return;
}
uint16_t uscounter_stop_timer(void) {
	uint16_t cnt;

	CLEAR_BIT(USCOUNTER_TCBx.INTCTRL, TCB_CAPT_bm);
	CLEAR_BIT(USCOUNTER_TCBx.CTRLA, TCB_ENABLE_bm);
	cnt = read_reg16(&USCOUNTER_TCBx.CNT);

	return cnt;
}

void uscounter_resume_timer(void) {
	SET_BIT(USCOUNTER_TCBx.CTRLA, TCB_ENABLE_bm);
	return;
}
uint16_t uscounter_pause_timer(void) {
	uint16_t cnt;

	CLEAR_BIT(USCOUNTER_TCBx.CTRLA, TCB_ENABLE_bm);
	cnt = read_reg16(&USCOUNTER_TCBx.CNT);

	return cnt;
}
#endif // IS_TCB(USCOUNTER_TIMER)

//
// PWM
//
#if NEED_TCB_PWM
#  define TIMER_CLOCK G_freq_TCBCLK
#  define TIMER_HZ PWM_FREQUENCY_HZ
#  define TIMER_TOP TCB_DUTY_CYCLE_SCALE
#  include "time_TCB_calc_PWM.h"
   DEBUG_CPP_MACRO(TIMER_SRC)
/*
#  define TCB_PWM_CTRLA_FLAGS (TIMER_SRC|TCB_CTRLA_FLAGS)
#  define TCB_PWM_CTRLB_FLAGS (TCB_CNTMODE_PWM8_gc)
*/
static const uint8_t TCB_PWM_CTRLA_FLAGS = (TIMER_SRC|TCB_CTRLA_FLAGS);
static const uint8_t TCB_PWM_CTRLB_FLAGS = (TCB_CNTMODE_PWM8_gc);
#if TCB_PWM_16BIT_REG_WORKAROUND
static const uint8_t TCB_PWM_16BIT_REG_WORKAROUND_CCMPL = TIMER_TOP;
#endif

void TCB_PWM_init(TCB_t *TCBx) {
	TCBx->EVCTRL = 0;
	TCBx->INTCTRL = 0;
	TCBx->CTRLA = TCB_PWM_CTRLA_FLAGS;
	TCBx->CTRLB = TCB_PWM_CTRLB_FLAGS;
	write_reg16(&TCBx->CNT, 0);
	// In PWM mode, CCMPL controls the period and CCMPH controls the duty cycle
	TCBx->CCMPL = TIMER_TOP;
#if TCB_PWM_16BIT_REG_WORKAROUND
	TCBx->CCMPH = 0;
#endif
	// The manual recommends clearing the interrupt flags after configuration,
	// which is done by writing a '1' to them
	CLEAR_TCB_INTERRUPTS(*TCBx);

	return;
}

#  undef TIMER_CLOCK
#  undef TIMER_HZ
#  undef TIMER_TOP
#  undef TIMER_SRC

err_t TCB_pwm_off(TCB_t *TCBx) {
	TCBx->CTRLA = TCB_PWM_CTRLA_FLAGS;
	TCBx->CTRLB = TCB_PWM_CTRLB_FLAGS;

	return ERR_OK;
}
err_t TCB_pwm_set(TCB_t *TCBx, uint16_t duty_cycle) {
#if TCB_PWM_16BIT_REG_WORKAROUND
	TCBx->CCMPL = TCB_PWM_16BIT_REG_WORKAROUND_CCMPL;
#endif
	TCBx->CCMPH = TCB_DUTY_CYCLE_ADJUST(duty_cycle);

	return ERR_OK;
}
err_t TCB_pwm_on(TCB_t *TCBx, uint16_t duty_cycle) {
	TCB_pwm_set(TCBx, duty_cycle);
	TCBx->CTRLB = TCB_PWM_CTRLB_FLAGS | TCB_CCMPEN_bm;
	TCBx->CTRLA = TCB_PWM_CTRLA_FLAGS | TCB_ENABLE_bm;

	return ERR_OK;
}
#endif //NEED_TCB_PWM
