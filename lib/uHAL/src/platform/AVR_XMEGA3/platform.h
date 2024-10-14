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
// platform.h
// Platform-specific shim for the frontend
// NOTES:
//   This file is for defining platform-specific features for the frontend
//   that need to be known before configuration and should only be included
//   from there.
//
//   The definitions of the various peripherals and associated macros are in
//   the divice-specific io*.h files in the GCC AVR toolchain.
//
#ifndef _uHAL_PLATFORM_XMEGA3_H
#define _uHAL_PLATFORM_XMEGA3_H

#define PLATFORM_INTERFACE_H "interface/platform/avr_xmega3.h"

#include "ulib/include/time.h"
#include "ulib/include/types.h"

// Needed for RAMSTART and RAMEND
#include <avr/io.h>
// Needed for cli()/sei() in _NOW_MS()
#include <avr/interrupt.h>

// The AVR headers define this as a macro, but that conflicts with an identically-
// named function in ulib when the AVR header is #included first
#undef bit_is_set

#define RAM_PRESENT ((RAMEND - RAMSTART) + 1U)
#define RAM_BASE    RAMSTART
//
// Oscillator frequency
#if ! F_OSC
# if F_CPU
#  define F_OSC F_CPU
//#  warning "Using F_CPU as F_OSC"
# else
// We can't guess at the oscillator frequency because even the internal
// oscillator can be either 16MHz or 20MHz
#  error "F_OSC (the oscillator frequency) must be defined and > 0"
# endif
#endif
#if ! F_CORE
# undef F_CORE
# if F_CPU
#  define F_CORE F_CPU
# else
#  define F_CORE F_OSC
# endif
#endif
#ifndef F_LS_OSC
# define F_LS_OSC 32768UL
#endif
#ifndef uHAL_USE_INTERNAL_OSC
# define uHAL_USE_INTERNAL_OSC 1
#endif
#ifndef uHAL_USE_INTERNAL_LS_OSC
# define uHAL_USE_INTERNAL_LS_OSC 1
#endif

//
// Bus clock frequencies
#define G_freq_CORECLK  F_CORE
#define G_freq_CPUCLK   G_freq_CORECLK
#define G_freq_ADCCLK   G_freq_CORECLK
#define G_freq_IOCLK    G_freq_CORECLK
#define G_freq_UARTCLK  G_freq_IOCLK
#define G_freq_SPICLK   G_freq_IOCLK
#define G_freq_TWICLK   G_freq_IOCLK
#define G_freq_TCACLK   G_freq_IOCLK
#define G_freq_TCBCLK   G_freq_IOCLK

#if ! uHAL_USE_INTERNAL_OSC && RTT_USE_EXTCLK_SRC
# define F_RTT_OSC F_OSC
# define RTC_CLKSEL_gc RTC_CLKSEL_EXTCLK_gc
# include "time_RTT_calc_clkdiv.h"

#elif uHAL_USE_INTERNAL_LS_OSC
# define G_freq_RTTCLK (F_LS_OSC/32U)
# define RTC_CLKSEL_gc RTC_CLKSEL_INT32K_gc
# define RTC_PRESCALER_1KHz_DIV_gc RTC_PRESCALER_DIV32_gc
# define RTC_PERIOD_1KHz_CYC_gc RTC_PERIOD_CYC32_gc

#else
# define F_RTT_OSC F_LS_OSC
# define RTC_CLKSEL_gc RTC_CLKSEL_TOSC32K_gc
# include "time_RTT_calc_clkdiv.h"
#endif

//
// MCU-specific configuration
#define HAVE_AVR_XMEGA3 1
#if defined(__AVR_ATtiny402__)
# define HAVE_AVR_XMEGA3_SERIES_0 1
# include "platform/t402.h"
#else
# error "Unhandled device"
#endif
#include "platform_base_pins.h"

//
// Value returned on ADC conversion error
#define ERR_ADC ((adc_t )-1)
//
// Ideal voltage on the Vcc pin
#if ! REGULATED_VOLTAGE_mV
# define REGULATED_VOLTAGE_mV 5000U
#endif
//
// Voltage of the internal reference in mV
#ifndef INTERNAL_VREF_mV
# define INTERNAL_VREF_mV 1100U
#endif

//
// Ideally this would be in time_private.h but we need the definitions in order
// to calculate SYSTICKS_PER_S for RTC.c.
#define TIMER_NONE      0
#define TIMER_TCA0      1
#define TIMER_TCA0_HIGH 2
#define TIMER_TCA0_LOW  3
#define TIMER_TCB0      4
#define TIMER_TCB1      5
#define TIMER_TCB2      6
#define TIMER_TCB3      7
#define TIMER_RTT       8

#ifndef SYSTICK_TIMER
# define SYSTICK_TIMER TIMER_RTT
#endif
#ifndef SYSTICKS_PER_S
// The default systick timer is not the RTT so we can safely do this check here
// even though we haven't actually set the default yet
# if SYSTICK_TIMER == TIMER_RTT
#  define SYSTICKS_PER_S (G_freq_RTTCLK)
# else
#  define SYSTICKS_PER_S (1000U)
# endif
#endif
#ifndef uHAL_USE_RTC_EMULATION
# define uHAL_USE_RTC_EMULATION uHAL_USE_RTC
#endif


typedef struct {
	register8_t *port;
	uint8_t mask;
} gpio_quick_t;

typedef enum {
	GPIO_MODE_RESET = 0, // Reset state of the pin
	GPIO_MODE_PP,    // Push-pull output
	GPIO_MODE_IN,    // Input
	GPIO_MODE_AIN,   // Analog input
	GPIO_MODE_HiZ,   // High-impedence mode

	GPIO_MODE_RESET_ALIAS = GPIO_MODE_AIN,
	GPIO_MODE_HiZ_ALIAS   = GPIO_MODE_AIN
} gpio_mode_t;
// This allows checking if we have a mode via the preprocessor while still getting
// the benefits that an enum provides
#define GPIO_MODE_RESET GPIO_MODE_RESET
#define GPIO_MODE_PP    GPIO_MODE_PP
#define GPIO_MODE_IN    GPIO_MODE_IN
#define GPIO_MODE_AIN   GPIO_MODE_AIN
#define GPIO_MODE_HiZ   GPIO_MODE_HiZ
#define GPIO_MODE_RESET_ALIAS GPIO_MODE_RESET_ALIAS
#define GPIO_MODE_HiZ_ALIAS   GPIO_MODE_HiZ_ALIAS

#include "platform/common/uart_buf.h"
typedef struct {
	USART_t *uartx;
	gpio_pin_t rx_pin;
	gpio_pin_t tx_pin;
#if UART_INPUT_BUFFER_BYTES > 0 && ENABLE_UART_LISTENING
	volatile uart_buffer_t rx_buf;
#endif
} uart_port_t;

typedef struct {
	register8_t *pinctrl;
	// May be able to save a byte by using bit fields here, but there shouldn't
	// be very many of these structs and this way we don't need to shift any bits
	// so we probably come out ahead.
	uint8_t trigger;
	uint8_t old_trigger;
} gpio_listen_t;

typedef struct {
	gpio_pin_t pin;
} pwm_output_t;

#define GPIO_QUICK_READ(_qpin_) (SELECT_BITS(*((_qpin_).port), (_qpin_).mask) != 0)

extern volatile utime_t G_sys_msticks;
/*
#define NOW_MS() ({ utime_t _n_; READ_VOLATILE(_n_, G_sys_msticks); _n_; })
*/
// There are two advantages to an inline function here:
//   1. It's more portable (the macro uses a GNU extension)
//   2. The compiler may *not* inline it, which can save quite a bit of space for
//      something where a few tens of microseconds in overhead probably don't
//      matter
/*
ALWAYS_INLINE utime_t _NOW_MS(void) {
	utime_t n;

	READ_VOLATILE(n, G_sys_msticks)

	return n;
}
*/
ALWAYS_INLINE utime_t _NOW_MS(void) {
	utime_t n;

	// We can reasonably assume that interrupts are enabled or else there would
	// be no updated systick to check against
#if ! uHAL_SKIP_OTHER_CHECKS
	uint8_t sreg = SREG;
#endif

	cli();
	n = G_sys_msticks;
	sei();

#if ! uHAL_SKIP_OTHER_CHECKS
	SREG = sreg;
#endif

	return n;
}
#define NOW_MS() _NOW_MS()


#endif // _uHAL_PLATFORM_XMEGA3_H
