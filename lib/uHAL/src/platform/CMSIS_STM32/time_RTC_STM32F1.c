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
// time_RTC_STM32F1.h
// Manage the STM32F1 RTC
// NOTES:
//    This was split off from time.c because the RTC peripherals differ so
//    much between the STM32F1 and the other lines that code can't really be
//    reused
//
#include "common.h"

#if HAVE_STM32F1_RTC
#include "time_RTC.h"
#include "system.h"

// We have 20 bits of prescaler
#define RTC_PSC_MAX (0x100000U)

#if G_freq_PCLK1 < (4 * G_freq_RTC)
# error "The APB1 bus clock must be at least 4X the RTC clock"
#endif


static uint8_t cfg_enabled = 0;

#if uHAL_USE_HIBERNATE
bool RTC_alarm_is_set(void) {
	return BIT_IS_SET(EXTI->IMR, RTC_ALARM_EXTI_LINE);
}
#endif

// We need the internal RTC stuff to wake up from sleep so we only enable
// the interface shims
#if uHAL_USE_RTC
static err_t _set_RTC_seconds(utime_t s);
static utime_t _get_RTC_seconds(void);

err_t set_RTC_seconds(utime_t s) {
	return _set_RTC_seconds(s);
}
utime_t get_RTC_seconds(void) {
	return _get_RTC_seconds();
}
#endif

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

/*
void RTC_adj_clock(int32_t ppm) {
	uint32_t adj, uppm;

	// The adjustment works by slowing the clock down by the specified
	// number of cycles per 2^20, which works out to around 0.95 PPM
	adj = SELECT_BITS(BKP->RTCCR, BKP_RTCCR_CAL) >> BKP_RTCCR_CAL_Pos;
	if (ppm >= 0) {
		uppm = ppm;
		// Convert PPM to steps, rounded
		adj += ((uppm * 0x100000U) + 500000U) / 1000000U;
	} else {
		uppm = -ppm;
		if (adj > uppm) {
			adj = 0;
		} else {
			adj -= uppm;
		}
	}
	if (adj >= 0x80U) {
		adj = 0x80U - 1U;
	}

	MODIFY_BITS(BKP->RTCCR, BKP_RTCCR_CAL,
		adj << BKP_RTCCR_CAL_Pos
	);

	return;
}
// This variation of the normal set_RTC_prediv() speeds up the clock by
// lowering the prescaler to allow setting the RTC calibration value because
// the calibration value can only slow down the clock
void set_RTC_prediv(uint32_t psc) {
	uint32_t cal_per_psc, adj_psc, adj_cal;

	assert(psc <= RTC_PSC_MAX);
	if (psc > RTC_PSC_MAX) {
		return;
	}

	cfg_enable();

	// Calculate the number of calibrate steps equal to one prescaler step,
	// rounded
	cal_per_psc = (0x100000U + (psc/2U)) / psc;
	// Get us in the middle of the range, rounded
	adj_psc = ((0x80U / 2U) + (cal_per_psc / 2U)) / cal_per_psc;
	adj_cal = adj_psc * cal_per_psc;
	MODIFY_BITS(BKP->RTCCR, BKP_RTCCR_CAL,
		adj_cal << BKP_RTCCR_CAL_Pos
	);

	psc -= (1U + adj_psc);
	WRITE_SPLITREG32(psc, RTC->PRLH, RTC->PRLL);

	cfg_disable();

	return;
}
*/
void set_RTC_prediv(uint32_t psc) {
	assert(psc <= RTC_PSC_MAX);
	if (psc > RTC_PSC_MAX) {
		return;
	}

	cfg_enable();

	if (psc > 0) {
		psc -= 1U;
	}
	WRITE_SPLITREG32(psc, RTC->PRLH, RTC->PRLL);

	cfg_disable();

	return;
}
void RTC_init(void) {
	BD_write_enable();
	MODIFY_BITS(RCC->BDCR, RCC_BDCR_RTCSEL|RCC_BDCR_RTCEN,
#if uHAL_USE_INTERNAL_LS_OSC
		RCC_BDCR_RTCSEL_LSI | // Use LSI as the clock source
#else
		RCC_BDCR_RTCSEL_LSE | // Use LSE as the clock source
#endif
		RCC_BDCR_RTCEN      | // Enable the RTC
		0);
	BD_write_disable();

#if uHAL_USE_INTERNAL_LS_OSC
	calibrate_RTC_clock();
#else
	set_RTC_prediv(G_freq_RTC);
#endif

	NVIC_SetPriority(RTC_Alarm_IRQn, RTC_ALARM_IRQp);

	return;
}
static utime_t _get_RTC_seconds(void) {
	uint32_t rtcs;

	wait_for_sync();
	READ_SPLITREG32(rtcs, RTC->CNTH, RTC->CNTL);

	return rtcs;
}
static err_t _set_RTC_seconds(utime_t s) {
	cfg_enable();
	WRITE_SPLITREG32(s, RTC->CNTH, RTC->CNTL);
	cfg_disable();

	return ERR_OK;
}

#if uHAL_USE_HIBERNATE
void set_RTC_alarm(utime_t time) {
	if (time == 0) {
		return;
	}

	cfg_enable();
	time = _get_RTC_seconds() + time;
	WRITE_SPLITREG32(time, RTC->ALRH, RTC->ALRL);

	// Clear the alarm interrupt flag
	CLEAR_BIT(RTC->CRL, RTC_CRL_ALRF);
	cfg_disable();

	// Clear wakeup flag by writing 1 to CWUF
	SET_BIT(PWR->CR, PWR_CR_CWUF);
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
#endif // uHAL_USE_HIBERNATE


#endif // HAVE_STM32F1_RTC
