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
//    This file is for anything common to F4 devices and should only be included
//    by the specific device's header (e.g. stm32f401.h)
//
#ifndef _uHAL_PLATFORM_CMSIS_STM32F4_H
#define _uHAL_PLATFORM_CMSIS_STM32F4_H

#define HAVE_STM32F4 1

#include <stm32f4xx.h>

#define CMSIS_VERSION __STM32F4xx_CMSIS_VERSION
#define CMSIS_NAME "STM32F4xx"

// There's a typo in stm32f4xx.h that breaks compilation when using the
// version number
#if HAVE_BROKEN_CMSIS_VERSION
# undef __STM32F4xx_CMSIS_VERSION
# define __STM32F4xx_CMSIS_VERSION ((__STM32F4xx_CMSIS_VERSION_MAIN << 24U)\
                                    | (__STM32F4xx_CMSIS_VERSION_SUB1 << 16U)\
                                    | (__STM32F4xx_CMSIS_VERSION_SUB2 << 8U )\
                                    | (__STM32F4xx_CMSIS_VERSION_RC))
#endif

#define HAVE_STM32F1_ADC    0
#define HAVE_STM32F1_VREF   0
#define HAVE_STM32F1_GPIO   0
#define HAVE_STM32F1_RTC    0
#define HAVE_STM32F1_TIMERS 0
#define HAVE_STM32F1_PLL    0
#define HAVE_STM32F1_HSI    0
#define HAVE_STM32F1_LSI    0
#define HAVE_STM32F1_FLASH  0
#define HAVE_AHB2      1
#define HAVE_AHB_RESET 1

#define G_freq_HSI_DEFAULT 16000000UL
#define G_freq_LSI_DEFAULT 32768UL

#if ! ADC_MAX
# define ADC_MAX 0x0FFFU
#endif

//
// Voltage of the internal reference in mV
#if ! defined(INTERNAL_VREF_mV) || INTERNAL_VREF_mV <= 0
# undef INTERNAL_VREF_mV
// Per the STM32F4 datasheet the internal Vref can be between 1.18V and 1.24V,
// with 1.21V being typical
# define INTERNAL_VREF_mV 1210U
#endif

#define EXTI_PREG SYSCFG
#define EXTI_PREG_CLOCKEN RCC_PERIPH_SYSCFG

// Use the low-power voltage regulator and power down flash memory when in
// HIBERNATE_NO_PERIPHERALS_SLOW_WAKE
#define PWR_CR_SLOW_WAKE_MASK (PWR_CR_LPDS|PWR_CR_FPDS)
#define PWR_CR_SLOW_WAKE_BITS ((1U << PWR_CR_LPDS_Pos) | (1U << PWR_CR_FPDS_Pos))


#endif // _uHAL_PLATFORM_CMSIS_STM32F4_H
