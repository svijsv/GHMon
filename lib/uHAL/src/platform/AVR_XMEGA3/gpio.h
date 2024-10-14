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
#ifndef _uHAL_PLATFORM_XMEGA3_GPIO_H
#define _uHAL_PLATFORM_XMEGA3_GPIO_H

#include "common.h"


// Initialize the GPIO peripherals
void gpio_init(void);

// Get the operating mode of a pin
// All output modes are returned as GPIO_MODE_PP
// GPIO_MODE_HiZ and GPIO_MODE_RESET may be returned as GPIO_MODE_AIN
//gpio_mode_t gpio_get_mode(gpio_pin_t pin);

// This exposes low-level gpio functionality to enable a bug work-around
// for I2C
void gpio_clear_outbit(gpio_pin_t pin);


#endif // _uHAL_PLATFORM_XMEGA3_GPIO_H
