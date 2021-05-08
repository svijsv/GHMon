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

// Sleep for no more than this many seconds; this will generally be a hardware
// constraint or a guard against any programming errors that may result in an
// erroneous wakeup time because there's no other reason to wake up without
// one of the other periods elapsing. MAX_WAKEUP_SECONDS should be small enough
// that it won't overflow a 32 bit integer when added to the current uptime.
#define MAX_SLEEP_SECONDS (24 * 60 * 60) // 24 hours

//
// Interrupt priorities
// Range 0-15, overlap is OK
// Lower number is higher priority
#define SYSTICK_IRQp     1
#define BUTTON_IRQp      2
#define RTC_ALARM_IRQp   3
#define UARTx_IRQp       4
#define SLEEP_ALARM_IRQp 5


/*
* Types
*/


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
