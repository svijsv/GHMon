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
// time_calib.h
// Manage clock calibration
// NOTES:
//
#include "common.h"
#include "time_RTC.h"

#ifndef uHAL_USE_TIM5_CALIBRATION
# if defined(TIM5)
#  define USE_TIM5_CALIBRATION 1
# else
#  define USE_TIM5_CALIBRATION 0
# endif
#else
# define USE_TIM5_CALIBRATION uHAL_USE_TIM5_CALIBRATION
#endif

#if USE_TIM5_CALIBRATION
#include "time_private.h"
#include "system.h"

// This is the number of captures to perform when calibrating an oscillator
// with TIM5
// With a 1MHz timer clock and a 16-bit counter the timer will overflow
// in ~65ms; on devices with 32-bit counters we can go up to 4294 seconds
// The maximum total LS cycles we can detect without overflowing CNT is:
//    X / CNT_max  ==  LS_Hz / HS_Hz
//    X  ==  (CNT_max * LS_Hz) / HS_Hz
//    X  ==  (2^16 * 32768) / 1000000  ==  ~2147 (~65ms)
//    X  ==  (2^16 * 10000) / 1000000  ==   ~655 (~65ms)
//    X  ==  (2^16 * 15000) / 1000000  ==   ~983 (~65ms)
//    X  ==  (2^16 * 20000) / 1000000  ==  ~1310 (~65ms)
//    X  ==  (2^16 * 70000) / 1000000  ==  ~4587 (~65ms)
// The time a given number of captures takes is just:
//    C / LS_Hz seconds
#define TIM5_CALIBRATION_CAPTURE_COUNT (512U)
#define TIM5_CALIBRATION_TIMEOUT_MS (500U)
#define TIM5_CALIBRATION_FREQ_HZ (1000000UL)

#define TIM_OR_TI4_RMP_GPIO (0b00U << TIM_OR_TI4_RMP_Pos)
#define TIM_OR_TI4_RMP_LSI  (0b01U << TIM_OR_TI4_RMP_Pos)
#define TIM_OR_TI4_RMP_LSE  (0b10U << TIM_OR_TI4_RMP_Pos)
#define TIM_OR_TI4_RMP_RTC  (0b11U << TIM_OR_TI4_RMP_Pos)


//
// Useful notes when working with this:
//    It can take several seconds for the LSE to stabilize after being enabled,
//    check the datasheets. And it's a temperature-dependent time period!
//
err_t tim5_oscillator_calibration(osc_calib_t mode, uint32_t *LS_cycles, uint32_t *HS_cycles) {
	uint32_t ret = ERR_OK;
	uint32_t cnt;
	const uiter_t cap_cnt = (TIM5_CALIBRATION_CAPTURE_COUNT/8U);
	utime_t timeout;
	bool clock_enabled = clock_is_enabled(RCC_PERIPH_TIM5);
	// PWM sets the timer up in a compatible way, for everything else we save
	// and restore it avoid trashing whatever's there
	TIM_TypeDef tim_state;

#if ! HAVE_STM32F1_TIMERS
	uint16_t or_reg;
	if (mode == CALIB_LSI) {
		or_reg = TIM_OR_TI4_RMP_LSI;
	} else {
		or_reg = TIM_OR_TI4_RMP_LSE;
	}
#else
	bool unmap_tim5ch4 = false;
	if (mode != CALIB_LSI) {
		return ERR_NOTSUP;
	}
#endif

	if (!clock_enabled) {
		clock_enable(RCC_PERIPH_TIM5);
	}

	if (BIT_IS_SET(TIM5->CR1, TIM_CR1_CEN)) {
		ret = ERR_RETRY;
		goto END;
	}

#if HAVE_STM32F1_TIMERS
	unmap_tim5ch4 = gpio_remap_tim5ch4();
#endif

	tim_state = *TIM5;
	// Start with a blank slate
	clock_init(RCC_PERIPH_TIM5);

	if (systick_is_enabled()) {
		timeout = SET_TIMEOUT_MS(TIM5_CALIBRATION_TIMEOUT_MS);
	} else {
		timeout = (utime_t )-1;
	}

	TIM5->PSC = QUICK_PSC(TIMER_5, TIM5_CALIBRATION_FREQ_HZ);
	TIM5->ARR = TIM2_5_MAX_CNT;
	// Generate an update event to reload the register values
	SET_BIT(TIM5->EGR, TIM_EGR_UG);
	while (BIT_IS_SET(TIM5->EGR, TIM_EGR_UG)) {
		// Nothing to do here
	}

	TIM5->CCMR2 = (
		(0b0001UL << TIM_CCMR2_IC4F_Pos  ) | // Wait for 2 consecutive events to trigger a capture
		(0b11UL   << TIM_CCMR2_IC4PSC_Pos) | // Capture every 8th event
		(0b01UL   << TIM_CCMR2_CC4S_Pos  ) | // Work with TI4
		0UL
	);
#if ! HAVE_STM32F1_TIMERS
	TIM5->OR = or_reg;
#endif
	TIM5->CCER = TIM_CCER_CC4E;
	TIM5->CR1 = TIM_CR1_CEN;
	TIM5->SR = 0;

	// Ignore the first capture because we don't know where the LS was in it's
	// cycle when we started looking
	while (!BIT_IS_SET(TIM5->SR, TIM_SR_CC4IF)) {
		if (TIMES_UP(timeout)) {
			ret = ERR_TIMEOUT;
			goto RESET_END;
		}
	}
	cnt = TIM5->CCR4;

	for (uiter_t i = 0; i < cap_cnt; ++i) {
		TIM5->SR = 0;
		//CLEAR_BIT(TIM5->SR, TIM_SR_CC4IF);
		// We don't check for a timeout here because the only condition we care
		// about is when the measured clock is unexpectedly absent and the pre-
		// measurement above will catch that
		while (!BIT_IS_SET(TIM5->SR, TIM_SR_CC4IF)) {
			// Nothing to do here
		}
	}
	TIM5->CR1 = 0;
	TIM5->CCER = 0;
	cnt = TIM5->CCR4 - cnt;

	if (LS_cycles != NULL) {
		*LS_cycles = TIM5_CALIBRATION_CAPTURE_COUNT;
	}
	if (HS_cycles != NULL) {
		*HS_cycles = cnt;
	}

RESET_END:
#if HAVE_STM32F1_TIMERS
	if (unmap_tim5ch4) {
		gpio_unremap_tim5ch4();
	}
#endif
	clock_init(RCC_PERIPH_TIM5);
	*TIM5 = tim_state;
	// Generate an update event to reload the register values
	SET_BIT(TIM5->EGR, TIM_EGR_UG);
	while (BIT_IS_SET(TIM5->EGR, TIM_EGR_UG)) {
		// Nothing to do here
	}
	TIM5->SR = 0;

END:
	if (!clock_enabled) {
		clock_disable(RCC_PERIPH_TIM5);
	}
	return ret;
}
#endif

#if USE_TIM5_CALIBRATION && uHAL_USE_INTERNAL_LS_OSC
err_t calibrate_RTC_clock(void) {
	err_t ret = ERR_OK;
	uint32_t LS_cycles, HS_cycles;
	uint32_t lsi_hz;

	if ((ret = tim5_oscillator_calibration(CALIB_LSI, &LS_cycles, &HS_cycles)) != ERR_OK) {
		return ret;
	}
	lsi_hz = (LS_cycles * TIM5_CALIBRATION_FREQ_HZ) / HS_cycles;

	set_RTC_prediv(lsi_hz);

	return ERR_OK;
}

#else
err_t calibrate_RTC_clock(void) {
	set_RTC_prediv(G_freq_RTC);

	return ERR_OK;
}
#endif
