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

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/usart.h>


#if USE_SERIAL

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


/*
* Interrupt handlers
*/
OPTIMIZE_FUNCTION \
void UARTx_IRQHandler(void) {
	nvic_disable_irq(UARTx_IRQn);
	nvic_clear_pending_irq(UARTx_IRQn);

	// There are no other status flags to clear for this IRQ

	SET_BIT(G_IRQs, UART_IRQ);

	return;
}


/*
* Functions
*/
err_t uart_init(void) {
	rcc_periph_clock_enable(UARTx_RCC);
	rcc_periph_reset_pulse(UARTx_RST);

	usart_set_baudrate(UARTx, UART_BAUDRATE);
	usart_set_databits(UARTx, 8);
	usart_set_flow_control(UARTx, USART_FLOWCONTROL_NONE);
	usart_set_mode(UARTx, USART_MODE_TX_RX);
	usart_set_parity(UARTx, USART_PARITY_NONE);
	usart_set_stopbits(UARTx, USART_STOPBITS_1);
	usart_enable_rx_interrupt(UARTx);
	usart_enable(UARTx);

	// Peripheral pin modes specified in reference manual section 9.1.11
	gpio_set_mode(UARTx_TX_PIN, GPIO_MODE_PP_AF, GPIO_LOW);
	gpio_set_mode(UARTx_RX_PIN, GPIO_MODE_IN,    GPIO_FLOAT);

	nvic_set_priority(UARTx_IRQn, UARTx_IRQp);

	return EOK;
}
void uart_on(void) {
	// Peripheral pin modes specified in reference manual section 9.1.11
	gpio_set_mode(UARTx_TX_PIN, GPIO_MODE_PP_AF, GPIO_LOW);
	gpio_set_mode(UARTx_RX_PIN, GPIO_MODE_IN,    GPIO_FLOAT);

	rcc_periph_clock_enable(UARTx_RCC);

	return;
}
void uart_off(void) {
	rcc_periph_clock_disable(UARTx_RCC);

	gpio_set_mode(UARTx_TX_PIN, GPIO_MODE_HiZ, GPIO_FLOAT);
	gpio_set_mode(UARTx_RX_PIN, GPIO_MODE_HiZ, GPIO_FLOAT);

	return;
}

err_t uart_transmit_block(const uint8_t *buffer, uint32_t size, utime_t timeout) {
	err_t res;

	res = EOK;
	timeout = SET_TIMEOUT(timeout);

	for (uint32_t i = 0; i < size; ++i) {
		usart_send_blocking(UARTx, buffer[i]);
		if (TIMES_UP(timeout)) {
			res = ETIMEOUT;
			break;
		}
	}
	while (!usart_get_flag(UARTx, USART_SR_TC)) {
		if (TIMES_UP(timeout)) {
			res = ETIMEOUT;
			break;
		}
	}

	return res;
}
err_t uart_receive_block(uint8_t *buffer, uint32_t size, utime_t timeout) {
	err_t res;

	res = EOK;
	timeout = SET_TIMEOUT(timeout);

	for (uint32_t i = 0; i < size; ++i) {
		while (!usart_get_flag(UARTx, USART_SR_RXNE)) {
			if (TIMES_UP(timeout)) {
				res = ETIMEOUT;
				break;
			}
		}
		buffer[i] = usart_recv(UARTx);
	}

	return res;
}


#endif // USE_SERIAL
#ifdef __cplusplus
 }
#endif
