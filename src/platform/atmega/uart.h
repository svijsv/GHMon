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


#if USE_UART

/*
* Static values
*/


/*
* Types
*/
struct uart_port_t {
	uint16_t ubrr;
	pin_t rx_pin;
	pin_t tx_pin;
	uint8_t uart_portno;
};


/*
* Variable declarations
*/


/*
* Function prototypes
*/
// Turn the UART receive interrupt on and off
void uart_listen_on(const uart_port_t *p);
void uart_listen_off(const uart_port_t *p);

/*
* Macros
*/
#define IS_UART0(rx_pin, tx_pin) ((PINID(rx_pin) == PINID_RX0) || (PINID(tx_pin) != PINID_TX0))


#endif // USE_UART

#endif // _PLATFORM_ATMEGA_UART_H
#ifdef __cplusplus
 }
#endif
