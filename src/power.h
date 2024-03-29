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
// power.h
// Manage power to peripherals
// NOTES:
//

#ifdef __cplusplus
 extern "C" {
#endif
#ifndef _POWER_H
#define _POWER_H

/*
* Includes
*/
#include "config.h"
#include "common.h"


/*
* Static values
*/


/*
* Types
*/


/*
* Variable declarations (defined in power.c)
*/


/*
* Function prototypes (defined in power.c)
*/
// Initialize the power-management system
void power_init(void);

// Power the various peripherals on and off
void power_on_sensors(void);
void power_off_sensors(void);
void power_on_SPI(void);
void power_off_SPI(void);
void power_on_I2C(void);
void power_off_I2C(void);

// Power a specific pin on and off
// power_{on,off}_output() will handle PWM for any pin specified by PWM_PINS
// These are now declared in interface.h
/*
void power_on_PWM_output(pin_t pin, uint16_t duty_cycle);
void power_off_PWM_output(pin_t pin);
void power_on_output(pin_t pin);
void power_off_output(pin_t pin);
void power_on_input(pin_t pin);
void power_off_input(pin_t pin);
*/


/*
* Macros
*/


#endif // _POWER_H
#ifdef __cplusplus
 }
#endif
