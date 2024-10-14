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
//    This file is for hardware configuration of STM32F103 devices and should
//    only be included by platform.h
//
#ifndef _uHAL_PLATFORM_CMSIS_STM32F103_H
#define _uHAL_PLATFORM_CMSIS_STM32F103_H

#define HAVE_STM32F103 1

#include "stm32f1.h"

#if ! defined(RAM_PRESENT) || RAM_PRESENT <= 0
# undef  RAM_PRESENT
# if defined(STM32F103x4)
#  define RAM_PRESENT 0x01800U // 6KB
# elif defined(STM32F103x6)
#  define RAM_PRESENT 0x02800U // 10KB
# elif defined(STM32F103xB) || defined(STM32F103x8)
#  define RAM_PRESENT 0x05000U // 20KB
# elif defined(STM32F103xC)
#  define RAM_PRESENT 0x0C000U // 48KB
# elif defined(STM32F103xD) || defined(STM32F103xE)
#  define RAM_PRESENT 0x0FFFFU // 64KB
# elif defined(STM32F103xF) || defined(STM32F103xG)
#  define RAM_PRESENT 0x18000U // 98304B, 96KB
# else
#  error "Unhandled device"
# endif
#endif

//
// This is used when setting up the PLL clock source
//
// The PLL output minimum isn't mentioned anywhere in the reference manual
// from what I can find but *is* mentioned in the data sheet in the section
// 'PLL characteristics'
// There's also a maximum but I don't care about that because it's just the
// sysclock maximum.
#define PLL_OUTPUT_MIN_HZ 16000000UL
//#define PLL_OUTPUT_MAX_HZ 72000000UL


#endif // _uHAL_PLATFORM_CMSIS_STM32F103_H
