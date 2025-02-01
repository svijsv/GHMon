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
// reconcile.h
// Enable required subsystems and check for test conflicts
//
// NOTES:
//    The pin IDs aren't defined yet so we can't check for test incompatibilities
//    that way, we handle it in gether_test.h instead.
//
#ifndef _CONFIG_RECONCILE_H
#define _CONFIG_RECONCILE_H

#if TEST_LED
/*
# if TEST_LED_PINCTRL && LED_PIN == LED_PINCTRL_PIN
#  error "TEST_LED and TEST_LED_PINCTRL can't be run at the same time when they use the same pin."
# endif
# if TEST_LED_PINCTRL2 && LED_PIN == LED_PINCTRL2_PIN
#  error "TEST_LED and TEST_LED_PINCTRL2 can't be run at the same time when they use the same pin."
# endif
# if TEST_PWM && LED_PIN == PWM_LED_PIN
#  error "TEST_LED and TEST_PWM can't be run at the same time when they use the same pin."
# endif
*/
#endif

#if TEST_LED_PINCTRL
# undef uHAL_USE_HIGH_LEVEL_GPIO
# define uHAL_USE_HIGH_LEVEL_GPIO 1

/*
# if TEST_LED_PINCTRL2 && LED_PINCTRL_PIN == LED_PINCTRL2_PIN
#  error "TEST_LED_PINCTRL and TEST_LED_PINCTRL2 can't be run at the same time when they use the same pin."
# endif
# if TEST_PWM && LED_PINCTRL_PIN == PWM_LED_PIN
#  error "TEST_PINCTRL_LED and TEST_PWM can't be run at the same time when they use the same pin."
# endif
*/
#endif

#if TEST_LED_PINCTRL2
# undef uHAL_USE_EXPERIMENTAL_GPIO_INTERFACE
# define uHAL_USE_EXPERIMENTAL_GPIO_INTERFACE 1

/*
# if TEST_PWM && LED_PINCTRL2_PIN == PWM_LED_PIN
#  error "TEST_PINCTRL2_LED and TEST_PWM can't be run at the same time when they use the same pin."
# endif
*/
#endif

#if TEST_PWM
# undef uHAL_USE_PWM
# define uHAL_USE_PWM 1
#endif

#if TEST_BUTTON
# undef uHAL_USE_HIGH_LEVEL_GPIO
# define uHAL_USE_HIGH_LEVEL_GPIO 1
#endif

#if TEST_USCOUNTER
# undef uHAL_USE_USCOUNTER
# define uHAL_USE_USCOUNTER 1
#endif

#if TEST_DATE
#endif

#if TEST_ADC
# undef uHAL_USE_ADC
# define uHAL_USE_ADC 1
#endif

#if TEST_SD
# undef uHAL_USE_SPI
# undef uHAL_USE_SD
# define uHAL_USE_SPI 1
# define uHAL_USE_SD 1
#endif

#if TEST_UART_LISTEN
# undef uHAL_USE_UART
# undef uHAL_USE_UART_COMM
# undef ENABLE_UART_LISTENING
# define uHAL_USE_UART 1
# define uHAL_USE_UART_COMM 1
# define ENABLE_UART_LISTENING 1

# if TEST_TERMINAL
#  error "TEST_TERMINAL and TEST_UART_LISTEN can't both be enabled"
# endif
#endif

#if TEST_TERMINAL
# undef uHAL_USE_UART
# undef uHAL_USE_UART_COMM
# undef uHAL_USE_TERMINAL
# undef uHAL_USE_HIGH_LEVEL_GPIO
# define uHAL_USE_UART 1
# define uHAL_USE_UART_COMM 1
# define uHAL_USE_TERMINAL 1
# define uHAL_USE_HIGH_LEVEL_GPIO 1
#endif

#if TEST_SSD1306
# undef uHAL_USE_I2C
# undef uHAL_USE_DISPLAY_SSD1306
# define uHAL_USE_I2C 1
# define uHAL_USE_DISPLAY_SSD1306 1
#endif

#if TEST_SLEEP
# undef uHAL_USE_HIBERNATE
# define uHAL_USE_HIBERNATE 1

# if TEST_HIBERNATE
#  warning "TEST_SLEEP will override TEST_HIBERNATE"
# endif
#endif

#if TEST_HIBERNATE
# undef uHAL_USE_HIBERNATE
# define uHAL_USE_HIBERNATE 1

# if TEST_HIBERNATE_FOREVER
#  warning "TEST_HIBERNATE will override TEST_HIBERNATE_FOREVER"
# endif
#endif

#if TEST_HIBERNATE_FOREVER
#endif


#endif // _CONFIG_RECONCILE_H
