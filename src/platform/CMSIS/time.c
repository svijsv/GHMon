// SPDX-License-Identifier: GPL-3.0-only
/***********************************************************************
*                                                                      *
*                                                                      *
* Copyright 2021 svijsv                                                *
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
// NOTES:
//

#ifdef __cplusplus
 extern "C" {
#endif

//#define DEBUG 1
/*
* Includes
*/
#include "time.h"
#include "system.h"

/*
* Static values
*/
// Divide the number of cycles per ms by this in a dumb delay to account for
// overhead
#define DUMB_DELAY_DIV 7


/*
* Types
*/


/*
* Variables
*/
// System ticks, milliseconds
volatile utime_t G_sys_uticks;

/*
* Local function prototypes
*/
static uint16_t calculate_TIM2_5_prescaler(uint16_t hz);
static void systick_init(void);
static void RTC_init(void);
static void timers_init(void);


/*
* Interrupt handlers
*/
OPTIMIZE_FUNCTION \
void SysTick_Handler(void) {
	++G_sys_uticks;
	return;
}
/*
void RTC_IRQHandler(void) {
	// This handler is called by both the RTC alarm and the RTC second
	// interrupts when RTC_CRH_ALRIE is set
	if (LIKELY(BIT_IS_SET(RTC->CRL, RTC_CRL_SECF))) {
		++sys_sticks;
		unSET_BIT(RTC->CRL, RTC_CRL_SECF);
	}

	return;
}
*/
OPTIMIZE_FUNCTION \
void RTC_Alarm_IRQHandler(void) {
	NVIC_DisableIRQ(RTC_Alarm_IRQn);
	NVIC_ClearPendingIRQ(RTC_Alarm_IRQn);

	CLEAR_BIT(RTC->CRL,
		RTC_CRL_ALRF  | // Clear the alarm interrupt flag
		0);

	// Clear the EXTI line interrupt flag
	// This is set to 1 to clear
	SET_BIT(EXTI->PR, RTC_ALARM_EXTI_LINE);
	// Disable the alarm EXTI interrupt
	CLEAR_BIT(EXTI->IMR, RTC_ALARM_EXTI_LINE);

	return;
}
OPTIMIZE_FUNCTION \
void SleepAlarm_IRQHandler(void) {
	NVIC_DisableIRQ(SLEEP_ALARM_IRQn);
	NVIC_ClearPendingIRQ(SLEEP_ALARM_IRQn);

	// CLEAR_BIT(SLEEP_ALARM_TIM->SR, TIM_SR_UIF);
	SLEEP_ALARM_TIM->SR = 0;
	// Configured to disable itself
	// CLEAR_BIT(SLEEP_ALARM_TIM->CR1, TIM_CR1_CEN);
	clock_disable(&RCC->APB1ENR, SLEEP_ALARM_CLOCKEN);

	return;
}


/*
* Functions
*/
void time_init(void) {
	systick_init();
	RTC_init();
	timers_init();

	return;
}
//
// Set up systick timer
static void systick_init(void) {
	uint32_t period;

	G_sys_uticks = 0;
	// G_sys_sticks = 0;

	/*
	// SysTick_Config() defined in CMSIS/ARM/core_cm3.h
	if (SysTick_Config(G_freq_HCLK / 1000U) != 0) {
		ERROR_STATE("SysTick config");
	}
	*/

	// Re-implement SysTick_Config() from CMSIS/ARM/core_cm3.h so we can use
	// HCLK/8 as our clock source (but why?...)
	period = (G_freq_HCLK/8) / 1000;
	assert(period > 0);
	assert(SysTick_LOAD_RELOAD_Msk >= (period-1));

	SysTick->LOAD = (uint32_t )(period - 1);
	NVIC_SetPriority(SysTick_IRQn, SYSTICK_IRQp);
	SysTick->VAL = 0;
	MODIFY_BITS(SysTick->CTRL, SysTick_CTRL_CLKSOURCE_Msk|SysTick_CTRL_TICKINT_Msk|SysTick_CTRL_ENABLE_Msk,
		(0b0 << SysTick_CTRL_CLKSOURCE_Pos) | // Keep 0 for HCLCK/8; set to 1 for HCLK
		(0b1 << SysTick_CTRL_TICKINT_Pos  ) | // Enable the interrupt
		(0b1 << SysTick_CTRL_ENABLE_Pos   ) | // Enable the counter
		0);

	return;
}
//
// Set up RTC
// TODO: Load date from backup registers? AN2821 may shed light on that.
// TODO: Reset BDRST to allow changing RTC settings if they differ from current
static void RTC_init(void) {
	uint32_t period;
	uint32_t clock;

	// The RTC clock is configured in the backup domain so enable the power
	// interface clock and the backup domain interface clock
	clock_enable(&RCC->APB1ENR, RCC_APB1ENR_PWREN|RCC_APB1ENR_BKPEN);

	// Try for LSE, HSE/128, or LSI clock sources in that order
	// Only LSE will work as a wakeup from deep sleep.
	// Period is set equal to the RTC clock frequency-1 to give 1s ticks
	if (BIT_IS_SET(RCC->BDCR, RCC_BDCR_LSERDY)) {
		clock = RCC_BDCR_RTCSEL_LSE;
		period = G_freq_LSE-1;
	} else  if (BIT_IS_SET(RCC->CR, RCC_CR_HSERDY)) {
		clock = RCC_BDCR_RTCSEL_HSE;
		period = (G_freq_HSE / 128)-1;
	} else {
		SET_BIT(RCC->CSR, RCC_CSR_LSION);
		while (!BIT_IS_SET(RCC->CSR, RCC_CSR_LSIRDY)) {
			// Nothing to do here
			// TODO: Timeout?
		}

		clock = RCC_BDCR_RTCSEL_LSI;
		period = G_freq_LSI-1;
	}

	BD_write_enable();
	MODIFY_BITS(RCC->BDCR, RCC_BDCR_RTCSEL|RCC_BDCR_RTCEN,
		clock          |
		RCC_BDCR_RTCEN | // Enable the RTC
		0);
	BD_write_disable();

	// Wait until the RTC registries are synchronized with the RTC core
	CLEAR_BIT(RTC->CRL, RTC_CRL_RSF);
	while (!BIT_IS_SET(RTC->CRL, RTC_CRL_RSF)) {
		// Nothing to do here
		// TODO: Timeout?
	}

	RTC_cfg_enable(1000);

	/*
	// There's something in the reference manual about synchronizing the alarm
	// and second counters but I didn't look into it because I'm only using the
	// alarm right now.
	MODIFY_BITS(RTC->CRH, RTC_CRH_SECIE|RTC_CRH_ALRIE,
		0b1 << RTC_CRH_SECIE_Pos | // Enable the second interrupt
		0b1 << RTC_CRH_ALRIE_Pos | // Enable the alarm interrupt
		0);
	*/

	// Set the prescaler
	WRITE_SPLITREG(period, RTC->PRLH, RTC->PRLL);

	RTC_cfg_disable(1000);

	NVIC_SetPriority(RTC_Alarm_IRQn, RTC_ALARM_IRQp);
	// Only need this for a seconds counter, which we aren't doing
	// NVIC_SetPriority(RTC_IRQn, SECONDS_IRQp);
	// NVIC_EnableIRQ(RTC_IRQn);

	return;
}
//
// Configure the sleep() timer
static void timers_init(void) {
	// Timers 2-7 and 12-14 are on APB1
	// Timers 1 and 8-11 are on APB2
	clock_init(&RCC->APB1ENR, &RCC->APB1RSTR, SLEEP_ALARM_CLOCKEN);

	MODIFY_BITS(SLEEP_ALARM_TIM->CR1, TIM_CR1_ARPE|TIM_CR1_CMS|TIM_CR1_DIR|TIM_CR1_OPM,
		(0b1  << TIM_CR1_ARPE_Pos) | // 0 to disable reload register buffer
		(0b00 << TIM_CR1_CMS_Pos ) | // 0 to disable bidirectional counting
		(0b0 << TIM_CR1_DIR_Pos  ) | // 0 to use as an upcounter
		(0b1 << TIM_CR1_OPM_Pos  ) | // 1 to automatically disable on update events
		0);

	SLEEP_ALARM_TIM->PSC = calculate_TIM2_5_prescaler(1000*TIM_MS_PERIOD);
	NVIC_SetPriority(SLEEP_ALARM_IRQn, SLEEP_ALARM_IRQp);

	return;
}

utime_t get_RTC_seconds(void) {
	uint32_t rtcs;

	READ_SPLITREG(rtcs, RTC->CNTH, RTC->CNTL);

	return rtcs;
}

void delay(utime_t ms) {
	utime_t timer;

#if DEBUG
	uint32_t systick_mask = SysTick_CTRL_TICKINT_Msk|SysTick_CTRL_ENABLE_Msk;
	if (SELECT_BITS(SysTick->CTRL, systick_mask) != systick_mask) {
		uart_on();
		LOGGER("Someone is using delay() without systick...");
		dumb_delay(ms);
		return;
	}
#endif // DEBUG

	timer = SET_TIMEOUT(ms);
	while (!TIMES_UP(timer)) {
		// Nothing to do here
	}

	return;
}
// https:// stackoverflow.com/questions/7083482/how-to-prevent-gcc-from-optimizing-out-a-busy-wait-loop
void dumb_delay(utime_t ms) {
	uint32_t cycles;

	cycles = ms * (G_freq_HCLK/(1000*DUMB_DELAY_DIV));

	for (uint32_t i = 0; i < cycles; ++i) {
		// Count some clock cycles
		__asm__ volatile("" : "+g" (i) : :);
	}

	return;
}
void dumber_delay(uint32_t cycles) {
	for (uint32_t i = 0; i < cycles; ++i) {
		// Count some clock cycles
		__asm__ volatile("" : "+g" (i) : :);
	}

	return;
}

err_t set_time(uint8_t hour, uint8_t minute, uint8_t second) {
	err_t res;
	uint32_t now;

	if ((hour > 24) || (minute > 59) || (second > 59)) {
		return EUSAGE;
	}

	// Conserve the date part of the RTC
	// Don't call get_RTC_seconds() from the macro SNAP_TO_FACTOR() or it will
	// be called twice.
	now = get_RTC_seconds();
	now = SNAP_TO_FACTOR(now, DAYS) + time_to_seconds(hour, minute, second);

	if ((res = RTC_cfg_enable(1000)) == EOK) {
		WRITE_SPLITREG(now, RTC->CNTH, RTC->CNTL);
		res = RTC_cfg_disable(1000);
	}

	return res;
}
err_t set_date(uint8_t year, uint8_t month, uint8_t day) {
	err_t res;
	uint32_t now;

	if ((year > (0xFFFFFFFF/YEARS)) || (!IS_BETWEEN(month, 1, 12)) || (!IS_BETWEEN(day, 1, 31))) {
		return EUSAGE;
	}

	// Conserve the time part of the RTC
	now = (get_RTC_seconds() % DAYS) + date_to_seconds(year, month, day);

	if ((res = RTC_cfg_enable(1000)) == EOK) {
		WRITE_SPLITREG(now, RTC->CNTH, RTC->CNTL);
		res = RTC_cfg_disable(1000);
	}

	return res;
}

void set_RTC_alarm(utime_t time) {
	assert(time > 0);
	time = get_RTC_seconds() + time;

	RTC_cfg_enable(100);
	WRITE_SPLITREG(time, RTC->ALRH, RTC->ALRL);

	CLEAR_BIT(RTC->CRL, RTC_CRL_ALRF); // Clear the alarm interrupt flag
	RTC_cfg_disable(100);

	// Clear the EXTI line interrupt flag
	// This is set to 1 to clear
	SET_BIT(EXTI->PR, RTC_ALARM_EXTI_LINE);
	// Enable the alarm EXTI rising-edge trigger
	// This is mandatory to wake from stop mode
	SET_BIT(EXTI->RTSR, RTC_ALARM_EXTI_LINE);
	// Enable the alarm EXTI interrupt
	SET_BIT(EXTI->IMR, RTC_ALARM_EXTI_LINE);

	NVIC_ClearPendingIRQ(RTC_Alarm_IRQn);
	NVIC_EnableIRQ(RTC_Alarm_IRQn);

	return;
}
void stop_RTC_alarm(void) {
	NVIC_DisableIRQ(RTC_Alarm_IRQn);
	NVIC_ClearPendingIRQ(RTC_Alarm_IRQn);

	// Clear the EXTI line interrupt flag
	// This is set to 1 to clear
	SET_BIT(EXTI->PR, RTC_ALARM_EXTI_LINE);
	// Disable the alarm EXTI rising-edge trigger
	CLEAR_BIT(EXTI->RTSR, RTC_ALARM_EXTI_LINE);
	// Disable the alarm EXTI interrupt
	CLEAR_BIT(EXTI->IMR, RTC_ALARM_EXTI_LINE);

	// Clear the alarm interrupt flag
	CLEAR_BIT(RTC->CRL, RTC_CRL_ALRF);

	return;
}

// hz is the target Hz, e.g. 1000 for 1ms timing
static uint16_t calculate_TIM2_5_prescaler(uint16_t hz) {
	uint16_t prescaler;

	assert(((G_freq_PCLK1/hz) <= (0xFFFF/2)) || (SELECT_BITS(RCC->CFGR, RCC_CFGR_PPRE1) == 0));

	// Clocks:
	// Timers 2-7 and 12-14
	//   PCLK1*1 if PCLK1 prescaler is 1
	//   PCLK1*2 otherwise
	// Timers 1 and 8-11
	//   PCLK2*1 if PCLK2 prescaler is 1
	//   PCLK2*2 otherwise
	// The maximum prescaler is 0xFFFF
	prescaler = G_freq_PCLK1 / hz;
	if (SELECT_BITS(RCC->CFGR, RCC_CFGR_PPRE1) != 0) {
		prescaler *= 2;
	}

	assert(prescaler != 0);
	// A prescaler of 0 divides by 1 so we need to adjust.
	--prescaler;

	return prescaler;
}
void set_sleep_alarm(uint16_t ms) {
	assert(ms != 0);
#if TIM_MS_PERIOD > 1
	assert((0xFFFF/TIM_MS_PERIOD) >= ms);
#endif // TIM_MS_PERIOD > 1

	clock_enable(&RCC->APB1ENR, SLEEP_ALARM_CLOCKEN);

	// Set the reload value and generate an update event to load it
	SLEEP_ALARM_TIM->ARR = ms * TIM_MS_PERIOD;
	SET_BIT(SLEEP_ALARM_TIM->EGR, TIM_EGR_UG);

	SLEEP_ALARM_TIM->SR = 0x0000;                 // Clear all event flags
	SET_BIT(SLEEP_ALARM_TIM->DIER, TIM_DIER_UIE); // Enable update interrupts
	NVIC_ClearPendingIRQ(SLEEP_ALARM_IRQn);
	NVIC_EnableIRQ(SLEEP_ALARM_IRQn);
	SET_BIT(SLEEP_ALARM_TIM->CR1, TIM_CR1_CEN);   // Enable the timer

	return;
}
void stop_sleep_alarm(void) {
	NVIC_DisableIRQ(SLEEP_ALARM_IRQn);
	NVIC_ClearPendingIRQ(SLEEP_ALARM_IRQn);

	// unSET_BIT(SLEEP_ALARM_TIM->SR, TIM_SR_UIF);
	SLEEP_ALARM_TIM->SR = 0;
	// Configured to disable itself
	// unSET_BIT(SLEEP_ALARM_TIM->CR1, TIM_CR1_CEN);

	clock_disable(&RCC->APB1ENR, SLEEP_ALARM_CLOCKEN);

	return;
}


#ifdef __cplusplus
 }
#endif
