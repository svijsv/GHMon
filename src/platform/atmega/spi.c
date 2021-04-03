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
// spi.c
// Manage the SPI peripheral
// NOTES:
//

#ifdef __cplusplus
 extern "C" {
#endif

//#define DEBUG 1
/*
* Includes
*/
#include "spi.h"
#include "system.h"

#include <avr/io.h>
#include <avr/power.h>


#if USE_SPI

#if (PINID(SPIx_SCK_PIN) != PINID_B5) || (PINID(SPIx_SS_PIN) != PINID_B2) || (PINID(SPIx_MOSI_PIN) != PINID_B3) || (PINID(SPIx_MISO_PIN) != PINID_B4)
# error "Incorrect pin(s) set for SPI"
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


/*
* Functions
*/
// The SPI peripheral needs to be re-configured if it's powered off so
// initialization is just a stub; everything happens in spi_on()
void spi_init(void) {
	return;
}
void spi_on(void) {
	power_spi_enable();

#if ((G_freq_SPICLK/128) >= (SPI_SPEED - (SPI_SPEED/20)))
# define USE_2X 0b0
# define SPRx   0b11
#elif ((G_freq_SPICLK/64) >= (SPI_SPEED - (SPI_SPEED/20)))
# define USE_2X 0b0
# define SPRx   0b10
#elif ((G_freq_SPICLK/32) >= (SPI_SPEED - (SPI_SPEED/20)))
# define USE_2X 0b1
# define SPRx   0b10
#elif ((G_freq_SPICLK/16) >= (SPI_SPEED - (SPI_SPEED/20)))
# define USE_2X 0b0
# define SPRx   0b01
#elif ((G_freq_SPICLK/8) >= (SPI_SPEED - (SPI_SPEED/20)))
# define USE_2X 0b1
# define SPRx   0b01
#elif ((G_freq_SPICLK/4) >= (SPI_SPEED - (SPI_SPEED/20)))
# define USE_2X 0b0
# define SPRx   0b00
#else
# define USE_2X 0b1
# define SPRx   0b00
#endif

	MODIFY_BITS(SPCR, _BV(MSTR)|_BV(CPOL)|_BV(CPHA)|_BV(SPR1)|_BV(SPR0),
		(0b1 << MSTR) | // Enable master mode
		(0b0 << CPOL) | // Clock is low when idle
		(0b0 << CPHA) | // Sample leading edge of clock transition
		(SPRx << SPR0) | // Set (part of) the prescaler
		0);
	MODIFY_BITS(SPSR, _BV(SPI2X),
		(USE_2X << SPI2X) // Set the rest of the prescaler
		);

	// MOSI, SCK, and SS are managed by software but MISO is forced into input
	// by the peripheral
	// Per the datasheet, SS must be either an output or held high while SPI
	// is enabled or else it will automatically switch to slave mode
	gpio_set_mode(SPIx_SCK_PIN,  GPIO_MODE_PP, GPIO_LOW);
	gpio_set_mode(SPIx_MOSI_PIN, GPIO_MODE_PP, GPIO_LOW);
	// MISO will be forced into input when SPI is enabled, all this does is
	// turn on the pullup
	gpio_set_mode(SPIx_MISO_PIN, GPIO_MODE_IN, GPIO_HIGH);
#if GPIO_GET_BIAS(SPIx_SS_PIN) == BIAS_HIGH
	gpio_set_mode(SPIx_SS_PIN, GPIO_MODE_PP, GPIO_HIGH);
#else
	// Setting low has a smaller chance of causing problems for switched
	// devices and it can be set to whatever by wherever this is called from
	// soon enough
	gpio_set_mode(SPIx_SS_PIN, GPIO_MODE_PP, GPIO_LOW);
#endif

	// Enable the SPI peripheral
	SET_BIT(SPCR, _BV(SPE));

	return;
}
void spi_off(void) {
	CLEAR_BIT(SPCR, _BV(SPE));
	power_spi_disable();

	gpio_set_mode(SPIx_SCK_PIN,  GPIO_MODE_HiZ, GPIO_FLOAT);
	gpio_set_mode(SPIx_MOSI_PIN, GPIO_MODE_HiZ, GPIO_FLOAT);
	gpio_set_mode(SPIx_MISO_PIN, GPIO_MODE_HiZ, GPIO_FLOAT);
	// Leave it to the caller to handle this part, they know more than we do
	//gpio_set_mode(SPIx_SS_PIN, GPIO_MODE_HiZ, GPIO_FLOAT);

	return;
}

err_t spi_exchange_byte(uint8_t tx, uint8_t *rx, utime_t timeout) {
	err_t res;

	assert(rx != NULL);

	res = EOK;
	timeout = SET_TIMEOUT(timeout);

	SPDR = tx;
	while (!BIT_IS_SET(SPSR, _BV(SPIF))) {
		if (TIMES_UP(timeout)) {
			res = ETIMEOUT;
			break;
		}
	}
	*rx = SPDR;

	return res;
}
err_t spi_receive_block(uint8_t *rx_buffer, txsize_t rx_size, uint8_t tx, utime_t timeout) {
	err_t res;
	txsize_t i;

	assert(rx_buffer != NULL);
	assert(rx_size > 0);

	res = EOK;
	timeout = SET_TIMEOUT(timeout);

	SPDR = tx;
	rx_size -= 1;
	for (i = 0; i < rx_size; ++i) {
		while (!BIT_IS_SET(SPSR, _BV(SPIF))) {
			if (TIMES_UP(timeout)) {
				res = ETIMEOUT;
				goto END;
			}
		}
		rx_buffer[i] = SPDR;
		SPDR = tx;
	}
	while (!BIT_IS_SET(SPSR, _BV(SPIF))) {
		if (TIMES_UP(timeout)) {
			res = ETIMEOUT;
			break;
		}
	}
	rx_buffer[i] = SPDR;

END:
	return res;
}
err_t spi_transmit_block(const uint8_t *tx_buffer, txsize_t tx_size, utime_t timeout) {
	err_t res;
	uint8_t rx;
	txsize_t i;

	assert(tx_buffer != NULL);
	assert(tx_size > 0);

	res = EOK;
	timeout = SET_TIMEOUT(timeout);

	SPDR = tx_buffer[0];
	for (i = 1; i < tx_size; ++i) {
		while (!BIT_IS_SET(SPSR, _BV(SPIF))) {
			if (TIMES_UP(timeout)) {
				res = ETIMEOUT;
				goto END;
			}
		}
		rx = SPDR;
		SPDR = tx_buffer[i];
	}
	while (!BIT_IS_SET(SPSR, _BV(SPIF))) {
		if (TIMES_UP(timeout)) {
			res = ETIMEOUT;
			break;
		}
	}
	rx = SPDR;
	UNUSED(rx);

END:
	return res;
}


#endif // USE_SPI

#ifdef __cplusplus
 }
#endif
