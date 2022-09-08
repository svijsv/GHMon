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


#if USE_UART

/*
* Static values
*/
// Helper macros for below
#if USE_STM32F1_GPIO
// STM32F1 requires that pins either all be remapped or all left alone.
# define IS_UART1(rx_pin, tx_pin) (((PINID(rx_pin) == PIN_UART1_RX)       && (PINID(tx_pin) == PIN_UART1_TX)) || \
                                  ((PINID(rx_pin) == PIN_UART1_REMAP_RX) && (PINID(tx_pin) == PIN_UART1_REMAP_TX)))
#else // ! STM32F1_GPIO
// Non-F1 STM32s allow mixing remapped and default pins
# define IS_UART1(rx_pin, tx_pin) (((PINID(rx_pin) == PIN_UART1_RX) || (PINID(rx_pin) == PIN_UART1_REMAP_RX)) && \
                                  ((PINID(tx_pin) == PIN_UART1_TX) || (PINID(tx_pin) == PIN_UART1_REMAP_TX)))
#endif // ! STM32F1_GPIO

// USART2 and USART3 can be remapped, but the remapped pins aren't accessible
// on the bluepill
#if defined(USART2)
# define IS_UART2(rx_pin, tx_pin) ((PINID(rx_pin) == PIN_UART2_RX) && (PINID(tx_pin) == PIN_UART2_TX))
#else
# define IS_UART2(rx_pin, tx_pin) (false)
#endif
#if defined(USART3)
# define IS_UART3(rx_pin, tx_pin) ((PINID(rx_pin) == PIN_UART3_RX) && (PINID(tx_pin) == PIN_UART3_TX))
#else
# define IS_UART3(rx_pin, tx_pin) (false)
#endif

//
// Determine UART peripheral used for serial communication.
// USART1
#if USE_UART_COMM
# if IS_UART1(UART_COMM_RX_PIN, UART_COMM_TX_PIN)
#  define UART_COMM_IRQn       USART1_IRQn
#  define UART_COMM_IRQHandler USART1_IRQHandler
#  define UART_COMM_CLOCKEN    RCC_PERIPH_UART1

// USART2
# elif IS_UART2(UART_COMM_RX_PIN, UART_COMM_TX_PIN)
#  define UART_COMM_IRQn       USART2_IRQn
#  define UART_COMM_IRQHandler USART2_IRQHandler
#  define UART_COMM_CLOCKEN    RCC_PERIPH_UART2

// USART3
# elif IS_UART3(UART_COMM_RX_PIN, UART_COMM_TX_PIN)
#  define UART_COMM_IRQn       USART3_IRQn
#  define UART_COMM_IRQHandler USART3_IRQHandler
#  define UART_COMM_CLOCKEN    RCC_PERIPH_UART3

# else
#  error "Can't determine comm UART peripheral"
# endif // IS_UARTx()

// This isn't used for anything anymore but it's good to get the warning
// during compilation instead failing at runtime
# if (UART_COMM_CLOCKEN & RCC_BUS_MASK) == RCC_BUS_APB1
#  define UART_COMM_BUSFREQ G_freq_PCLK1
# elif (UART_COMM_CLOCKEN & RCC_BUS_MASK) == RCC_BUS_APB2
#  define UART_COMM_BUSFREQ G_freq_PCLK2
# elif (UART_COMM_CLOCKEN & RCC_BUS_MASK) == RCC_BUS_AHB1
#  define UART_COMM_BUSFREQ G_freq_HCLK
# else
#  error "Can't determine comm UART bus clock"
# endif
#endif // USE_UART_COMM


/*
* Types
*/
struct uart_port_t {
	volatile __IO USART_TypeDef *uart;
	// Need to know the pins and clock when turning the peripheral on or off.
	rcc_periph_t clocken;
	pin_t rx_pin;
	pin_t tx_pin;
};


/*
* Variable declarations (defined in uart.c)
*/


/*
* Function prototypes (defined in uart.c)
*/


/*
* Macros
*/


#endif // USE_UART

#endif // _PLATFORM_CMSIS_UART_H
#ifdef __cplusplus
 }
#endif
