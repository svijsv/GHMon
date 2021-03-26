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
// uart.h
// Manage the UART peripheral
// NOTES:
//   Prototypes for most of the related functions are in interface.h
//

#ifdef __cplusplus
 extern "C" {
#endif
#ifndef _PLATFORM_ATMEGA_UART_H
#define _PLATFORM_ATMEGA_UART_H

/*
* Includes
*/
#include "common.h"


#if USE_SERIAL

/*
* Static values
*/


/*
* Types
*/


/*
* Variable declarations
*/


/*
* Function prototypes
*/
// Initialize the UART interface at the chosen baud rate
// The interface is always configured as 8 data bits with 1 stop bit and no
// parity bit with a baud rate of UART_BAUDRATE.
void uart_init(void);

// Turn the UART receive interrupt on and off
void uart_listen_on(void);
void uart_listen_off(void);

/*
* Macros
*/


#endif // USE_SERIAL

#endif // _PLATFORM_ATMEGA_UART_H
#ifdef __cplusplus
 }
#endif
