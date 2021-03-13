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
#ifndef _PLATFORM_CMSIS_UART_H
#define _PLATFORM_CMSIS_UART_H

/*
* Includes
*/
#include "config.h"
#include "stm32f103.h"


#if USE_SERIAL

/*
* Static values
*/


/*
* Types
*/


/*
* Variable declarations (defined in uart.c)
*/


/*
* Function prototypes (defined in uart.c)
*/
// Initialize the UART interface at the chosen baud rate
// The interface is always configured as 8 data bits with 1 stop bit and no
// parity bit with a baud rate of UART_BAUDRATE.
err_t uart_init(void);


/*
* Macros
*/


#endif // USE_SERIAL

#endif // _PLATFORM_CMSIS_UART_H
#ifdef __cplusplus
 }
#endif
