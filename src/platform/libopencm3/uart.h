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
#ifndef _PLATFORM_LIBOPENCM3_UART_H
#define _PLATFORM_LIBOPENCM3_UART_H

/*
* Includes
*/
#include "common.h"


#if USE_SERIAL

/*
* Static values
*/
//
// Handle UARTx
// USART1 remapped
#if (UARTx_TX_PIN == PB6) && (UARTx_RX_PIN == PB7)
# define UART1_DO_REMAP 1
#endif
// USART2 and USART3 can be remapped, but the remapped pins aren't accessible
// on the bluepill

// USART1
#if ((UARTx_TX_PIN == PA9) && (UARTx_RX_PIN == PA10)) || UART1_DO_REMAP
# define UARTx USART1
# define UARTx_IRQn       NVIC_USART1_IRQ
# define UARTx_IRQHandler usart1_isr
# define UARTx_RCC RCC_USART1
# define UARTx_RST RST_USART1

// USART2
#elif (UARTx_TX_PIN == PA2) && (UARTx_RX_PIN == PA3)
# define UARTx USART2
# define UARTx_IRQn       NVIC_USART2_IRQ
# define UARTx_IRQHandler usart2_isr
# define UARTx_RCC RCC_USART2
# define UARTx_RST RST_USART2

// USART3
#elif (UARTx_TX_PIN == PB10) && (UARTx_RX_PIN == PB11)
# define UARTx USART3
# define UARTx_IRQn       NVIC_USART3_IRQ
# define UARTx_IRQHandler usart3_isr
# define UARTx_RCC RCC_USART3
# define UARTx_RST RST_USART3

#else
# error "Can't determine UART peripheral"
#endif // UARTx


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

#endif // _PLATFORM_LIBOPENCM3_UART_H
#ifdef __cplusplus
 }
#endif
