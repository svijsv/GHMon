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
//

#include "time_private.h"

#if uHAL_USE_USCOUNTER

#if IS_TCA0(USCOUNTER_TIMER)
# include "time_TCA.h"
#elif IS_TCB(USCOUNTER_TIMER)
# include "time_TCB.h"
#endif

DEBUG_CPP_MACRO(USCOUNTER_TIMER)

// Using the same peripheral for the systick and micro-second timers wouldn't
// work because they both need to set the prescaler somewhat precisely
#if USCOUNTER_TIMER == SYSTICK_TIMER
# error "USCOUNTER_TIMER and SYSTICK_TIMER can't be the same peripheral"
#endif

#define USCOUNTER_TICKS_PER_uS ((G_freq_CORECLK / USCOUNTER_CLKDIV) / 1000000U)
#if USCOUNTER_TICKS_PER_uS <= 0
# error "Unable to establish USCOUNTER_TIMER with this core frequency/timer combination"
#endif
//DEBUG_CPP_MACRO(USCOUNTER_CLKDIV)
DEBUG_CPP_MACRO(USCOUNTER_TICKS_PER_uS)
DEBUG_CPP_MACRO(USCOUNTER_MAX)

// The number of clock rollovers during a micro-second counter run
#if USCOUNTER_MAX <= 0xFF
static volatile uint16_t uscounter_overflows = 0;
#else
static volatile uint8_t uscounter_overflows = 0;
#endif

ISR(USCOUNTER_ISR) {
	CLEAR_USCOUNTER_INTFLAG();
	++uscounter_overflows;
}

err_t uscounter_on(void) {
	return ERR_OK;
}
err_t uscounter_start(void) {
	uscounter_start_timer();
	//CLEAR_USCOUNTER_INTFLAG();
	uscounter_overflows = 0;

	return ERR_OK;
}
err_t uscounter_off(void) {
	uscounter_stop_timer();
	return ERR_OK;
}
static uint_fast32_t adjust_cnt(uint16_t cnt, uint32_t overflows) {
	uint_fast32_t counter;

	//counter = (overflows * (uint32_t )USCOUNTER_MAX) / USCOUNTER_TICKS_PER_uS;
	counter = overflows * (uint32_t )(USCOUNTER_MAX / USCOUNTER_TICKS_PER_uS);
#if USCOUNTER_IS_DOWNCOUNT
	counter += (USCOUNTER_MAX - cnt) / USCOUNTER_TICKS_PER_uS;
#else
	counter += cnt / USCOUNTER_TICKS_PER_uS;
#endif

	return counter;
}
uint_fast32_t uscounter_stop(void) {
	uint_fast32_t overflows;
	uint16_t cnt;

	cnt = uscounter_stop_timer();
	overflows = uscounter_overflows;

	return (adjust_cnt(cnt, overflows));
}
uint_fast32_t uscounter_read(void) {
	uint_fast32_t overflows;
	uint16_t cnt;

	// Stopping and starting such a sensitive timer isn't ideal but there's a
	// race condition when reading uscounter_overflows otherwise
	cnt = uscounter_pause_timer();
	overflows = uscounter_overflows;
	uscounter_resume_timer();
	return (adjust_cnt(cnt, overflows));
}


#endif // uHAL_USE_USCOUNTER
