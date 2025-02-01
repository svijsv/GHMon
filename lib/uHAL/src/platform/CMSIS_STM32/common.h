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
// common.h
// Platform-specific common header
// NOTES:
//
#ifndef _uHAL_PLATFORM_CMSIS_COMMON_H
#define _uHAL_PLATFORM_CMSIS_COMMON_H


#include "include/interface.h"

#include "ulib/include/debug.h"
#include "ulib/include/types.h"
#include "ulib/include/bits.h"
#include "ulib/include/time.h"
#include "ulib/include/util.h"

// For some reason these values aren't always defined
#if !defined(RCC_BDCR_RTCSEL_LSE)
# define RCC_BDCR_RTCSEL_NOCLOCK 0
# define RCC_BDCR_RTCSEL_LSE (0b01U << RCC_BDCR_RTCSEL_Pos)
# define RCC_BDCR_RTCSEL_LSI (0b10U << RCC_BDCR_RTCSEL_Pos)
# define RCC_BDCR_RTCSEL_HSE (0b11U << RCC_BDCR_RTCSEL_Pos)
#endif

#define NEED_RTC (uHAL_USE_RTC || uHAL_USE_UPTIME || uHAL_USE_HIBERNATE)
#define USE_RTC_UPTIME (uHAL_USE_UPTIME && ! uHAL_USE_UPTIME_EMULATION)

#endif // _uHAL_PLATFORM_CMSIS_COMMON_H
