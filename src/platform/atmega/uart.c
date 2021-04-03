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


#if USE_SERIAL

#if (PINID(UARTx_RX_PIN) != PINID_D0) || (PINID(UARTx_TX_PIN) != PINID_D1)
# error "Incorrect pin(s) set for UART"
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


/*
* Interrupt handlers
*/
ISR(USART_RX_vect) {
	// The received byte must be read before the ISR exits to clear RXC0 or it
	// will just be re-entered immediately
	// But we disable the interrupt anyway so it doesn't matter
	//rx_byte = UDR0;

	CLEAR_BIT(UCSR0B, _BV(RXCIE0));
	SET_BIT(G_IRQs, UART_IRQf);
}


/*
* Functions
*/
void uart_init(void) {
	uint16_t ubrr;

	power_usart0_enable();

	// TODO: If the error value is too great, use the U2X0 bit
	ubrr = (G_freq_IOCLK / (16L*UART_BAUDRATE)) - 1;
	WRITE_SPLITREG16(ubrr, UBRR0H, UBRR0L);

	UCSR0A = _BV(UDRE0); // Make sure the 'data register empty' bit is set
	UCSR0B = _BV(RXEN0)|_BV(TXEN0); // Enable transmission and reception
	// The default frame configuration is 8n1 which is what we want, so nothing
	// in UCSR0C needs to be set; set it anyway though so we know for sure what
	// state the register is in
	UCSR0C = _BV(UCSZ00)|_BV(UCSZ01);

	// The USART peripheral needs to be re-enabled if the power is turned off
	// so it wouldn't accomplish anything to shut it off here, and we don't
	// want it off anyway because it's used for logging output
	//power_usart0_disable();

	return;
}
void uart_on(void) {
	uart_init();

	return;
}
void uart_off(void) {
	CLEAR_BIT(UCSR0B, _BV(RXCIE0)|_BV(RXEN0)|_BV(TXEN0));
	power_usart0_disable();

	return;
}
void uart_listen_on(void) {
	CLEAR_BIT(UCSR0A, _BV(RXC0));
	SET_BIT(UCSR0B, _BV(RXCIE0));

	return;
}
void uart_listen_off(void) {
	CLEAR_BIT(UCSR0B, _BV(RXCIE0));

	return;
}

err_t uart_transmit_block(const uint8_t *buffer, txsize_t size, utime_t timeout) {
	err_t res;

	res = EOK;
	timeout = SET_TIMEOUT(timeout);

	// TXC0 is cleared by writing '1' to it
	SET_BIT(UCSR0A, _BV(TXC0));
	for (txsize_t i = 0; i < size; ++i) {
		while (!BIT_IS_SET(UCSR0A, _BV(UDRE0))) {
			if (TIMES_UP(timeout)) {
				res = ETIMEOUT;
				goto END;
			}
		}
		UDR0 = buffer[i];
	}
	while (!BIT_IS_SET(UCSR0A, _BV(UDRE0)) || !BIT_IS_SET(UCSR0A, _BV(TXC0))) {
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
		while (!BIT_IS_SET(UCSR0A, _BV(RXC0))) {
			if (TIMES_UP(timeout)) {
				res = ETIMEOUT;
				goto END;
			}
		}
		buffer[i] = UDR0;
	}

END:
	return res;
}


#endif // USE_SERIAL
#ifdef __cplusplus
 }
#endif
