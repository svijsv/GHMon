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
// time_RTC_STM32Fx.h
// Manage the non-STM32F1 RTC
// NOTES:
//    This was split off from time.c because the RTC peripherals differ so
//    much between the STM32F1 and the other lines that code can't really be
//    reused
//
//    This file should only be included by time.c
//
//    Wakeup uses RTC alarm A
//
//    The time is stored internally in BCD format
//
#if INCLUDED_BY_TIME_C

/*
* Includes
*/


/*
* Static values
*/
#define RTC_DR_DATE_MASK (RTC_DR_YT|RTC_DR_YU|RTC_DR_MT|RTC_DR_MU|RTC_DR_DT|RTC_DR_DU)
#define RTC_TR_TIME_MASK (RTC_TR_HT|RTC_TR_HU|RTC_TR_MNT|RTC_TR_MNU|RTC_TR_ST|RTC_TR_SU)


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

	// Clear the alarm interrupt enable flag
	CLEAR_BIT(RTC->ISR, RTC_ISR_ALRAF);
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
// These devices use BCD, I use seconds, so heres the conversion...
// Partially copied from the ST HAL
static uint8_t byte_to_bcd(uint8_t byte) {
	uint32_t tmp;

	tmp = (uint8_t )(byte / 10U) << 4U;
	return tmp | (byte % 10U);
}
static uint8_t bcd_to_byte(uint8_t bcd) {
	uint32_t tmp;

	tmp = ((uint8_t )(bcd & (uint8_t )0xF0) >> (uint8_t )0x4) * 10U;
	return (tmp + (bcd & (uint8_t )0x0F));
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
		RTC->WPR = 0xCA;
		RTC->WPR = 0x53;
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
		RTC->WPR = 0x00;

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
static void RTC_init(void) {
	cfg_enable();
	// Backup domain register writes are enabled by cfg_enable()
	//BD_write_enable();
	MODIFY_BITS(RCC->BDCR, RCC_BDCR_RTCSEL|RCC_BDCR_RTCEN,
		RCC_BDCR_RTCSEL_LSE | // Use the LSE as the clock source
		RCC_BDCR_RTCEN      | // Enable the RTC
		0);
	//BD_write_disable();

	// Calendar initialization procedure as per the reference
	// The defaults are fine though
	/*
	if (!BIT_IS_SET(RTC->ISR, RTC_ISR_INITS)) {
		SET_BIT(RTC->ISR, RTC_ISR_INIT);
		while (!BIT_IS_SET(RTC->ISR, RTC_ISR_INITF)) {
			// Nothing to do here
		}

		// RTC_PRER needs to be modified with two separate writes
		// The reset values for the prescalers are already those suggested
		// in the reference manual (127 and 255)
		MODIFY_BITS(RTC->PRER, RTC_PRER_PREDIV_S,
			255 << RTC_PRER_PREDIV_S_Pos
			);
		MODIFY_BITS(RTC->PRER, RTC_PRER_PREDIV_A,
			127 << RTC_PRER_PREDIV_A_Pos
			);

		// 24-hour format
		CLEAR_BIT(RTC->CR, RTC_CR_FMT);

		CLEAR_BIT(RTC->ISR, RTC_ISR_INIT);
	}
	*/
	wait_for_sync();

	cfg_disable();
	NVIC_SetPriority(RTC_Alarm_IRQn, RTC_ALARM_IRQp);

	return;
}
utime_t get_RTC_seconds(void) {
	uint32_t tr, dr;
	uint8_t hour, minute, second;
	uint8_t year, month, day;

	wait_for_sync();

	// Use local variables to avoid repeatedly accessing the volatile registers
	// Reading TR locks DR until it's read
	tr = RTC->TR;
	dr = RTC->DR;

	hour   = bcd_to_byte(GATHER_BITS(tr, 0x3F, RTC_TR_HU_Pos));
	minute = bcd_to_byte(GATHER_BITS(tr, 0x7F, RTC_TR_MNU_Pos));
	second = bcd_to_byte(GATHER_BITS(tr, 0x7F, RTC_TR_SU_Pos));

	year  = bcd_to_byte(GATHER_BITS(dr, 0xFF, RTC_DR_YU_Pos));
	month = bcd_to_byte(GATHER_BITS(dr, 0x1F, RTC_DR_MU_Pos));
	day   = bcd_to_byte(GATHER_BITS(dr, 0x3F, RTC_DR_DU_Pos));

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
err_t set_RTC_seconds(utime_t s) {
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

	return EOK;
}

void set_RTC_alarm(utime_t time) {
	uint32_t tr, tmp;
	uint8_t hour, minute, second;

	assert(time <= HIBERNATE_MAX_S);

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
	hour   = bcd_to_byte(GATHER_BITS(tr, 0x3F, RTC_TR_HU_Pos));
	minute = bcd_to_byte(GATHER_BITS(tr, 0x7F, RTC_TR_MNU_Pos));
	second = bcd_to_byte(GATHER_BITS(tr, 0x7F, RTC_TR_SU_Pos));

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
		(0b1    << RTC_ALRMAR_MSK4_Pos) | // Ignore date part of the alarm
		(0b0    << RTC_ALRMAR_MSK3_Pos) | // Use hour
		(hour   << RTC_ALRMAR_HU_Pos)   | // Set hour
		(0b0    << RTC_ALRMAR_MSK2_Pos) | // Use minute
		(minute << RTC_ALRMAR_MNU_Pos)  | // Set minute
		(0b0    << RTC_ALRMAR_MSK1_Pos) | // Use second
		(second << RTC_ALRMAR_SU_Pos)   | // Set second
		0);

	// Clear the alarm interrupt flag
	CLEAR_BIT(RTC->ISR, RTC_ISR_ALRAF);
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

#endif // INCLUDED_BY_TIME_C
