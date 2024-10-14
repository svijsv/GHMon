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
//   The RTC peripheral consists of two parts, the Real-Time Counter (RTC) (which
//   is used here for waking from sleep) and the Periodic Interrupt Timer (PIT)
//   which is optionally used for the system 1ms tick.
//
//   In principle it's possible to use either the PIT or the RTC for either
//   function (systick and wake-up timer). When used as the wake-up timer, the
//   PIT would allow using a deeper sleep state but we'd have to disable then
//   re-enable both devices (which interferes with PIT timing) in order to make
//   sure we restart the prescaler. With the RTC as the wake-up timer we only
//   lose at most 1ms to prescaler ambiguity so that's not a big problem.
//
//   There's a hardware bug on some devices where writing to RTC.CTRLA resets
//   the device prescaler.
//
//   There's a hardware bug on some devices where writing either the RTC_RTCEN
//   or RTC_PITEN bits to 0 will stop the other device, regardless of the
//   current value of the bit.
//
//   In order to overcome the hardware bugs, it's necessary to keep both the
//   RTC and the PIT permanently enabled. We control them instead by simply
//   disabling and enabling the interrupts, which has the unfortunate side-
//   effect of decreasing the accuracy of our sleep durations (and calibration)
//   because the peripheral prescaler runs continuously.
//
//   My reading of the documentation was that RTC.CNT and PIT both share the same
//   prescaler but it seems that the RTC prescaler which feeds RTC.CNT is what
//   shares the peripheral prescaler.
//

#include "time_RTT.h"


// Try to avoid confusion by referring to the Real Time Counter as the Real
// Time Timer
//static RTC_t* const RTT = &RTC;
#define RTT RTC

// Wait for the RTT or PIT registers to synchronize with the peripheral
#define WAIT_FOR_RTT() do {} while (RTT.STATUS != 0);
#define WAIT_FOR_PIT() do {} while (RTT.PITSTATUS != 0);
// Clear the interrupt flags
#define CLEAR_RTT_INTERRUPTS() (RTT.INTFLAGS = RTC_OVF_bm | RTC_CMP_bm)
#define CLEAR_PIT_INTERRUPTS() (RTT.PITINTFLAGS = RTC_PI_bm)

DEBUG_CPP_MACRO(G_freq_RTTCLK)
DEBUG_CPP_MACRO(RTC_CLKSEL_gc)
DEBUG_CPP_MACRO(RTC_PRESCALER_1KHz_DIV_gc)
DEBUG_CPP_MACRO(RTC_PERIOD_1KHz_CYC_gc)

/*
// The internal prescaler runs continuously while either the RTC or the PIT is
// enabled. In order to reset it, turn both off and then re-enable them.
static reset_RTC_prescaler(void) {
	uint8_t ctrla, pitctrla;

	WAIT_FOR_RTT();
	WAIT_FOR_PIT();
	ctrla = RTC.CTRLA:
	pitctrla = RTC.PITCTRLA:

	RTC.CTRLA = 0;
	RTC.PITCTRLA = 0;
	WAIT_FOR_RTT();
	WAIT_FOR_PIT();

	RTC.CTRLA = ctrla;
	RTC.PITCTRLA = pitctrla;

	return;
}
*/

//
// Set the Real Time Timer to it's default state
void RTT_init(void) {
	// The RTT is used by the device during startup, so make sure it's not busy
	// before doing anything with it
	WAIT_FOR_RTT();
	RTT.CTRLA = 0;
	RTT.CLKSEL = RTC_CLKSEL_gc;
	RTT.INTCTRL = 0;
	CLEAR_RTT_INTERRUPTS();

	WAIT_FOR_PIT();
	RTT.PITCTRLA = 0;
	RTT.PITINTCTRL = 0;
	CLEAR_PIT_INTERRUPTS();

	return;
}

#if SYSTICK_TIMER == TIMER_RTT
# define RTT_PITCTRLA_INIT    (RTC_PERIOD_1KHz_CYC_gc | RTC_PITEN_bm)

void systick_init(void) {
	WAIT_FOR_PIT();

	RTT.PITCTRLA = RTT_PITCTRLA_INIT;
	RTT.PITINTCTRL = 0;
	CLEAR_PIT_INTERRUPTS();

	WAIT_FOR_PIT();

	return;
}
void enable_systick(void) {
	WAIT_FOR_PIT();
	RTT.PITINTFLAGS = RTC_PI_bm;
	RTT.PITINTCTRL = RTC_PI_bm;

	return;
}
void disable_systick(void) {
	WAIT_FOR_PIT();
	RTT.PITINTCTRL = 0;

	return;
}
bool systick_is_enabled(void) {
	return (BIT_IS_SET(RTT.PITINTCTRL, RTC_PI_bm));
}
#endif // SYSTICK_TIMER == TIMER_RTT

#if uHAL_USE_HIBERNATE
//
// Set up RTT calibration
// These values were chosen assuming a ~1KHz clock is used for the RTT/PIT
//
// Don't count sleep periods < this when deciding when to recalibration
# define RTT_CALIBRATE_MINIMUM_MS (512UL)
//
// Calibration period is this many clock cycles
# ifndef RTT_CALIBRATE_CYCLES
#  define RTT_CALIBRATE_CYCLES (512UL)
# endif
//
// This is the expected number of milliseconds during a calibration period
// To minimize errors, this should be a factor of 1000
// Things will break if it's > 1000
//# define RTT_CALIBRATE_MS (500UL)
# define RTT_CALIBRATE_MS ((1000UL * RTT_CALIBRATE_CYCLES) / G_freq_RTTCLK)

//
// Maximum value of the RTT's PER register
# define RTT_PER_MAX (0xFFFFUL)
//
// There's no point in going to deep sleep if the time it takes to wake up/
// go to sleep is longer than the sleep period, so refuse if a period < this
// is requested
# define RTT_PER_MIN (10UL)

# define RTT_CTRLA_INIT (RTC_PRESCALER_1KHz_DIV_gc | RTC_RUNSTDBY_bm | RTC_RTCEN_bm)

DEBUG_CPP_MACRO(RTT_CALIBRATE_MS)

volatile bool wakeup_alarm_is_set = false;
static volatile uint16_t last_wakeup_CNT;
//
// rtt_calibration is the number of milliseconds counted in one RTT_CALIBRATE_CYCLES
// period
static uint16_t rtt_calibration = RTT_CALIBRATE_MS;

static void enable_wakeup_interrupt(void) {
	WAIT_FOR_RTT();
	RTT.INTFLAGS = RTC_OVF_bm;
	RTT.INTCTRL = RTC_OVF_bm;
	return;
}
static void disable_wakeup_interrupt(void) {
	WAIT_FOR_RTT();
	RTT.INTCTRL = 0;
	return;
}

ISR(RTC_CNT_vect) {
	disable_wakeup_interrupt();
	// Write '1' to reset interrupt flags
	RTT.INTFLAGS = RTC_OVF_bm;
	wakeup_alarm_is_set = false;

	// The CNT is 0 (or thereabout) because it rolls over before the ISR is
	// entered.
	last_wakeup_CNT = read_reg16(&RTT.PER) + 1;
}

//
//
// Set up the Real Time Timer used to wake from sleep
void wakeup_alarm_init(void) {
	WAIT_FOR_RTT();

	RTT.CTRLA = RTT_CTRLA_INIT;
	RTT.INTCTRL = 0;

	CLEAR_RTT_INTERRUPTS();

#if RTT_RECALIBRATE_INTERVAL_S > 0
	calibrate_RTT();
#endif

	return;
}
// If the accuracy of this time-keeping mattered more, I could set up an
// independent calibration timer and do more in the RTT ISR
void calibrate_RTT(void) {
#if SYSTICK_TIMER != TIMER_RTT
	utime_t pre_calib, post_calib;
	uint8_t sreg;

	wakeup_alarm_is_set = true;

	WAIT_FOR_RTT();
	write_reg16(&RTT.PER, RTT_CALIBRATE_CYCLES-1U);
	write_reg16(&RTT.CNT, 0);
	ENABLE_INTERRUPTS(sreg);
	//
	// There's an error introduced both here and when the systick count is read
	// again after the calibration period because we don't know where in it's
	// period the systick timer is; hopefully the two errors cancel each other
	// out on balance...
	READ_VOLATILE(pre_calib, G_sys_msticks);
	//
	// The prescaler (NOT 'CNT') restarts from 0 when the the RTT was previously
	// disabled but if it was already enabled it starts from some unknowable
	// position. Unfortunately we need to keep it running in case the PIT is
	// enabled because there's a hardware bug in many of the devices which causes
	// the PIT to stop working when the RTC is disabled (and vice-versa), so
	// there's yet another error introduced here.
	enable_wakeup_interrupt();
	do {
		// Nothing to do here
	} while (wakeup_alarm_is_set);

	READ_VOLATILE(post_calib, G_sys_msticks);
	rtt_calibration = (uint16_t )(post_calib - pre_calib);
	assert(rtt_calibration > 0);

	RESTORE_INTERRUPTS(sreg);

#else // SYSTICK_TIMER != TIMER_RTT
	rtt_calibration = RTT_CALIBRATE_MS;
#endif // SYSTICK_TIMER != TIMER_RTT

	return;
}
void adj_RTT_calibration(int_fast16_t adj) {
	if (adj < 0) {
		uint16_t adj_p = -adj;
		if (adj_p > rtt_calibration) {
			// rtt_calibration == 0 is verboten, we divide by it
			rtt_calibration = 1;
		} else {
			rtt_calibration -= adj_p;
		}
	} else {
		uint16_t adj_p = adj;

		if (0xFFFFU - adj_p > rtt_calibration) {
			rtt_calibration += adj_p;
		} else {
			rtt_calibration = 0xFFFFU;
		}
	}

	return;
}
void set_RTT_calibration(uint_fast16_t cal) {
	if (cal > 0) {
		rtt_calibration = cal;
	}

	return;
}
uint_fast16_t get_RTT_calibration(void) {
	return rtt_calibration;
}

//
// For convenience sake, set_wakeup_alarm()  calibrates the RTT timer so
// it may use up some of the time it says would be slept.
// This may return a time greater than 'ms'.
// If this returns >0 without setting wakeup_alarm_is_set, that means the
// whole sleep period was already used up.
uint16_t set_wakeup_alarm(uint16_t ms) {
	uint16_t per, adjust_ms = 0;

	assert(rtt_calibration > 0);

	wakeup_alarm_is_set = false;

#if RTT_RECALIBRATE_INTERVAL_S > 0
	if (ms >= RTT_CALIBRATE_MINIMUM_MS) {
		static utime_t prev_calib = 0;
		utime_t tmp;

		tmp = get_RTC_seconds();
		if ((tmp - prev_calib) >= RTT_RECALIBRATE_INTERVAL_S) {
			prev_calib = tmp;
			calibrate_RTT();

			adjust_ms = rtt_calibration;
			if (ms > adjust_ms) {
				ms -= adjust_ms;
			} else {
				ms = 0;
				goto END;
			}
		}
	}
#endif

	// Need to make sure PER doesn't overflow, so calculate the maximum number of
	// milliseconds we can sleep without that happening
	if (rtt_calibration < RTT_CALIBRATE_CYCLES) {
		// This will throw a compiler warning about truncation but that would
		// only be an issue when rtt_calibration > RTT_CALIBRATE_CYCLES
		const uint16_t x = ((RTT_PER_MAX * (uint32_t )rtt_calibration) / RTT_CALIBRATE_CYCLES) + 1U;
		if (ms > x) {
			ms = x;
		}
	}

	per = ((ms * RTT_CALIBRATE_CYCLES) / rtt_calibration)-1;
	if (per > 0) {
		wakeup_alarm_is_set = true;

		WAIT_FOR_RTT();
		write_reg16(&RTT.PER, per);
		write_reg16(&RTT.CNT, 0);
		enable_wakeup_interrupt();
	} else {
		ms = 0;
	}

#if RTT_RECALIBRATE_INTERVAL_S > 0
END:
#endif
	return ms + adjust_ms;
}
void stop_wakeup_alarm(void) {
	if (wakeup_alarm_is_set) {
		last_wakeup_CNT = read_reg16(&RTT.CNT);
		disable_wakeup_interrupt();
		wakeup_alarm_is_set = false;
	}

	return;
}
//
// To calculate how much time was remaining:
//    have_time / wanted_time == CNT / PER
//    have_time == (CNT / PER) * wanted_time
//    have_time == (CNT * wanted_time) / PER
//    remaining_time == wanted_time - have_time
uint16_t wakeup_alarm_ms_used(void) {
	uint16_t wanted_ms, per, cnt;

	if (wakeup_alarm_is_set) {
		return 0;
	}
	per = read_reg16(&RTT.PER);
	cnt = last_wakeup_CNT;
	wanted_ms = (uint32_t )((per + 1U) * (uint32_t )rtt_calibration) / RTT_CALIBRATE_CYCLES;

	return (uint32_t )((uint32_t )cnt * (uint32_t )wanted_ms) / per;
}
#endif // uHAL_USE_HIBERNATE
