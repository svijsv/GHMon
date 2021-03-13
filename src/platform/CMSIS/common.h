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
// Enter light sleep for this many seconds before entering deep sleep to give
// time to enter the command terminal because UART interrupts are disabled
// during stop mode
#define LIGHT_SLEEP_PERIOD (5) // 5 seconds
// If a deep sleep would last fewer than this many seconds, light sleep a bit
// longer instead
#define MIN_DEEP_SLEEP_PERIOD 2

// Sleep for no more than this many seconds; this will generally be a hardware
// constraint or a guard against any programming errors that may result in an
// erroneous wakeup time because there's no other reason to wake up without
// one of the other periods elapsing. MAX_WAKEUP_PERIOD should be small enough
// that it won't overflow a 32 bit integer when added to the current uptime.
#define MAX_SLEEP_PERIOD (24 * 60 * 60) // 24 hours

// Frequency of the low-speed external oscillator (Hz)
#define LSE_VALUE 32768
// Frequency of the high-speed external oscillator (Hz)
#define HSE_VALUE 8000000

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
// Timers
// SLEEP_ALARM_TIM must be on APB1 (alarms 2-7 or 12-14)
#define SLEEP_ALARM_TIM TIM2
#define SLEEP_ALARM_CLOCKEN_Pos RCC_APB1ENR_TIM2EN_Pos
#define SLEEP_ALARM_CLOCKEN     (1 << SLEEP_ALARM_CLOCKEN_Pos)
#define SLEEP_ALARM_IRQn TIM2_IRQn
#define SleepAlarm_IRQHandler TIM2_IRQHandler

//
// Handle ADCx
// The ADC is internal, there's no reason for anyone to change it
#define ADCx_IS_ADC1 1
#define ADCx ADC1
#define RCC_APB2ENR_ADCxEN_Pos RCC_APB2ENR_ADC1EN_Pos
#define RCC_APB2ENR_ADCxEN     (1 << RCC_APB2ENR_ADC1EN_Pos)

//
// Handle UARTx
#if USE_SERIAL
// USART1 remapped
#if (UARTx_TX_PIN == PB6) && (UARTx_RX_PIN == PB7)
# define UART1_DO_REMAP 1
#endif
// USART2 and USART3 can be remapped, but the remapped pins aren't accessible
// on the bluepill

// USART1
#if ((UARTx_TX_PIN == PA9) && (UARTx_RX_PIN == PA10)) || UART1_DO_REMAP
# define UARTx USART1
# define UARTx_IRQn     USART1_IRQn
# define UARTx_IRQHandler USART1_IRQHandler
# define UARTx_APBxENR  RCC->APB2ENR
# define UARTx_APBxRSTR RCC->APB2RSTR
# define UARTx_CLOCKEN  RCC_APB2ENR_USART1EN
# define UARTx_BUSFREQ  G_freq_PCLK2

// USART2
#elif (UARTx_TX_PIN == PA2) && (UARTx_RX_PIN == PA3)
# define UARTx USART2
# define UARTx_IRQn     USART2_IRQn
# define UARTx_IRQHandler USART2_IRQHandler
# define UARTx_APBxENR  RCC->APB1ENR
# define UARTx_APBxRSTR RCC->APB1RSTR
# define UARTx_CLOCKEN  RCC_APB1ENR_USART2EN
# define UARTx_BUSFREQ  G_freq_PCLK1

// USART3
#elif (UARTx_TX_PIN == PB10) && (UARTx_RX_PIN == PB11)
# define UARTx USART3
# define UARTx_IRQn     USART3_IRQn
# define UARTx_IRQHandler USART3_IRQHandler
# define UARTx_APBxENR  RCC->APB1ENR
# define UARTx_APBxRSTR RCC->APB1RSTR
# define UARTx_CLOCKEN  RCC_APB1ENR_USART3EN
# define UARTx_BUSFREQ  G_freq_PCLK1

#else
# error "Can't determine UART peripheral"
#endif // UARTx
#endif // USE_SERIAL

//
// Handle SPIx
#if USE_SPI
// SPI1 remapped
#if (SPIx_SCK_PIN == PB3) && (SPIx_MISO_PIN == PB4) && (SPIx_MOSI_PIN == PB5)
# define SPI1_DO_REMAP 1
#endif

// SPI1
#if ((SPIx_SCK_PIN == PA5) && (SPIx_MISO_PIN == PA6) && (SPIx_MOSI_PIN == PA7)) || SPI1_DO_REMAP
# define SPIx SPI1
# define SPIx_APBxENR  RCC->APB2ENR
# define SPIx_APBxRSTR RCC->APB2RSTR
# define SPIx_CLOCKEN  RCC_APB2ENR_SPI1EN
# define SPIx_BUSFREQ  G_freq_PCLK2

// SPI2
#elif (SPIx_SCK_PIN == PB13) && (SPIx_MISO_PIN == PB14) && (SPIx_MOSI_PIN == PB15)
# define SPIx SPI2
# define SPIx_APBxENR  RCC->APB1ENR
# define SPIx_APBxRSTR RCC->APB1RSTR
# define SPIx_CLOCKEN  RCC_APB1ENR_SPI2EN
# define SPIx_BUSFREQ  G_freq_PCLK1

#elif !defined(SPIx_SCK_PIN) && !defined(SPIx_MISO_PIN) && !defined(SPIx_MOSI_PIN)
# error "No SPI peripheral set"
#else
# error "Can't determine SPI peripheral"
#endif // SPIx
#endif // USE_SPI

//
// Handle user button
#if ! BUTTON_PIN
	// Nothing to do here

#elif GPIO_GET_PINNO(BUTTON_PIN) == 0
# define BUTTON_IRQn       EXTI0_IRQn
# define Button_IRQHandler EXTI0_IRQHandler

#elif GPIO_GET_PINNO(BUTTON_PIN) == 1
# define BUTTON_IRQn       EXTI1_IRQn
# define Button_IRQHandler EXTI1_IRQHandler

#elif GPIO_GET_PINNO(BUTTON_PIN) == 2
# define BUTTON_IRQn       EXTI2_IRQn
# define Button_IRQHandler EXTI2_IRQHandler

#elif GPIO_GET_PINNO(BUTTON_PIN) == 3
# define BUTTON_IRQn       EXTI3_IRQn
# define Button_IRQHandler EXTI3_IRQHandler

#elif GPIO_GET_PINNO(BUTTON_PIN) == 4
# define BUTTON_IRQn       EXTI4_IRQn
# define Button_IRQHandler EXTI4_IRQHandler

#elif (GPIO_GET_PINNO(BUTTON_PIN) >= 5) && (GPIO_GET_PINNO(BUTTON_PIN) <= 9)
# define BUTTON_IRQn       EXTI9_5_IRQn
# define Button_IRQHandler EXTI9_5_IRQHandler

#elif (GPIO_GET_PINNO(BUTTON_PIN) >= 10) && (GPIO_GET_PINNO(BUTTON_PIN) <= 15)
# define BUTTON_IRQn       EXTI15_10_IRQn
# define Button_IRQHandler EXTI15_10_IRQHandler

#else
# error "Can't determine user button"
#endif // User button


/*
* Types
*/


/*
* Variable declarations (defined in stm32f103.c)
*/


/*
* Function prototypes (defined in stm32f103.c)
*/


/*
* Macros
*/
// An LED flash for use when debugging
//#define DFLASH(t) do { gpio_set_state(LED_PIN, GPIO_HIGH); dumb_delay(t); gpio_set_state(LED_PIN, GPIO_LOW); } while (0);
#define DFLASH(t) do { gpio_toggle_state(LED_PIN); dumb_delay(t); gpio_toggle_state(LED_PIN); } while (0);


#endif // _PLATFORM_CMSIS_COMMON_H
#ifdef __cplusplus
 }
#endif
