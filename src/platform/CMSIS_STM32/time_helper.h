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
#ifndef _uHAL_PLATFORM_CMSIS_TIME_HELPER_H
#define _uHAL_PLATFORM_CMSIS_TIME_HELPER_H


HELPER_STORAGE TIM_TypeDef* get_tim_from_id(uint_fast8_t tim_id) {
	switch (tim_id) {
#ifdef TIM1
	case TIMER_1:
		return TIM1;
#endif
#ifdef TIM2
	case TIMER_2:
		return TIM2;
#endif
#ifdef TIM3
	case TIMER_3:
		return TIM3;
#endif
#ifdef TIM4
	case TIMER_4:
		return TIM4;
#endif
#ifdef TIM5
	case TIMER_5:
		return TIM5;
#endif
#ifdef TIM6
	case TIMER_6:
		return TIM6;
#endif
#ifdef TIM7
	case TIMER_7:
		return TIM7;
#endif
#ifdef TIM8
	case TIMER_8:
		return TIM8;
#endif
#ifdef TIM9
	case TIMER_9:
		return TIM9;
#endif
#ifdef TIM10
	case TIMER_10:
		return TIM10;
#endif
#ifdef TIM11
	case TIMER_11:
		return TIM11;
#endif
#ifdef TIM12
	case TIMER_12:
		return TIM12;
#endif
#ifdef TIM13
	case TIMER_13:
		return TIM13;
#endif
#ifdef TIM14
	case TIMER_14:
		return TIM14;
#endif
	}

	return NULL;
}
HELPER_STORAGE rcc_periph_t get_rcc_from_id(uint_fast8_t tim_id) {
	switch (tim_id) {
#ifdef TIM1
	case TIMER_1:
		return RCC_PERIPH_TIM1;
#endif
#ifdef TIM2
	case TIMER_2:
		return RCC_PERIPH_TIM2;
#endif
#ifdef TIM3
	case TIMER_3:
		return RCC_PERIPH_TIM3;
#endif
#ifdef TIM4
	case TIMER_4:
		return RCC_PERIPH_TIM4;
#endif
#ifdef TIM5
	case TIMER_5:
		return RCC_PERIPH_TIM5;
#endif
#ifdef TIM6
	case TIMER_6:
		return RCC_PERIPH_TIM6;
#endif
#ifdef TIM7
	case TIMER_7:
		return RCC_PERIPH_TIM7;
#endif
#ifdef TIM8
	case TIMER_8:
		return RCC_PERIPH_TIM8;
#endif
#ifdef TIM9
	case TIMER_9:
		return RCC_PERIPH_TIM9;
#endif
#ifdef TIM10
	case TIMER_10:
		return RCC_PERIPH_TIM10;
#endif
#ifdef TIM11
	case TIMER_11:
		return RCC_PERIPH_TIM11;
#endif
#ifdef TIM12
	case TIMER_12:
		return RCC_PERIPH_TIM12;
#endif
#ifdef TIM13
	case TIMER_13:
		return RCC_PERIPH_TIM13;
#endif
#ifdef TIM14
	case TIMER_14:
		return RCC_PERIPH_TIM14;
#endif
	}

	return 0;
}
#if ! HAVE_STM32F1_GPIO
HELPER_STORAGE uint_fast8_t get_af_from_id(uint_fast8_t tim_id) {
	switch (tim_id) {
#ifdef TIM1
	case TIMER_1:
		return GPIOAF_TIM1;
#endif
#ifdef TIM2
	case TIMER_2:
		return GPIOAF_TIM2;
#endif
#ifdef TIM3
	case TIMER_3:
		return GPIOAF_TIM3;
#endif
#ifdef TIM4
	case TIMER_4:
		return GPIOAF_TIM4;
#endif
#ifdef TIM5
	case TIMER_5:
		return GPIOAF_TIM5;
#endif
#ifdef TIM6
	case TIMER_6:
		return GPIOAF_TIM6;
#endif
#ifdef TIM7
	case TIMER_7:
		return GPIOAF_TIM7;
#endif
#ifdef TIM8
	case TIMER_8:
		return GPIOAF_TIM8;
#endif
#ifdef TIM9
	case TIMER_9:
		return GPIOAF_TIM9;
#endif
#ifdef TIM10
	case TIMER_10:
		return GPIOAF_TIM10;
#endif
#ifdef TIM11
	case TIMER_11:
		return GPIOAF_TIM11;
#endif
#ifdef TIM12
	case TIMER_12:
		return GPIOAF_TIM12;
#endif
#ifdef TIM13
	case TIMER_13:
		return GPIOAF_TIM13;
#endif
#ifdef TIM14
	case TIMER_14:
		return GPIOAF_TIM14;
#endif
	}

	return 0;
}
#else // ! HAVE_STM32F1_GPIO
HELPER_STORAGE uint_fast8_t get_af_from_id(uint_fast8_t tim_id) {
	UNUSED(tim_id);
	return 0;
}
#endif


HELPER_STORAGE bool is_apb1_tim(uint_fast8_t tim_id) {
	// Timers 2-7 and 12-14 are on APB1
	// Timers 1 and 8-11 are on APB2
	// Not all timers are present on all hardware
	switch (tim_id) {
	case TIMER_1:
	case TIMER_8:
	case TIMER_9:
	case TIMER_10:
	case TIMER_11:
		return false;
	}
	return true;
}
HELPER_STORAGE bool is_pwm_tim(uint_fast8_t tim_id) {
	switch (tim_id) {
#if USE_TIMER1_PWM
	case TIMER_1:
#endif
#if USE_TIMER2_PWM
	case TIMER_2:
#endif
#if USE_TIMER3_PWM
	case TIMER_3:
#endif
#if USE_TIMER4_PWM
	case TIMER_4:
#endif
#if USE_TIMER5_PWM
	case TIMER_5:
#endif
#if USE_TIMER8_PWM
	case TIMER_8:
#endif
#if USE_TIMER9_PWM
	case TIMER_9:
#endif
#if USE_TIMER10_PWM
	case TIMER_10:
#endif
#if USE_TIMER11_PWM
	case TIMER_11:
#endif
#if USE_TIMER12_PWM
	case TIMER_12:
#endif
#if USE_TIMER13_PWM
	case TIMER_13:
#endif
#if USE_TIMER14_PWM
	case TIMER_14:
#endif
		return true;
	}
	return false;
}

// hz is the target rate of counter increase, e.g. 1000 to go up 1 each
// millisecond (or 1000 a second)
// Clocks:
// Timers 2-7 and 12-14
//   PCLK1*1 if PCLK1 prescaler is 1
//   PCLK1*2 otherwise
// Timers 1 and 8-11
//   PCLK2*1 if PCLK2 prescaler is 1
//   PCLK2*2 otherwise
HELPER_STORAGE uint16_t calculate_TIM_prescaler(rcc_periph_t tim_bus, uint32_t hz) {
	uint32_t psc;

	uHAL_assert(hz != 0);
	if (hz == 0) {
		hz = 1;
	}

	switch (SELECT_BITS(tim_bus, RCC_BUS_MASK)) {
	case RCC_BUS_APB1:
		psc = TIM_APB1_MAX_HZ;
		break;
	case RCC_BUS_APB2:
		psc = TIM_APB2_MAX_HZ;
		break;
	default:
		// Shouldn't reach this point
		uHAL_assert(false);
		return 0;
	}
	psc /= hz;
	uHAL_assert(psc > 0);
	uHAL_assert(psc <= TIM_MAX_PSC+1UL);
	if (psc > 0) {
		--psc;
	}
	if (psc > TIM_MAX_PSC) {
		psc = TIM_MAX_PSC;
	}

	return psc;
}
/*
HELPER_STORAGE uint16_t calculate_TIM_prescaler(uint_fast8_t timid, uint32_t hz) {
	uint32_t psc;

	uHAL_assert(hz != 0);

	//psc = (is_apb1_tim(timid) ? TIM_APB1_MAX_HZ/hz :  TIM_APB2_MAX_HZ/hz);
	psc = (is_apb1_tim(timid) ? TIM_APB1_MAX_HZ :  TIM_APB2_MAX_HZ);
	psc /= hz;

	uHAL_assert(psc > 0);
	uHAL_assert(psc <= TIM_MAX_PSC+1UL);
	if (psc > 0) {
		--psc;
	}
	if (psc > TIM_MAX_PSC) {
		psc = TIM_MAX_PSC;
	}

	return psc;
}
*/


#endif // _uHAL_PLATFORM_CMSIS_TIME_HELPER_H
