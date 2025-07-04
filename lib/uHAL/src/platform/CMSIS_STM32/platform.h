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
#ifndef _uHAL_PLATFORM_CMSIS_H
#define _uHAL_PLATFORM_CMSIS_H

#define PLATFORM_INTERFACE_H "interface/platform/cmsis.h"

#include "ulib/include/debug.h"
#include "ulib/include/types.h"
#include "ulib/include/time.h"

//
// Ideal voltage output by the on-board voltage regulator
// This is used in some device headers so must be defined before they're
// included
#if ! REGULATED_VOLTAGE_mV
# define REGULATED_VOLTAGE_mV 3300U
#endif

// CMSIS header files
// Both ulib/bits.h and stm32fXxx.h define the SET_BIT() and CLEAR_BIT()
// macros
#pragma push_macro("SET_BIT")
#pragma push_macro("CLEAR_BIT")
#undef SET_BIT
#undef CLEAR_BIT

#define HAVE_STM32 1
#if defined(STM32F103x6) || defined(STM32F103xB) || defined(STM32F103xE) || defined(STM32F103xG)
# include "platform/stm32f103.h"
#elif defined(STM32F401xC) || defined(STM32F401xE)
# include "platform/stm32f401.h"
#else
# error "Unsupported device"
#endif

#pragma pop_macro("SET_BIT")
#pragma pop_macro("CLEAR_BIT")

#define RAM_BASE SRAM_BASE

#ifndef uHAL_SWD_DEBUG
# define uHAL_SWD_DEBUG 1
#endif
#ifndef uHAL_JTAG_DEBUG
# define uHAL_JTAG_DEBUG 0
#endif
#ifndef uHAL_BACKUP_DOMAIN_RESET
# define uHAL_BACKUP_DOMAIN_RESET DEBUG
#endif

#ifndef uHAL_USE_INTERNAL_OSC
# define uHAL_USE_INTERNAL_OSC 1
#endif
#ifndef uHAL_USE_INTERNAL_LS_OSC
# define uHAL_USE_INTERNAL_LS_OSC 1
#endif

// Don't hibernate longer than this many seconds
// Must be < 24 hours for calendar-based RTCs due to how the wakeup timer
// is set
#define HIBERNATE_MAX_S ((24U * SECONDS_PER_HOUR) - 60U)

#ifndef uHAL_USE_UPTIME_EMULATION
# if uHAL_USE_RTC && uHAL_USE_UPTIME
#  define uHAL_USE_UPTIME_EMULATION 1
# else
#  define uHAL_USE_UPTIME_EMULATION 0
# endif
#endif

//
// Oscillator frequencies
//
#if uHAL_USE_INTERNAL_OSC
# if defined(F_OSC) && F_OSC > 0
#  define G_freq_HSI F_OSC
# endif
#else
# if ! F_OSC
#  error "F_OSC (the oscillator frequency) must be defined and > 0"
# endif
# define G_freq_HSE F_OSC
#endif
#if ! defined(G_freq_HSI)
# define G_freq_HSI G_freq_HSI_DEFAULT
#endif

#if uHAL_USE_INTERNAL_LS_OSC
# if defined(F_LS_OSC) && F_LS_OSC > 0
#  define G_freq_LSI F_LS_OSC
# endif
#else
# if defined(F_LS_OSC) && F_LS_OSC > 0
#  define G_freq_LSE F_LS_OSC
# else
#  define G_freq_LSE 32768UL
# endif
#endif
#if ! defined(G_freq_LSI)
# define G_freq_LSI G_freq_LSI_DEFAULT
#endif

//
// Clock frequencies
//
// Sysclock frequency
#if ! F_CORE
# undef F_CORE
# if F_CPU
#  define F_CORE F_CPU
# else
#  define F_CORE F_OSC
# endif
#endif
#define G_freq_CORE F_CORE

// AHB frequency
#if F_HCLK
# define G_freq_HCLK F_HCLK
#else
# define G_freq_HCLK G_freq_CORE
#endif

// APB1 frequency
// The maximum frequency of APB1 is generally 1/2 the maximum frequency of APB2
#if defined(F_PCLK1) && F_PCLK1 > 0
# define G_freq_PCLK1 F_PCLK1
#elif G_freq_HCLK <= G_freq_PCLK1_MAX
# define G_freq_PCLK1 (G_freq_HCLK)
#elif G_freq_HCLK/2 <= G_freq_PCLK1_MAX
# define G_freq_PCLK1 (G_freq_HCLK/2U)
#elif G_freq_HCLK/4 <= G_freq_PCLK1_MAX
# define G_freq_PCLK1 (G_freq_HCLK/4U)
#elif G_freq_HCLK/8 <= G_freq_PCLK1_MAX
# define G_freq_PCLK1 (G_freq_HCLK/8U)
#elif G_freq_HCLK/16 <= G_freq_PCLK1_MAX
# define G_freq_PCLK1 (G_freq_HCLK/16U)
#else
// This will throw a compile-time error in system.c
# define G_freq_PCLK1 (G_freq_PCLK1_MAX)
#endif

// APB2 frequency
// The maximum frequency of APB2 is generally either the same as the system
// clock or 1/2 of the system clock
#if defined(F_PCLK2) && F_PCLK2 > 0
# define G_freq_PCLK2 F_PCLK2
#elif G_freq_HCLK <= G_freq_PCLK2_MAX
# define G_freq_PCLK2 (G_freq_HCLK)
#elif G_freq_HCLK/2 <= G_freq_PCLK2_MAX
# define G_freq_PCLK2 (G_freq_HCLK/2U)
#elif G_freq_HCLK/4 <= G_freq_PCLK2_MAX
# define G_freq_PCLK2 (G_freq_HCLK/4U)
#elif G_freq_HCLK/8 <= G_freq_PCLK2_MAX
# define G_freq_PCLK2 (G_freq_HCLK/8U)
#elif G_freq_HCLK/16 <= G_freq_PCLK2_MAX
# define G_freq_PCLK2 (G_freq_HCLK/16U)
#else
// This will throw a compile-time error in system.c
# define G_freq_PCLK2 (G_freq_PCLK2_MAX)
#endif

// ADC clock frequency
#if F_ADC
# define G_freq_ADCCLK F_ADC
#else
# if (G_freq_PCLK2/2) <= G_freq_ADCCLK_MAX
#  define G_freq_ADCCLK (G_freq_PCLK2/2U)
# elif (G_freq_PCLK2/4) <= G_freq_ADCCLK_MAX
#  define G_freq_ADCCLK (G_freq_PCLK2/4U)
# elif (G_freq_PCLK2/6) <= G_freq_ADCCLK_MAX
#  define G_freq_ADCCLK (G_freq_PCLK2/6U)
# else
#  define G_freq_ADCCLK (G_freq_PCLK2/8U)
# endif
#endif
//
// Value returned on ADC conversion error
#define ERR_ADC ((adc_t )-1)

// Base pin IDs
#include "platform_base_pins.h"
// Alternate function pin mappings
#include "platform_AF.h"

// Peripheral control mappings
#include "common_periph.h"


// RCC_BUS_OFFSET is defined in common_periph.h
#if RCC_BUS_OFFSET <= 30
typedef uint_fast32_t rcc_periph_t;
#else
typedef uint_fast64_t rcc_periph_t;
#endif

typedef struct {
	__I uint32_t *idr;
	uint32_t mask;
} gpio_quick_t;

typedef struct {
	gpio_pin_t pin;
} gpio_listen_t;

typedef struct {
	uint8_t buffer[UART_INPUT_BUFFER_BYTES];
	uint8_t bytes;
} uart_buffer_t;
typedef struct {
	USART_TypeDef *uartx;
	// Need to know the pins and clock when turning the peripheral on or off.
	rcc_periph_t clocken;
	gpio_pin_t rx_pin;
	gpio_pin_t tx_pin;
	uint8_t gpio_af;
	// Enabling/disabling interrupts is easier if we track the IRQn rather than
	// recalculating each time
	uint8_t irqn;

#if UART_INPUT_BUFFER_BYTES > 0
	volatile uart_buffer_t rx_buf;
#endif
} uart_port_t;

typedef enum {
	GPIO_MODE_RESET = 0, // Reset state of the pin
	GPIO_MODE_PP,    // Push-pull output
	GPIO_MODE_PP_AF, // Alternate-function push-pull output
	GPIO_MODE_OD,    // Open-drain output
	GPIO_MODE_OD_AF, // Alternate-function open-drain output
	GPIO_MODE_IN,    // Input
	GPIO_MODE_IN_AF, // Alternate-function input
	GPIO_MODE_AIN,   // Analog input
	GPIO_MODE_HiZ,    // High-impedence mode

	GPIO_MODE_RESET_ALIAS = GPIO_MODE_AIN,
	GPIO_MODE_HiZ_ALIAS   = GPIO_MODE_AIN
} gpio_mode_t;
// This allows checking if we have a mode via the preprocessor while still getting
// the benefits that an enum provides
#define GPIO_MODE_RESET GPIO_MODE_RESET
#define GPIO_MODE_PP    GPIO_MODE_PP
#define GPIO_MODE_PP_AF GPIO_MODE_PP_AF
#define GPIO_MODE_OD    GPIO_MODE_OD
#define GPIO_MODE_OD_AF GPIO_MODE_OD_AF
#define GPIO_MODE_IN    GPIO_MODE_IN
#define GPIO_MODE_IN_AF GPIO_MODE_IN_AF
#define GPIO_MODE_AIN   GPIO_MODE_AIN
#define GPIO_MODE_HiZ   GPIO_MODE_HiZ
#define GPIO_MODE_RESET_ALIAS GPIO_MODE_RESET_ALIAS
#define GPIO_MODE_HiZ_ALIAS   GPIO_MODE_HiZ_ALIAS

typedef struct {
	TIM_TypeDef *TIMx;
	rcc_periph_t TIMxEN;
	gpio_pin_t pin;
	uint8_t channel;
} pwm_output_t;

extern volatile utime_t G_sys_msticks;

//#define GPIO_GET_PORT(pin) (((GPIO_GET_PORTMASK(pin)) == GPIO_PORTA_MASK) ? GPIOA : GPIOB)
INLINE GPIO_TypeDef* _GPIO_GET_PORT(gpio_pin_t pin) {
	// GPIO_GET_PORTMASK() hasn't been defined yet
	//switch (GPIO_GET_PORTMASK(pin)) {
	switch (pin & GPIO_PORT_MASK) {
#if HAVE_GPIO_PORTA
	case GPIO_PORTA_MASK:
		return GPIOA;
#endif
#if HAVE_GPIO_PORTB
	case GPIO_PORTB_MASK:
		return GPIOB;
#endif
#if HAVE_GPIO_PORTC
	case GPIO_PORTC_MASK:
		return GPIOC;
#endif
#if HAVE_GPIO_PORTD
	case GPIO_PORTD_MASK:
		return GPIOD;
#endif
#if HAVE_GPIO_PORTE
	case GPIO_PORTE_MASK:
		return GPIOE;
#endif
#if HAVE_GPIO_PORTF
	case GPIO_PORTF_MASK:
		return GPIOF;
#endif
#if HAVE_GPIO_PORTG
	case GPIO_PORTG_MASK:
		return GPIOG;
#endif
#if HAVE_GPIO_PORTH
	case GPIO_PORTH_MASK:
		return GPIOH;
#endif
#if HAVE_GPIO_PORTI
	case GPIO_PORTI_MASK:
		return GPIOI;
#endif
#if HAVE_GPIO_PORTJ
	case GPIO_PORTJ_MASK:
		return GPIOJ;
#endif
#if HAVE_GPIO_PORTK
	case GPIO_PORTK_MASK:
		return GPIOK;
#endif
#if HAVE_GPIO_PORTL
	case GPIO_PORTL_MASK:
		return GPIOL;
#endif
#if HAVE_GPIO_PORTM
	case GPIO_PORTM_MASK:
		return GPIOM;
#endif
#if HAVE_GPIO_PORTN
	case GPIO_PORTN_MASK:
		return GPION;
#endif
#if HAVE_GPIO_PORTO
	case GPIO_PORTO_MASK:
		return GPIOO;
#endif
	}

	return NULL;
}
#define GPIO_GET_PORT(_pin_) (_GPIO_GET_PORT(_pin_))
#define GPIO_QUICK_READ(_qpin_) (SELECT_BITS(*((_qpin_).idr), (_qpin_).mask) != 0)

// Quick pin access, for when you know what you want:
#define IS_GPIO_INPUT_HIGH(_pin_)  (BIT_IS_SET(GPIO_GET_PORT((_pin_))->IDR, GPIO_GET_PINMASK((_pin_))))
#define SET_GPIO_OUTPUT_HIGH(_pin_) (GPIO_GET_PORT((_pin_))->BSRR = GPIO_GET_PINMASK((_pin_)))
#if HAVE_STM32F1_GPIO
# define SET_GPIO_OUTPUT_LOW(_pin_)  (GPIO_GET_PORT((_pin_))->BRR = GPIO_GET_PINMASK((_pin_)))
#else
# define SET_GPIO_OUTPUT_LOW(_pin_)  (GPIO_GET_PORT((_pin_))->BSRR = (1UL << (GPIO_GET_PINNO((_pin_)) + GPIO_BSRR_BR0_Pos)))
//# define SET_GPIO_OUTPUT_LOW((_pin_))  (GPIO_GET_PORT((_pin_))->BSRR = (GPIO_GET_PINMASK((_pin_)) << GPIO_BSRR_BR0_Pos))
#endif

#define NOW_MS() (G_sys_msticks)


#endif // _uHAL_PLATFORM_CMSIS_H
