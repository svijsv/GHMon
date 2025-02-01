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

#include "common.h"
#if uHAL_USE_UART

#include "system.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/power.h>

// For memcpy() and memmove()
#include <string.h>


#include "platform/common/uart_buf.c"
#include "uart_find_periph.h"
#include "uart_define_irq.h"

#define CONFIG_8N1 (USART_CHSIZE_8BIT_gc | USART_SBMODE_1BIT_gc | USART_PMODE_DISABLED_gc)

// Clear status flags by writing '1' to them
#define CLEAR_STATUS(uartx) (uartx->STATUS = USART_TXCIF_bm | USART_RXSIF_bm | USART_ISFIF_bm | USART_BDF_bm)

DEBUG_CPP_MACRO(UART_INPUT_BUFFER_BYTES)

#ifdef UART_COMM_PORT
# define SET_DEFAULT_PORT(_p_) do { if ((_p_) == NULL) (_p_) = UART_COMM_PORT; } while (0)
#else
# define SET_DEFAULT_PORT(_p_)
#endif

#if ! uHAL_SKIP_INVALID_ARG_CHECKS
# define VERIFY_PORT(_p_) \
	do { \
		uHAL_assert(p != NULL); \
		uHAL_assert(p->uartx != NULL); \
		if (p == NULL) { \
			return ERR_BADARG; \
		} \
		if (p->uartx == NULL) { \
			return ERR_INIT; \
		} \
	} while (0)
#else
# define VERIFY_PORT(_p_) \
	do { \
		uHAL_assert(p != NULL); \
		uHAL_assert(p->uartx != NULL); \
	} while (0)
#endif

//#define BUFFER_OK(_buf_, _size_) ((_buf_) != NULL && (_size_) > 0)
#define BUFFER_OK(_buf_, _size_) ((_buf_) != NULL)

err_t uart_init_port(uart_port_t *p, const uart_port_cfg_t *conf) {
	SET_DEFAULT_PORT(p);

	uHAL_assert(conf != NULL);
	uHAL_assert(p != NULL);

# if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if ((conf == NULL) || (p == NULL)) {
		return ERR_BADARG;
	}
# endif

	if (false) {
		// Nothing to do here
# if HAVE_UART0
	} else if (IS_UART0(conf->rx_pin, conf->tx_pin)) {
		p->uartx = &USART0;
		ASSIGN_IRQ_PORT(0, p);
# endif
# if HAVE_UART1
	} else if (IS_UART1(conf->rx_pin, conf->tx_pin)) {
		p->uartx = &USART1;
		ASSIGN_IRQ_PORT(1, p);
# endif
# if HAVE_UART2
	} else if (IS_UART2(conf->rx_pin, conf->tx_pin)) {
		p->uartx = &USART2;
		ASSIGN_IRQ_PORT(2, p);
# endif
# if HAVE_UART3
	} else if (IS_UART3(conf->rx_pin, conf->tx_pin)) {
		p->uartx = &USART3;
		ASSIGN_IRQ_PORT(3, p);
# endif
	} else {
		return ERR_NOTSUP;
	}

	p->rx_pin = conf->rx_pin;
	p->tx_pin = conf->tx_pin;

	p->uartx->CTRLA = 0;
	p->uartx->CTRLC = (USART_CMODE_ASYNCHRONOUS_gc | CONFIG_8N1);

	if ((G_freq_UARTCLK/16U) > conf->baud_rate) {
		p->uartx->CTRLB = (USART_RXMODE_NORMAL_gc);
		// The manual gives the formula for calculating BAUD as BAUD = (64*CLK) / (16*Fbaud)
		// which we turn into BAUD = 4 * (CLK / Fbaud) to save some math cycles
		write_reg16(&p->uartx->BAUD, 4U * (G_freq_UARTCLK / conf->baud_rate));
	} else {
		p->uartx->CTRLB = (USART_RXMODE_CLK2X_gc);
		write_reg16(&p->uartx->BAUD, 8U * (G_freq_UARTCLK / conf->baud_rate));
	}

	CLEAR_STATUS(p->uartx);
#if ENABLE_UART_LISTENING
	uart_listen_off(p);
#endif
	uart_off(p);

	return ERR_OK;
}
err_t uart_on(const uart_port_t *p) {
	SET_DEFAULT_PORT(p);
	VERIFY_PORT(p);

	CLEAR_STATUS(p->uartx);
	gpio_set_mode(p->rx_pin, GPIO_MODE_IN, GPIO_FLOAT);
	gpio_set_mode(p->tx_pin, GPIO_MODE_PP, GPIO_HIGH);
	SET_BIT(p->uartx->CTRLB, (USART_TXEN_bm | USART_RXEN_bm));

	return ERR_OK;
}
err_t uart_off(const uart_port_t *p) {
	SET_DEFAULT_PORT(p);
	VERIFY_PORT(p);

	// There's a bug listed in the errata where disabling the transmitter doesn't
	// release the TX pin unless the reciever is still enabled
	//CLEAR_BIT(p->uartx->CTRLB, (USART_TXEN_bm | USART_RXEN_bm | USART_SFDEN_bm));
	CLEAR_BIT(p->uartx->CTRLB, USART_TXEN_bm | USART_SFDEN_bm);
	CLEAR_BIT(p->uartx->CTRLB, USART_RXEN_bm);
	CLEAR_BIT(p->uartx->CTRLA, (USART_RXCIE_bm | USART_RXSIE_bm));

	gpio_set_mode(p->rx_pin, GPIO_MODE_RESET, GPIO_FLOAT);
	gpio_set_mode(p->tx_pin, GPIO_MODE_RESET, GPIO_FLOAT);

	return ERR_OK;
}
bool uart_is_on(const uart_port_t *p) {
	SET_DEFAULT_PORT(p);
	return BIT_IS_SET(p->uartx->CTRLB, (USART_TXEN_bm | USART_RXEN_bm));
}

#if ENABLE_UART_LISTENING
err_t uart_listen_on(const uart_port_t *p) {
	SET_DEFAULT_PORT(p);
	VERIFY_PORT(p);

	// We enable the frame start interrupt just so we can wake from deep sleep
	// via UART input
	SET_BIT(p->uartx->CTRLA, (USART_RXCIE_bm | USART_RXSIE_bm));
	SET_BIT(p->uartx->CTRLB, (USART_SFDEN_bm));

	return ERR_OK;
}
err_t uart_listen_off(const uart_port_t *p) {
	SET_DEFAULT_PORT(p);
	VERIFY_PORT(p);

	CLEAR_BIT(p->uartx->CTRLA, (USART_RXCIE_bm | USART_RXSIE_bm));
	CLEAR_BIT(p->uartx->CTRLB, (USART_SFDEN_bm));

	return ERR_OK;
}
static bool _uart_is_listening(const uart_port_t *p) {
	return BIT_IS_SET(p->uartx->CTRLA, USART_RXCIE_bm);
}
bool uart_is_listening(const uart_port_t *p) {
	SET_DEFAULT_PORT(p);
	VERIFY_PORT(p);

	return _uart_is_listening(p);
}
bool uart_rx_is_available(const uart_port_t *p) {
#if UART_INPUT_BUFFER_BYTES > 0
	SET_DEFAULT_PORT(p);
	VERIFY_PORT(p);
	return (p->rx_buf.bytes != 0);
#else
	UNUSED(p);
	return false;
#endif
}
#endif // ENABLE_UART_LISTENING

err_t uart_transmit_block(uart_port_t *p, const uint8_t *buffer, txsize_t size, utime_t timeout) {
	err_t res;
	USART_t *uartx;
	txsize_t i = 0;

	SET_DEFAULT_PORT(p);
	VERIFY_PORT(p);

	uHAL_assert(BUFFER_OK(buffer, size));
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (!BUFFER_OK(buffer, size)) {
		return ERR_BADARG;
	}
	if (size == 0) {
		return ERR_OK;
	}
#endif

	uartx = p->uartx;
	res = ERR_OK;
	timeout = SET_TIMEOUT_MS(timeout);

	for (; i < size; ++i) {
		while (!BIT_IS_SET(uartx->STATUS, USART_DREIF_bm)) {
			if (TIMES_UP(timeout)) {
				res = ERR_TIMEOUT;
				goto END;
			}
		}
		uartx->TXDATAL = buffer[i];
	}
	while (!BIT_IS_SET(uartx->STATUS, USART_TXCIF_bm) || !BIT_IS_SET(uartx->STATUS, USART_DREIF_bm)) {
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
	volatile USART_t *uartx;

	SET_DEFAULT_PORT(p);
	VERIFY_PORT(p);

	uHAL_assert(BUFFER_OK(buffer, size));
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (!BUFFER_OK(buffer, size)) {
		return ERR_BADARG;
	}
	if (size == 0) {
		return ERR_OK;
	}
#endif

	uartx = p->uartx;
	res = ERR_OK;
	timeout = SET_TIMEOUT_MS(timeout);

#if ENABLE_UART_LISTENING
	bool reenable_listen = _uart_is_listening(p);
	if (reenable_listen) {
		uart_listen_off(p);
	}
#endif

	for (txsize_t i = eat_uart_buffer(p, buffer, size); i < size; ++i) {
		while (!BIT_IS_SET(uartx->STATUS, USART_RXCIF_bm)) {
			if (TIMES_UP(timeout)) {
				res = ERR_TIMEOUT;
				goto END;
			}
		}
		buffer[i] = uartx->RXDATAL;
	}

END:
#if ENABLE_UART_LISTENING
	if (reenable_listen) {
		uart_listen_on(p);
	}
#endif
	return res;
}


#endif // uHAL_USE_UART
