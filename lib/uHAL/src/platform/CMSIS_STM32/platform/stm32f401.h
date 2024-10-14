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
//    This file is for hardware configuration of STM32F401 devices and should
//    only be included by platform.h
//
#ifndef _uHAL_PLATFORM_CMSIS_STM32F401_H
#define _uHAL_PLATFORM_CMSIS_STM32F401_H

#define HAVE_STM32F401 1

// There's a typo in some versions of stm32f4xx.h that breaks compilation when
// using the CMSIS version number, set this to non-zero to work around it
#define HAVE_BROKEN_CMSIS_VERSION 1

#include "stm32f4.h"

#if ! defined(RAM_PRESENT) || RAM_PRESENT <= 0
# undef  RAM_PRESENT
# if defined(STM32F401xB) || defined(STM32F401xC)
#  define RAM_PRESENT 0x0FFFFU // 65536B, 64KB
# elif defined(STM32F401xD) || defined(STM32F401xE)
#  define RAM_PRESENT 0x18000U // 98304B, 96KB
# else
#  error "Unhandled device"
# endif
#endif

//
// See the RCC_CFGR register section of the manual for device-specific
// maximum bus frequencies
#define G_freq_HCLK_MAX  84000000UL
#define G_freq_PCLK1_MAX 42000000UL
#define G_freq_PCLK2_MAX 84000000UL
//
// Maximum ADC clock frequecy at 1.7V, found in data sheet
#define G_freq_ADCCLK_MAX_1_7 18000000UL
//
// Maximum ADC clock frequecy at 2.4V, found in data sheet
#define G_freq_ADCCLK_MAX_2_4 36000000UL
#define G_freq_ADCCLK_MAX G_freq_ADCCLK_MAX_2_4
//
// For non-F1 STM32s, assuming 3.3V Vcc, the number of wait states varies
// by line but on the conservative end would be 0 for <= 25MHz and +1 for
// each additional 25MHz above that, or more commonly 0 for <= 30MHz and
// +1 for each additional 30MHz above that up to a line-dependendent maximum.
// This information is in the Embedded Flash Memory Interface section of the
// reference manual but the subsection is different by device.
#define FLASH_WS_MAX 5U
#if REGULATED_VOLTAGE_mV
# if REGULATED_VOLTAGE_mV >= 2700
#  define FLASH_WS_STEP 30000000UL
# elif REGULATED_VOLTAGE_mV >= 2400
#  define FLASH_WS_STEP 24000000UL
# elif REGULATED_VOLTAGE_mV >= 2100
#  define FLASH_WS_STEP 18000000UL
# else
#  define FLASH_WS_STEP 16000000UL
# endif
#else
# define FLASH_WS_STEP 30000000UL
#endif

//
// This is used when setting up the PLL clock source
//
// The PLL output minimum isn't mentioned anywhere in the reference manual
// from what I can find but *is* mentioned in the data sheet in the section
// 'PLL characteristics'
// There's also a maximum but I don't care about that because it's just the
// sysclock maximum.
#define PLL_OUTPUT_MIN_HZ 24000000UL
#define PLL_PLLM_MIN 2U
#define PLL_PLLM_MAX 63U
#define PLL_VCO_INPUT_MIN_HZ 1000000UL
#define PLL_VCO_INPUT_MAX_HZ 2000000UL
//
// Acceptable VCO output is device-dependent; refer to the RCC PLL configuration
// register section of the reference manual
#define PLL_PLLN_MIN 192U
#define PLL_PLLN_MAX 432U
#define PLL_VCO_OUTPUT_MIN_HZ 192000000UL
#define PLL_VCO_OUTPUT_MAX_HZ 432000000UL


#endif // _uHAL_PLATFORM_CMSIS_STM32F401_H
