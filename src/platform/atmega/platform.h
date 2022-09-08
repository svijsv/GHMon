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
#ifndef _PLATFORM_ATMEGA_H
#define _PLATFORM_ATMEGA_H

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
#define GPIO_QUICK_T_IS_DEFINED 1

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
#define G_freq_CORECLK  4000000UL
#define G_freq_CPUCLK   G_freq_CORECLK
#define G_freq_IOCLK    G_freq_CORECLK
#define G_freq_ADCCLK   G_freq_CORECLK
#define G_freq_UARTCLK  G_freq_IOCLK
#define G_freq_SPICLK   G_freq_IOCLK
#define G_freq_TIM01CLK G_freq_IOCLK
#define G_freq_TIM2CLK  G_freq_IOCLK

// 10-bit ADC maximum value
// Per the reference manual, the max value actually represents the voltage
// on ARef minux 1 LSB
#define ADC_MAX (0x03FF+1)
// Ideal voltage output by the on-board voltage regulator
#if ! REGULATED_VOLTAGE_mV
# if BOARD_VCC
#  define _REGULATED_VOLTAGE_mV BOARD_VCC
# elif F_CPU > 8000000
#  define _REGULATED_VOLTAGE_mV 5000
# else
#  define _REGULATED_VOLTAGE_mV 3300
# endif
#endif
// Voltage of the internal reference in mV
#ifndef INTERNAL_VREF_mV
# define _INTERNAL_VREF_mV 1100
#endif


//
// Base pin definitions
//
// There is no port A for the ATMega328
//
// The normal pin/port/bias masks won't do because those only allow for 3
// ports and we need 4
#define GPIO_PIN_MASK  (0x07)
#define GPIO_PORT_MASK (0xE0)
#define GPIO_BIAS_MASK (0x18)
#define GPIO_PORT_OFFSET 5
#define GPIO_BIAS_OFFSET 3
//
// Port B
#define GPIO_PORTB 1
#define GPIO_PORTB_MASK (GPIO_PORTB << GPIO_PORT_OFFSET)
#define PINID_B0  (GPIO_PORTB_MASK|0x00)
#define PINID_B1  (GPIO_PORTB_MASK|0x01)
#define PINID_B2  (GPIO_PORTB_MASK|0x02)
#define PINID_B3  (GPIO_PORTB_MASK|0x03)
#define PINID_B4  (GPIO_PORTB_MASK|0x04)
#define PINID_B5  (GPIO_PORTB_MASK|0x05)
//#define PINID_B6  (GPIO_PORTB_MASK|0x06) // PB6 and PB7 are the oscillator
//#define PINID_B7  (GPIO_PORTB_MASK|0x07)
//
// Port C
// Port C only has 7 pins
#define GPIO_PORTC 2
#define GPIO_PORTC_MASK (GPIO_PORTC << GPIO_PORT_OFFSET)
#define PINID_C0  (GPIO_PORTC_MASK|0x00)
#define PINID_C1  (GPIO_PORTC_MASK|0x01)
#define PINID_C2  (GPIO_PORTC_MASK|0x02)
#define PINID_C3  (GPIO_PORTC_MASK|0x03)
#define PINID_C4  (GPIO_PORTC_MASK|0x04)
#define PINID_C5  (GPIO_PORTC_MASK|0x05)
//#define PINID_C6  (GPIO_PORTC_MASK|0x06) // PC6 is the reset pin
//
// Port D
#define GPIO_PORTD 3
#define GPIO_PORTD_MASK (GPIO_PORTD << GPIO_PORT_OFFSET)
#define PINID_D0  (GPIO_PORTD_MASK|0x00)
#define PINID_D1  (GPIO_PORTD_MASK|0x01)
#define PINID_D2  (GPIO_PORTD_MASK|0x02)
#define PINID_D3  (GPIO_PORTD_MASK|0x03)
#define PINID_D4  (GPIO_PORTD_MASK|0x04)
#define PINID_D5  (GPIO_PORTD_MASK|0x05)
#define PINID_D6  (GPIO_PORTD_MASK|0x06)
#define PINID_D7  (GPIO_PORTD_MASK|0x07)
//
// Port Z
// This port isn't real, it's used for analog pins 6 and 7 which have no
// digital buffers/GPIO mappings
#define GPIO_PORTZ 4
#define GPIO_PORTZ_MASK (GPIO_PORTZ << GPIO_PORT_OFFSET)
#define PINID_Z0  (GPIO_PORTZ_MASK|0x00)
#define PINID_Z1  (GPIO_PORTZ_MASK|0x01)
//
// Alternate function pins
// Communication
#define PINID_RX0  PINID_D0
#define PINID_TX0  PINID_D1
#define PINID_MISO PINID_B4
#define PINID_MOSI PINID_B3
#define PINID_SCK  PINID_B5
#define PINID_SS   PINID_B2
#define PINID_SCL  PINID_C5
#define PINID_SDA  PINID_C4
// Timer output/PWM
#define PINID_OC0A PINID_D6
#define PINID_OC0B PINID_D5
#define PINID_OC1A PINID_B1
#define PINID_OC1B PINID_B2
#define PINID_OC2A PINID_B3
#define PINID_OC2B PINID_D3
// ADC input
#define PINID_ADC0 PINID_C0
#define PINID_ADC1 PINID_C1
#define PINID_ADC2 PINID_C2
#define PINID_ADC3 PINID_C3
#define PINID_ADC4 PINID_C4
#define PINID_ADC5 PINID_C5
#define PINID_ADC6 PINID_Z0
#define PINID_ADC7 PINID_Z1
//
// Arduino digital pin mappings
#define PIN_0  PINID_D0
#define PIN_RX PINID_D0 // UART RX
#define PIN_1  PINID_D1
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


/*
* Variable declarations
*/
extern volatile utime_t G_sys_msticks;


/*
* Function prototypes
*/


/*
* Macros
*/
#define GPIO_QUICK_READ(qpin) (SELECT_BITS(*((qpin).port), (qpin).mask) != 0)

#define NOW_MS() ({ utime_t n; READ_VOLATILE(n, G_sys_msticks); n; })

// Use the micro-second counter
// Setting the prescaler starts the timer, so it needs to be reset on every
// call
// Try to set the prescaler such that each counter tick corresponds to one
// micro-second - this is only possible at 8MHz and 1MHz though, so the rest
// either have a smaller range or a reduced accuracy
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


#endif // _PLATFORM_ATMEGA_H
#ifdef __cplusplus
 }
#endif
