// SPDX-License-Identifier: GPL-3.0-only
/***********************************************************************
*                                                                      *
*                                                                      *
* Copyright 2021, 2024 svijsv                                          *
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

#include "uart.h"
#include "system.h"
#include "gpio.h"

// For memcpy() and memmove()
#include <string.h>

#if uHAL_USE_UART

#include "platform/common/uart_buf.c"
#include "uart_find_periph.h"
#include "uart_define_irq.h"

#if uHAL_USE_UART_COMM
# if ! defined(UART_COMM_CLOCKEN)
#  error "Can't determine UART communication peripheral"
# elif (UART_COMM_CLOCKEN & RCC_BUS_MASK) == RCC_BUS_APB1
#  define UART_COMM_BUSFREQ G_freq_PCLK1
# elif (UART_COMM_CLOCKEN & RCC_BUS_MASK) == RCC_BUS_APB2
#  define UART_COMM_BUSFREQ G_freq_PCLK2
# elif (UART_COMM_CLOCKEN & RCC_BUS_MASK) == RCC_BUS_AHB1
#  define UART_COMM_BUSFREQ G_freq_HCLK
# endif
# if ((UART_COMM_BUSFREQ/UART_COMM_BAUDRATE) > (0xFFFFU))
#  error "UART communication frequency is too low for the bus clock"
# endif
#endif

DEBUG_CPP_MACRO(UART_INPUT_BUFFER_BYTES)

#if defined(UART_COMM_PORT)
# define SET_DEFAULT_PORT(_p_) do { if ((_p_) == NULL) (_p_) = UART_COMM_PORT; } while (0)
#else
# define SET_DEFAULT_PORT(_p_)
#endif

static uint16_t calculate_baud_div(uint32_t baud, uint32_t busfreq);

err_t uart_init_port(uart_port_t *p, const uart_port_cfg_t *conf) {
	uint32_t tmp;

	SET_DEFAULT_PORT(p);

	assert(conf != NULL);
	assert(p != NULL);

# if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if ((conf == NULL) || (p == NULL)) {
		return ERR_BADARG;
	}
# endif

	//
	// USART peripherals
	if (false) {
		// Nothing to do here
#if HAVE_UART1
	} else if (IS_UART1_STRUCT(conf)) {
		p->uartx = USART1;
		p->irqn = USART1_IRQn;
		p->clocken = RCC_PERIPH_UART1;
		p->gpio_af = GPIOAF_UART1;
		ASSIGN_IRQ_PORT(1, p);
#endif
#if HAVE_UART2
	} else if (IS_UART2_STRUCT(conf)) {
		p->uartx = USART2;
		p->irqn = USART2_IRQn;
		p->clocken = RCC_PERIPH_UART2;
		p->gpio_af = GPIOAF_UART2;
		ASSIGN_IRQ_PORT(2, p);
#endif
#if HAVE_UART3
	} else if (IS_UART3_STRUCT(conf)) {
		p->uartx = USART3;
		p->irqn = USART3_IRQn;
		p->clocken = RCC_PERIPH_UART3;
		p->gpio_af = GPIOAF_UART3;
		ASSIGN_IRQ_PORT(3, p);
#endif
#if HAVE_UART6
	} else if (IS_UART6_STRUCT(conf)) {
		p->uartx = USART6;
		p->irqn = USART6_IRQn;
		p->clocken = RCC_PERIPH_UART6;
		p->gpio_af = GPIOAF_UART6;
		ASSIGN_IRQ_PORT(6, p);
#endif
	//
	// UART peripherals
#if HAVE_UART4
	} else if (IS_UART4_STRUCT(conf)) {
		p->uartx = USART4;
		p->irqn = USART4_IRQn;
		p->clocken = RCC_PERIPH_UART4;
		p->gpio_af = GPIOAF_UART4;
		ASSIGN_IRQ_PORT(4, p);
#endif
#if HAVE_UART5
	} else if (IS_UART5_STRUCT(conf)) {
		p->uartx = USART5;
		p->irqn = USART5_IRQn;
		p->clocken = RCC_PERIPH_UART5;
		p->gpio_af = GPIOAF_UART5;
		ASSIGN_IRQ_PORT(5, p);
#endif
#if HAVE_UART7
	} else if (IS_UART7_STRUCT(conf)) {
		p->uartx = USART7;
		p->irqn = USART7_IRQn;
		p->clocken = RCC_PERIPH_UART7;
		p->gpio_af = GPIOAF_UART7;
		ASSIGN_IRQ_PORT(7, p);
#endif
#if HAVE_UART8
	} else if (IS_UART8_STRUCT(conf)) {
		p->uartx = USART8;
		p->irqn = USART8_IRQn;
		p->clocken = RCC_PERIPH_UART8;
		p->gpio_af = GPIOAF_UART8;
		ASSIGN_IRQ_PORT(8, p);
#endif

	} else {
		return ERR_NOTSUP;
	}

	p->rx_pin = conf->rx_pin;
	p->tx_pin = conf->tx_pin;

	clock_init(p->clocken);

	MODIFY_BITS(p->uartx->CR1, USART_CR1_M|USART_CR1_PCE|USART_CR1_PS|USART_CR1_RXNEIE|USART_CR1_UE|USART_CR1_TE|USART_CR1_RE,
		(0b0 << USART_CR1_M_Pos     ) | // 0 for 8 data bits
		(0b0 << USART_CR1_PCE_Pos   ) | // 0 to disable parity
		(0b0 << USART_CR1_PS_Pos    ) | // 0 for even parity, 1 for odd parity
		(0b1 << USART_CR1_RXNEIE_Pos) | // RXNE interrupt enable
		(0b1 << USART_CR1_TE_Pos    ) | // Enable transmission
		(0b1 << USART_CR1_RE_Pos    ) | // Enable reception
		0);
	MODIFY_BITS(p->uartx->CR2, USART_CR2_STOP,
		(0b00 << USART_CR2_STOP_Pos) | // Keep at 00 for 1 stop bit
		0);

	switch (SELECT_BITS(p->clocken, RCC_BUS_MASK)) {
	case RCC_BUS_APB1:
		tmp = G_freq_PCLK1;
		break;
	case RCC_BUS_APB2:
		tmp = G_freq_PCLK2;
		break;
	case RCC_BUS_AHB1:
		tmp = G_freq_HCLK;
		break;
	default:
		return ERR_UNKNOWN;
		break;
	}
	tmp = calculate_baud_div(conf->baud_rate, tmp);
	if (tmp == 0) {
		return ERR_IMPOSSIBLE;
	}
	p->uartx->BRR = (uint16_t )tmp;

#if UART_INPUT_BUFFER_BYTES > 0
	p->rx_buf.bytes = 0;
#endif

	NVIC_SetPriority(p->irqn, UART_IRQp);
#if ENABLE_UART_LISTENING
	uart_listen_off(p);
#endif
	uart_off(p);

	return ERR_OK;
}
static void pins_on(const uart_port_t *p) {
	//assert(p != NULL);

	gpio_set_AF(p->rx_pin, p->gpio_af);
	gpio_set_AF(p->tx_pin, p->gpio_af);

	// Peripheral pin modes specified in the STM32F1 reference manual section
	// 9.1.11
	// I can't find specifications for the other devices, I'm assuming they
	// just need to be AF
	gpio_set_mode(p->tx_pin, GPIO_MODE_PP_AF, GPIO_FLOAT);
	gpio_set_mode(p->rx_pin, GPIO_MODE_IN_AF, GPIO_FLOAT);

	return;
}
static void pins_off(const uart_port_t *p) {
	//assert(p != NULL);

	gpio_set_mode(p->tx_pin, GPIO_MODE_RESET, GPIO_FLOAT);
	gpio_set_mode(p->rx_pin, GPIO_MODE_RESET, GPIO_FLOAT);

	return;
}
err_t uart_on(const uart_port_t *p) {
	SET_DEFAULT_PORT(p);

	assert(p != NULL);
	assert(p->uartx != NULL);
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (p == NULL) {
		return ERR_BADARG;
	}
	if (p->uartx == NULL) {
		return ERR_INIT;
	}
#endif

	clock_enable(p->clocken);
	pins_on(p);
	SET_BIT(p->uartx->CR1, USART_CR1_UE);

	return ERR_OK;
}
err_t uart_off(const uart_port_t *p) {
	SET_DEFAULT_PORT(p);

	assert(p != NULL);
	assert(p->uartx != NULL);
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (p == NULL) {
		return ERR_BADARG;
	}
	if (p->uartx == NULL) {
		return ERR_INIT;
	}
#endif

	CLEAR_BIT(p->uartx->CR1, USART_CR1_UE);
	pins_off(p);
	clock_disable(p->clocken);

	return ERR_OK;
}
bool uart_is_on(const uart_port_t *p) {
	SET_DEFAULT_PORT(p);

	assert(p != NULL);
	assert(p->uartx != NULL);
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (p == NULL) {
		return false;
	}
	if (p->uartx == NULL) {
		return false;
	}
#endif

	return (clock_is_enabled(p->clocken) && BIT_IS_SET(p->uartx->CR1, USART_CR1_UE));
}

#if ENABLE_UART_LISTENING
err_t uart_listen_on(const uart_port_t *p) {
	SET_DEFAULT_PORT(p);

	assert(p != NULL);
	assert(p->uartx != NULL);
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (p == NULL) {
		return ERR_BADARG;
	}
	if (p->uartx == NULL) {
		return ERR_INIT;
	}
#endif

	NVIC_ClearPendingIRQ(p->irqn);
	NVIC_EnableIRQ(p->irqn);

	return ERR_OK;
}
err_t uart_listen_off(const uart_port_t *p) {
	SET_DEFAULT_PORT(p);

	assert(p != NULL);
	assert(p->uartx != NULL);
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (p == NULL) {
		return ERR_BADARG;
	}
	if (p->uartx == NULL) {
		return ERR_INIT;
	}
#endif

	NVIC_DisableIRQ(p->irqn);
	NVIC_ClearPendingIRQ(p->irqn);

	return ERR_OK;
}
bool uart_is_listening(const uart_port_t *p) {
	SET_DEFAULT_PORT(p);

	assert(p != NULL);
	assert(p->uartx != NULL);
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (p == NULL) {
		return ERR_BADARG;
	}
	if (p->uartx == NULL) {
		return ERR_INIT;
	}
#endif

	return (NVIC_GetEnableIRQ(p->irqn) != 0);
}
bool uart_rx_is_available(const uart_port_t *p) {
#if UART_INPUT_BUFFER_BYTES > 0
	SET_DEFAULT_PORT(p);
	return (p->rx_buf.bytes != 0);
#else
	UNUSED(p);
	return false;
#endif
}
#endif // ENABLE_UART_LISTENING

err_t uart_transmit_block(uart_port_t *p, const uint8_t *buffer, txsize_t size, utime_t timeout) {
	err_t res;

	SET_DEFAULT_PORT(p);

	assert(p != NULL);
	assert(p->uartx != NULL);
	assert(buffer != NULL);
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (p == NULL || buffer == NULL) {
		return ERR_BADARG;
	}
	if (p->uartx == NULL) {
		return ERR_INIT;
	}
#endif

	res = ERR_OK;
	timeout = SET_TIMEOUT_MS(timeout);

	for (txsize_t i = 0; i < size; ++i) {
		p->uartx->DR = buffer[i];
		while (!BIT_IS_SET(p->uartx->SR, USART_SR_TXE)) {
			if (TIMES_UP(timeout)) {
				res = ERR_TIMEOUT;
				goto END;
			}
		}
	}
	while (!BIT_IS_SET(p->uartx->SR, USART_SR_TC)) {
		if (TIMES_UP(timeout)) {
			res = ERR_TIMEOUT;
			break;
		}
	}

END:
	return res;
}
err_t uart_receive_block(uart_port_t *p, uint8_t *buffer, txsize_t size, utime_t timeout) {
	err_t res;

	SET_DEFAULT_PORT(p);

	assert(p != NULL);
	assert(p->uartx != NULL);
	assert(buffer != NULL);
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (p == NULL || buffer == NULL) {
		return ERR_BADARG;
	}
	if (p->uartx == NULL) {
		return ERR_INIT;
	}
#endif

	if (size == 0) {
		return ERR_OK;
	}

	res = ERR_OK;
	timeout = SET_TIMEOUT_MS(timeout);

#if ENABLE_UART_LISTENING
	bool reenable_listen = uart_is_listening(p);
	if (reenable_listen) {
		uart_listen_off(p);
	}
#endif

	for (txsize_t i = eat_uart_buffer(p, buffer, size); i < size; ++i) {
		while (!BIT_IS_SET(p->uartx->SR, USART_SR_RXNE)) {
			if (TIMES_UP(timeout)) {
				res = ERR_TIMEOUT;
				goto END;
			}
		}
		buffer[i] = p->uartx->DR;
	}

END:
#if ENABLE_UART_LISTENING
	if (reenable_listen) {
		uart_listen_on(p);
	}
#endif
	return res;
}

static uint16_t calculate_baud_div(uint32_t baud, uint32_t busfreq) {
	uint32_t tmp;

	assert(baud != 0);
	assert(busfreq != 0);

	// From section 27.3.4 of the STM32F1 reference manual:
	//   baud = pclk/(16*div)
	// Therefore:
	//   (16*div) = pclk/baud
	// The baud rate is programmed into a 16-bit register configured as a
	// fixed-point rational number with 12 bits of mantissa and 4 bits of
	// fractional part, which (I think) makes the whole register (16*div).
	// That makes more sense to me than the description in the reference manual
	// does.
	tmp = busfreq/baud;
	if (tmp > 0xFFFFU) {
		return 0;
	}
	return tmp;
}


#endif // uHAL_USE_UART
