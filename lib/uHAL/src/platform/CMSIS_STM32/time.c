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
// time.c
// Manage the time-keeping peripherals
// NOTES:
//   Timers 1-4 are the 'primary' PWM timers, they control 4 pins each and
//   correspond to the labeled pins on bluepill/compatible board schematics so
//   when additional timers are present 1-4 should be reserved for PWM
//
//   The timers used for the sleep alarm and the microsecond counter can't be
//   used for PWM
//
//   The actual timers available vary by MCU, only 2 and 3 seem to be universal:
//      STM32F103x6    has timers 1-3
//      STM32F103x[B8] has timers 1-4
//      STM32F103x[EG] has timers 1-8
//   Other STM32 MCUs may have more or less than these
//
//   Some general-purpose timers include more functionality than others, but
//   they all have the same layout in memory so it's usually not necessary
//   to discern between them as long as any configuration bits are being set
//   to the default
//

#define INCLUDED_BY_TIME_C 1
#include "time_private.h"
#include "system.h"
#include "gpio.h"


#if ! SLEEP_ALARM_TIMER && uHAL_USE_HIBERNATE
# error "Unable to determine SLEEP_ALARM_TIMER"
#endif
#if ! USCOUNTER_TIMER && uHAL_USE_USCOUNTER
# error "Unable to determine USCOUNTER_TIMER"
#endif

#if uHAL_USE_HIBERNATE && !defined(SLEEP_ALARM_TIM)
# error "Invalid SLEEP_ALARM_TIMER"
#endif
#if uHAL_USE_USCOUNTER && !defined(USCOUNTER_TIM)
# error "Invalid USCOUNTER_TIMER"
#endif
#if USCOUNTER_TIMER && USCOUNTER_TIMER == SLEEP_ALARM_TIMER
//# error "USCOUNTER_TIMER and SLEEP_ALARM_TIMER must be different"
# warning "USCOUNTER_TIMER and SLEEP_ALARM_TIMER are the same, they can't be used at the same time"
# warning "USCOUNTER_TIMER and SLEEP_ALARM_TIMER are the same, max microsecond count may be very small"
#endif
DEBUG_CPP_MACRO(SLEEP_ALARM_TIMER)
DEBUG_CPP_MACRO(SLEEP_ALARM_IRQn)
DEBUG_CPP_MACRO(SLEEP_ALARM_IRQHandler)
DEBUG_CPP_MACRO(USCOUNTER_TIMER)
DEBUG_CPP_MACRO(USCOUNTER_IRQn)
DEBUG_CPP_MACRO(USCOUNTER_IRQHandler)

// Divide the number of cycles per ms by this in a dumb delay to account for
// overhead
#define DUMB_DELAY_DIV 8U


void time_init(void) {
	systick_init();
	RTC_init();
#if uHAL_USE_PWM
	pwm_init();
#endif
#if uHAL_USE_HIBERNATE
	sleep_alarm_timer_init();
#endif
#if uHAL_USE_USCOUNTER
	uscounter_timer_init();
#endif

	return;
}

void delay_ms(utime_t ms) {
	utime_t timer;

#if ! uHAL_SKIP_OTHER_CHECKS
	if (!systick_is_enabled()) {
# if DEBUG && uHAL_USE_UART_COMM
		if (uHAL_CHECK_STATUS(uHAL_FLAG_SERIAL_IS_UP)) {
			uart_on(UART_COMM_PORT);
			LOGGER("Someone is using delay_ms() without systick...");
		}
# endif
		dumb_delay_ms(ms);
		return;
	}
#endif

	timer = SET_TIMEOUT_MS(ms);
	while (!TIMES_UP(timer)) {
		// Nothing to do here
	}

	return;
}
// https://stackoverflow.com/questions/7083482/how-to-prevent-gcc-from-optimizing-out-a-busy-wait-loop
void dumb_delay_ms(utime_t ms) {
	uint32_t cycles;

	cycles = ms * (G_freq_CORE / (1000U * DUMB_DELAY_DIV));

	for (uint32_t i = 0; i < cycles; ++i) {
		// Count some clock cycles
		__asm__ volatile("" : "+g" (i) : :);
	}

	return;
}
void dumb_delay_cycles(uint_fast32_t cycles) {
	for (uint32_t i = 0; i < cycles; ++i) {
		// Count some clock cycles
		__asm__ volatile("" : "+g" (i) : :);
	}

	return;
}
