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
// system.c
// General platform initialization
// NOTES:
//

#include "system.h"
#include "adc.h"
#include "gpio.h"
#include "spi.h"
#include "time.h"
#include "i2c.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay_basic.h>
#include <avr/wdt.h>


void pre_hibernate_hook_caller(utime_t *s, sleep_mode_t *sleep_mode, uHAL_flags_t flags);
void post_hibernate_hook_caller(utime_t s, sleep_mode_t sleep_mode, uHAL_flags_t flags);


#if 0 && DEBUG
// The default for an unexpected ISR is to reset the device; we should go
// into an infinite loop instead to maybe make debugging a bit easier
// https://www.nongnu.org/avr-libc/user-manual/group__avr__interrupts.html
// Edit: Or maybe not, I don't know.
ISR(BADISR_vect) {
	while (1) {
		// Nothing to do here
	}
}
#endif

void platform_init(void) {
	// The watchdog timer remains active after most resets and should be
	// cleared early as a precaution
	// It shouldn't ahve been set anywhere, but doing this won't hurt...
	// https://www.nongnu.org/avr-libc/user-manual/group__avr__watchdog.html
	RSTCTRL.RSTFR = 0;
	wdt_disable();

	// Disable interrupts for peripheral setup
	cli();

	// Set the prescaler before changing the clock source so that if the source
	// is above the maximum supported main clock frequency the system never
	// exceeds it
#if (F_OSC % F_CORE) != 0
# error "Can't obtain F_CORE from F_OSC: F_OSC not divisible by F_CORE"
#elif (F_OSC / F_CORE) == 1
	_PROTECTED_WRITE(CLKCTRL.MCLKCTRLB, 0);
#elif (F_OSC / F_CORE) == 2
	_PROTECTED_WRITE(CLKCTRL.MCLKCTRLB, (CLKCTRL_PDIV_2X_gc | CLKCTRL_PEN_bm));
#elif (F_OSC / F_CORE) == 4
	_PROTECTED_WRITE(CLKCTRL.MCLKCTRLB, (CLKCTRL_PDIV_4X_gc | CLKCTRL_PEN_bm));
#elif (F_OSC / F_CORE) == 6
	_PROTECTED_WRITE(CLKCTRL.MCLKCTRLB, (CLKCTRL_PDIV_6X_gc | CLKCTRL_PEN_bm));
#elif (F_OSC / F_CORE) == 8
	_PROTECTED_WRITE(CLKCTRL.MCLKCTRLB, (CLKCTRL_PDIV_8X_gc | CLKCTRL_PEN_bm));
#elif (F_OSC / F_CORE) == 10
	_PROTECTED_WRITE(CLKCTRL.MCLKCTRLB, (CLKCTRL_PDIV_10X_gc | CLKCTRL_PEN_bm));
#elif (F_OSC / F_CORE) == 12
	_PROTECTED_WRITE(CLKCTRL.MCLKCTRLB, (CLKCTRL_PDIV_12X_gc | CLKCTRL_PEN_bm));
#elif (F_OSC / F_CORE) == 16
	_PROTECTED_WRITE(CLKCTRL.MCLKCTRLB, (CLKCTRL_PDIV_16X_gc | CLKCTRL_PEN_bm));
#elif (F_OSC / F_CORE) == 24
	_PROTECTED_WRITE(CLKCTRL.MCLKCTRLB, (CLKCTRL_PDIV_24X_gc | CLKCTRL_PEN_bm));
#elif (F_OSC / F_CORE) == 32
	_PROTECTED_WRITE(CLKCTRL.MCLKCTRLB, (CLKCTRL_PDIV_32X_gc | CLKCTRL_PEN_bm));
#elif (F_OSC / F_CORE) == 48
	_PROTECTED_WRITE(CLKCTRL.MCLKCTRLB, (CLKCTRL_PDIV_48X_gc | CLKCTRL_PEN_bm));
#elif (F_OSC / F_CORE) == 64
	_PROTECTED_WRITE(CLKCTRL.MCLKCTRLB, (CLKCTRL_PDIV_64X_gc | CLKCTRL_PEN_bm));
#else
# error "Can't obtain F_CORE from F_OSC: No usable prescaler"
#endif
DEBUG_CPP_MACRO(F_OSC / F_CORE);

#if uHAL_USE_INTERNAL_OSC
	// The minimum frequecy OSC20M can produce when set to 16MHz mode is 250KHz
	// and the maximum OSCULP32K can produce is 32.768KHz, so pick a point in
	// between and assume anything outside those theoretical bounds is a result
	// of measuring the actual frequency of the (innaccurate) internal oscillators
# if F_OSC <= 100000
	_PROTECTED_WRITE(CLKCTRL.MCLKCTRLA, CLKCTRL_CLKSEL_OSC_OSCULP32K_gc);
# else
	_PROTECTED_WRITE(CLKCTRL.MCLKCTRLA, CLKCTRL_CLKSEL_OSC20M_gc);
# endif
#else // uHAL_USE_INTERNAL_OSC
	// Some devices can use an external 32KHz crystal as the clock source but
	// it's slightly more complicated to handle and who would need that anyway?
	_PROTECTED_WRITE(CLKCTRL.MCLKCTRLA, CLKCTRL_CLKSEL_EXTCLK_gc);
#endif // uHAL_USE_INTERNAL_OSC

	// Wait for clock source to change
	while (BIT_IS_SET(CLKCTRL.MCLKSTATUS, CLKCTRL_SOSC_bm)) {
		// Nothing to do here
	}

	gpio_init();

	time_init();

#if uHAL_USE_UART_COMM
	const uart_port_cfg_t uart_cfg = {
		.rx_pin = UART_COMM_RX_PIN,
		.tx_pin = UART_COMM_TX_PIN,
		.baud_rate = UART_COMM_BAUDRATE,
	};
	if (uart_init_port(UART_COMM_PORT, &uart_cfg) == ERR_OK) {
		uart_on(UART_COMM_PORT);
		serial_init();
	} else {
		error_state_crude();
	}
#endif

#if uHAL_USE_SPI
	spi_init();
#endif

#if uHAL_USE_I2C
	i2c_init();
#endif

// These ranges for reference voltage selection are just guess work right
// now
#if INTERNAL_VREF_mV <= 800
# define VREF_REFSEL VREF_ADC0REFSEL_0V55_gc
#elif INTERNAL_VREF_mV <= 1300
# define VREF_REFSEL VREF_ADC0REFSEL_1V1_gc
#elif INTERNAL_VREF_mV <= 2000
# define VREF_REFSEL VREF_ADC0REFSEL_1V5_gc
#elif INTERNAL_VREF_mV <= 3500
# define VREF_REFSEL VREF_ADC0REFSEL_2V5_gc
#else // INTERNAL_VREF_mV == 4300
# define VREF_REFSEL VREF_ADC0REFSEL_4V34_gc
#endif
	MODIFY_BITS(VREF.CTRLA, VREF_ADC0REFSEL_gm, VREF_REFSEL);

#if uHAL_USE_ADC
	adc_init();
#endif

	// Enable interrupts
	sei();

	return;
}

void platform_reset(void) {
	pre_reset_hook();
	_PROTECTED_WRITE(RSTCTRL.SWRR, RSTCTRL_SWRE_bm);
}

#if uHAL_USE_HIBERNATE
// set_sleep_mode() must be called with the appropriate argument prior to
// calling _sleep_ms()
static void _sleep_ms(utime_t ms, uint8_t flags, uint_t *wakeups) {
	uint16_t period, try;
	uint8_t sreg;
	uint_t wu = 0;

	SAVE_INTERRUPTS(sreg);
	while (ms != 0) {
		try = (ms < 0xFFFFU) ? ms : 0xFFFFU;
		period = set_wakeup_alarm(try);
		ms = (ms > period) ? (ms - period) : 0;
		// The alarm can't always be set to an exact period, so delay_ms() off any
		// excess
		// Make sure systick is enabled for delay_ms()
		if (period == 0) {
			delay_ms(ms);
			break;
		}

		// The systick is disabled during sleep so that the interrupts don't
		// wake us up
		disable_systick();
		while (wakeup_alarm_is_set && !IRQ_IS_WAITING(flags)) {
			cli();
			sleep_enable();
#if defined(sleep_bod_disable)
			sleep_bod_disable();
#endif
			sei();
			// One last check to make sure we don't accidentally sleep forever...
			if (wakeup_alarm_is_set && !IRQ_IS_WAITING(flags)) {
				sleep_cpu();
			}
			sleep_disable();

			// If desired keep sleeping until the wakeup alarm triggers
			if (wakeup_alarm_is_set) {
				++wu;
				if (IRQ_IS_WAITING(flags)) {
					ms = 0;
					stop_wakeup_alarm();
				}
			}
#if uHAL_USE_RTC && uHAL_USE_RTC_EMULATION
			add_RTC_millis(wakeup_alarm_ms_used());
#endif
		}
		stop_wakeup_alarm();
		enable_systick();
	}
	RESTORE_INTERRUPTS(sreg);

	if (wakeups != NULL) {
		*wakeups = wu;
	}

	return;
}
void sleep_ms(utime_t ms) {
	set_sleep_mode(SLEEP_MODE_IDLE);
	_sleep_ms(ms, 0, NULL);

	return;
}
void hibernate_s(utime_t s, sleep_mode_t sleep_mode, uHAL_flags_t flags) {
	uint8_t set_mode;
	uint_t wakeups;

	pre_hibernate_hook_caller(&s, &sleep_mode, flags);
	const utime_t begin_s = s;

	if (uHAL_CHECK_STATUS(uHAL_FLAG_INHIBIT_HIBERNATION)) {
		sleep_mode = HIBERNATE_LIGHT;
	}
#if uHAL_HIBERNATE_LIMIT
	if (sleep_mode > uHAL_HIBERNATE_LIMIT) {
		sleep_mode = uHAL_HIBERNATE_LIMIT;
	}
#endif

	switch (sleep_mode) {
	case HIBERNATE_LIGHT:
		set_mode = SLEEP_MODE_IDLE;
		break;
	case HIBERNATE_MAX:
		set_mode = SLEEP_MODE_PWR_DOWN;
		break;
//	case HIBERNATE_DEEP:
	default:
		set_mode = SLEEP_MODE_STANDBY;
		break;
	}

	if ((!IRQ_IS_WAITING(flags)) && (s != 0)) {
		//FIXME:
		// When waking from deep sleep, there's a frame error on the RX side of
		// transmitted messages without this pause and I can't determine the
		// reason
#if uHAL_USE_UART
		static const uint8_t delay = 10;
		delay_ms(delay);
#else
		static const uint8_t delay = 0;
#endif
		set_sleep_mode(set_mode);
		_sleep_ms((s*1000U)-delay, flags, &wakeups);
	}

	if (IRQ_IS_REQUESTED) {
		LOGGER("Hibernation ending with uHAL_status at 0x%02X", (uint_t )uHAL_status);
	}
	if (wakeups > 1) {
		LOGGER("Hibernation was interrupted %u times", (uint_t )wakeups);
	}

	post_hibernate_hook_caller(begin_s, sleep_mode, flags);

	return;
}
#endif // uHAL_USE_HIBERNATE

void hibernate(sleep_mode_t sleep_mode, uHAL_flags_t flags) {
	uint8_t set_mode, sreg;

	pre_hibernate_hook_caller(NULL, &sleep_mode, flags);
/*
#if uHAL_USE_HIBERNATE
	if (s != (utime_t )-1) {
		hibernate_s(s, sleep_mode, flags);
		return;
	}
#endif
*/

	if (uHAL_CHECK_STATUS(uHAL_FLAG_INHIBIT_HIBERNATION)) {
		sleep_mode = HIBERNATE_LIGHT;
	}
#if uHAL_HIBERNATE_LIMIT
	if (sleep_mode > uHAL_HIBERNATE_LIMIT) {
		sleep_mode = uHAL_HIBERNATE_LIMIT;
	}
#endif

	switch (sleep_mode) {
	case HIBERNATE_LIGHT:
		set_mode = SLEEP_MODE_IDLE;
		break;
	case HIBERNATE_MAX:
		set_mode = SLEEP_MODE_PWR_DOWN;
		break;
//	case HIBERNATE_DEEP:
	default:
		set_mode = SLEEP_MODE_STANDBY;
		break;
	}

	//FIXME:
	// When waking from deep sleep, there's a frame error on the RX side of
	// transmitted messages without this pause and I can't determine the
	// reason
#if uHAL_USE_UART
	delay_ms(10);
#endif
	set_sleep_mode(set_mode);

	SAVE_INTERRUPTS(sreg);
	disable_systick();
	cli();
	sleep_enable();
#if defined(sleep_bod_disable)
	sleep_bod_disable();
#endif
	sei();
	sleep_cpu();
	sleep_disable();
	enable_systick();
	RESTORE_INTERRUPTS(sreg);

	post_hibernate_hook_caller(0, sleep_mode, flags);

	return;
}
