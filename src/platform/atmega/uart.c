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

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/power.h>


#if USE_UART

#if USE_UART_COMM && !IS_UART0(UART_COMM_RX_PIN, UART_COMM_TX_PIN)
# error "Incorrect pin(s) set for comm UART"
#endif

/*
* Static values
*/
#if USE_ALL_UARTS
# define UART_PORT_COUNT 1
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


/*
* Interrupt handlers
*/
ISR(USART_RX_vect) {
	// The received byte must be read before the ISR exits to clear RXC0 or it
	// will just be re-entered immediately
	// But we disable the interrupt anyway so it doesn't matter
	//rx_byte = UDR0;

	CLEAR_BIT(UCSR0B, _BV(RXCIE0));
	SET_BIT(G_IRQs, UART_COMM_IRQf);
}


/*
* Functions
*/
const uart_port_t* uart_init_port(const uart_port_conf_t *conf) {
	uart_port_t *p;

	assert(conf != NULL);

	for (uiter_t i = 0; i < uart_ports_used; ++i) {
		if ((uart_ports[i].rx_pin == conf->rx_pin) && (uart_ports[i].tx_pin == conf->tx_pin)) {
			return &uart_ports[i];
		}
	}
	// TODO: Make sure not to use more than UART_PORT_COUNT ports.
	p = &uart_ports[uart_ports_used];
	++uart_ports_used;

	if (IS_UART0(conf->rx_pin, conf->tx_pin)) {
		p->uart_portno = 0;
	} else {
		NOTIFY("Attempted to initialize unknown UART port on pins 0x%02X and 0x%02X", (uint )conf->rx_pin, (uint )conf->tx_pin);
		ERROR_STATE("Attempted to initialize unknown UART port");
	}

	p->rx_pin = conf->rx_pin;
	p->tx_pin = conf->tx_pin;

	// TODO: If the error value is too great, use the U2X0 bit.
	p->ubrr = (G_freq_IOCLK / (16L*conf->baud_rate)) - 1;

	// The USART peripheral needs to be re-enabled each time the power is
	// turned on so it wouldn't accomplish anything to set it up here.
	uart_off(p);

	return p;
}
void uart_on(const uart_port_t *p) {
	if (p == NULL) {
		p = comm_port;
	}
	assert(p != NULL);

	switch (p->uart_portno) {
	case 0:
		power_usart0_enable();
		WRITE_SPLITREG16(p->ubrr, UBRR0H, UBRR0L);
		UCSR0A = _BV(UDRE0); // Make sure the 'data register empty' bit is set
		UCSR0B = _BV(RXEN0)|_BV(TXEN0); // Enable transmission and reception
		// The default frame configuration is 8n1 which is what we want, so
		// nothing in UCSR0C needs to be set; set it anyway though so we know
		// for sure what state the register is in.
		UCSR0C = _BV(UCSZ00)|_BV(UCSZ01);
		break;
	}

	return;
}
void uart_off(const uart_port_t *p) {
	if (p == NULL) {
		p = comm_port;
	}
	assert(p != NULL);

	switch (p->uart_portno) {
	case 0:
		CLEAR_BIT(UCSR0B, _BV(RXCIE0)|_BV(RXEN0)|_BV(TXEN0));
		power_usart0_disable();
		break;
	}

	return;
}
void uart_listen_on(const uart_port_t *p) {
	if (p == NULL) {
		p = comm_port;
	}
	assert(p != NULL);

	switch (p->uart_portno) {
	case 0:
		CLEAR_BIT(UCSR0A, _BV(RXC0));
		SET_BIT(UCSR0B, _BV(RXCIE0));
		break;
	}

	return;
}
void uart_listen_off(const uart_port_t *p) {
	if (p == NULL) {
		p = comm_port;
	}
	assert(p != NULL);

	switch (p->uart_portno) {
	case 0:
		CLEAR_BIT(UCSR0B, _BV(RXCIE0));
		break;
	}

	return;
}

err_t uart_transmit_block(const uart_port_t *p, const uint8_t *buffer, txsize_t size, utime_t timeout) {
	err_t res;
	volatile uint8_t *ucsra;
	volatile uint8_t *udr;

	if (p == NULL) {
		p = comm_port;
	}
	assert(p != NULL);

	switch (p->uart_portno) {
	case 0:
		ucsra = &UCSR0A;
		udr   = &UDR0;
		break;
	}
	res = EOK;
	timeout = SET_TIMEOUT(timeout);

	// TXC0 is cleared by writing '1' to it
	SET_BIT(*ucsra, _BV(TXC0));
	for (txsize_t i = 0; i < size; ++i) {
		while (!BIT_IS_SET(*ucsra, _BV(UDRE0))) {
			if (TIMES_UP(timeout)) {
				res = ETIMEOUT;
				goto END;
			}
		}
		*udr = buffer[i];
	}
	while (!BIT_IS_SET(*ucsra, _BV(UDRE0)) || !BIT_IS_SET(*ucsra, _BV(TXC0))) {
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
	volatile uint8_t *ucsra;
	volatile uint8_t *udr;

	if (p == NULL) {
		p = comm_port;
	}
	assert(p != NULL);

	switch (p->uart_portno) {
	case 0:
		ucsra = &UCSR0A;
		udr   = &UDR0;
		break;
	}
	res = EOK;
	timeout = SET_TIMEOUT(timeout);

	for (txsize_t i = 0; i < size; ++i) {
		while (!BIT_IS_SET(*ucsra, _BV(RXC0))) {
			if (TIMES_UP(timeout)) {
				res = ETIMEOUT;
				goto END;
			}
		}
		buffer[i] = *udr;
	}

END:
	return res;
}


#endif // USE_UART
#ifdef __cplusplus
 }
#endif
