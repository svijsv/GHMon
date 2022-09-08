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

#if USE_UART_COMM && ((UART_COMM_BUSFREQ/UART_COMM_BAUDRATE) > (0xFFFF))
# error "UART comm frequency is too low for the system clock"
#endif


/*
* Static values
*/
#if USE_ALL_UARTS
# define UART_PORT_COUNT 3
#elif USE_UART_COMM
# define UART_PORT_COUNT 1
#else
# error "UART subsystem is active with no users"
#endif


/*
* Types
*/


/*
* Variables
*/
static uart_port_t uart_ports[UART_PORT_COUNT];
static uint8_t uart_ports_used = 0;


/*
* Local function prototypes
*/
static uint16_t calculate_baud_div(uint32_t baud, uint32_t busfreq);


/*
* Interrupt handlers
*/
#if USE_UART_COMM
OPTIMIZE_FUNCTION \
void UART_COMM_IRQHandler(void) {
	NVIC_DisableIRQ(UART_COMM_IRQn);
	NVIC_ClearPendingIRQ(UART_COMM_IRQn);

	// There are no other status flags to clear for this IRQ

	// UART_COMM_IRQf is the main loop's IRQ flag, UART_COMM_IRQn is for the CMSIS
	SET_BIT(G_IRQs, UART_COMM_IRQf);

	return;
}
#endif // USE_UART_COMM


/*
* Functions
*/
const uart_port_t* uart_init_port(const uart_port_conf_t *conf) {
	uart_port_t *p;
	uint8_t af;
	uint32_t busfreq = 0;

	assert(conf != NULL);

	// TODO:
	// Some day I may want to do some error checking, for instance to detect
	// separate initializations that specify different sets of pins corresponding
	// to the same port (e.g. one uses PIN_UART1_RX and the other PIN_UART1_REMAP_RX)
	// or an attempt to use more than UART_PORT_COUNT ports.
	for (uiter_t i = 0; i < uart_ports_used; ++i) {
		if ((uart_ports[i].rx_pin == conf->rx_pin) && (uart_ports[i].tx_pin == conf->tx_pin)) {
			return &uart_ports[i];
		}
	}
	// TODO: Make sure not to use more than UART_PORT_COUNT ports
	p = &uart_ports[uart_ports_used];
	++uart_ports_used;

	if (IS_UART1(conf->rx_pin, conf->tx_pin)) {
		p->uart = USART1;
		p->clocken = RCC_PERIPH_UART1;
		af = AF_UART1;
#if defined(USART2)
	} else if (IS_UART2(conf->rx_pin, conf->tx_pin)) {
		p->uart = USART2;
		p->clocken = RCC_PERIPH_UART2;
		af = AF_UART2;
#endif
#if defined(USART3)
	} else if (IS_UART3(conf->rx_pin, conf->tx_pin)) {
		p->uart = USART3;
		p->clocken = RCC_PERIPH_UART3;
		af = AF_UART3;
#endif
	} else {
		NOTIFY("Attempted to initialize unknown UART port on pins 0x%02X and 0x%02X", (uint )conf->rx_pin, (uint )conf->tx_pin);
		ERROR_STATE("Attempted to initialize unknown UART port");
	}

	p->rx_pin = conf->rx_pin;
	p->tx_pin = conf->tx_pin;
	// When using an STM32F1 both pins need to be either remapped or left
	// alone, but the above call to IS_UART() confirmed that we're in one of
	// those states already.
#if USE_STM32F1_GPIO
	if ((PINID(p->rx_pin) == PIN_UART1_REMAP_RX) && (PINID(p->tx_pin) == PIN_UART1_REMAP_TX)) {
		gpio_remap_uart1();
	}
#else
	gpio_set_AF(p->rx_pin, af);
	gpio_set_AF(p->tx_pin, af);
#endif

	clock_init(p->clocken);

	MODIFY_BITS(p->uart->CR1, USART_CR1_M|USART_CR1_PCE|USART_CR1_PS|USART_CR1_RXNEIE|USART_CR1_UE|USART_CR1_TE|USART_CR1_RE,
		(0b0 << USART_CR1_M_Pos     ) | // 0 for 8 data bits
		(0b0 << USART_CR1_PCE_Pos   ) | // 0 to disable parity
		(0b0 << USART_CR1_PS_Pos    ) | // 0 for even parity, 1 for odd parity
		(0b1 << USART_CR1_RXNEIE_Pos) | // RXNE interrupt enable
		(0b1 << USART_CR1_TE_Pos    ) | // Enable transmission
		(0b1 << USART_CR1_RE_Pos    ) | // Enable reception
		0);
	MODIFY_BITS(p->uart->CR2, USART_CR2_STOP,
		(0b00 << USART_CR2_STOP_Pos) | // Keep at 00 for 1 stop bit
		0);

	switch (SELECT_BITS(p->clocken, RCC_BUS_MASK)) {
	case RCC_BUS_APB1:
		busfreq = G_freq_PCLK1;
		break;
	case RCC_BUS_APB2:
		busfreq = G_freq_PCLK2;
		break;
	case RCC_BUS_AHB1:
		busfreq = G_freq_HCLK;
		break;
	default:
		NOTIFY("Unable to determine bus frequency for UART port on pins 0x%02X and 0x%02X", (uint )p->rx_pin, (uint )p->tx_pin);
		ERROR_STATE("Unable to determine bus frequency for UART port");
		break;
	}
	p->uart->BRR = calculate_baud_div(conf->baud_rate, busfreq);

	uart_off(p);

	return p;
}
static void pins_on(const uart_port_t *p) {
	//assert(p != NULL);

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

	gpio_set_mode(p->tx_pin, GPIO_MODE_HiZ, GPIO_FLOAT);
	gpio_set_mode(p->rx_pin, GPIO_MODE_HiZ, GPIO_FLOAT);

	return;
}
void uart_on(const uart_port_t *p) {
	if (p == NULL) {
		p = comm_port;
	}
	assert(p != NULL);

	clock_enable(p->clocken);
	pins_on(p);
	SET_BIT(p->uart->CR1, USART_CR1_UE);

	return;
}
void uart_off(const uart_port_t *p) {
	if (p == NULL) {
		p = comm_port;
	}
	assert(p != NULL);

	CLEAR_BIT(p->uart->CR1, USART_CR1_UE);
	pins_off(p);
	clock_disable(p->clocken);

	return;
}

err_t uart_transmit_block(const uart_port_t *p, const uint8_t *buffer, txsize_t size, utime_t timeout) {
	err_t res;

	assert(buffer != NULL);

	if (p == NULL) {
		p = comm_port;
	}
	assert(p != NULL);

	res = EOK;
	timeout = SET_TIMEOUT(timeout);

	for (txsize_t i = 0; i < size; ++i) {
		p->uart->DR = buffer[i];
		while (!BIT_IS_SET(p->uart->SR, USART_SR_TXE)) {
			if (TIMES_UP(timeout)) {
				res = ETIMEOUT;
				goto END;
			}
		}
	}
	while (!BIT_IS_SET(p->uart->SR, USART_SR_TC)) {
		if (TIMES_UP(timeout)) {
			res = ETIMEOUT;
			break;
		}
	}

END:
	return res;
}
err_t uart_receive_block(const uart_port_t *p, uint8_t *buffer, txsize_t size, utime_t timeout) {
	err_t res;

	assert(buffer != NULL);

	if (p == NULL) {
		p = comm_port;
	}
	assert(p != NULL);

	res = EOK;
	timeout = SET_TIMEOUT(timeout);

	for (txsize_t i = 0; i < size; ++i) {
		while (!BIT_IS_SET(p->uart->SR, USART_SR_RXNE)) {
			if (TIMES_UP(timeout)) {
				res = ETIMEOUT;
				goto END;
			}
		}
		buffer[i] = p->uart->DR;
	}

END:
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
	if (tmp > 0xFFFF) {
		NOTIFY("UART baud rate %u is too low for the system clock", (uint )baud);
		ERROR_STATE("UART baud rate %u is too low for the system clock");
	}
	return tmp;
}


#endif // USE_UART
#ifdef __cplusplus
 }
#endif
