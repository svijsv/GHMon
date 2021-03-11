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
// gpio.h
// Manage the GPIO peripheral
// NOTES:
//   Prototypes for most of the related functions are in interface.h
//

#ifdef __cplusplus
 extern "C" {
#endif
#ifndef _PLATFORM_CMSIS_GPIO_H
#define _PLATFORM_CMSIS_GPIO_H

/*
* Includes
*/
#include "config.h"
#include "stm32f103.h"


/*
* Static values
*/


/*
* Types
*/
//
// Most associated types are defined in interface.h
//


/*
* Variable declarations (defined in gpio.c)
*/


/*
* Function prototypes (defined in gpio.c)
*/
// Initialize the GPIO peripherals
void gpio_init(void);

// Get the operating mode of a pin
// GPIO_MODE_RESET is returned as GPIO_MODE_IN
// GPIO_MODE_HiZ is returned as GPIO_MODE_AIN
//gpio_mode_t gpio_get_mode(pin_t pin);


/*
* Macros
*/


#endif // _PLATFORM_CMSIS_GPIO_H
#ifdef __cplusplus
 }
#endif
