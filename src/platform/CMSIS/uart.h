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
#if (PINID(UART_TX_PIN) == PIN_UART1_REMAP_TX) && (PINID(UART_RX_PIN) == PIN_UART1_REMAP_RX)
# define UART1_DO_REMAP 1
#endif

#if USE_STM32F1_GPIO
# define IS_UART1_TX ((PINID(UART_TX_PIN) == PIN_UART1_TX) || UART1_DO_REMAP)
# define IS_UART1_RX ((PINID(UART_RX_PIN) == PIN_UART1_RX) || UART1_DO_REMAP)
#else
# define IS_UART1_TX ((PINID(UART_TX_PIN) == PIN_UART1_TX) || (PINID(UART_TX_PIN) == PIN_UART1_REMAP_TX))
# define IS_UART1_RX ((PINID(UART_RX_PIN) == PIN_UART1_RX) || (PINID(UART_RX_PIN) == PIN_UART1_REMAP_RX))
#endif
// USART2 and USART3 can be remapped, but the remapped pins aren't accessible
// on the bluepill
#define IS_UART2_TX ((PINID(UART_TX_PIN) == PIN_UART2_TX))
#define IS_UART2_RX ((PINID(UART_RX_PIN) == PIN_UART2_RX))
#define IS_UART3_TX ((PINID(UART_TX_PIN) == PIN_UART3_TX))
#define IS_UART3_RX ((PINID(UART_RX_PIN) == PIN_UART3_RX))

// USART1
#if IS_UART1_TX && IS_UART1_RX
# define UARTx USART1
# define UARTx_IRQn       USART1_IRQn
# define UARTx_IRQHandler USART1_IRQHandler
# define UARTx_CLOCKEN    RCC_PERIPH_UART1
# define UARTx_AF         AF_UART1

// USART2
#elif IS_UART2_TX && IS_UART2_RX
# define UARTx USART2
# define UARTx_IRQn       USART2_IRQn
# define UARTx_IRQHandler USART2_IRQHandler
# define UARTx_CLOCKEN    RCC_PERIPH_UART2
# define UARTx_AF         AF_UART2

// USART3
#elif IS_UART3_TX && IS_UART3_RX
# define UARTx USART3
# define UARTx_IRQn       USART3_IRQn
# define UARTx_IRQHandler USART3_IRQHandler
# define UARTx_CLOCKEN    RCC_PERIPH_UART3
# define UARTx_AF         AF_UART3

#else
# error "Can't determine UART peripheral"
#endif // UARTx

#if (UARTx_CLOCKEN & RCC_BUS_MASK) == RCC_BUS_APB1
# define UARTx_BUSFREQ G_freq_PCLK1
#elif (UARTx_CLOCKEN & RCC_BUS_MASK) == RCC_BUS_APB2
# define UARTx_BUSFREQ G_freq_PCLK2
#elif (UARTx_CLOCKEN & RCC_BUS_MASK) == RCC_BUS_AHB1
# define UARTx_BUSFREQ G_freq_HCLK
#else
# error "Can't determine UART bus clock"
#endif


/*
* Types
*/


/*
* Variable declarations (defined in uart.c)
*/


/*
* Function prototypes (defined in uart.c)
*/
// Initialize the UART interface
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
