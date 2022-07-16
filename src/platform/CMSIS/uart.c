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
// uart.c
// Manage the UART peripheral
// NOTES:
//

#ifdef __cplusplus
 extern "C" {
#endif

//#define DEBUG 1
/*
* Includes
*/
#include "uart.h"
#include "system.h"
#include "gpio.h"


#if USE_UART

#if (UARTx_BUSFREQ/UART_BAUDRATE) > (0xFFFF)
# error "UART frequency is too low for the system clock"
#endif


/*
* Static values
*/


/*
* Types
*/


/*
* Variables
*/


/*
* Local function prototypes
*/
static uint16_t calculate_baud_div(uint32_t baud);


/*
* Interrupt handlers
*/
OPTIMIZE_FUNCTION \
void UARTx_IRQHandler(void) {
	NVIC_DisableIRQ(UARTx_IRQn);
	NVIC_ClearPendingIRQ(UARTx_IRQn);

	// There are no other status flags to clear for this IRQ

	// UART_IRQf is the main loop's IRQ flag, UARTx_IRQn is for the CMSIS
	SET_BIT(G_IRQs, UART_IRQf);

	return;
}


/*
* Functions
*/
err_t uart_init(void) {
	clock_init(UARTx_CLOCKEN);

	gpio_set_AF(UART_TX_PIN, UARTx_AF);
	gpio_set_AF(UART_RX_PIN, UARTx_AF);

	MODIFY_BITS(UARTx->CR1, USART_CR1_M|USART_CR1_PCE|USART_CR1_PS|USART_CR1_RXNEIE|USART_CR1_UE|USART_CR1_TE|USART_CR1_RE,
		(0b0 << USART_CR1_M_Pos     ) | // 0 for 8 data bits
		(0b0 << USART_CR1_PCE_Pos   ) | // 0 to disable parity
		(0b0 << USART_CR1_PS_Pos    ) | // 0 for even parity, 1 for odd parity
		(0b1 << USART_CR1_RXNEIE_Pos) | // RXNE interrupt enable
		(0b1 << USART_CR1_TE_Pos    ) | // Enable transmission
		(0b1 << USART_CR1_RE_Pos    ) | // Enable reception
		0);
	MODIFY_BITS(UARTx->CR2, USART_CR2_STOP,
		(0b00 << USART_CR2_STOP_Pos) | // Keep at 00 for 1 stop bit
		0);

	UARTx->BRR = calculate_baud_div(UART_BAUDRATE);

	NVIC_SetPriority(UARTx_IRQn, UARTx_IRQp);

	// Don't turn UART off after initializing; that's only done for hibernation
	// when it can't be used anyway
	//uart_off();
	uart_on();

	return EOK;
}
static void pins_on(void) {
	// Peripheral pin modes specified in the STM32F1 reference manual section
	// 9.1.11
	// I can't find specifications for the other devices, I'm assuming they
	// just need to be AF
	gpio_set_mode(UART_TX_PIN, GPIO_MODE_PP_AF, GPIO_FLOAT);
	gpio_set_mode(UART_RX_PIN, GPIO_MODE_IN_AF, GPIO_FLOAT);

	return;
}
static void pins_off(void) {
	gpio_set_mode(UART_TX_PIN, GPIO_MODE_HiZ, GPIO_FLOAT);
	gpio_set_mode(UART_RX_PIN, GPIO_MODE_HiZ, GPIO_FLOAT);

	return;
}
void uart_on(void) {
	clock_enable(UARTx_CLOCKEN);
	pins_on();
	SET_BIT(UARTx->CR1, USART_CR1_UE);

	return;
}
void uart_off(void) {
	CLEAR_BIT(UARTx->CR1, USART_CR1_UE);
	pins_off();
	clock_disable(UARTx_CLOCKEN);

	return;
}

err_t uart_transmit_block(const uint8_t *buffer, txsize_t size, utime_t timeout) {
	err_t res;

	res = EOK;
	timeout = SET_TIMEOUT(timeout);

	for (txsize_t i = 0; i < size; ++i) {
		UARTx->DR = buffer[i];
		while (!BIT_IS_SET(UARTx->SR, USART_SR_TXE)) {
			if (TIMES_UP(timeout)) {
				res = ETIMEOUT;
				goto END;
			}
		}
	}
	while (!BIT_IS_SET(UARTx->SR, USART_SR_TC)) {
		if (TIMES_UP(timeout)) {
			res = ETIMEOUT;
			break;
		}
	}

END:
	return res;
}
err_t uart_receive_block(uint8_t *buffer, txsize_t size, utime_t timeout) {
	err_t res;

	res = EOK;
	timeout = SET_TIMEOUT(timeout);

	for (txsize_t i = 0; i < size; ++i) {
		while (!BIT_IS_SET(UARTx->SR, USART_SR_RXNE)) {
			if (TIMES_UP(timeout)) {
				res = ETIMEOUT;
				goto END;
			}
		}
		buffer[i] = UARTx->DR;
	}

END:
	return res;
}

static uint16_t calculate_baud_div(uint32_t baud) {
	// From section 27.3.4 of the STM32F1 reference manual:
	//   baud = pclk/(16*div)
	// Therefore:
	//   (16*div) = pclk/baud
	// The baud rate is programmed into a 16-bit register configured as a
	// fixed-point rational number with 12 bits of mantissa and 4 bits of
	// fractional part, which (I think) makes the whole register (16*div).
	// That makes more sense to me than the description in the reference manual
	// does.
	return (UARTx_BUSFREQ/baud);
}


#endif // USE_UART
#ifdef __cplusplus
 }
#endif
