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
// time_RTC_STM32F1.h
// Manage the STM32F1 RTC
// NOTES:
//    This was split off from time.c because the RTC peripherals differ so
//    much between the STM32F1 and the other lines that code can't really be
//    reused
//
//    This file should only be included by time.c
//
// TODO: Reset BDRST to allow changing RTC settings if they differ from current
//
#if INCLUDED_BY_TIME_C

/*
* Includes
*/


/*
* Static values
*/


/*
* Types
*/


/*
* Variables
*/
static uint8_t cfg_enabled = 0;


/*
* Local function prototypes
*/


/*
* Interrupt handlers
*/
OPTIMIZE_FUNCTION \
void RTC_Alarm_IRQHandler(void) {
	NVIC_DisableIRQ(RTC_Alarm_IRQn);
	NVIC_ClearPendingIRQ(RTC_Alarm_IRQn);

	// Clear the alarm interrupt flag
	CLEAR_BIT(RTC->CRL, RTC_CRL_ALRF);
	// Clear the EXTI line interrupt flag
	// This is set to 1 to clear
	SET_BIT(EXTI->PR, RTC_ALARM_EXTI_LINE);
	// Disable the alarm EXTI interrupt
	CLEAR_BIT(EXTI->IMR, RTC_ALARM_EXTI_LINE);

	return;
}


/*
* Functions
*/
//
// Set up RTC
// The RTC clock is configured in the backup domain so the power interface
// clock and the backup domain interface clock need to be enabled before this
// is called.
static void cfg_enable(void) {
	if (cfg_enabled == 0) {
		BD_write_enable();

		while (!BIT_IS_SET(RTC->CRL, RTC_CRL_RTOFF)) {
			// Nothing to do here
		}

		SET_BIT(RTC->CRL, RTC_CRL_CNF);
		while (!BIT_IS_SET(RTC->CRL, RTC_CRL_CNF)) {
			// Nothing to do here
		}
	}
	++cfg_enabled;

	return;
}
static void cfg_disable(void) {
	if (cfg_enabled != 0) {
		--cfg_enabled;
	}
	if (cfg_enabled == 0) {
		CLEAR_BIT(RTC->CRL, RTC_CRL_CNF);
		while (BIT_IS_SET(RTC->CRL, RTC_CRL_CNF)) {
			// Nothing to do here
		}

		while (!BIT_IS_SET(RTC->CRL, RTC_CRL_RTOFF)) {
			// Nothing to do here
		}

		BD_write_disable();
	}

	return;
}
static void wait_for_sync(void) {
	cfg_enable();
	// Wait until the RTC registries are synchronized with the RTC core
	CLEAR_BIT(RTC->CRL, RTC_CRL_RSF);
	while (!BIT_IS_SET(RTC->CRL, RTC_CRL_RSF)) {
		// Nothing to do here
	}
	cfg_disable();

	return;
}
static void RTC_init(void) {
	BD_write_enable();
	MODIFY_BITS(RCC->BDCR, RCC_BDCR_RTCSEL|RCC_BDCR_RTCEN,
		RCC_BDCR_RTCSEL_LSE | // Use the LSE as the clock source
		RCC_BDCR_RTCEN      | // Enable the RTC
		0);
	BD_write_disable();

	cfg_enable();
	// Set the prescaler
	WRITE_SPLITREG(G_freq_LSE-1, RTC->PRLH, RTC->PRLL);
	cfg_disable();

	NVIC_SetPriority(RTC_Alarm_IRQn, RTC_ALARM_IRQp);

	return;
}
utime_t get_RTC_seconds(void) {
	uint32_t rtcs;

	wait_for_sync();
	READ_SPLITREG(rtcs, RTC->CNTH, RTC->CNTL);

	return rtcs;
}
err_t set_RTC_seconds(utime_t s) {
	cfg_enable();
	WRITE_SPLITREG(s, RTC->CNTH, RTC->CNTL);
	cfg_disable();

	return EOK;
}

void set_RTC_alarm(utime_t time) {
	if (time == 0) {
		return;
	}

	cfg_enable();
	time = get_RTC_seconds() + time;
	WRITE_SPLITREG(time, RTC->ALRH, RTC->ALRL);

	// Clear the alarm interrupt flag
	CLEAR_BIT(RTC->CRL, RTC_CRL_ALRF);
	cfg_disable();

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

#endif // INCLUDED_BY_TIME_C
