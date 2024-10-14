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
// time_RTC_STM32Fx.h
// Manage the non-STM32F1 RTC
// NOTES:
//    This was split off from time.c because the RTC peripherals differ so
//    much between the STM32F1 and the other lines that code can't really be
//    reused
//
//    Wakeup uses RTC alarm A
//
//    The time is stored internally in BCD format
//
#include "common.h"

#if ! HAVE_STM32F1_RTC
#include "time_RTC.h"
#include "system.h"


// 2^7 in PREDIV_A and 2^15 in PREDIV_S
// Max input clock is therefore 0x80*0x8000, a bit less than 4.2MHz
#define RTC_PSC_A_MAX (0x80U)
#define RTC_PSC_S_MAX (0x8000U)
#define RTC_PSC_MAX (RTC_PSC_A_MAX * RTC_PSC_S_MAX)

#define RTC_DR_DATE_MASK (RTC_DR_YT|RTC_DR_YU|RTC_DR_MT|RTC_DR_MU|RTC_DR_DT|RTC_DR_DU)
#define RTC_TR_TIME_MASK (RTC_TR_HT|RTC_TR_HU|RTC_TR_MNT|RTC_TR_MNU|RTC_TR_ST|RTC_TR_SU)

// Per the manual, when the APB1 clock is < 7X the rtc clock the calendar
// registers may give corrupted results when read and so it should be read
// again
#if G_freq_PCLK1 < (7 * G_freq_RTC)
# define REREAD_CAL_REG 1
#else
# define REREAD_CAL_REG 0
#endif

static uint_fast8_t cfg_enabled = 0;

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

	// Clear the alarm interrupt enable flag
	CLEAR_BIT(RTC->ISR, RTC_ISR_ALRAF);
	// Clear the EXTI line interrupt flag
	// This is set to 1 to clear
	SET_BIT(EXTI->PR, RTC_ALARM_EXTI_LINE);
	// Disable the alarm EXTI interrupt
	CLEAR_BIT(EXTI->IMR, RTC_ALARM_EXTI_LINE);

	return;
}


// These devices use BCD, I use seconds, so heres the conversion...
// Partially copied from the ST HAL
static uint8_t byte_to_bcd(uint8_t byte) {
	uint32_t tmp;

	tmp = (uint8_t )(byte / 10U) << 4U;
	return tmp | (byte % 10U);
}
static uint8_t bcd_to_byte(uint8_t bcd) {
	uint32_t tmp;

	tmp = ((uint8_t )(bcd & (uint8_t )0xF0U) >> (uint8_t )0x4U) * 10U;
	return (tmp + (bcd & (uint8_t )0x0FU));
}

//
// Set up RTC
// The RTC clock is configured in the backup domain so the power interface
// clock and the backup domain interface clock need to be enabled before this
// is called.
// TODO: Load date from backup registers? AN2821 may shed light on that.
// TODO: Reset BDRST to allow changing RTC settings if they differ from current
static void cfg_enable(void) {
	if (cfg_enabled == 0) {
		BD_write_enable();

		// Unlock sequence as per the reference
		RTC->WPR = 0xCAU;
		RTC->WPR = 0x53U;
	}
	++cfg_enabled;

	return;
}
static void cfg_disable(void) {
	if (cfg_enabled != 0) {
		--cfg_enabled;
	}
	if (cfg_enabled == 0) {
		// Writing any invalid key will re-lock the registers
		RTC->WPR = 0x00U;

		BD_write_disable();
	}

	return;
}
static void wait_for_sync(void) {
	cfg_enable();
	// Wait until the RTC registries are synchronized with the RTC core
	CLEAR_BIT(RTC->ISR, RTC_ISR_RSF);
	while (!BIT_IS_SET(RTC->ISR, RTC_ISR_RSF)) {
		// Nothing to do here
	}
	cfg_disable();

	return;
}

void set_RTC_prediv(uint32_t psc) {
	uint32_t prediv_a, prediv_s;

	assert(psc > 0 && psc <= RTC_PSC_MAX);
	if (psc <= 0 || psc > RTC_PSC_MAX) {
		return;
	}

	cfg_enable();

	if (psc < RTC_PSC_A_MAX) {
		prediv_a = psc - 1U;
		prediv_s = 0U;
	} else if ((psc % RTC_PSC_A_MAX) == 0) {
		prediv_a = RTC_PSC_A_MAX - 1U;
		prediv_s = (psc / RTC_PSC_A_MAX) - 1U;
	} else {
		for (prediv_a = RTC_PSC_A_MAX; prediv_a > 1; --prediv_a) {
			if ((psc % prediv_a) == 0) {
				break;
			} else if ((psc / prediv_a) > RTC_PSC_S_MAX) {
				++prediv_a;
				break;
			}
		}
		prediv_s = (psc / prediv_a) - 1U;
		--prediv_a;
	}
	// I thought maybe this would turn out to be simpler, but alas the range
	// checks mean it is not
	/*
	} else {
		prediv_a = 0x01U;

		while ((prediv_a & psc) == 0U) {
			prediv_a << 1U;
		}
		if (prediv_a > RTC_PSC_A_MAX) {
			prediv_a = RTC_PSC_A_MAX;
		}
		prediv_s = (psc / prediv_a);
		while (prediv_s > RTC_PSC_S_MAX) {
			--prediv_a;
			prediv_s = (psc / prediv_a);
		}
	}
	*/

	// RTC_PRER needs to be modified with two separate writes
	MODIFY_BITS(RTC->PRER, RTC_PRER_PREDIV_A,
		prediv_a << RTC_PRER_PREDIV_A_Pos
		);
	MODIFY_BITS(RTC->PRER, RTC_PRER_PREDIV_S,
		prediv_s << RTC_PRER_PREDIV_S_Pos
		);

	cfg_disable();

	return;
}
void RTC_init(void) {
	cfg_enable();
	// Backup domain register writes are enabled by cfg_enable()
	//BD_write_enable();
	MODIFY_BITS(RCC->BDCR, RCC_BDCR_RTCSEL|RCC_BDCR_RTCEN,
#if uHAL_USE_INTERNAL_LS_OSC
		RCC_BDCR_RTCSEL_LSI | // Use LSI as the clock source
#else
		RCC_BDCR_RTCSEL_LSE | // Use LSE as the clock source
#endif
		RCC_BDCR_RTCEN      | // Enable the RTC
		0);
	//BD_write_disable();

	// Calendar initialization procedure as per the reference
	// The defaults are fine though
	if (!BIT_IS_SET(RTC->ISR, RTC_ISR_INITS)) {
		SET_BIT(RTC->ISR, RTC_ISR_INIT);
		while (!BIT_IS_SET(RTC->ISR, RTC_ISR_INITF)) {
			// Nothing to do here
		}

#if uHAL_USE_INTERNAL_LS_OSC
		calibrate_RTC_clock();
#else
		set_RTC_prediv(G_freq_RTC);
#endif

		// 24-hour format
		CLEAR_BIT(RTC->CR, RTC_CR_FMT);

		CLEAR_BIT(RTC->ISR, RTC_ISR_INIT);
	}

	wait_for_sync();
	cfg_disable();
	NVIC_SetPriority(RTC_Alarm_IRQn, RTC_ALARM_IRQp);

	return;
}
static utime_t _get_RTC_seconds(void) {
	uint32_t tr, dr;
	uint8_t hour, minute, second;
	uint8_t year, month, day;

	wait_for_sync();

#if REREAD_CAL_REG
	uint32_t tr2, dr2;

	do {
		//FIXME: Is this supposed to wait for synchronization between reads?
		tr = RTC->TR;
		dr = RTC->DR;

		tr2 = RTC->TR;
		dr2 = RTC->DR;
	 } while ((tr != tr2) || (dr != dr2));
#else
	tr = RTC->TR;
	dr = RTC->DR;
#endif

	hour   = bcd_to_byte(GATHER_BITS(tr, 0x3FU, RTC_TR_HU_Pos));
	minute = bcd_to_byte(GATHER_BITS(tr, 0x7FU, RTC_TR_MNU_Pos));
	second = bcd_to_byte(GATHER_BITS(tr, 0x7FU, RTC_TR_SU_Pos));

	year  = bcd_to_byte(GATHER_BITS(dr, 0xFFU, RTC_DR_YU_Pos));
	month = bcd_to_byte(GATHER_BITS(dr, 0x1FU, RTC_DR_MU_Pos));
	day   = bcd_to_byte(GATHER_BITS(dr, 0x3FU, RTC_DR_DU_Pos));

	return date_to_seconds(year, month, day) + time_to_seconds(hour, minute, second);
}
static void calendarcfg_enable(void) {
	cfg_enable();
	SET_BIT(RTC->ISR, RTC_ISR_INIT);
	while (!BIT_IS_SET(RTC->ISR, RTC_ISR_INITF)) {
		// Nothing to do here
	}

	return;
}
static void calendarcfg_disable(void) {
	CLEAR_BIT(RTC->ISR, RTC_ISR_INIT);
	wait_for_sync();
	cfg_disable();

	return;
}
static err_t _set_RTC_seconds(utime_t s) {
	uint32_t tr, dr;
	uint8_t hour, minute, second;
	uint8_t year, month, day;

	seconds_to_time(s, &hour, &minute, &second);
	seconds_to_date(s, &year, &month, &day);

	wait_for_sync();

	tr =
		((uint32_t )byte_to_bcd(hour)   << RTC_TR_HU_Pos) |
		((uint32_t )byte_to_bcd(minute) << RTC_TR_MNU_Pos) |
		((uint32_t )byte_to_bcd(second) << RTC_TR_SU_Pos);
	dr =
		((uint32_t )byte_to_bcd(year)  << RTC_DR_YU_Pos) |
		((uint32_t )byte_to_bcd(month) << RTC_DR_MU_Pos) |
		((uint32_t )byte_to_bcd(day)   << RTC_DR_DU_Pos);

	calendarcfg_enable();
	MODIFY_BITS(RTC->TR, RTC_TR_TIME_MASK, tr);
	MODIFY_BITS(RTC->DR, RTC_DR_DATE_MASK, dr);
	calendarcfg_disable();

	return ERR_OK;
}

#if uHAL_USE_HIBERNATE
void set_RTC_alarm(utime_t time) {
	uint32_t tr, tmp;
	uint8_t hour, minute, second;

	assert(time <= HIBERNATE_MAX_S);
	// We only look at the time part of the clock when setting the alarm to
	// simplify things which works fine as long as we don't exceed one day
	assert(time < SECONDS_PER_DAY);

	if (time == 0) {
		return;
	}

	//
	// According to the reference manual, the correct order to do this is:
	//    Configure and enable the EXTI line and select rising edge sensitivity
	//    Configure and enable the RTC_Alarm IRQ channel
	//    Configure the RTC to generate alarms
	//
	// Clear the EXTI line interrupt flag
	// This is set to 1 to clear
	SET_BIT(EXTI->PR, RTC_ALARM_EXTI_LINE);
	// Enable the alarm EXTI rising-edge trigger
	// This is mandatory to wake from stop mode
	SET_BIT(EXTI->RTSR, RTC_ALARM_EXTI_LINE);
	// Enable the alarm EXTI interrupt
	SET_BIT(EXTI->IMR, RTC_ALARM_EXTI_LINE);
	// Enable the NVIC interrupt
	NVIC_ClearPendingIRQ(RTC_Alarm_IRQn);
	NVIC_EnableIRQ(RTC_Alarm_IRQn);

	wait_for_sync();

	tr = RTC->TR;
	hour   = bcd_to_byte(GATHER_BITS(tr, 0x3FU, RTC_TR_HU_Pos));
	minute = bcd_to_byte(GATHER_BITS(tr, 0x7FU, RTC_TR_MNU_Pos));
	second = bcd_to_byte(GATHER_BITS(tr, 0x7FU, RTC_TR_SU_Pos));

	tmp = time + time_to_seconds(hour, minute, second);
	seconds_to_time(tmp, &hour, &minute, &second);
	hour   = byte_to_bcd(hour);
	minute = byte_to_bcd(minute);
	second = byte_to_bcd(second);

	cfg_enable();
	// The documentation is unclear with regard to whether one or both of
	// ALRAE and ALRAIE need to be cleared to configure the alarm or if it
	// differs between alarms A and B
	CLEAR_BIT(RTC->CR, RTC_CR_ALRAE|RTC_CR_ALRAIE);
	while (!BIT_IS_SET(RTC->ISR, RTC_ISR_ALRAWF)) {
		// Nothing to do here
	}

	RTC->ALRMAR = (
		(0b1U   << RTC_ALRMAR_MSK4_Pos) | // Ignore date part of the alarm
		(0b0U   << RTC_ALRMAR_MSK3_Pos) | // Use hour
		(hour   << RTC_ALRMAR_HU_Pos)   | // Set hour
		(0b0U   << RTC_ALRMAR_MSK2_Pos) | // Use minute
		(minute << RTC_ALRMAR_MNU_Pos)  | // Set minute
		(0b0U   << RTC_ALRMAR_MSK1_Pos) | // Use second
		(second << RTC_ALRMAR_SU_Pos)   | // Set second
		0);

	// Clear the alarm interrupt flag
	CLEAR_BIT(RTC->ISR, RTC_ISR_ALRAF);
	// Clear wakeup flag by writing 1 to CWUF
	SET_BIT(PWR->CR, PWR_CR_CWUF);
	// Set the alarm; again it's unclear which of these flags matter.
	SET_BIT(RTC->CR, RTC_CR_ALRAE|RTC_CR_ALRAIE);
	cfg_disable();

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
	CLEAR_BIT(RTC->ISR, RTC_ISR_ALRAF);

	return;
}
#endif // uHAL_USE_HIBERNATE


#endif // ! HAVE_STM32F1_RTC
