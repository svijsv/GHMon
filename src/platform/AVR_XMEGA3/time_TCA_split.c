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
// time_TCA_split.c
// Manage the TCA peripheral
//
// NOTES:
//

#include "time_TCA.h"

#if USE_SPLIT_TCA0

#define TCA_SPLIT_HCMPxEN (TCA_SPLIT_HCMP0EN_bm | TCA_SPLIT_HCMP1EN_bm | TCA_SPLIT_HCMP2EN_bm)
#define TCA_SPLIT_LCMPxEN (TCA_SPLIT_LCMP0EN_bm | TCA_SPLIT_LCMP1EN_bm | TCA_SPLIT_LCMP2EN_bm)
#define TCA_SPLIT_CMPxEN  (TCA_SPLIT_HCMPxEN | TCA_SPLIT_LCMPxEN)
#define TCA_SPLIT_INTERRUPTS (TCA_SPLIT_CMPxEN | TCA_SPLIT_LUNF_bm | TCA_SPLIT_HUNF_bm)

// When a TCA timer is used for PWM the relevent CMPxEN bit will be set
// When used for the systick or micro-second counter, one of the interrupts
// will be set
#define TCAx_IS_USED(_TCAx_) \
	((SELECT_BITS((_TCAx_).SPLIT.CTRLB, TCA_SPLIT_CMPxEN) != 0) || \
	 (SELECT_BITS((_TCAx_).SPLIT.INTCTRL, TCA_SPLIT_INTERRUPTS) != 0))

#define CLEAR_TCA_SPLIT_INTERRUPTS(_TCAx_)  ((_TCAx_).SPLIT.INTFLAGS = TCA_SPLIT_INTERRUPTS)

//
// SYSTICK
//
#if IS_SPLIT_TCA0(SYSTICK_TIMER)
# if SYSTICK_TIMER == TIMER_TCA0_HIGH
#  define SYSTICK_UNF_bm TCA_SPLIT_HUNF_bm
#  define SYSTICK_PER  (TCA0.SPLIT.HPER)
#  define SYSTICK_NPER (TCA0.SPLIT.LPER)
# elif SYSTICK_TIMER == TIMER_TCA0_LOW
#  define SYSTICK_UNF_bm TCA_SPLIT_LUNF_bm
#  define SYSTICK_PER  (TCA0.SPLIT.LPER)
#  define SYSTICK_NPER (TCA0.SPLIT.HPER)
# else
#  error "Unhandled SYSTICK_TIMER"
# endif

# define TIMER_CLOCK G_freq_TCACLK
# define COUNTER_MAX TCA0_MAX_CNT
# define TIMER_HZ 1000U
# include "time_TCA_calc_exact.h"
  DEBUG_CPP_MACRO(TIMER_TOP)
  DEBUG_CPP_MACRO(TIMER_PSC)
  DEBUG_CPP_MACRO(TIMER_ERROR)
  //DEBUG_CPP_MSG("Real TCA0 PWM frequency is core_clock/(tca_psc*pwm_scale)")

/*
// To test a value divide the clock Hz by the prescaler multiplied by the
// TOP value plus 1, e.g. 4000000/(16*250) gives a TOP value of 249 to get
// 1ms timer from a 4MHz clock
void calc_TCA_split_1ms(uint16_t *top, uint8_t *psc) {
	assert(top != NULL);
	assert(psc != NULL);

	switch (G_freq_TCACLK) {
	case 20000000:
		// Actually 1.001ms
		*psc = TCA_SPLIT_CLKSEL_DIV256_gc;
		*top = 77;
		break;
	case 16000000:
		*psc = TCA_SPLIT_CLKSEL_DIV64_gc;
		*top = 249;
		break;
	case 10000000:
		// Actually 1.001ms
		*psc = TCA_SPLIT_CLKSEL_DIV64_gc;
		*top = 155;
		break;
	case 8000000:
		*psc = TCA_SPLIT_CLKSEL_DIV64_gc;
		*top = 124;
		break;
	case 5000000:
		// Actually 1.001ms
		*psc = TCA_SPLIT_CLKSEL_DIV64_gc;
		*top = 77;
		break;
	case 4000000:
		*psc = TCA_SPLIT_CLKSEL_DIV16_gc;
		*top = 249;
		break;
	case 2000000:
		*psc = TCA_SPLIT_CLKSEL_DIV8_gc;
		*top = 249;
		break;
	case 2500000:
		// Actually 0.9984ms
		*psc = TCA_SPLIT_CLKSEL_DIV16_gc;
		*top = 155;
		break;
	case 1000000:
		*psc = TCA_SPLIT_CLKSEL_DIV8_gc;
		*top = 124;
		break;
	default:
		calc_TCA_XHz_exact(1000, top, psc);
		break;
	}

	return;
}
*/

void systick_init(void) {
	TCAx_RESET(TCA0);

	TCA0.SPLIT.INTCTRL = 0;
	TCA0.SPLIT.CTRLD = TCA_SPLIT_SPLITM_bm;
	TCA0.SPLIT.CTRLA = TIMER_PSC | TCA_CTRLA_FLAGS;
	TCA0.SPLIT.CTRLB = 0;
	TCA0.SPLIT.CTRLC = 0;
	TCA0.SPLIT.HCNT = 0;
	TCA0.SPLIT.LCNT = 0;
	SYSTICK_PER = TIMER_TOP;
	SYSTICK_NPER = TCA0_MAX_CNT;
	//SYSTICK_NPER = TCA0_DUTY_CYCLE_SCALE;
	CLEAR_TCA_SPLIT_INTERRUPTS(TCA0);

	return;
}
void enable_systick(void) {
	SET_BIT(TCA0.SPLIT.INTCTRL, SYSTICK_UNF_bm);
	SET_BIT(TCA0.SPLIT.CTRLA, TCA_SPLIT_ENABLE_bm);

	return;
}
void disable_systick(void) {
	CLEAR_BIT(TCA0.SPLIT.INTCTRL, SYSTICK_UNF_bm);
	if (!TCAx_IS_USED(TCA0)) {
		CLEAR_BIT(TCA0.SPLIT.CTRLA, TCA_SPLIT_ENABLE_bm);
	}

	return;
}
bool systick_is_enabled(void) {
	return BIT_IS_SET(TCA0.SPLIT.CTRLA, TCA_SPLIT_ENABLE_bm) && BIT_IS_SET(TCA0.SPLIT.INTCTRL, SYSTICK_UNF_bm);
}

# undef TIMER_CLOCK
# undef COUNTER_MAX
# undef TIMER_HZ
# undef TIMER_TOP
# undef TIMER_PSC
# undef TIMER_ERROR
#endif // IS_SPLIT_TCA0(SYSTICK_TIMER)

//
// USCOUNTER
//
#if IS_SPLIT_TCA0(USCOUNTER_TIMER)
# if USCOUNTER_TIMER == TIMER_TCA0_HIGH
#  define USCOUNTER_UNF_bm TCA_SPLIT_HUNF_bm
#  define USCOUNTER_CNT (TCA0.SPLIT.HCNT)
#  define USCOUNTER_PER  (TCA0.SPLIT.HPER)
#  define USCOUNTER_NPER (TCA0.SPLIT.LPER)
# elif USCOUNTER_TIMER == TIMER_TCA0_LOW
#  define USCOUNTER_UNF_bm TCA_SPLIT_LUNF_bm
#  define USCOUNTER_CNT (TCA0.SPLIT.LCNT)
#  define USCOUNTER_PER  (TCA0.SPLIT.LPER)
#  define USCOUNTER_NPER (TCA0.SPLIT.HPER)
# else
#  error "Unhandled USCOUNTER_TIMER"
# endif

void uscounter_init(void) {
	TCAx_RESET(TCA0);

	TCA0.SPLIT.INTCTRL = 0;
	TCA0.SPLIT.CTRLD = TCA_SPLIT_SPLITM_bm;
	TCA0.SPLIT.CTRLA = TCA_SPLIT_USCOUNTER_CLKSEL_DIV | TCA_CTRLA_FLAGS;
	TCA0.SPLIT.CTRLB = 0;
	TCA0.SPLIT.CTRLC = 0;
	TCA0.SPLIT.HCNT = 0;
	TCA0.SPLIT.LCNT = 0;
	USCOUNTER_PER = USCOUNTER_MAX;
	USCOUNTER_NPER = USCOUNTER_MAX;
	CLEAR_TCA_SPLIT_INTERRUPTS(TCA0);

	return;
}

void uscounter_start_timer(void) {
	// TCA split mode only supports down-count
	USCOUNTER_CNT = USCOUNTER_MAX;
	TCA0.SPLIT.INTFLAGS = USCOUNTER_UNF_bm;
	SET_BIT(TCA0.SPLIT.INTCTRL, USCOUNTER_UNF_bm);
	SET_BIT(TCA0.SPLIT.CTRLA, TCA_SPLIT_ENABLE_bm);

	return;
}
uint16_t uscounter_stop_timer(void) {
	uint16_t cnt;

	CLEAR_BIT(TCA0.SPLIT.INTCTRL, USCOUNTER_UNF_bm);
	if (!TCAx_IS_USED(TCA0)) {
		CLEAR_BIT(TCA0.SPLIT.CTRLA, TCA_SPLIT_ENABLE_bm);
	}
	cnt = USCOUNTER_CNT;

	return cnt;
}

void uscounter_resume_timer(void) {
	SET_BIT(TCA0.SPLIT.CTRLA, TCA_SPLIT_ENABLE_bm);
	return;
}
uint16_t uscounter_pause_timer(void) {
	uint16_t cnt;

	// FIXME: Find a way to not cause problems when sharing with PWM...
	CLEAR_BIT(TCA0.SPLIT.CTRLA, TCA_SPLIT_ENABLE_bm);
	cnt = USCOUNTER_CNT;

	return cnt;
}

#endif // IS_SPLIT_TCA0(USCOUNTER_TIMER)

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
	//calc_TCA_PWM_psc(PWM_FREQUENCY_HZ, &top, &psc);

	TCAx->SPLIT.INTCTRL = 0;
	TCAx->SPLIT.CTRLD = TCA_SPLIT_SPLITM_bm;
	//TCAx->SPLIT.CTRLA = psc | TCA_CTRLA_FLAGS;
	TCAx->SPLIT.CTRLA = TIMER_PSC | TCA_CTRLA_FLAGS;
	TCAx->SPLIT.CTRLB = 0;
	TCAx->SPLIT.CTRLC = 0;
	TCAx->SPLIT.HCNT = 0;
	TCAx->SPLIT.LCNT = 0;
	//TCAx->SPLIT.HPER = top;
	//TCAx->SPLIT.LPER = top;
	TCAx->SPLIT.HPER = TIMER_TOP;
	TCAx->SPLIT.LPER = TIMER_TOP;

	CLEAR_TCA_SPLIT_INTERRUPTS(*TCAx);

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
	CLEAR_BIT(TCAx->SPLIT.CTRLB, wo_bm);

	// If this is the only user of the timer, disable it
	// If there's another PWM output enabled, the WO bit will be set
	// If the systick uses it, the interrupt will be enabled
	if (!TCAx_IS_USED(*TCAx)) {
		CLEAR_BIT(TCAx->SPLIT.CTRLA, TCA_SPLIT_ENABLE_bm);
	}

	return ERR_OK;
}
err_t TCA_pwm_set(TCA_t *TCAx, uint8_t wo_bm, uint16_t duty_cycle) {
	uint8_t off_at;

	// PWM output is still allowed from the timer used for the system tick or
	// micro-second counter so we can't just do the normal duty cycle adjustment
#if IS_TCA0(SYSTICK_TIMER) || IS_TCA0(USCOUNTER_TIMER)
	uint8_t top = 0;
	// The benefit of using two switch blocks is that the 32-bit multiplication
	// and division can be done once in total instead of once per possible output
	// which saves program space
	switch (wo_bm) {
	case TCA_SPLIT_LCMP0EN_bm:
	case TCA_SPLIT_LCMP1EN_bm:
	case TCA_SPLIT_LCMP2EN_bm:
		top = TCAx->SPLIT.LPER;
		break;
	case TCA_SPLIT_HCMP0EN_bm:
	case TCA_SPLIT_HCMP1EN_bm:
	case TCA_SPLIT_HCMP2EN_bm:
		top = TCAx->SPLIT.HPER;
		break;
	}
	off_at = ((uint32_t )duty_cycle * top) / PWM_DUTY_CYCLE_SCALE;
#else
	off_at = TCA0_DUTY_CYCLE_ADJUST(duty_cycle);
#endif

	switch (wo_bm) {
	case TCA_SPLIT_LCMP0EN_bm:
		TCAx->SPLIT.LCMP0 = off_at;
		break;
	case TCA_SPLIT_LCMP1EN_bm:
		TCAx->SPLIT.LCMP1 = off_at;
		break;
	case TCA_SPLIT_LCMP2EN_bm:
		TCAx->SPLIT.LCMP2 = off_at;
		break;
	case TCA_SPLIT_HCMP0EN_bm:
		TCAx->SPLIT.HCMP0 = off_at;
		break;
	case TCA_SPLIT_HCMP1EN_bm:
		TCAx->SPLIT.HCMP1 = off_at;
		break;
	case TCA_SPLIT_HCMP2EN_bm:
		TCAx->SPLIT.HCMP2 = off_at;
		break;
	}

	return ERR_OK;
}
err_t TCA_pwm_on(TCA_t *TCAx, uint8_t wo_bm, uint16_t duty_cycle) {
	TCA_pwm_set(TCAx, wo_bm, duty_cycle);

	SET_BIT(TCAx->SPLIT.CTRLB, wo_bm);
	SET_BIT(TCAx->SPLIT.CTRLA, TCA_SPLIT_ENABLE_bm);

	return ERR_OK;
}
#endif // uHAL_USE_PWM


#endif // USE_SPLIT_TCA
