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

// CMSIS header files
// Both ulib/bits.h and stm32fXxx.h define the SET_BIT() and CLEAR_BIT()
// macros
#pragma push_macro("SET_BIT")
#pragma push_macro("CLEAR_BIT")
#undef SET_BIT
#undef CLEAR_BIT
#if defined(STM32F1)
# include <stm32f1xx.h>
# define CMSIS_VERSION __STM32F1_CMSIS_VERSION
# define CMSIS_NAME "STM32F1"
# define USE_STM32F1_ADC    1
# define USE_STM32F1_GPIO   1
# define USE_STM32F1_RTC    1

#elif defined(STM32F4xx)
# include <stm32f4xx.h>
# define CMSIS_VERSION __STM32F4xx_CMSIS_VERSION
# define CMSIS_NAME "STM32F4xx"

// There's a typo in stm32f4xx.h that breaks compilation when using the
// version number
# undef __STM32F4xx_CMSIS_VERSION
# define __STM32F4xx_CMSIS_VERSION ((__STM32F4xx_CMSIS_VERSION_MAIN << 24)\
                                   |(__STM32F4xx_CMSIS_VERSION_SUB1 << 16)\
                                   |(__STM32F4xx_CMSIS_VERSION_SUB2 << 8 )\
                                   |(__STM32F4xx_CMSIS_VERSION_RC))

#else
# error "Unsupported architecture"
#endif
#pragma pop_macro("SET_BIT")
#pragma pop_macro("CLEAR_BIT")
// There are macros with the same name as these but different functions in
// use
#undef READ_REG
#undef WRITE_REG


/*
* Types
*/
typedef struct {
	volatile __IO uint32_t *idr;
	uint32_t mask;
} gpio_quick_t;
#define GPIO_QUICK_T_IS_DEFINED 1
typedef enum {
	GPIO_MODE_RESET = 0, // Reset state of the pin
	GPIO_MODE_PP,    // Push-pull output
	GPIO_MODE_PP_AF, // Alternate-function push-pull output
	GPIO_MODE_OD,    // Open-drain output
	GPIO_MODE_OD_AF, // Alternate-function open-drain output
	GPIO_MODE_IN,    // Input
	GPIO_MODE_IN_AF, // Alternate-function input
	GPIO_MODE_AIN,   // Analog input
	GPIO_MODE_HiZ    // High-impedence mode
} gpio_mode_t;
#define GPIO_MODE_T_IS_DEFINED 1


/*
* Static values
*/
#if ! RAM_PRESENT
// 20480 (0x5000) bytes of RAM
# define RAM_PRESENT 0x5000
#endif
#define RAM_BASE SRAM_BASE

//
// Oscillator frequencies
#if defined(STM32F1)
// The reference manual gives a range of 30KHz-60KHz for the LSI on the F1
# define G_freq_LSI 40000
# define G_freq_HSI 8000000
# if ! HSE_FREQUENCY
#  define HSE_FREQUENCY 8000000
# endif
#else // !STM32F1
# define G_freq_LSI 32768
# define G_freq_HSI 16000000
# if ! HSE_FREQUENCY
#  define HSE_FREQUENCY 25000000
# endif
#endif // !STM32F1
#define G_freq_HSE HSE_FREQUENCY
#define G_freq_LSE 32768
//
// Clock frequencies
#define G_freq_HCLK  4000000
//#define G_freq_HCLK (HSE_FREQUENCY/4)
#define G_freq_PCLK1 (G_freq_HCLK/2)
#define G_freq_PCLK2 (G_freq_HCLK/1)
// On the STM32F1, the ADC input clock must not exceed 14MHz
#define G_freq_ADC   (G_freq_PCLK2/2)

// 12-bit ADC maximum value
#define ADC_MAX 0x0FFF
// Voltage of the internal reference in mV
#if ! INTERNAL_VREF_mV
# if defined(STM32F1)
// Per the STM32F1 datasheet the internal Vref can be between 1.16V and 1.24V,
// with 1.20V being typical.
#  define INTERNAL_VREF_mV 1200
# else
// Per the STM32F4 datasheet the internal Vref can be between 1.18V and 1.24V,
// with 1.21V being typical
#  define INTERNAL_VREF_mV 1210
# endif
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
#define PINID_A13 (GPIO_PORTA_MASK|0x0D)
#define PINID_A14 (GPIO_PORTA_MASK|0x0E)
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
// Alternate function pin mappings
#include "platform_AF.h"
//
// Bluepill pin mappings
// Should be accurate for pretty much all other boards too, though there are
// some that use a more Arduino-like scheme or have additional pins
// Not all pins will be broken out on all boards
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
// PINID_A13 and PINID_A14 are used for SWDIO and SWCLK when DEBUG is enabled
#if ! DEBUG
# define PIN_A13 PINID_A13
# define PIN_A14 PINID_A14
#endif
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
#if USE_STM32F1_GPIO
# define SET_GPIO_OUTPUT_LOW(pin)  (GPIO_GET_PORT(pin)->BRR = GPIO_GET_PINMASK(pin))
#else
# define SET_GPIO_OUTPUT_LOW(pin)  (GPIO_GET_PORT(pin)->BSRR = (1UL << (GPIO_GET_PINNO(pin) + GPIO_BSRR_BR0_Pos)))
#endif

// Use the micro-second counter
uint16_t G_uscounter_psc;
uint16_t G_sleep_alarm_psc;
// Because time.h is local to the CMSIS backend the timers need to be defined
// here
// This definition must be kept in sync with the corresponding definitions in
// time.h
#if defined(TIM5)
# define SLEEP_ALARM_TIM TIM5
#else
# define SLEEP_ALARM_TIM TIM2
#endif
// Timers 2 and 5 have 16 bit counters on the STM32F1s and 32 bit counters
// on the other lines
#if defined(STM32F1)
# define SLEEP_TIM_MAX_CNT 0xFFFF
#else
# define SLEEP_TIM_MAX_CNT 0xFFFFFFFF
#endif
#define USCOUNTER_TIM         SLEEP_ALARM_TIM
#define USCOUNTER_TIM_MAX_CNT SLEEP_TIM_MAX_CNT
#define USCOUNTER_START() \
	do { \
		USCOUNTER_TIM->PSC = G_uscounter_psc; \
		USCOUNTER_TIM->ARR = USCOUNTER_TIM_MAX_CNT; \
		SET_BIT(USCOUNTER_TIM->EGR, TIM_EGR_UG); /* Generate an update event to reset the counter */ \
		SET_BIT(USCOUNTER_TIM->CR1, TIM_CR1_CEN); /* Enable the timer */ \
	} while (0);
#define USCOUNTER_STOP(counter) \
	do { \
		(counter) = USCOUNTER_TIM->CNT; \
		CLEAR_BIT(USCOUNTER_TIM->CR1, TIM_CR1_CEN); /* Disable the timer */ \
		USCOUNTER_TIM->PSC = G_sleep_alarm_psc; \
	} while (0);
//#define USCOUNTER_GET() (USCOUNTER_TIM->CNT)

#endif // _PLATFORM_CMSIS_H
#ifdef __cplusplus
 }
#endif
