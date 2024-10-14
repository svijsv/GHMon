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
// gpio.h
// Manage the GPIO peripheral
// NOTES:
//   Prototypes for most of the related functions are in interface.h
//
#ifndef _uHAL_PLATFORM_CMSIS_GPIO_H
#define _uHAL_PLATFORM_CMSIS_GPIO_H


#include "common.h"


typedef uint_fast8_t gpio_af_t;

// Initialize the GPIO peripherals
void gpio_init(void);

// Set the alternate function associated with a pin
// This does nothing for F1-line devices, they have a different method of
// pin remapping
void gpio_set_AF(gpio_pin_t pin, gpio_af_t af);

#if HAVE_STM32F1_GPIO
// Remap the UART1 pins:
void gpio_remap_uart1(void);
# ifdef TIM5
// Remap TIM5 channel 4 between the default PA3 and the LSI oscillator
// Returns 'true' if the channel was already remapped when called and false
// otherwise
bool gpio_remap_tim5ch4(void);
bool gpio_unremap_tim5ch4(void);
# endif
#endif


#endif // _uHAL_PLATFORM_CMSIS_GPIO_H
