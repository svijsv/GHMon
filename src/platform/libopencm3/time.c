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

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/rtc.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/exti.h>
#include <libopencm3/stm32/timer.h>

/*
* Static values
*/
// Divide the number of cycles per ms by this in a dumb delay to account for
// overhead
#define DUMB_DELAY_DIV 7

// EXTI line used for RTC alarm
#define RTC_ALARM_EXTI_LINE EXTI17


/*
* Types
*/


/*
* Variables
*/
// System ticks, milliseconds
volatile utime_t G_sys_uticks;

volatile bool sleep_alarm_is_set;
volatile bool RTC_alarm_is_set;

/*
* Local function prototypes
*/
static uint16_t calculate_TIM2_5_prescaler(uint32_t hz);
static void systick_init(void);
static void RTC_init(void);
static void timers_init(void);


/*
* Interrupt handlers
*/
OPTIMIZE_FUNCTION \
void sys_tick_handler(void) {
	++G_sys_uticks;
	return;
}

OPTIMIZE_FUNCTION \
void rtc_alarm_isr(void) {
	nvic_disable_irq(NVIC_RTC_ALARM_IRQ);
	nvic_clear_pending_irq(NVIC_RTC_ALARM_IRQ);

	exti_disable_request(RTC_ALARM_EXTI_LINE);
	exti_reset_request(RTC_ALARM_EXTI_LINE);
	rtc_clear_flag(RTC_ALR);

	RTC_alarm_is_set = false;

	return;
}
OPTIMIZE_FUNCTION \
void SleepAlarm_IRQHandler(void) {
	nvic_disable_irq(SLEEP_ALARM_IRQn);
	nvic_clear_pending_irq(SLEEP_ALARM_IRQn);

	timer_clear_flag(SLEEP_ALARM_TIM, TIM_SR_UIF);
	timer_disable_irq(SLEEP_ALARM_TIM, TIM_DIER_UIE);

	sleep_alarm_is_set = false;

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
	systick_counter_disable();
	systick_interrupt_disable();

	nvic_set_priority(NVIC_SYSTICK_IRQ, SYSTICK_IRQp);
	systick_set_frequency(1000, rcc_ahb_frequency);
	systick_clear();
	G_sys_uticks = 0;

	systick_interrupt_enable();
	systick_counter_enable();

	return;
}
//
// Set up RTC
static void RTC_init(void) {
	rtc_auto_awake(RCC_LSE, G_freq_LSE-1);
	nvic_set_priority(NVIC_RTC_ALARM_IRQ, RTC_ALARM_IRQp);

	return;
}
void set_RTC_alarm(utime_t time) {
	assert(time > 0);

	rtc_clear_flag(RTC_ALR);

	// Clear the EXTI line interrupt flag
	exti_reset_request(RTC_ALARM_EXTI_LINE);
	// Enable the alarm EXTI rising-edge trigger
	// This is mandatory to wake from stop mode
	exti_set_trigger(RTC_ALARM_EXTI_LINE, EXTI_TRIGGER_RISING);
	// Enable the alarm EXTI interrupt
	exti_enable_request(RTC_ALARM_EXTI_LINE);

	nvic_clear_pending_irq(NVIC_RTC_ALARM_IRQ);
	nvic_enable_irq(NVIC_RTC_ALARM_IRQ);

	rtc_set_alarm_time(rtc_get_counter_val() + time);
	RTC_alarm_is_set = true;

	return;
}
void stop_RTC_alarm(void) {
	nvic_disable_irq(NVIC_RTC_ALARM_IRQ);
	nvic_clear_pending_irq(NVIC_RTC_ALARM_IRQ);

	exti_disable_request(RTC_ALARM_EXTI_LINE);
	exti_reset_request(RTC_ALARM_EXTI_LINE);
	rtc_clear_flag(RTC_ALR);

	RTC_alarm_is_set = false;

	return;
}
utime_t get_RTC_seconds(void) {
	return rtc_get_counter_val();
}

//
// Configure the sleep() and uscounter timers
static void timers_init(void) {
	//
	// Sleep timer
	//
	rcc_periph_clock_enable(SLEEP_ALARM_RCC);
	rcc_periph_reset_pulse(SLEEP_ALARM_RST);

	timer_disable_preload(SLEEP_ALARM_TIM);
	timer_set_alignment(SLEEP_ALARM_TIM, TIM_CR1_CMS_EDGE);
	timer_direction_up(SLEEP_ALARM_TIM);
	timer_one_shot_mode(SLEEP_ALARM_TIM);
	timer_set_prescaler(SLEEP_ALARM_TIM, calculate_TIM2_5_prescaler(1000*TIM_MS_PERIOD));

	nvic_set_priority(SLEEP_ALARM_IRQn, SLEEP_ALARM_IRQp);
	rcc_periph_clock_disable(SLEEP_ALARM_RCC);

	//
	// USCOUNTER timer
	//
	rcc_periph_clock_enable(USCOUNTER_RCC);
	rcc_periph_reset_pulse(USCOUNTER_RST);

	timer_disable_preload(USCOUNTER_TIM);
	timer_set_alignment(USCOUNTER_TIM, TIM_CR1_CMS_EDGE);
	timer_direction_up(USCOUNTER_TIM);
	timer_one_shot_mode(USCOUNTER_TIM);
	timer_set_prescaler(USCOUNTER_TIM, calculate_TIM2_5_prescaler(1000000));

	rcc_periph_clock_disable(USCOUNTER_RCC);

	return;
}
// hz is the target Hz, e.g. 1000 for 1ms timing
static uint16_t calculate_TIM2_5_prescaler(uint32_t hz) {
	uint16_t prescaler;

	assert(((rcc_apb1_frequency/hz) <= (0xFFFF/2)) || (rcc_apb1_frequency == rcc_ahb_frequency));

	// Clocks:
	// Timers 2-7 and 12-14
	//   PCLK1*1 if PCLK1 prescaler is 1
	//   PCLK1*2 otherwise
	// Timers 1 and 8-11
	//   PCLK2*1 if PCLK2 prescaler is 1
	//   PCLK2*2 otherwise
	// The maximum prescaler is 0xFFFF
	prescaler = rcc_apb1_frequency / hz;
	if (rcc_apb1_frequency != rcc_ahb_frequency) {
		prescaler *= 2;
	}

	assert(prescaler != 0);
	// A prescaler of 0 divides by 1 so we need to adjust.
	prescaler -= 1;

	return prescaler;
}
void set_sleep_alarm(uint16_t ms) {
	assert(ms != 0);
#if TIM_MS_PERIOD > 1
	assert((0xFFFF/TIM_MS_PERIOD) >= ms);
#endif

	rcc_periph_clock_enable(SLEEP_ALARM_RCC);

	// Set the reload value and generate an update event to load it
	timer_set_period(SLEEP_ALARM_TIM, ms * TIM_MS_PERIOD);
	timer_generate_event(SLEEP_ALARM_TIM, TIM_EGR_UG);

	timer_clear_flag(SLEEP_ALARM_TIM, TIM_SR_UIF);
	timer_enable_irq(SLEEP_ALARM_TIM, TIM_DIER_UIE);

	nvic_clear_pending_irq(SLEEP_ALARM_IRQn);
	nvic_enable_irq(SLEEP_ALARM_IRQn);

	sleep_alarm_is_set = true;
	timer_enable_counter(SLEEP_ALARM_TIM);

	return;
}
void stop_sleep_alarm(void) {
	nvic_disable_irq(SLEEP_ALARM_IRQn);
	nvic_clear_pending_irq(SLEEP_ALARM_IRQn);

	timer_disable_counter(SLEEP_ALARM_TIM);
	timer_clear_flag(SLEEP_ALARM_TIM, TIM_SR_UIF);

	rcc_periph_clock_disable(SLEEP_ALARM_RCC);

	return;
}
void uscounter_on(void) {
	rcc_periph_clock_enable(USCOUNTER_RCC);
	return;
}
void uscounter_off(void) {
	rcc_periph_clock_disable(USCOUNTER_RCC);
	return;
}

void delay(utime_t ms) {
	utime_t timer;

	timer = SET_TIMEOUT(ms);
	while (!TIMES_UP(timer)) {
		// Nothing to do here
	}

	return;
}
// https:// stackoverflow.com/questions/7083482/how-to-prevent-gcc-from-optimizing-out-a-busy-wait-loop
void dumb_delay(utime_t ms) {
	uint32_t cycles;

	cycles = ms * (rcc_ahb_frequency/(1000*DUMB_DELAY_DIV));

	for (uint32_t i = cycles; i > 0; --i) {
		// Count some clock cycles
		__asm__ volatile("" : "+g" (i) : :);
	}

	return;
}
void dumber_delay(uint32_t cycles) {
	for (uint32_t i = cycles; i > 0; --i) {
		// Count some clock cycles
		__asm__ volatile("" : "+g" (i) : :);
	}

	return;
}

err_t set_time(uint8_t hour, uint8_t minute, uint8_t second) {
	uint32_t now;

	if ((hour > 24) || (minute > 59) || (second > 59)) {
		return EUSAGE;
	}

	// Conserve the date part of the RTC
	// Don't call get_RTC_seconds() from the macro SNAP_TO_FACTOR() or it will
	// be called twice.
	now = rtc_get_counter_val();
	now = SNAP_TO_FACTOR(now, DAYS) + time_to_seconds(hour, minute, second);
	rtc_set_counter_val(now);

	return EOK;
}
err_t set_date(uint8_t year, uint8_t month, uint8_t day) {
	uint32_t now;

	if ((year > (0xFFFFFFFF/YEARS)) || (!IS_BETWEEN(month, 1, 12)) || (!IS_BETWEEN(day, 1, 31))) {
		return EUSAGE;
	}

	// Conserve the time part of the RTC
	now = (rtc_get_counter_val() % DAYS) + date_to_seconds(year, month, day);
	rtc_set_counter_val(now);

	return EOK;
}


#ifdef __cplusplus
 }
#endif
