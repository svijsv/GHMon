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
// system.h
// General platform initialization
// NOTES:
//   Prototypes for some of the related functions are in interface.h
//
#ifndef _uHAL_PLATFORM_CMSIS_SYSTEM_H
#define _uHAL_PLATFORM_CMSIS_SYSTEM_H


#include "common.h"

//
// Interrupt priorities
// Range 0-15, overlap is OK
// Lower number is higher priority
#define SYSTICK_IRQp     1
#define GPIO_IRQp        2
#define RTC_ALARM_IRQp   3
#define UART_IRQp        4
#define SLEEP_ALARM_IRQp 5
#define USCOUNTER_IRQp   6


// Initialize/Enable/Disable one or more peripheral clocks
// Multiple clocks can be ORed together if they're on the same bus
void clock_init(rcc_periph_t periph_clock);
void clock_enable(rcc_periph_t periph_clock);
void clock_disable(rcc_periph_t periph_clock);
// Check if a peripheral clock is enabled.
bool clock_is_enabled(rcc_periph_t periph_clock);

// Enable/Disable writes to backup-domain registers
void BD_write_enable(void);
void BD_write_disable(void);


#endif // _uHAL_PLATFORM_CMSIS_SYSTEM_H
