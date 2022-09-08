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


/*
* Static values
*/
// Enter light sleep for this many seconds before entering deep sleep to give
// time to enter the command terminal because UART interrupts are disabled
// during stop mode
#define LIGHT_SLEEP_SECONDS (5) // 5 seconds
// If a deep sleep would last fewer than this many seconds, light sleep a bit
// longer instead
#define MIN_DEEP_SLEEP_SECONDS 2
// Don't hibernate longer than this many seconds
// Must be < 24 hours for calendar-based RTCs
#define HIBERNATE_MAX_S ((24 * HOURS) - 60)

//
// Interrupt priorities
// Range 0-15, overlap is OK
// Lower number is higher priority
#define SYSTICK_IRQp     1
#define BUTTON_IRQp      2
#define RTC_ALARM_IRQp   3
#define UART_COMM_IRQp   4
#define SLEEP_ALARM_IRQp 5

// Peripheral control mappings
#include "common_periph.h"

//
// Clock prescalers
//
// HCLK is system clock divided by this
#define HCLK_PRESCALER_Msk (0b1111 << RCC_CFGR_HPRE_Pos)
#define HCLK_PRESCALER_1   (0b0000 << RCC_CFGR_HPRE_Pos)
#define HCLK_PRESCALER_2   (0b1000 << RCC_CFGR_HPRE_Pos)
#define HCLK_PRESCALER_4   (0b1001 << RCC_CFGR_HPRE_Pos)
#define HCLK_PRESCALER_8   (0b1010 << RCC_CFGR_HPRE_Pos)
#define HCLK_PRESCALER_16  (0b1011 << RCC_CFGR_HPRE_Pos)
#define HCLK_PRESCALER_64  (0b1100 << RCC_CFGR_HPRE_Pos)
#define HCLK_PRESCALER_128 (0b1101 << RCC_CFGR_HPRE_Pos)
#define HCLK_PRESCALER_256 (0b1110 << RCC_CFGR_HPRE_Pos)
#define HCLK_PRESCALER_512 (0b1111 << RCC_CFGR_HPRE_Pos)
// PCLK1 is HCLK divided by this
#define PCLK1_PRESCALER_Msk (0b111 << RCC_CFGR_PPRE1_Pos)
#define PCLK1_PRESCALER_1   (0b000 << RCC_CFGR_PPRE1_Pos)
#define PCLK1_PRESCALER_2   (0b100 << RCC_CFGR_PPRE1_Pos)
#define PCLK1_PRESCALER_4   (0b101 << RCC_CFGR_PPRE1_Pos)
#define PCLK1_PRESCALER_8   (0b110 << RCC_CFGR_PPRE1_Pos)
#define PCLK1_PRESCALER_16  (0b111 << RCC_CFGR_PPRE1_Pos)
// PCLK2 is HCLK divided by this
#define PCLK2_PRESCALER_Msk (0b111 << RCC_CFGR_PPRE2_Pos)
#define PCLK2_PRESCALER_1   (0b000 << RCC_CFGR_PPRE2_Pos)
#define PCLK2_PRESCALER_2   (0b100 << RCC_CFGR_PPRE2_Pos)
#define PCLK2_PRESCALER_4   (0b101 << RCC_CFGR_PPRE2_Pos)
#define PCLK2_PRESCALER_8   (0b110 << RCC_CFGR_PPRE2_Pos)
#define PCLK2_PRESCALER_16  (0b111 << RCC_CFGR_PPRE2_Pos)

// Stuff for compatibility between different platforms
#if defined(STM32F1)
# define FLASH_ACR_PRFTEN_Pos FLASH_ACR_PRFTBE_Pos
# define FLASH_ACR_PRFTEN     FLASH_ACR_PRFTBE
#endif
#if ! USE_STM32F1_RTC
# define RCC_BDCR_RTCSEL_NOCLOCK 0
# define RCC_BDCR_RTCSEL_LSE (0b01 << RCC_BDCR_RTCSEL_Pos)
# define RCC_BDCR_RTCSEL_LSI (0b10 << RCC_BDCR_RTCSEL_Pos)
# define RCC_BDCR_RTCSEL_HSE (0b11 << RCC_BDCR_RTCSEL_Pos)
#endif


/*
* Types
*/
// RCC_BUS_OFFSET is defined in common_periph.h
#if RCC_BUS_OFFSET <= 30
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
