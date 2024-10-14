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
//
// NOTES:
//   Most of the code is contained the time_X.c files in the same directory
//   as this one.
//
//   The MCU has a peripheral called the 'RTC' which stands for 'Real Time
//   *Counter*'. This file includes emulation for a 'Real Time *Clock*'. To
//   avoid confusion, the 'Real Time *Counter*' is being refered to from now
//   on as the 'Real Time *Timer*', or RTT.
//
//   The RTT interrupt can't wake from power-down sleep mode, but unfortunately
//   I can't get the PIT to work correctly.
//
//   The microsecond counter is... imperfect, and will not give good results
//   when the core clock isn't a multiple of 1000000.
//
//   When a PWM output shares a timer with the systick or microsecond counter,
//   it will be more limited in it's frequency, resolution, and response time.
//
//   When changing a PWM duty cycle, the current on-time may overshoot if the
//   new value is smaller than the old value except when using TCA in single
//   mode in which case there are buffer registers for the compare values. I
//   can see a few possible work-arounds but none are good:
//      * Waiting for the timer to roll over or to be less than the new compare
//      value delays the rest of the program and is still likely to miss for
//      very small compare values
//      * Setting an interrupt to fire and update on roll-over won't work if
//      we e.g. go to sleep beforehand and it still will likey miss for very
//      small compare values
//      * Resetting the counter just guarantees the period will be too long
//   So for any situation where this overshoot is undesireable it's recommended
//   to use TCA in single (16-bit) mode.
//
#include "time_private.h"
#include "time_TCB.h"

#include <util/delay_basic.h>


void time_init(void) {
	RTT_init();
	systick_init();

#if uHAL_USE_USCOUNTER
	uscounter_init();
#endif

#if uHAL_USE_PWM
# if HAVE_TCA0
	TCA_PWM_init(&TCA0);
# endif
# if ! DISABLE_TCB0_PWM
	TCB_PWM_init(&TCB0);
# endif
# if ! DISABLE_TCB1_PWM
	TCB_PWM_init(&TCB1);
# endif
# if ! DISABLE_TCB2_PWM
	TCB_PWM_init(&TCB2);
# endif
# if ! DISABLE_TCB3_PWM
	TCB_PWM_init(&TCB3);
# endif
#endif

	// The wake alarm needs to be initialized after the systick timer because it uses
	// that for calibration
	enable_systick();
#if uHAL_USE_HIBERNATE
	wakeup_alarm_init();
#endif

	return;
}
//
// Delay stuff
//
void delay_ms(utime_t ms) {
	utime_t timer;

	timer = SET_TIMEOUT_MS(ms);
	while (!TIMES_UP(timer)) {
		// Nothing to do here
	}

	return;
}
void dumb_delay_ms(utime_t ms) {
	dumb_delay_cycles((G_freq_CORECLK / 1000U) * ms);
	return;
}
void dumb_delay_cycles(uint32_t cycles) {
	uint32_t count;

	// Probably the overhead of calling the function itself is more than 4
	// cycles, so let's not worry about this
	if (cycles < 4U) {
		return;
	}

	// 4 cycles per iteration in _delay_loop_2()
	count = SHIFT_DIV_4(cycles);
	while (count > 0xFFFFU) {
		_delay_loop_2(0xFFFFU);
		count -= 0xFFFFU;
	}
	if (count > 0) {
		_delay_loop_2(count);
	}

	return;
}
