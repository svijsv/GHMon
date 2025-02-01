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

#if NEED_RTC
#if ! HAVE_STM32F1_RTC
#include "time_RTC.h"
#include "system.h"


// This is defined in RTC.c
utime_t RTC_datetime_to_second_counter(const datetime_t *datetime, utime_t now);

// 2^7 in PREDIV_A and 2^15 in PREDIV_S
// Max input clock is therefore 0x80*0x8000, a bit less than 4.2MHz
#define RTC_PSC_A_MAX (0x80U)
#define RTC_PSC_S_MAX (0x8000U)
#define RTC_PSC_MAX (RTC_PSC_A_MAX * RTC_PSC_S_MAX)

#define RTC_DR_YEAR_MASK  (RTC_DR_YT | RTC_DR_YU)
#define RTC_DR_MONTH_MASK (RTC_DR_MT | RTC_DR_MU)
#define RTC_DR_DAY_MASK   (RTC_DR_DT | RTC_DR_DU)
#define RTC_DR_DATE_MASK (RTC_DR_YEAR_MASK | RTC_DR_MONTH_MASK | RTC_DR_DAY_MASK)

#define RTC_TR_HOUR_MASK   (RTC_TR_HT  | RTC_TR_HU)
#define RTC_TR_MINUTE_MASK (RTC_TR_MNT | RTC_TR_MNU)
#define RTC_TR_SECOND_MASK (RTC_TR_ST  | RTC_TR_SU)
#define RTC_TR_TIME_MASK (RTC_TR_HOUR_MASK | RTC_TR_MINUTE_MASK | RTC_TR_SECOND_MASK)

// Per the manual, when the APB1 clock is < 7X the rtc clock the shadow registers
// must be bypassed and the calendar registers may give corrupted results when
// read and so should be read again
#if G_freq_PCLK1 < (7 * G_freq_RTC)
# define APB1_IS_SLOW 1
#else
# define APB1_IS_SLOW 0
#endif

// The RTC only stores 2 digit years but we need to be able to return the correct
// time with both a set RTC (that is, a date) and an unset RTC (an uptime), so
// we track the 100s and 1000s ourselves.
#if uHAL_USE_RTC
static time_year_t year_base = TIME_YEAR_0 - (TIME_YEAR_0 % 100);
#else
static time_year_t year_base = 0;
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
static err_t _set_RTC_datetime(const datetime_t *datetime, utime_t new_s);
static err_t _get_RTC_datetime(datetime_t *datetime);

err_t set_RTC_seconds(utime_t s) {
	return _set_RTC_seconds(s);
}
utime_t get_RTC_seconds(void) {
	return _get_RTC_seconds();
}

err_t set_RTC_datetime(const datetime_t *datetime) {
	return _set_RTC_datetime(datetime, 0);
}
err_t get_RTC_datetime(datetime_t *datetime) {
	return _get_RTC_datetime(datetime);
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
static uint_fast8_t byte_to_bcd(uint_fast8_t byte) {
	uint32_t tmp;

	tmp = (uint_fast8_t )(byte / 10U) << 4U;
	return tmp | (byte % 10U);
}
static uint_fast8_t bcd_to_byte(uint_fast8_t bcd) {
	uint32_t tmp;

	tmp = ((uint_fast8_t )(bcd & (uint_fast8_t )0xF0U) >> (uint_fast8_t )0x4U) * 10U;
	return (tmp + (bcd & (uint_fast8_t )0x0FU));
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

	uHAL_assert(psc > 0 && psc <= RTC_PSC_MAX);
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

		RTC->CR =
			0U << RTC_CR_FMT_Pos |                           // 24-hour format
			(APB1_IS_SLOW ? 1U : 0U) << RTC_CR_BYPSHAD_Pos | // Bypass the shadow registers if APB1 is slow
			0U;

		CLEAR_BIT(RTC->ISR, RTC_ISR_INIT);
	}

	if (uHAL_USE_RTC) {
		uint32_t tmp = RTC->RTC_BKP_DATE_REG;
		if (tmp != 0) {
			year_base = tmp;
		}
	}

	wait_for_sync();

	cfg_disable();
	NVIC_SetPriority(RTC_Alarm_IRQn, RTC_ALARM_IRQp);

	return;
}
static void calendarcfg_disable(void) {
	CLEAR_BIT(RTC->ISR, RTC_ISR_INIT);
	wait_for_sync();
	cfg_disable();

	return;
}
static void calendarcfg_enable(void) {
	cfg_enable();
	SET_BIT(RTC->ISR, RTC_ISR_INIT);
	while (!BIT_IS_SET(RTC->ISR, RTC_ISR_INITF)) {
		// Nothing to do here
	}

	return;
}

static err_t _get_RTC_datetime(datetime_t *datetime) {
	uint32_t tr, dr;

	uHAL_assert(datetime != NULL);
	if (!uHAL_SKIP_INVALID_ARG_CHECKS && datetime == NULL) {
		return ERR_BADARG;
	}

	wait_for_sync();

#if APB1_IS_SLOW
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

	datetime->hour   = bcd_to_byte(GATHER_BITS(tr, 0x3FU, RTC_TR_HU_Pos));
	datetime->minute = bcd_to_byte(GATHER_BITS(tr, 0x7FU, RTC_TR_MNU_Pos));
	datetime->second = bcd_to_byte(GATHER_BITS(tr, 0x7FU, RTC_TR_SU_Pos));

	// The RTC only stores 2 digit years
	datetime->year  = bcd_to_byte(GATHER_BITS(dr, 0xFFU, RTC_DR_YU_Pos)) + year_base;
	datetime->month = bcd_to_byte(GATHER_BITS(dr, 0x1FU, RTC_DR_MU_Pos));
	datetime->day   = bcd_to_byte(GATHER_BITS(dr, 0x3FU, RTC_DR_DU_Pos));

	return ERR_OK;
}
static utime_t _get_RTC_seconds(void) {
	datetime_t datetime;

	if (_get_RTC_datetime(&datetime) == ERR_OK) {
		return datetime_to_seconds(&datetime);
	}
	return 0;
}

static err_t _set_RTC_datetime(const datetime_t *datetime, utime_t new_s) {
	uint32_t tr = 0, dr = 0;

	uHAL_assert(datetime != NULL);
	if (!uHAL_SKIP_INVALID_ARG_CHECKS && datetime == NULL) {
		return ERR_BADARG;
	}

#if uHAL_USE_UPTIME_EMULATION
	utime_t old_s = _get_RTC_seconds();
	if (new_s == 0) {
		new_s = RTC_datetime_to_second_counter(datetime, old_s);
	}
	fix_uptime(new_s, old_s);
#endif

	// Only change the date if it's set in the new structure
	if ((datetime->year | datetime->month | datetime->day) != 0) {
		uHAL_assert((IS_IN_RANGE(datetime->month, 1, 12)) && (IS_IN_RANGE(datetime->day, 1, 31)));
		if (!uHAL_SKIP_INVALID_ARG_CHECKS && ((!IS_IN_RANGE(datetime->month, 1, 12)) || (!IS_IN_RANGE(datetime->day, 1, 31)))) {
			return ERR_BADARG;
		}

		// The RTC only stores 2 digit years
		uint_fast8_t year = datetime->year % 100;
		if (uHAL_USE_RTC) {
			year_base = datetime->year - year;
		}
		dr =
			((uint32_t )byte_to_bcd(year)            << RTC_DR_YU_Pos) |
			((uint32_t )byte_to_bcd(datetime->month) << RTC_DR_MU_Pos) |
			((uint32_t )byte_to_bcd(datetime->day)   << RTC_DR_DU_Pos);
	}

	// Only change the time if it's set in the new structure
	if ((datetime->hour | datetime->minute | datetime->second) != 0) {
		uHAL_assert((datetime->hour <= 23) || (datetime->minute <= 59) || (datetime->second <= 59));
		if (!uHAL_SKIP_INVALID_ARG_CHECKS && ((datetime->hour > 23) || (datetime->minute > 59) || (datetime->second > 59))) {
			return ERR_BADARG;
		}
		tr =
			((uint32_t )byte_to_bcd(datetime->hour)   << RTC_TR_HU_Pos) |
			((uint32_t )byte_to_bcd(datetime->minute) << RTC_TR_MNU_Pos) |
			((uint32_t )byte_to_bcd(datetime->second) << RTC_TR_SU_Pos);
	}

	calendarcfg_enable();
	if (tr != 0) {
		MODIFY_BITS(RTC->TR, RTC_TR_TIME_MASK, tr);
	}
	if (dr != 0) {
		MODIFY_BITS(RTC->DR, RTC_DR_DATE_MASK, dr);
		if (uHAL_USE_RTC) {
			RTC->RTC_BKP_DATE_REG = year_base;
		}
	}
	calendarcfg_disable();

	return ERR_OK;
}
static err_t _set_RTC_seconds(utime_t s) {
	datetime_t datetime;

	seconds_to_datetime(s, &datetime);
	return _set_RTC_datetime(&datetime, s);
}

#if uHAL_USE_HIBERNATE
void set_RTC_alarm(utime_t time) {
	uint32_t tr, tmp;

	uHAL_assert(time <= HIBERNATE_MAX_S);
	// We only look at the time part of the clock when setting the alarm to
	// simplify things which works fine as long as we don't exceed one day
	uHAL_assert(time < SECONDS_PER_DAY);

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

	datetime_t dt;
	tr = RTC->TR;
	dt.hour   = bcd_to_byte(GATHER_BITS(tr, 0x3FU, RTC_TR_HU_Pos));
	dt.minute = bcd_to_byte(GATHER_BITS(tr, 0x7FU, RTC_TR_MNU_Pos));
	dt.second = bcd_to_byte(GATHER_BITS(tr, 0x7FU, RTC_TR_SU_Pos));

	tmp = time + time_to_seconds(&dt);
	seconds_to_time(tmp, &dt);
	dt.hour   = byte_to_bcd(dt.hour);
	dt.minute = byte_to_bcd(dt.minute);
	dt.second = byte_to_bcd(dt.second);

	cfg_enable();
	// The documentation is unclear with regard to whether one or both of
	// ALRAE and ALRAIE need to be cleared to configure the alarm or if it
	// differs between alarms A and B
	CLEAR_BIT(RTC->CR, RTC_CR_ALRAE|RTC_CR_ALRAIE);
	while (!BIT_IS_SET(RTC->ISR, RTC_ISR_ALRAWF)) {
		// Nothing to do here
	}

	RTC->ALRMAR = (
		(0b1U  << RTC_ALRMAR_MSK4_Pos) | // Ignore date part of the alarm
		(0b0U  << RTC_ALRMAR_MSK3_Pos) | // Use hour
		(0b0U  << RTC_ALRMAR_MSK2_Pos) | // Use minute
		(0b0U  << RTC_ALRMAR_MSK1_Pos) | // Use second
		((uint_t )dt.hour   << RTC_ALRMAR_HU_Pos)   | // Set hour
		((uint_t )dt.minute << RTC_ALRMAR_MNU_Pos)  | // Set minute
		((uint_t )dt.second << RTC_ALRMAR_SU_Pos)   | // Set second
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

#if USE_RTC_UPTIME
err_t init_uptime(void) {
	calendarcfg_enable();
	RTC->TR = 0;
	//RTC->DR = 0;
	RTC->DR =
		(1U << RTC_DR_MU_Pos) |
		(1U << RTC_DR_DU_Pos)
		;
	calendarcfg_disable();

	return ERR_OK;
}
err_t set_uptime(utime_t uptime_seconds) {
	return _set_RTC_seconds(uptime_seconds);
}
err_t adj_uptime(itime_t adjustment_seconds) {
	return _set_RTC_seconds(_get_RTC_seconds() - adjustment_seconds);
}
utime_t get_uptime(void) {
	return _get_RTC_seconds();
}
#endif


#endif // ! HAVE_STM32F1_RTC
#endif // NEED_RTC
