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
// config.h
// Program configuration file
// NOTES:
//
#ifndef _CONFIG_H
#define _CONFIG_H

#include "../lib/uHAL/config/config_uHAL.h"

//
// Helpful macros
//
#define PRINT_REG(name) (PRINTF("%s: 0x%02X\r\n", #name, (uint_t )name))
#define PRINT_UVAL(name) (PRINTF("%s: %u\r\n", #name, (uint_t )name))
#define PRINT_SVAL(name) (PRINTF("%s: %i\r\n", #name, (int_t )name))


//
// Basic system configuration
//
#undef uHAL_USE_SUBSYSTEM_DEFAULT
#define uHAL_USE_SUBSYSTEM_DEFAULT 1
#undef uHAL_USE_DRIVER_DEFAULT
#define uHAL_USE_DRIVER_DEFAULT    1

#define PRINT_PLATFORM_INFO 0
#define PAUSE_MS 5000U

#undef uHAL_USE_RTC
#define uHAL_USE_RTC 1

// Useful for AVR, size with these set is closer to release build but with
// compile info and debugging symbols
// LOGGER() and ERROR_STATE() are responsible for the remaining difference i think
#undef uHAL_USE_SMALL_CODE
#if defined(HAVE_AVR_XMEGA3) && HAVE_AVR_XMEGA3 > 0
# define uHAL_USE_SMALL_CODE 2
# define ERROR_STATE(msg)     error_state_crude()
# define ERROR_STATE_NOF(msg) error_state_crude()
#else
# define uHAL_USE_SMALL_CODE 0
# undef uHAL_USE_UART
# undef uHAL_USE_UART_COMM
# define uHAL_USE_UART 1
# define uHAL_USE_UART_COMM 1
#endif

#undef UART_COMM_BUFFER_BYTES
#undef TERMINAL_BUFFER_BYTES
#if uHAL_USE_SMALL_CODE
// Setting NDEBUG instead of redefining assert() results in assert() still being
// used if debug.h is included before the config file (which it may or may not be)
# define NDEBUG 1
//# undef assert
//# define assert(_x_) (void )0U
# define UART_COMM_BUFFER_BYTES 0U
# define TERMINAL_BUFFER_BYTES  0U
#else
# define UART_COMM_BUFFER_BYTES 64U
# define TERMINAL_BUFFER_BYTES  64U
#endif

#undef uHAL_SKIP_INVALID_ARG_CHECKS
#undef uHAL_SKIP_INIT_CHECKS
#undef uHAL_SKIP_OTHER_CHECKS
#define uHAL_SKIP_INVALID_ARG_CHECKS uHAL_USE_SMALL_CODE
#define uHAL_SKIP_INIT_CHECKS uHAL_USE_SMALL_CODE
#define uHAL_SKIP_OTHER_CHECKS uHAL_USE_SMALL_CODE


//
// Test configuration
//
#define TEST_LED          1
#define TEST_LED_PINCTRL  0
#define TEST_LED_PINCTRL2 0

#define TEST_PWM 0
#define TEST_PWM_STEPS 3
//#undef PWM_FREQUENCY_HZ
//#define PWM_FREQUENCY_HZ 50000UL

#define TEST_BUTTON 0
#define TEST_BUTTON_DEBOUNCE_MS 100U
#define TEST_BUTTON_PRESS_PERIOD_MS 1000U
#define TEST_BUTTON_PRESS_PERIOD_MAX_COUNT 10U

#define TEST_USCOUNTER 0

#define TEST_DATE 0
//#define TEST_DATE_HOUR     3
#define TEST_DATE_HOUR     0
#define TEST_DATE_MINUTE  44
#define TEST_DATE_SECOND  11
//#define TEST_DATE_YEAR    24
#define TEST_DATE_YEAR     0
#define TEST_DATE_MONTH    9
#define TEST_DATE_DAY     18

#define TEST_ADC 0
#undef ADC_SAMPLE_uS
#define ADC_SAMPLE_uS 10
#undef ADC_SAMPLE_COUNT
#define ADC_SAMPLE_COUNT 4U

#define TEST_SD 0
#define TEST_SD_FILE_NAME "SD_TEST.txt"
#define TEST_SD_READ_BUF_SIZE 64
//#undef SPI_FREQUENCY_HZ
//#define SPI_FREQUENCY_HZ 100000UL

#define TEST_UART_LISTEN 0
#undef UART_INPUT_BUFFER_BYTES
#define UART_INPUT_BUFFER_BYTES 8U
//#define UART_INPUT_BUFFER_BYTES 0U

#define TEST_TERMINAL 0
#undef TERMINAL_HAVE_EXTRA_CMDS
#define TERMINAL_HAVE_EXTRA_CMDS TEST_TERMINAL
#define TEST_TERMINAL_LED_PIN LED_PIN

#define TEST_SSD1306 0
#define TEST_SSD1306_ADDR 0x3CU
//#define TEST_SSD1306_ADDR 0x78U // Boards that claim to be this are probably wrong

#define TEST_SLEEP 0

#undef uHAL_ANNOUNCE_HIBERNATE
#define uHAL_ANNOUNCE_HIBERNATE 0

#define TEST_HIBERNATE 1
//#undef uHAL_HIBERNATE_LIMIT
//#define uHAL_HIBERNATE_LIMIT HIBERNATE_DEEP
// Deep sleep interferes with PWM (at least on STM32s)
#define TEST_HIBERNATE_MODE HIBERNATE_LIGHT

// Best not to enable this unless TEST_BUTTON or TEST_UART_LISTEN are set.
#define TEST_HIBERNATE_FOREVER 0
#define TEST_HIBERNATE_FOREVER_MODE HIBERNATE_LIGHT

#define TEST_RESET 0
#define TEST_RESET_LOOPS 3

#include "reconcile.h"

#endif // _CONFIG_H
