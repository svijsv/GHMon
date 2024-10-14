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
// time_*_.c
// Manage the sleep timer
// NOTES:
//

#include "common.h"

#if uHAL_USE_HIBERNATE

#include "time_private.h"
#include "system.h"


#if USCOUNTER_TIMER == SLEEP_ALARM_TIMER
uint16_t sleep_timer_psc;
#endif


// Set the number of ticks per millisecond for the sleep alarm timer
// At PCLKx speeds >~65MHz with a PCLKx prescaler of 1 or PCLKx speeds >~32MHz
// with any other PCLKx prescaler, the 16 bit timer prescaler would overflow
// if the ratio were kept at 1
// Increasing this decreases the maximum duration proportionately
// Must be at least 1
#if (SELECT_BITS(SLEEP_ALARM_CLOCKEN, RCC_BUS_MASK) == RCC_BUS_APB1)
# define SLEEP_ALARM_BUSFREQ G_freq_PCLK1
#elif (SELECT_BITS(SLEEP_ALARM_CLOCKEN, RCC_BUS_MASK) == RCC_BUS_APB2)
# define SLEEP_ALARM_BUSFREQ G_freq_PCLK2
#else
# error "Unable to determine sleep alarm bus frequency"
#endif
#if SLEEP_ALARM_BUSFREQ == G_freq_HCLK
# define SLEEP_TIM_MS_TICKS ((SLEEP_ALARM_BUSFREQ / (TIM_MAX_PSC * 1000UL)) + 1UL)
#else
# define SLEEP_TIM_MS_TICKS ((SLEEP_ALARM_BUSFREQ / (TIM_MAX_PSC * 1000UL)) + 1UL)
#endif
#if SLEEP_ALARM_TIMER >= TIMER_2 && SLEEP_ALARM_TIMER <= TIMER_5
# define SLEEP_TIM_MAX_CNT TIM2_5_MAX_CNT
#else
# define SLEEP_TIM_MAX_CNT TIM_MAX_CNT
#endif
#define SLEEP_TIM_MAX_MS (SLEEP_TIM_MAX_CNT / SLEEP_TIM_MS_TICKS)

DEBUG_CPP_MACRO(SLEEP_TIM_MS_TICKS)
DEBUG_CPP_MACRO(SLEEP_TIM_MAX_MS)


void SLEEP_ALARM_IRQHandler(void) {
	NVIC_DisableIRQ(SLEEP_ALARM_IRQn);
	NVIC_ClearPendingIRQ(SLEEP_ALARM_IRQn);

	//CLEAR_BIT(SLEEP_ALARM_TIMER->SR, TIM_SR_UIF);
	SLEEP_ALARM_TIM->SR = 0;
	// Configured to disable itself
	//CLEAR_BIT(SLEEP_ALARM_TIM->CR1, TIM_CR1_CEN);

	return;
}

void sleep_alarm_timer_init(void) {
	uint16_t psc;

	clock_init(SLEEP_ALARM_CLOCKEN);

	MODIFY_BITS(SLEEP_ALARM_TIM->CR1, TIM_CR1_ARPE|TIM_CR1_CMS|TIM_CR1_DIR|TIM_CR1_OPM,
		(0b0U  << TIM_CR1_ARPE_Pos) | // 0 to disable reload register buffer
		(0b00U << TIM_CR1_CMS_Pos ) | // 0 to disable bidirectional counting
		(0b0U  << TIM_CR1_DIR_Pos ) | // 0 to use as an upcounter
		(0b1U  << TIM_CR1_OPM_Pos ) | // 1 to automatically disable on update events
		0);
	psc = calculate_TIM_prescaler(SLEEP_ALARM_CLOCKEN, 1000UL * SLEEP_TIM_MS_TICKS);
	//psc = calculate_TIM_prescaler(SLEEP_ALARM_TIMER, 1000UL * SLEEP_TIM_MS_TICKS);
#if SLEEP_ALARM_TIMER == USCOUNTER_TIMER
	sleep_timer_psc = psc;
#endif
	SLEEP_ALARM_TIM->PSC = psc;
	NVIC_SetPriority(SLEEP_ALARM_IRQn, SLEEP_ALARM_IRQp);

	clock_disable(SLEEP_ALARM_CLOCKEN);

	return;
}

uint32_t set_sleep_alarm(uint32_t ms) {
	assert(ms != 0);
	if (ms > SLEEP_TIM_MAX_MS) {
		ms = SLEEP_TIM_MAX_MS;
	}
	clock_enable(SLEEP_ALARM_CLOCKEN);

#if ! uHAL_SKIP_OTHER_CHECKS
	CLEAR_BIT(SLEEP_ALARM_TIM->CR1, TIM_CR1_CEN);

	// The prescaler is reset when the uscounter is stopped, but we can
	// guard against forgetfulness a bit here
# if USCOUNTER_TIMER == SLEEP_ALARM_TIMER
	SLEEP_ALARM_TIM->PSC = sleep_timer_psc;
# endif
#endif

	// Set the reload value and generate an update event to load it
	SLEEP_ALARM_TIM->ARR = ms * SLEEP_TIM_MS_TICKS;
	SET_BIT(SLEEP_ALARM_TIM->EGR, TIM_EGR_UG);
	// Without this check, the interrupt may be triggered immediately and
	// then disabled, preventing the timer from waking us from sleep
	// This has been observed with an STM32F103C8 when the timer is on APB1
	// or APB2 and the bus speed is the same as AHB
	while (BIT_IS_SET(SLEEP_ALARM_TIM->EGR, TIM_EGR_UG)) {
		// Nothing to do here
	}

	// Clear all event flags
	SLEEP_ALARM_TIM->SR = 0;
	// Enable update interrupts
	SET_BIT(SLEEP_ALARM_TIM->DIER, TIM_DIER_UIE);
	NVIC_ClearPendingIRQ(SLEEP_ALARM_IRQn);
	NVIC_EnableIRQ(SLEEP_ALARM_IRQn);
	// Enable the timer
	SET_BIT(SLEEP_ALARM_TIM->CR1, TIM_CR1_CEN);

	return ms;
}
void stop_sleep_alarm(void) {
	NVIC_DisableIRQ(SLEEP_ALARM_IRQn);
	NVIC_ClearPendingIRQ(SLEEP_ALARM_IRQn);

	// Configured to disable itself, but this may be called before the alarm
	// goes off
	CLEAR_BIT(SLEEP_ALARM_TIM->CR1, TIM_CR1_CEN);
	// Disable update interrupts
	CLEAR_BIT(SLEEP_ALARM_TIM->DIER, TIM_DIER_UIE);
	// Clear all event flags
	SLEEP_ALARM_TIM->SR = 0;

	clock_disable(SLEEP_ALARM_CLOCKEN);

	return;
}

bool sleep_alarm_is_set() {
#if SLEEP_ALARM_TIMER != USCOUNTER_TIMER
	return (clock_is_enabled(SLEEP_ALARM_CLOCKEN) && BIT_IS_SET(SLEEP_ALARM_TIM->CR1, TIM_CR1_CEN));
#else
	return (clock_is_enabled(SLEEP_ALARM_CLOCKEN) && BIT_IS_SET(SLEEP_ALARM_TIM->CR1, TIM_CR1_CEN) && (SLEEP_ALARM_TIM->PSC == sleep_timer_psc));
#endif
}


#endif // uHAL_USE_HIBERNATE
