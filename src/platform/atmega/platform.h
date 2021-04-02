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
// platform.h
// Platform-specific shim for the frontend
// NOTES:
//   This file is for defining platform-specific features for the frontend
//   that need to be known before configuration and should only be included
//   from there.
//

#ifdef __cplusplus
 extern "C" {
#endif
#ifndef _PLATFORM_H
#define _PLATFORM_H

/*
* Includes
*/
// Needed for SHIFT_MUL|DIV_*()
#include "ulib/math.h"

// Needed for RAMSTART and RAMEND
#include <avr/io.h>

/*
* Types
*/
typedef struct {
	volatile uint8_t *port;
	uint8_t mask;
} gpio_quick_t;


/*
* Static values
*/
#define RAM_PRESENT ((RAMEND - RAMSTART) + 1)
#define RAM_BASE    RAMSTART

// External oscillator frequency
#define G_freq_OSC F_CPU
// Bus clock frequencies
// The system clock is set to G_freq_CORECLK in platform_init() with the
// assumption that F_CPU (by way of G_freq_OSC) is the frequency of the
// external oscillator
#define G_freq_CORECLK 4000000UL
#define G_freq_CPUCLK G_freq_CORECLK
#define G_freq_IOCLK  G_freq_CORECLK
#define G_freq_ADCCLK G_freq_CORECLK
#define G_freq_UARTCLK G_freq_IOCLK
#define G_freq_SPICLK G_freq_IOCLK
#define G_freq_TIM01CLK G_freq_IOCLK
#define G_freq_TIM2CLK G_freq_IOCLK

// 10-bit ADC maximum value
// Per the reference manual, the max value actually represents the voltage
// on ARef minux 1 LSB
#define ADC_MAX (0x03FF+1)
// Ideal voltage output by the on-board voltage regulator
#ifndef REGULATED_VOLTAGE
# if BOARD_VCC
#  define REGULATED_VOLTAGE BOARD_VCC
# elif F_CPU > 8000000
#  define REGULATED_VOLTAGE 5000
# else
#  define REGULATED_VOLTAGE 3300
# endif
#endif
// Consider the regulated voltage low if it falls below this
#ifndef REGULATED_VOLTAGE_LOW
# define REGULATED_VOLTAGE_LOW (REGULATED_VOLTAGE-(REGULATED_VOLTAGE/20))
#endif
// Voltage of the internal reference in mV
#ifndef INTERNAL_VREF
# define INTERNAL_VREF 1100
#endif


//
// Base pin definitions
// Only ports A and B are supported; none of the rest are broken out except
// possibly a few on port C that interfere with the RTC
//
// There is no port A for the ATMega328
//
// Port B
#define GPIO_PORTB 0x20
#define PINID_B0  (GPIO_PORTB|0x00)
#define PINID_B1  (GPIO_PORTB|0x01)
#define PINID_B2  (GPIO_PORTB|0x02)
#define PINID_B3  (GPIO_PORTB|0x03)
#define PINID_B4  (GPIO_PORTB|0x04)
#define PINID_B5  (GPIO_PORTB|0x05)
//#define PINID_B6  (GPIO_PORTB|0x06) // PB6 and PB7 are the oscillator
//#define PINID_B7  (GPIO_PORTB|0x07)
//
// Port C
// Port C only has 7 pins
#define GPIO_PORTC 0x40
#define PINID_C0  (GPIO_PORTC|0x00)
#define PINID_C1  (GPIO_PORTC|0x01)
#define PINID_C2  (GPIO_PORTC|0x02)
#define PINID_C3  (GPIO_PORTC|0x03)
#define PINID_C4  (GPIO_PORTC|0x04)
#define PINID_C5  (GPIO_PORTC|0x05)
//#define PINID_C6  (GPIO_PORTC|0x06) // PC6 is the reset pin
//
// Port D
#define GPIO_PORTD 0x60
#define PINID_D0  (GPIO_PORTD|0x00)
#define PINID_D1  (GPIO_PORTD|0x01)
#define PINID_D2  (GPIO_PORTD|0x02)
#define PINID_D3  (GPIO_PORTD|0x03)
#define PINID_D4  (GPIO_PORTD|0x04)
#define PINID_D5  (GPIO_PORTD|0x05)
#define PINID_D6  (GPIO_PORTD|0x06)
#define PINID_D7  (GPIO_PORTD|0x07)
//
// Port Z
// This port isn't real, it's used for analog pins 6 and 7 which have no
// digital buffers/GPIO mappings
#define GPIO_PORTZ 0x80
#define PINID_Z0  (GPIO_PORTZ|0x00)
#define PINID_Z1  (GPIO_PORTZ|0x01)
//
// Arduino digital pin mappings
#define PIN_RX PINID_D0 // UART RX
#define PIN_TX PINID_D1 // UART TX
#define PIN_2  PINID_D2
#define PIN_3  PINID_D3
#define PIN_4  PINID_D4
#define PIN_5  PINID_D5
#define PIN_6  PINID_D6
#define PIN_7  PINID_D7
#define PIN_8  PINID_B0
#define PIN_9  PINID_B1
#define PIN_10 PINID_B2 // SPI SS
#define PIN_11 PINID_B3 // SPI MOSI
#define PIN_12 PINID_B4 // SPI MISO
#define PIN_13 PINID_B5 // SPI SCK
//
// Arduino analog pin mappings
#define PIN_A0 PINID_C0
#define PIN_A1 PINID_C1
#define PIN_A2 PINID_C2
#define PIN_A3 PINID_C3
#define PIN_A4 PINID_C4
#define PIN_A5 PINID_C5
#define PIN_A6 PINID_Z0
#define PIN_A7 PINID_Z1

#define BIAS_LOW  0x08
#define BIAS_HIGH 0x10


/*
* Variable declarations
*/


/*
* Function prototypes
*/


/*
* Macros
*/
#define GPIO_PIN_MASK  (0x07)
#define GPIO_GET_PINNO(pin) ((pin) & GPIO_PIN_MASK)
#define GPIO_GET_PINMASK(pin) AS_BIT((uint32_t )GPIO_GET_PINNO((pin)))
// This is in case the pin number is ever anything but it's masked value
#define GPIO_PINNO(no) (no)

#define GPIO_PORT_MASK (0xE0)
#define GPIO_GET_PORTNO(pin) (((pin) & GPIO_PORT_MASK) >> 5)
#define GPIO_GET_PORT(pin) ((pin) & GPIO_PORT_MASK)

#define GPIO_BIAS_MASK (0x18)
//#define GPIO_GET_BIAS(pin) ((((pin) & GPIO_BIAS_MASK) == BIAS_HIGH) ? GPIO_HIGH : (((pin) & GPIO_BIAS_MASK) == BIAS_LOW) ? GPIO_LOW : GPIO_FLOAT)
#define GPIO_GET_BIAS(pin) ((pin) & GPIO_BIAS_MASK)

#define PINID(pin) ((pin) & (GPIO_PIN_MASK | GPIO_PORT_MASK))

#define GPIO_QUICK_READ(qpin) (SELECT_BITS(*((qpin).port), (qpin).mask) != 0)

#define NOW_MS() ({ utime_t n; READ_VOLATILE(n, G_sys_uticks); n; })

// Ideally the prescaler would just be set to the highest value and the number
// of micro-seconds determined by prescaler counter * 0xff + timer counter,
// but the prescaler can't be read
// Instead, try to set the prescaler such that each timer counter corresponds
// to one micro-second - this is only possible at 8MHz and 1MHz though, so
// the rest either have a smaller range or a reduced accuracy.
#if G_freq_TIM01CLK == 16000000
# define USCOUNTER_START() \
	do { \
		TCNT0 = 0; \
		MODIFY_BITS(TCCR0B, 0b111, 0b010); /* Prescaler 8 */ \
	} while (0);
# define USCOUNTER_STOP(counter) \
	do { \
		CLEAR_BIT(TCCR0B, 0b111); \
		counter = SHIFT_DIV_2(TCNT0); \
	} while (0);
#elif G_freq_TIM01CLK == 8000000
# define USCOUNTER_START() \
	do { \
		TCNT0 = 0; \
		MODIFY_BITS(TCCR0B, 0b111, 0b010); /* Prescaler 8 */ \
	} while (0);
# define USCOUNTER_STOP(counter) \
	do { \
		CLEAR_BIT(TCCR0B, 0b111); \
		counter = TCNT0; \
	} while (0);
#elif G_freq_TIM01CLK == 4000000
# define USCOUNTER_START() \
	do { \
		TCNT0 = 0; \
		MODIFY_BITS(TCCR0B, 0b111, 0b010); /* Prescaler 8 */ \
	} while (0);
# define USCOUNTER_STOP(counter) \
	do { \
		CLEAR_BIT(TCCR0B, 0b111); \
		counter = SHIFT_MUL_2(TCNT0); \
	} while (0);
#elif G_freq_TIM01CLK == 2000000
# define USCOUNTER_START() \
	do { \
		TCNT0 = 0; \
		MODIFY_BITS(TCCR0B, 0b111, 0b001); /* Prescaler 1 */ \
	} while (0);
# define USCOUNTER_STOP(counter) \
	do { \
		CLEAR_BIT(TCCR0B, 0b111); \
		counter = SHIFT_DIV_2(TCNT0); \
	} while (0);
#elif G_freq_TIM01CLK == 1000000
# define USCOUNTER_START() \
	do { \
		TCNT0 = 0; \
		MODIFY_BITS(TCCR0B, 0b111, 0b001); /* Prescaler 1 */ \
	} while (0);
# define USCOUNTER_STOP(counter) \
	do { \
		CLEAR_BIT(TCCR0B, 0b111); \
		counter = TCNT0; \
	} while (0);
#else
# error "CPU frequency must be 16MHz, 8MHz, 4MHz, 2MHz, or 1MHz"
#endif


#endif // _PLATFORM_H
#ifdef __cplusplus
 }
#endif
