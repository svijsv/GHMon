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
// common.h
// Platform-specific common header
// NOTES:
//

#ifdef __cplusplus
 extern "C" {
#endif
#ifndef _PLATFORM_CMSIS_COMMON_H
#define _PLATFORM_CMSIS_COMMON_H

/*
* Includes
*/
#include "platform/interface.h"
#include "platform.h"

#include "ulib/assert.h"
#include "ulib/types.h"
#include "ulib/bits.h"
#include "ulib/time.h"
#include "ulib/util.h"

// CMSIS header files
// Both ulib/bits.h and stm32f1xx.h define the SET_BIT() and CLEAR_BIT()
// macros; they're identical so just use the later-defined ones.
#undef SET_BIT
#undef CLEAR_BIT
#include <stm32f1xx.h>
#include <stm32f103xb.h>


/*
* Static values
*/
#if USE_INTERNAL_CLOCK
# define G_freq_OSC G_freq_HSI
#else
# define G_freq_OSC G_freq_HSE
#endif

// Enter light sleep for this many seconds before entering deep sleep to give
// time to enter the command terminal because UART interrupts are disabled
// during stop mode
#define LIGHT_SLEEP_SECONDS (5) // 5 seconds
// If a deep sleep would last fewer than this many seconds, light sleep a bit
// longer instead
#define MIN_DEEP_SLEEP_SECONDS 2

//
// Interrupt priorities
// Range 0-15, overlap is OK
// Lower number is higher priority
#define SYSTICK_IRQp     1
#define BUTTON_IRQp      2
#define RTC_ALARM_IRQp   3
#define UARTx_IRQp       4
#define SLEEP_ALARM_IRQp 5

//
// RCC control
// A single value is used instead of separate values for the clock and enable
// bits in order to avoid having to define and track both for each peripheral,
// but this means having to choose between 64 bit values or losing access to
// the high bits. At present the highest two bits are used to indicate the bus
// which means both that any peripheral that's controlled by those bits can't
// be enabled and that AHB2 and AHB3 aren't handled because there aren't
// enough possible values. Neither of these are problems on STM32F1 devices
// which have no AHB2 or AHB3 and nothing in the high two bits, and there
// isn't anything important lost on other devices from what I can see but it
// should be easy enough to convert to a 64-bit integer if required.
//
// Not all possible values are defined, only those that I anticipate using.
//
// Registers
#define RCC_BUS_OFFSET 6
#define RCC_BUS_MASK (0x03LL << RCC_BUS_OFFSET)
#define RCC_BUS_APB1 (0x01LL << RCC_BUS_OFFSET)
#define RCC_BUS_APB2 (0x02LL << RCC_BUS_OFFSET)
#define RCC_BUS_AHB1 (0x03LL << RCC_BUS_OFFSET)
//#define RCC_BUS_AHB2 (0x04LL << RCC_BUS_OFFSET)
//#define RCC_BUS_AHB3 (0x05LL << RCC_BUS_OFFSET)
//
// Peripheral enable/disable bits
// AHB1
#define RCC_PERIPH_SDIO  (RCC_BUS_AHB1|RCC_AHBENR_SDIOEN)
// APB1
#define RCC_PERIPH_TIM2  (RCC_BUS_APB1|RCC_APB1ENR_TIM2EN)
#define RCC_PERIPH_TIM3  (RCC_BUS_APB1|RCC_APB1ENR_TIM3EN)
#define RCC_PERIPH_TIM4  (RCC_BUS_APB1|RCC_APB1ENR_TIM4EN)
#define RCC_PERIPH_TIM5  (RCC_BUS_APB1|RCC_APB1ENR_TIM5EN)
#define RCC_PERIPH_TIM6  (RCC_BUS_APB1|RCC_APB1ENR_TIM6EN)
#define RCC_PERIPH_TIM7  (RCC_BUS_APB1|RCC_APB1ENR_TIM7EN)
#define RCC_PERIPH_TIM12 (RCC_BUS_APB1|RCC_APB1ENR_TIM12EN)
#define RCC_PERIPH_TIM13 (RCC_BUS_APB1|RCC_APB1ENR_TIM13EN)
#define RCC_PERIPH_TIM14 (RCC_BUS_APB1|RCC_APB1ENR_TIM14EN)
#define RCC_PERIPH_SPI2  (RCC_BUS_APB1|RCC_APB1ENR_SPI2EN)
#define RCC_PERIPH_SPI3  (RCC_BUS_APB1|RCC_APB1ENR_SPI3EN)
#define RCC_PERIPH_UART2 (RCC_BUS_APB1|RCC_APB1ENR_USART2EN)
#define RCC_PERIPH_UART3 (RCC_BUS_APB1|RCC_APB1ENR_USART3EN)
#define RCC_PERIPH_UART4 (RCC_BUS_APB1|RCC_APB1ENR_UART4EN)
#define RCC_PERIPH_UART5 (RCC_BUS_APB1|RCC_APB1ENR_UART5EN)
#define RCC_PERIPH_I2C1  (RCC_BUS_APB1|RCC_APB1ENR_I2C1EN)
#define RCC_PERIPH_I2C2  (RCC_BUS_APB1|RCC_APB1ENR_I2C2EN)
#define RCC_PERIPH_BKP   (RCC_BUS_APB1|RCC_APB1ENR_BKPEN)
#define RCC_PERIPH_PWR   (RCC_BUS_APB1|RCC_APB1ENR_PWREN)
#define RCC_PERIPH_DAC   (RCC_BUS_APB1|RCC_APB1ENR_DACEN)
// APB2
#define RCC_PERIPH_AFIO  (RCC_BUS_APB2|RCC_APB2ENR_AFIOEN)
#define RCC_PERIPH_GPIOA (RCC_BUS_APB2|RCC_APB2ENR_IOPAEN)
#define RCC_PERIPH_GPIOB (RCC_BUS_APB2|RCC_APB2ENR_IOPBEN)
#define RCC_PERIPH_GPIOC (RCC_BUS_APB2|RCC_APB2ENR_IOPCEN)
#define RCC_PERIPH_GPIOD (RCC_BUS_APB2|RCC_APB2ENR_IOPDEN)
#define RCC_PERIPH_GPIOE (RCC_BUS_APB2|RCC_APB2ENR_IOPEEN)
#define RCC_PERIPH_GPIOF (RCC_BUS_APB2|RCC_APB2ENR_IOPFEN)
#define RCC_PERIPH_GPIOG (RCC_BUS_APB2|RCC_APB2ENR_IOPGEN)
#define RCC_PERIPH_ADC1  (RCC_BUS_APB2|RCC_APB2ENR_ADC1EN)
#define RCC_PERIPH_ADC2  (RCC_BUS_APB2|RCC_APB2ENR_ADC2EN)
#define RCC_PERIPH_TIM1  (RCC_BUS_APB2|RCC_APB2ENR_TIM1EN)
#define RCC_PERIPH_SPI1  (RCC_BUS_APB2|RCC_APB2ENR_SPI1EN)
#define RCC_PERIPH_TIM8  (RCC_BUS_APB2|RCC_APB2ENR_TIM8EN)
#define RCC_PERIPH_UART1 (RCC_BUS_APB2|RCC_APB2ENR_USART1EN)
#define RCC_PERIPH_ADC3  (RCC_BUS_APB2|RCC_APB2ENR_ADC3EN)
#define RCC_PERIPH_TIM9  (RCC_BUS_APB2|RCC_APB2ENR_TIM9EN)
#define RCC_PERIPH_TIM10 (RCC_BUS_APB2|RCC_APB2ENR_TIM10EN)
#define RCC_PERIPH_TIM11 (RCC_BUS_APB2|RCC_APB2ENR_TIM11EN)


/*
* Types
*/
#if RCC_BUS_OFFSET < 8
typedef uint32_t rcc_periph_t;
#else
typedef uint64_t rcc_periph_t;
#endif


/*
* Variable declarations
*/


/*
* Function prototypes
*/


/*
* Macros
*/
// An LED flash for use when debugging
//#define DFLASH(t) do { gpio_set_state(LED_PIN, GPIO_HIGH); dumb_delay_ms(t); gpio_set_state(LED_PIN, GPIO_LOW); } while (0);
#define DFLASH(t) do { gpio_toggle_state(LED_PIN); dumb_delay_ms(t); gpio_toggle_state(LED_PIN); } while (0);


#endif // _PLATFORM_CMSIS_COMMON_H
#ifdef __cplusplus
 }
#endif
