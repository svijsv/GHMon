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
#ifndef _PLATFORM_LIBOPENCM3_H
#define _PLATFORM_LIBOPENCM3_H

/*
* Includes
*/
#include "ulib/types.h"

#include <libopencmsis/core_cm3.h>
#include <libopencm3/stm32/timer.h>


/*
* Types
*/
typedef struct {
	volatile __IO uint32_t *idr;
	uint32_t mask;
} gpio_quick_t;


/*
* Static values
*/
// 20480 (0x5000) bytes of RAM, starting at address 0x200000000
#define RAM_PRESENT 0x5000
#define RAM_BASE    0x20000000

// Oscillator frequencies
// The reference manual gives a range of 30KHz-60KHz for the LSI
#define G_freq_LSI 40000
#define G_freq_HSI 8000000
#define G_freq_HSE 8000000
#define G_freq_LSE 32768

// 12-bit ADC maximum value
#define ADC_MAX 0x0FFF
// Ideal voltage output by the on-board voltage regulator
#ifndef REGULATED_VOLTAGE
# define REGULATED_VOLTAGE 3300
#endif // REGULATED_VOLTAGE
// Consider the regulated voltage low if it falls below this
#ifndef REGULATED_VOLTAGE_LOW
# define REGULATED_VOLTAGE_LOW 3100
#endif // REGULATED_VOLTAGE_LOW
// Voltage of the internal reference in mV
// Per the STM32F1 datasheet the internal Vref can be between 1.16V and 1.24V,
// with 1.20V being typical.
#ifndef INTERNAL_VREF
# define INTERNAL_VREF 1200
#endif  // INTERNAL_VREF

//
// Base pin definitions
// Only ports A and B are supported; none of the rest are broken out except
// possibly a few on port C that interfere with the RTC
//
// Port A
#define GPIO_PORTA 0x10
#define PA0  (GPIO_PORTA|0x00)
#define PA1  (GPIO_PORTA|0x01)
#define PA2  (GPIO_PORTA|0x02)
#define PA3  (GPIO_PORTA|0x03)
#define PA4  (GPIO_PORTA|0x04)
#define PA5  (GPIO_PORTA|0x05)
#define PA6  (GPIO_PORTA|0x06)
#define PA7  (GPIO_PORTA|0x07)
#define PA8  (GPIO_PORTA|0x08)
#define PA9  (GPIO_PORTA|0x09)
#define PA10 (GPIO_PORTA|0x0A)
#define PA11 (GPIO_PORTA|0x0B)
#define PA12 (GPIO_PORTA|0x0C)
// PA13 and PA14 are used for SWDIO and SWCLK
//#define PA13 (GPIO_PORTA|0x0D)
//#define PA14 (GPIO_PORTA|0x0E)
#define PA15 (GPIO_PORTA|0x0F)
// Port B
#define GPIO_PORTB 0x20
#define PB0  (GPIO_PORTB|0x00)
#define PB1  (GPIO_PORTB|0x01)
// PB2 is reset
//#define PB2  (GPIO_PORTB|0x02)
#define PB3  (GPIO_PORTB|0x03)
#define PB4  (GPIO_PORTB|0x04)
#define PB5  (GPIO_PORTB|0x05)
#define PB6  (GPIO_PORTB|0x06)
#define PB7  (GPIO_PORTB|0x07)
#define PB8  (GPIO_PORTB|0x08)
#define PB9  (GPIO_PORTB|0x09)
#define PB10 (GPIO_PORTB|0x0A)
#define PB11 (GPIO_PORTB|0x0B)
#define PB12 (GPIO_PORTB|0x0C)
#define PB13 (GPIO_PORTB|0x0D)
#define PB14 (GPIO_PORTB|0x0E)
#define PB15 (GPIO_PORTB|0x0F)

#define BIAS_LOW  0x40
#define BIAS_HIGH 0x80


/*
* Variable declarations
*/


/*
* Function prototypes
*/


/*
* Macros
*/
#define GPIO_PIN_MASK  (0x0F)
#define GPIO_GET_PINNO(pin) ((pin) & GPIO_PIN_MASK)
#define GPIO_GET_PINMASK(pin) AS_BIT((uint32_t )GPIO_GET_PINNO((pin)))
#define GPIO_PINNO(no) (no)

#define GPIO_PORT_MASK (0x30)
#define GPIO_GET_PORTNO(pin) (((pin) & GPIO_PORT_MASK) >> 4)
#define GPIO_GET_PORT(pin) ((((pin) & GPIO_PORT_MASK) == GPIO_PORTA) ? GPIOA : GPIOB)

#define GPIO_BIAS_MASK (0xC0)
#define GPIO_GET_BIAS(pin) ((pin) & GPIO_BIAS_MASK)

#define PINID(pin) ((pin) & (GPIO_PIN_MASK | GPIO_PORT_MASK))

#define GPIO_QUICK_READ(qpin) (SELECT_BITS(*((qpin).idr), (qpin).mask) != 0)

// There's a name conflict between my interface and libopencm3, this works
// around that:
#define gpio_set_mode gpio_set_mode_OVERRIDE

# define USCOUNTER_START() \
	do { \
		timer_generate_event(TIM3, TIM_EGR_UG); /* Generate an update event to reset the counter */ \
		timer_enable_counter(TIM3); /* Enable the timer */ \
	} while (0);
# define USCOUNTER_STOP(counter) \
	do { \
		timer_disable_counter(TIM3); /* Disable the timer */ \
		counter = TIM3_CNT; \
	} while (0);


#endif // _PLATFORM_LIBOPENCM3_H
#ifdef __cplusplus
 }
#endif
