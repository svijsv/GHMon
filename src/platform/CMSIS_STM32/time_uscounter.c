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
// Manage the microsecond timer
// NOTES:
//

#include "common.h"

#if uHAL_USE_USCOUNTER
#include "time_private.h"
#include "system.h"


#if USCOUNTER_TIMER == SLEEP_ALARM_TIMER
static uint16_t uscounter_timer_psc;
extern uint16_t sleep_timer_psc;
#endif


// Timers 2 through 5 have 16 bit counters on the STM32F1s and 32 bit counters
// in other lines
#if USCOUNTER_TIMER >= TIMER_2 && USCOUNTER_TIMER <= TIMER_5
# define USCOUNTER_TIM_MAX_CNT TIM2_5_MAX_CNT
#else
# define USCOUNTER_TIM_MAX_CNT TIM_MAX_CNT
#endif

#if USCOUNTER_TIM_MAX_CNT > 0xFFFFUL
# define USE_COUNTER_OVERFLOWS 0
#else
# if ! TIMERS_SHARE_INTERRUPT(USCOUNTER_TIMER, SLEEP_ALARM_TIMER)
#  define USE_COUNTER_OVERFLOWS 1
# else
// We can't count overflows when shareing a timer because we also share an
// IRQ
#  define USE_COUNTER_OVERFLOWS 0
# endif
#endif


DEBUG_CPP_MACRO(SLEEP_TIM_MS_TICKS)
DEBUG_CPP_MACRO(SLEEP_TIM_MAX_MS)


#if USE_COUNTER_OVERFLOWS
static volatile uint_t uscounter_overflows = 0;
#endif

#if USE_COUNTER_OVERFLOWS
void USCOUNTER_IRQHandler(void) {
	//CLEAR_BIT(USCOUNTER_TIMER->SR, TIM_SR_UIF);
	USCOUNTER_TIM->SR = 0;

	++uscounter_overflows;

	return;
}
#endif


void uscounter_timer_init(void) {
#if USCOUNTER_TIMER != SLEEP_ALARM_TIMER
	clock_init(USCOUNTER_CLOCKEN);

# if USE_COUNTER_OVERFLOWS
	NVIC_SetPriority(USCOUNTER_IRQn, USCOUNTER_IRQp);
# endif

	MODIFY_BITS(USCOUNTER_TIM->CR1, TIM_CR1_ARPE|TIM_CR1_CMS|TIM_CR1_DIR|TIM_CR1_OPM,
		(0b0U  << TIM_CR1_ARPE_Pos) | // 0 to disable reload register buffer
		(0b00U << TIM_CR1_CMS_Pos ) | // 0 to disable bidirectional counting
		(0b0U  << TIM_CR1_DIR_Pos ) | // 0 to use as an upcounter
# if USE_COUNTER_OVERFLOWS
		(0b0U  << TIM_CR1_OPM_Pos ) | // 1 to automatically disable on update events
# else
		(0b1U  << TIM_CR1_OPM_Pos ) | // 1 to automatically disable on update events
# endif
		0);
	USCOUNTER_TIM->PSC = calculate_TIM_prescaler(USCOUNTER_CLOCKEN, 1000000UL);

	// Set the reload value and generate an update event to load it
	USCOUNTER_TIM->ARR = USCOUNTER_TIM_MAX_CNT;
	SET_BIT(USCOUNTER_TIM->EGR, TIM_EGR_UG);
	while (BIT_IS_SET(USCOUNTER_TIM->EGR, TIM_EGR_UG)) {
		// Nothing to do here
	}

	// Clear all event flags
	USCOUNTER_TIM->SR = 0;

	clock_disable(USCOUNTER_CLOCKEN);

#else
	uscounter_timer_psc = calculate_TIM_prescaler(USCOUNTER_CLOCKEN, 1000000UL);
	//uscounter_timer_psc = calculate_TIM_prescaler(USCOUNTER_TIMER, 1000000UL);
#endif

	return;
}

err_t uscounter_on(void) {
#if ! uHAL_SKIP_OTHER_CHECKS && USCOUNTER_TIMER == SLEEP_ALARM_TIMER
	if (clock_is_enabled(USCOUNTER_CLOCKEN) && (USCOUNTER_TIM->PSC == sleep_timer_psc)) {
		return ERR_RETRY;
	}
#endif

	clock_enable(USCOUNTER_CLOCKEN);
#if USCOUNTER_TIMER == SLEEP_ALARM_TIMER
	USCOUNTER_TIM->PSC = uscounter_timer_psc;
	USCOUNTER_TIM->ARR = USCOUNTER_TIM_MAX_CNT;
#endif

	return ERR_OK;
}
err_t uscounter_off(void) {
	uscounter_stop();
#if USCOUNTER_TIMER == SLEEP_ALARM_TIMER
	USCOUNTER_TIM->PSC = sleep_timer_psc;
	//USCOUNTER_TIM->ARR = 0;
#endif
	clock_disable(USCOUNTER_CLOCKEN);

	return ERR_OK;
}

err_t uscounter_start(void) {
	// Generate an update event to reload the registers
	SET_BIT(USCOUNTER_TIM->EGR, TIM_EGR_UG);
	while (BIT_IS_SET(USCOUNTER_TIM->EGR, TIM_EGR_UG)) {
		// Nothing to do here
	}
	// Clear all event flags
	USCOUNTER_TIM->SR = 0;

	SET_BIT(USCOUNTER_TIM->CR1, TIM_CR1_CEN);

#if USE_COUNTER_OVERFLOWS
	// Enable update interrupts
	SET_BIT(USCOUNTER_TIM->DIER, TIM_DIER_UIE);
	NVIC_ClearPendingIRQ(USCOUNTER_IRQn);
	NVIC_EnableIRQ(USCOUNTER_IRQn);

	uscounter_overflows = 0;
#endif

	return ERR_OK;
}
uint_fast32_t uscounter_stop(void) {
	uint_fast32_t cnt;

	CLEAR_BIT(USCOUNTER_TIM->CR1, TIM_CR1_CEN);

#if USE_COUNTER_OVERFLOWS
	NVIC_DisableIRQ(USCOUNTER_IRQn);
	NVIC_ClearPendingIRQ(USCOUNTER_IRQn);
	CLEAR_BIT(USCOUNTER_TIM->DIER, TIM_DIER_UIE);
#endif

	USCOUNTER_TIM->SR = 0;

	cnt = USCOUNTER_TIM->CNT;
#if USE_COUNTER_OVERFLOWS
	cnt += uscounter_overflows * USCOUNTER_TIM_MAX_CNT;
#endif

	return cnt;
}
uint_fast32_t uscounter_read(void) {
	uint_fast32_t cnt;

	cnt = USCOUNTER_TIM->CNT;
#if USE_COUNTER_OVERFLOWS
	cnt += uscounter_overflows * USCOUNTER_TIM_MAX_CNT;
#endif

	return cnt;
}


#endif // uHAL_USE_USCOUNTER
