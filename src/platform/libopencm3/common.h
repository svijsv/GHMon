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
#ifndef _PLATFORM_LIBOPENCM3_COMMON_H
#define _PLATFORM_LIBOPENCM3_COMMON_H

/*
* Includes
*/
#include "platform/interface.h"
#include "platform.h"
#include "config.h"

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
#define LIGHT_SLEEP_PERIOD 5
// If a deep sleep would last fewer than this many seconds, light sleep a bit
// longer instead
#define MIN_DEEP_SLEEP_PERIOD 2

// Sleep for no more than this many seconds; this will generally be a hardware
// constraint or a guard against any programming errors that may result in an
// erroneous wakeup time because there's no other reason to wake up without
// one of the other periods elapsing. MAX_WAKEUP_PERIOD should be small enough
// that it won't overflow a 32 bit integer when added to the current uptime.
#define MAX_SLEEP_PERIOD (24 * 60 * 60) // 24 hours

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
//#define DFLASH(t) do { gpio_set_state(LED_PIN, GPIO_HIGH); dumb_delay(t); gpio_set_state(LED_PIN, GPIO_LOW); } while (0);
#define DFLASH(t) do { gpio_toggle_state(LED_PIN); dumb_delay(t); gpio_toggle_state(LED_PIN); } while (0);

// Quick pin access, for when you know what you want:
#define IS_GPIO_INPUT_HIGH(pin)  (GPIO_IDR(BIT_IS_SET(GPIO_GET_PORT(pin)), GPIO_GET_PINMASK(pin)))
#define SET_GPIO_OUTPUT_HIGH(pin) SET_BIT(GPIO_ODR(GPIO_GET_PORT(pin)), GPIO_GET_PINMASK(pin))
#define SET_GPIO_OUTPUT_LOW(pin)  CLEAR_BIT(GPIO_ODR(GPIO_GET_PORT(pin)), GPIO_GET_PINMASK(pin))


#endif // _PLATFORM_LIBOPENCM3_COMMON_H
#ifdef __cplusplus
 }
#endif
