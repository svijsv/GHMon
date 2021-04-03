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
#include "common.h"


#if USE_SERIAL

/*
* Static values
*/
//
// Handle UARTx
// USART1 remapped
#if (PINID(UARTx_TX_PIN) == PINID_B6) && (PINID(UARTx_RX_PIN) == PINID_B7)
# define UART1_DO_REMAP 1
#endif
// USART2 and USART3 can be remapped, but the remapped pins aren't accessible
// on the bluepill

// USART1
#if ((PINID(UARTx_TX_PIN) == PINID_A9) && (PINID(UARTx_RX_PIN) == PINID_A10)) || UART1_DO_REMAP
# define UARTx USART1
# define UARTx_IRQn     USART1_IRQn
# define UARTx_IRQHandler USART1_IRQHandler
# define UARTx_APBxENR  RCC->APB2ENR
# define UARTx_APBxRSTR RCC->APB2RSTR
# define UARTx_CLOCKEN  RCC_APB2ENR_USART1EN
# define UARTx_BUSFREQ  G_freq_PCLK2

// USART2
#elif (PINID(UARTx_TX_PIN) == PINID_A2) && (PINID(UARTx_RX_PIN) == PINID_A3)
# define UARTx USART2
# define UARTx_IRQn     USART2_IRQn
# define UARTx_IRQHandler USART2_IRQHandler
# define UARTx_APBxENR  RCC->APB1ENR
# define UARTx_APBxRSTR RCC->APB1RSTR
# define UARTx_CLOCKEN  RCC_APB1ENR_USART2EN
# define UARTx_BUSFREQ  G_freq_PCLK1

// USART3
#elif (PINID(UARTx_TX_PIN) == PINID_B10) && (PINID(UARTx_RX_PIN) == PINID_B11)
# define UARTx USART3
# define UARTx_IRQn     USART3_IRQn
# define UARTx_IRQHandler USART3_IRQHandler
# define UARTx_APBxENR  RCC->APB1ENR
# define UARTx_APBxRSTR RCC->APB1RSTR
# define UARTx_CLOCKEN  RCC_APB1ENR_USART3EN
# define UARTx_BUSFREQ  G_freq_PCLK1

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

#endif // _PLATFORM_CMSIS_UART_H
#ifdef __cplusplus
 }
#endif
