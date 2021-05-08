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
#ifndef _PLATFORM_CMSIS_H
#define _PLATFORM_CMSIS_H

/*
* Includes
*/
#include "ulib/types.h"

// CMSIS header file
#include <stm32f103xb.h>


/*
* Types
*/
typedef struct {
	volatile __IO uint32_t *idr;
	uint32_t mask;
} gpio_quick_t;
#define GPIO_QUICK_T_IS_DEFINED 1


/*
* Static values
*/
// 20480 (0x5000) bytes of RAM, starting at address 0x200000000
#define RAM_PRESENT 0x5000
#define RAM_BASE    0x20000000

// Oscillator frequencies, named to mesh with the bus frequency variables
// The reference manual gives a range of 30KHz-60KHz for the LSI
#define G_freq_LSI 40000
#define G_freq_HSI 8000000
#define G_freq_HSE 8000000
#define G_freq_LSE 32768

// 12-bit ADC maximum value
#define ADC_MAX 0x0FFF
// Voltage of the internal reference in mV
// Per the STM32F1 datasheet the internal Vref can be between 1.16V and 1.24V,
// with 1.20V being typical.
#ifndef INTERNAL_VREF_mV
# define INTERNAL_VREF_mV 1200
#endif

//
// Base pin definitions
//
// Only ports A and B are supported; none of the rest are broken out except
// possibly a few on port C that interfere with the RTC
//
// Port A
#define GPIO_PORTA 1
#define GPIO_PORTA_MASK (GPIO_PORTA << GPIO_PORT_OFFSET)
#define PINID_A0  (GPIO_PORTA_MASK|0x00)
#define PINID_A1  (GPIO_PORTA_MASK|0x01)
#define PINID_A2  (GPIO_PORTA_MASK|0x02)
#define PINID_A3  (GPIO_PORTA_MASK|0x03)
#define PINID_A4  (GPIO_PORTA_MASK|0x04)
#define PINID_A5  (GPIO_PORTA_MASK|0x05)
#define PINID_A6  (GPIO_PORTA_MASK|0x06)
#define PINID_A7  (GPIO_PORTA_MASK|0x07)
#define PINID_A8  (GPIO_PORTA_MASK|0x08)
#define PINID_A9  (GPIO_PORTA_MASK|0x09)
#define PINID_A10 (GPIO_PORTA_MASK|0x0A)
#define PINID_A11 (GPIO_PORTA_MASK|0x0B)
#define PINID_A12 (GPIO_PORTA_MASK|0x0C)
// PINID_A13 and PINID_A14 are used for SWDIO and SWCLK
//#define PINID_A13 (GPIO_PORTA_MASK|0x0D)
//#define PINID_A14 (GPIO_PORTA_MASK|0x0E)
#define PINID_A15 (GPIO_PORTA_MASK|0x0F)
//
// Port B
#define GPIO_PORTB 2
#define GPIO_PORTB_MASK (GPIO_PORTB << GPIO_PORT_OFFSET)
#define PINID_B0  (GPIO_PORTB_MASK|0x00)
#define PINID_B1  (GPIO_PORTB_MASK|0x01)
// PINID_B2 is reset
//#define PINID_B2  (GPIO_PORTB_MASK|0x02)
#define PINID_B3  (GPIO_PORTB_MASK|0x03)
#define PINID_B4  (GPIO_PORTB_MASK|0x04)
#define PINID_B5  (GPIO_PORTB_MASK|0x05)
#define PINID_B6  (GPIO_PORTB_MASK|0x06)
#define PINID_B7  (GPIO_PORTB_MASK|0x07)
#define PINID_B8  (GPIO_PORTB_MASK|0x08)
#define PINID_B9  (GPIO_PORTB_MASK|0x09)
#define PINID_B10 (GPIO_PORTB_MASK|0x0A)
#define PINID_B11 (GPIO_PORTB_MASK|0x0B)
#define PINID_B12 (GPIO_PORTB_MASK|0x0C)
#define PINID_B13 (GPIO_PORTB_MASK|0x0D)
#define PINID_B14 (GPIO_PORTB_MASK|0x0E)
#define PINID_B15 (GPIO_PORTB_MASK|0x0F)

//
// Bluepill pin mappings
#define PIN_A0  PINID_A0
#define PIN_A1  PINID_A1
#define PIN_A2  PINID_A2
#define PIN_A3  PINID_A3
#define PIN_A4  PINID_A4
#define PIN_A5  PINID_A5
#define PIN_A6  PINID_A6
#define PIN_A7  PINID_A7
#define PIN_A8  PINID_A8
#define PIN_A9  PINID_A9
#define PIN_A10 PINID_A10
#define PIN_A11 PINID_A11
#define PIN_A12 PINID_A12
//#define PIN_SWDIO PINID_A13
//#define PIN_SWCLK PINID_A14
#define PIN_A15 PINID_A15

#define PIN_B0  PINID_B0
#define PIN_B1  PINID_B1
#define PIN_B3  PINID_B3
#define PIN_B4  PINID_B4
#define PIN_B5  PINID_B5
#define PIN_B6  PINID_B6
#define PIN_B7  PINID_B7
#define PIN_B8  PINID_B8
#define PIN_B9  PINID_B9
#define PIN_B10 PINID_B10
#define PIN_B11 PINID_B11
#define PIN_B12 PINID_B12
#define PIN_B13 PINID_B13
#define PIN_B14 PINID_B14
#define PIN_B15 PINID_B15


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
#define GPIO_GET_PORT(pin) (((GPIO_GET_PORTMASK(pin)) == GPIO_PORTA_MASK) ? GPIOA : GPIOB)
#define GPIO_QUICK_READ(qpin) (SELECT_BITS(*((qpin).idr), (qpin).mask) != 0)

// Quick pin access, for when you know what you want:
#define IS_GPIO_INPUT_HIGH(pin)  (BIT_IS_SET(GPIO_GET_PORT(pin)->IDR, GPIO_GET_PINMASK(pin)))
#define SET_GPIO_OUTPUT_HIGH(pin) (GPIO_GET_PORT(pin)->BSRR = GPIO_GET_PINMASK(pin))
#define SET_GPIO_OUTPUT_LOW(pin)  (GPIO_GET_PORT(pin)->BRR  = GPIO_GET_PINMASK(pin))

// Use the micro-second counter
# define USCOUNTER_START() \
	do { \
		SET_BIT(TIM3->EGR, TIM_EGR_UG); /* Generate an update event to reset the counter */ \
		SET_BIT(TIM3->CR1, TIM_CR1_CEN); /* Enable the timer */ \
	} while (0);
# define USCOUNTER_STOP(counter) \
	do { \
		CLEAR_BIT(TIM3->CR1, TIM_CR1_CEN); /* Disable the timer */ \
		(counter) = TIM3->CNT; \
	} while (0);

#endif // _PLATFORM_CMSIS_H
#ifdef __cplusplus
 }
#endif
