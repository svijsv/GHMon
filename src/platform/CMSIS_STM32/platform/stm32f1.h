// SPDX-License-Identifier: GPL-3.0-only
/***********************************************************************
*                                                                      *
*                                                                      *
* Copyright 2024 svijsv                                                *
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
//    This file is for anything common to F1 devices and should only be included
//    by the specific device's header (e.g. stm32f103.h)
//
#ifndef _uHAL_PLATFORM_CMSIS_STM32F1_H
#define _uHAL_PLATFORM_CMSIS_STM32F1_H

#define HAVE_STM32F1 1

#include <stm32f1xx.h>
#define CMSIS_VERSION __STM32F1_CMSIS_VERSION
#define CMSIS_NAME "STM32F1"
#define HAVE_STM32F1_ADC    1
#define HAVE_STM32F1_VREF   1
#define HAVE_STM32F1_GPIO   1
#define HAVE_STM32F1_RTC    1
#define HAVE_STM32F1_TIMERS 1
#define HAVE_STM32F1_PLL    1
#define HAVE_STM32F1_HSI    1
#define HAVE_STM32F1_LSI    1
#define HAVE_STM32F1_FLASH  1
#define HAVE_AHB2           0
#define HAVE_AHB_RESET      0
//
// See the RCC_CFGR register section of the manual for device-specific
// maximum bus frequencies
#define G_freq_HCLK_MAX  72000000UL
#define G_freq_PCLK1_MAX 36000000UL
#define G_freq_PCLK2_MAX 72000000UL
//
// The reference manual give 8MHz for the HSI
#define G_freq_HSI_DEFAULT 8000000UL
//
// The reference manual gives a range of 30KHz-60KHz for the LSI on the F1
#define G_freq_LSI_DEFAULT 40000UL
//
// Maximum ADC clock frequecy at 1.7V, found in data sheet
#define G_freq_ADCCLK_MAX_1_7 14000000UL
//
// Maximum ADC clock frequecy at 2.4V, found in data sheet
#define G_freq_ADCCLK_MAX_2_4 14000000UL
#define G_freq_ADCCLK_MAX G_freq_ADCCLK_MAX_2_4
//
// 12-bit ADC maximum value
#if ! ADC_MAX
# define ADC_MAX 0x0FFFU
#endif

//
// Voltage of the internal reference in mV
#if ! defined(INTERNAL_VREF_mV) || INTERNAL_VREF_mV <= 0
# undef INTERNAL_VREF_mV
// Per the STM32F1 datasheet the internal Vref can be between 1.16V and 1.24V,
// with 1.20V being typical.
# define INTERNAL_VREF_mV 1200U
#endif

//
// For STM32F1 use a flash latency of 0 for speeds <= 24MHz, 1 for
// 24-48MHz, and 2 for anything higher.
// This information is in section 3.3.3 ('Memory and Bus Architecture ->
// Memory Map -> Embedded Flash Memory') of the reference manual.
#define FLASH_WS_MAX 2U
#define FLASH_WS_STEP 24000000UL

#define EXTI_PREG AFIO
#define EXTI_PREG_CLOCKEN RCC_PERIPH_AFIO
#define FLASH_ACR_PRFTEN_Pos FLASH_ACR_PRFTBE_Pos
#define FLASH_ACR_PRFTEN     FLASH_ACR_PRFTBE

// Use the low-power voltage regulator when in HIBERNATE_NO_PERIPHERALS_SLOW_WAKE
#define PWR_CR_SLOW_WAKE_MASK (PWR_CR_LPDS)
#define PWR_CR_SLOW_WAKE_BITS (1U << PWR_CR_LPDS_Pos)


#endif // _uHAL_PLATFORM_CMSIS_STM32F1_H
