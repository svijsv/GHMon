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
// time_TCA_single.c
// Manage the TCA peripheral
//
// NOTES:
//

#include "time_TCA.h"

#if ! USE_SPLIT_TCA0

#define TCA_SINGLE_CMPxEN (TCA_SINGLE_CMP0EN_bm | TCA_SINGLE_CMP1EN_bm | TCA_SINGLE_CMP2EN_bm)
#define TCA_SINGLE_INTERRUPTS (TCA_SINGLE_CMPxEN | TCA_SINGLE_OVF_bm)

// When a TCA timer is used for PWM the relevent CMPxEN bit will be set
// When used for the systick or micro-second counter, one of the interrupts
// will be set
#define TCAx_IS_USED(_TCAx_) \
	((SELECT_BITS((_TCAx_).SINGLE.CTRLB, TCA_SINGLE_CMPxEN) != 0) || \
	 (SELECT_BITS((_TCAx_).SINGLE.INTCTRL, TCA_SINGLE_INTERRUPTS) != 0))

#define CLEAR_TCA_INTERRUPTS(_TCAx_) ((_TCAx_).SINGLE.INTFLAGS = TCA_SINGLE_INTERRUPTS)

//
// SYSTICK
//
#if IS_SINGLE_TCA0(SYSTICK_TIMER)
# define TIMER_CLOCK G_freq_TCACLK
# define COUNTER_MAX TCA0_MAX_CNT
# define TIMER_HZ 1000U
# include "time_TCA_calc_exact.h"
  DEBUG_CPP_MACRO(TIMER_TOP)
  DEBUG_CPP_MACRO(TIMER_PSC)
  DEBUG_CPP_MACRO(TIMER_ERROR)

/*
void calc_TCA_single_1ms(uint16_t *top, uint8_t *psc) {
	// TCA_XHz_exact() just divides the clock by 1000 anyway, so skip a step
	// and do it here
	*top = G_freq_TCACLK / 1000;
	*psc = TCA_SINGLE_CLKSEL_DIV1_gc;
	return;
}
*/

void systick_init(void) {
	TCAx_RESET(TCA0);

	TCA0.SINGLE.EVCTRL = 0;
	TCA0.SINGLE.INTCTRL = 0;
	TCA0.SINGLE.CTRLD = 0;
	TCA0.SINGLE.CTRLA = TIMER_PSC | TCA_CTRLA_FLAGS;
	TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_SINGLESLOPE_gc;
	TCA0.SINGLE.CTRLC = 0;
	write_reg16(&TCA0.SINGLE.CNT, 0);
	write_reg16(&TCA0.SINGLE.PER, TIMER_TOP);

	CLEAR_TCA_INTERRUPTS(TCA0);

	return;
}
void enable_systick(void) {
	SET_BIT(TCA0.SINGLE.INTCTRL, TCA_SINGLE_OVF_bm);
	SET_BIT(TCA0.SINGLE.CTRLA, TCA_SINGLE_ENABLE_bm);

	return;
}
void disable_systick(void) {
	CLEAR_BIT(TCA0.SINGLE.INTCTRL, TCA_SINGLE_OVF_bm);
	if (!TCAx_IS_USED(TCA0)) {
		CLEAR_BIT(TCA0.SINGLE.CTRLA, TCA_SINGLE_ENABLE_bm);
	}

	return;
}
bool systick_is_enabled(void) {
	return BIT_IS_SET(TCA0.SINGLE.CTRLA, TCA_SPLIT_ENABLE_bm) && BIT_IS_SET(TCA0.SINGLE.INTCTRL, TCA_SINGLE_OVF_bm);
}

# undef TIMER_CLOCK
# undef COUNTER_MAX
# undef TIMER_HZ
# undef TIMER_TOP
# undef TIMER_PSC
# undef TIMER_ERROR
#endif // IS_SINGLE_TCA0(SYSTICK_TIMER)

//
// USCOUNTER
//
#if IS_SINGLE_TCA0(USCOUNTER_TIMER)
void uscounter_init(void) {
	TCAx_RESET(TCA0);

	TCA0.SINGLE.EVCTRL = 0;
	TCA0.SINGLE.INTCTRL = 0;
	TCA0.SINGLE.CTRLD = 0;
	TCA0.SINGLE.CTRLA = TCA_SINGLE_USCOUNTER_CLKSEL_DIV | TCA_CTRLA_FLAGS;
	TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_SINGLESLOPE_gc;
	TCA0.SINGLE.CTRLC = 0;
	write_reg16(&TCA0.SINGLE.CNT, 0);
	write_reg16(&TCA0.SINGLE.PER, USCOUNTER_MAX);

	CLEAR_TCA_INTERRUPTS(TCA0);

	return;
}

void uscounter_start_timer(void) {
	// The WG mode used with TCA0 counts from bottom to top and generates an
	// interrupt at 0, so if set to 0 initially we get an extra uscounter_overflows
	TCA0.SINGLE.CNT = 1;
	TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm;
	SET_BIT(TCA0.SINGLE.INTCTRL, TCA_SINGLE_OVF_bm);
	SET_BIT(TCA0.SINGLE.CTRLA, TCA_SINGLE_ENABLE_bm);

	return;
}
uint16_t uscounter_stop_timer(void) {
	uint16_t cnt;

	CLEAR_BIT(TCA0.SINGLE.INTCTRL, TCA_SINGLE_OVF_bm);
	if (!TCAx_IS_USED(TCA0)) {
		CLEAR_BIT(TCA0.SINGLE.CTRLA, TCA_SINGLE_ENABLE_bm);
	}
	cnt = read_reg16(&TCA0.SINGLE.CNT);

	return cnt;
}

void uscounter_resume_timer(void) {
	SET_BIT(TCA0.SINGLE.CTRLA, TCA_SINGLE_ENABLE_bm);
	return;
}
uint16_t uscounter_pause_timer(void) {
	uint16_t cnt;

	CLEAR_BIT(TCA0.SINGLE.CTRLA, TCA_SINGLE_ENABLE_bm);
	cnt = read_reg16(&TCA0.SINGLE.CNT);

	return cnt;
}
#endif // IS_SINGLE_TCA0(USCOUNTER_TIMER)

//
// PWM
//
#if uHAL_USE_PWM
# if !IS_TCA0(SYSTICK_TIMER) && !IS_TCA0(USCOUNTER_TIMER)
#  define TIMER_CLOCK G_freq_TCACLK
#  define TIMER_HZ PWM_FREQUENCY_HZ
#  define TIMER_TOP TCA0_DUTY_CYCLE_SCALE
#  include "time_TCA_calc_PWM.h"
   DEBUG_CPP_MACRO(TIMER_PSC)

void TCA_PWM_init(TCA_t *TCAx) {
	TCAx_RESET(*TCAx);

	TCAx->SINGLE.EVCTRL = 0;
	TCAx->SINGLE.INTCTRL = 0;
	TCAx->SINGLE.CTRLD = 0;
	TCAx->SINGLE.CTRLA = TIMER_PSC | TCA_CTRLA_FLAGS;
	TCAx->SINGLE.CTRLB = TCA_SINGLE_WGMODE_SINGLESLOPE_gc;
	TCAx->SINGLE.CTRLC = 0;
	write_reg16(&TCAx->SINGLE.CNT, 0);
	write_reg16(&TCAx->SINGLE.CNT, TIMER_TOP);

	CLEAR_TCA_INTERRUPTS(*TCAx);

	return;
}
#  undef TIMER_CLOCK
#  undef TIMER_HZ
#  undef TIMER_TOP
#  undef TIMER_PSC

# else // !IS_TCA0(SYSTICK_TIMER) && !IS_TCA0(USCOUNTER_TIMER)
void TCA_PWM_init(TCA_t *TCAx) {
	UNUSED(TCAx);
	return;
}
# endif // !IS_TCA0(SYSTICK_TIMER) && !IS_TCA0(USCOUNTER_TIMER)

err_t TCA_pwm_off(TCA_t *TCAx, uint8_t wo_bm) {
	CLEAR_BIT(TCAx->SINGLE.CTRLB, wo_bm);

	// If this is the only user of the timer, disable it
	// If there's another PWM output enabled, the WO bit will be set
	// If the systick uses it, the interrupt will be enabled
	if (!TCAx_IS_USED(*TCAx)) {
		CLEAR_BIT(TCAx->SINGLE.CTRLA, TCA_SINGLE_ENABLE_bm);
	}

	return ERR_OK;
}
err_t TCA_pwm_set(TCA_t *TCAx, uint8_t wo_bm, uint16_t duty_cycle) {
	uint16_t off_at;

#if IS_TCA0(SYSTICK_TIMER) || IS_TCA0(USCOUNTER_TIMER)
	off_at = ((uint32_t )duty_cycle * (uint32_t )(TCAx->SINGLE.PER)) / PWM_DUTY_CYCLE_SCALE;
#else
	off_at = TCA0_DUTY_CYCLE_ADJUST(duty_cycle);
#endif

	switch (wo_bm) {
	case TCA_SINGLE_CMP0EN_bm:
		write_reg16(&TCAx->SINGLE.CMP0BUF, off_at);
		break;
	case TCA_SINGLE_CMP1EN_bm:
		write_reg16(&TCAx->SINGLE.CMP1BUF, off_at);
		break;
	case TCA_SINGLE_CMP2EN_bm:
		write_reg16(&TCAx->SINGLE.CMP2BUF, off_at);
		break;
	}

	return ERR_OK;
}
err_t TCA_pwm_on(TCA_t *TCAx, uint8_t wo_bm, uint16_t duty_cycle) {
	TCA_pwm_set(TCAx, wo_bm, duty_cycle);

	SET_BIT(TCAx->SINGLE.CTRLB, wo_bm);
	SET_BIT(TCAx->SINGLE.CTRLA, TCA_SINGLE_ENABLE_bm);

	return ERR_OK;
}
#endif // uHAL_USE_PWM

#endif // ! USE_SPLIT_TCA0
