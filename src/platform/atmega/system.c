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
// system.c
// General platform initialization
// NOTES:
//

#ifdef __cplusplus
 extern "C" {
#endif

//#define DEBUG 1
/*
* Includes
*/
#include "system.h"
#include "adc.h"
#include "gpio.h"
#include "spi.h"
#include "time.h"
#include "uart.h"
#include "i2c.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>
#include <util/delay_basic.h>


#if (G_freq_OSC / G_freq_CORECLK) != 1 && (G_freq_OSC / G_freq_CORECLK) != 2 && (G_freq_OSC / G_freq_CORECLK) != 4 && (G_freq_OSC / G_freq_CORECLK) != 8
# error "The oscillator frequency must be 1x, 2x, 4x, or 8x the core clock frequency"
#endif

/*
* Static values
*/
#if BUTTON_PIN
# if PINID(BUTTON_PIN) == PINID_D2
#  define BUTTON_ISR INT0_vect
#  define BUTTON_EIMSK 0b01
# elif PINID(BUTTON_PIN) == PINID_D3
#  define BUTTON_ISR INT1_vect
#  define BUTTON_EIMSK 0b10
# else
#  error "Usupported BUTTON_PIN; use PIN_2 or PIN_3"
# endif
#endif // BUTTON_PIN



// CFG_* flags 0xF0 reserved for local use
#define CFG_DEEPSLEEP 0x10


/*
* Types
*/


/*
* Variables
*/
#if USE_TERMINAL
static volatile bool button_wakeup = true;
#endif


/*
* Local function prototypes
*/
static inline __attribute__((always_inline)) \
void sysflash(void);
static void enable_button_ISR(void);
static void disable_button_ISR(void);

/*
* Interrupt handlers
*/
#if DEBUG
// The default for an unexpected ISR is to reset the device; we should go
// into an infinite loop instead to maybe make debugging a bit easier
// https://www.nongnu.org/avr-libc/user-manual/group__avr__interrupts.html
ISR(BADISR_vect) {
	while (1) {
		// Nothing to do here
	}
}
#endif

#if BUTTON_PIN
ISR(BUTTON_ISR) {
	sleep_disable();

	// Let the user know we got the interrupt even if we're not going to do
	// anything for a while
	// Can't use led_flash(), we may not have delay_ms() if we're coming out of
	// hibernate_s() or sleep_ms().
	// led_flash(1, DELAY_ACK);
	sysflash();

	SET_BIT(G_IRQs, BUTTON_IRQf);
#if USE_TERMINAL
	button_wakeup = true;
#endif
}
#endif // BUTTON_PIN

/*
* Functions
*/
void platform_init(void) {
	uint8_t new_psc;

	// The watchdog timer remains active after most resets and should be
	// cleared early as a precaution
	// https://www.nongnu.org/avr-libc/user-manual/group__avr__watchdog.html
	MCUSR = 0;
	wdt_disable();

	// Power off all the peripherals; they will be turned on as needed
	power_all_disable();

	// Disable interrupts for clock setup
	cli();

	//old_psc = SELECT_BITS(CLKPR, 0b1111 << CLKPS0);
	//G_freq_OSC = (uint32_t )F_CPU << old_psc;
	switch (G_freq_OSC / G_freq_CORECLK) {
	case 8:
		new_psc = 0b0011 << CLKPS0;
		break;
	case 4:
		new_psc = 0b0010 << CLKPS0;
		break;
	case 2:
		new_psc = 0b0001 << CLKPS0;
		break;
	case 1:
		new_psc = 0b0000 << CLKPS0;
		break;
	// We made sure G_freq_OSC/G_freq_CORECLK was 8/4/2/1 above
	/*
	default:
		break;
	*/
	}
	CLKPR = _BV(CLKPCE);
	CLKPR = new_psc;

	// Enable interrupts
	sei();

	gpio_init();
	gpio_set_mode(LED_PIN, GPIO_MODE_PP, GPIO_LOW);
	sysflash();

	time_init();
	sysflash();

#if USE_SERIAL
	uart_init();
	sysflash();
#endif

#if USE_SPI
	spi_init();
	sysflash();
#endif

#if USE_I2C
	i2c_init();
	sysflash();
#endif

#if USE_ADC
	adc_init();
	sysflash();
#endif

#if BUTTON_PIN
# if (GPIO_GET_BIAS(BUTTON_PIN) == BIAS_HIGH)
   // Trigger on falling edge
#  define BUTTON_TRIGGER 0b0001
# else
   // Trigger on rising edge
#  define BUTTON_TRIGGER 0b0011
# endif
# if PINID(BUTTON_PIN) == PINID_D2
	SET_BIT(EICRA, BUTTON_TRIGGER); // Trigger INT0
# else
	SET_BIT(EICRA, BUTTON_TRIGGER << 2); // Trigger INT1
# endif
	disable_button_ISR();
	power_on_input(BUTTON_PIN);
# undef BUTTON_TRIGGER
#endif // BUTTON_PIN

	return;
}
static void enable_button_ISR(void) {
#if BUTTON_PIN
	SET_BIT(EIMSK, BUTTON_EIMSK);
#endif
}
static void disable_button_ISR(void) {
#if BUTTON_PIN
	CLEAR_BIT(EIMSK, BUTTON_EIMSK);
#endif
}
static inline __attribute__((always_inline)) \
void sysflash(void) {
#if GPIO_GET_PORT(LED_PIN) == GPIO_PORTB
	SET_BIT(PORTB, GPIO_GET_PINMASK(LED_PIN));
#elif GPIO_GET_PORT(LED_PIN) == GPIO_PORTC
	SET_BIT(PORTC, GPIO_GET_PINMASK(LED_PIN));
#elif GPIO_GET_PORT(LED_PIN) == GPIO_PORTD
	SET_BIT(PORTD, GPIO_GET_PINMASK(LED_PIN));
#endif

	// 65536 loops @ 4 cycles per loop works out to ~1/15th of a second per
	// _delay_loop2() call at 4MHz
	for (uiter_t i = 4; i != 0; --i) {
		_delay_loop_2(0xFFFF);
	}

#if GPIO_GET_PORT(LED_PIN) == GPIO_PORTB
	CLEAR_BIT(PORTB, GPIO_GET_PINMASK(LED_PIN));
#elif GPIO_GET_PORT(LED_PIN) == GPIO_PORTC
	CLEAR_BIT(PORTC, GPIO_GET_PINMASK(LED_PIN));
#elif GPIO_GET_PORT(LED_PIN) == GPIO_PORTD
	CLEAR_BIT(PORTD, GPIO_GET_PINMASK(LED_PIN));
#endif

	// Short delay to keep flashes from bleeding into each other
	_delay_loop_2(0xFFFF);

	return;
}

// set_sleep_mode() must be called with the appropriate argument prior to
// calling _sleep_ms()
OPTIMIZE_FUNCTION \
static void _sleep_ms(utime_t ms, uint8_t flags) {
	uint16_t period, try;

	if (BIT_IS_SET(flags, CFG_ALLOW_INTERRUPTS)) {
		enable_button_ISR();
	}
	while (ms != 0) {
		try = (ms <= 0xFFFF) ? ms : 0xFFFF;
		period = set_wakeup_alarm(try);
		ms = (ms > period) ? (ms - period) : 0;

		// The alarm can't always be set to an exact period, so delay_ms() off any
		// excess
		// Make sure systick is enabled for delay_ms()
		if (period == 0) {
			delay_ms(ms);
			break;
		}

		while (wakeup_alarm_is_set) {
			// The systick is disabled during sleep so that the interrupts don't
			// wake us up
			disable_systick();
			cli();
			sleep_enable();
			sleep_bod_disable();
			sei();
			sleep_cpu();
			sleep_disable();
			enable_systick();

			// If desired keep sleeping until the wakeup alarm triggers
			if (wakeup_alarm_is_set) {
				if (BIT_IS_SET(flags, CFG_ALLOW_INTERRUPTS)) {
					ms = 0;
					break;
				} else {
					sysflash();
					sysflash();
				}
			}
		}
		stop_wakeup_alarm();
	}
	if (BIT_IS_SET(flags, CFG_ALLOW_INTERRUPTS)) {
		disable_button_ISR();
	}

	return;
}
OPTIMIZE_FUNCTION \
void sleep_ms(utime_t ms) {
	set_sleep_mode(SLEEP_MODE_IDLE);
	_sleep_ms(ms, 0);

	return;
}
OPTIMIZE_FUNCTION \
void hibernate_s(utime_t s, uint8_t flags) {
	if (s == 0) {
		return;
	}

#if USE_TERMINAL
	if (button_wakeup && (BIT_IS_SET(flags, CFG_ALLOW_INTERRUPTS))) {
		uint16_t w;

		button_wakeup = false;
		w = ((LIGHT_SLEEP_SECONDS + MIN_DEEP_SLEEP_SECONDS) > s) ? s: LIGHT_SLEEP_SECONDS;
		s -= w;

		NOTIFY("Waiting %u seconds for UART input", (uint )w);
		uart_listen_on();
		set_sleep_mode(SLEEP_MODE_IDLE);
		_sleep_ms(w*1000, flags);
		uart_listen_off();
	} else
#endif // USE_TERMINAL
	if (s < MIN_DEEP_SLEEP_SECONDS) {
		LOGGER("Sleeping lightly %u seconds", (uint )s);
		set_sleep_mode(SLEEP_MODE_IDLE);
		_sleep_ms(s*1000, flags);
		s = 0;
	}

	if ((G_IRQs == 0) && (s != 0)) {
		LOGGER("Sleeping deeply %u seconds", (uint )s);
		set_sleep_mode(SLEEP_MODE_PWR_DOWN);
		_sleep_ms(s*1000, flags);
	}
	if (G_IRQs != 0) {
		LOGGER("Hibernation ending with G_IRQs at 0x%02X", (uint )G_IRQs);
	}

	return;
}


#ifdef __cplusplus
 }
#endif
