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

#include <libopencm3/stm32/spi.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/cm3/nvic.h>


#if USE_SPI

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
static uint8_t calculate_prescaler(uint32_t goal);


/*
* Interrupt handlers
*/


/*
* Functions
*/
void spi_init(void) {
	rcc_periph_clock_enable(SPIx_RCC);
	rcc_periph_reset_pulse(SPIx_RST);

	spi_set_clock_phase_0(SPIx);
	spi_set_clock_polarity_0(SPIx);
	spi_set_dff_8bit(SPIx);
	spi_send_msb_first(SPIx);
	spi_set_full_duplex_mode(SPIx);
	spi_set_master_mode(SPIx);
	spi_enable_software_slave_management(SPIx);
	spi_set_baudrate_prescaler(SPIx, calculate_prescaler(SPI_SPEED));

	// There doesn't seem to be a function for setting the SS pin high
	// internally
	SET_BIT(SPI_CR1(SPIx), SPI_CR1_SSI);

	spi_off();

	return;
}
static uint8_t calculate_prescaler(uint32_t goal) {
	uint8_t scaler;

	if ((SPIx_BUSFREQ / 256) >= goal) {
		scaler = SPI_CR1_BR_FPCLK_DIV_256;
	} else
	if ((SPIx_BUSFREQ / 128) >= goal) {
		scaler = SPI_CR1_BR_FPCLK_DIV_128;
	} else
	if ((SPIx_BUSFREQ / 64) >= goal) {
		scaler = SPI_CR1_BR_FPCLK_DIV_64;
	} else
	if ((SPIx_BUSFREQ / 32) >= goal) {
		scaler = SPI_CR1_BR_FPCLK_DIV_32;
	} else
	if ((SPIx_BUSFREQ / 16) >= goal) {
		scaler = SPI_CR1_BR_FPCLK_DIV_16;
	} else
	if ((SPIx_BUSFREQ / 8) >= goal) {
		scaler = SPI_CR1_BR_FPCLK_DIV_8;
	} else
	if ((SPIx_BUSFREQ / 4) >= goal) {
		scaler = SPI_CR1_BR_FPCLK_DIV_4;
	} else {
		scaler = SPI_CR1_BR_FPCLK_DIV_2;
	}

	return scaler;
}

void spi_on(void) {
	// Reference manual section 9.1.11 lists pin configuration for peripherals.
	gpio_set_mode(SPIx_SCK_PIN,  GPIO_MODE_PP_AF, GPIO_LOW);
	gpio_set_mode(SPIx_MOSI_PIN, GPIO_MODE_PP_AF, GPIO_LOW);
	gpio_set_mode(SPIx_MISO_PIN, GPIO_MODE_IN,    GPIO_HIGH);

	rcc_periph_clock_enable(SPIx_RCC);
	spi_enable(SPIx);

	return;
}
void spi_off(void) {
	// spi_clean_disable() expects there to be another RX and will go into an
	// infinite loop if there isn't one
	//spi_clean_disable(SPIx);
	while ((SPI_SR(SPIx) & SPI_SR_BSY) || (!(SPI_SR(SPIx) & SPI_SR_TXE))) {
		// Nothing to do here
	}
	spi_disable(SPIx);
	rcc_periph_clock_disable(SPIx_RCC);

	gpio_set_mode(SPIx_SCK_PIN,  GPIO_MODE_HiZ, GPIO_LOW);
	gpio_set_mode(SPIx_MOSI_PIN, GPIO_MODE_HiZ, GPIO_LOW);
	gpio_set_mode(SPIx_MISO_PIN, GPIO_MODE_HiZ, GPIO_LOW);

	return;
}

err_t spi_exchange_byte(uint8_t tx, uint8_t *rx, utime_t timeout) {
	assert(rx != NULL);
	UNUSED(timeout);

	*rx = spi_xfer(SPIx, tx);

	return EOK;
}
err_t spi_receive_block(uint8_t *rx_buffer, uint32_t rx_size, uint8_t tx, utime_t timeout) {
	err_t res;
	uint32_t i;

	assert(rx_buffer != NULL);
	assert(rx_size > 0);

	res = EOK;
	timeout = SET_TIMEOUT(timeout);

	// The tx buffer stays one step ahead of the rx buffer throughout the loop
	// in order to keep the transfer continuous
	spi_write(SPIx, tx);
	rx_size -= 1;
	for (i = 0; i < rx_size; ++i) {
		spi_send(SPIx, tx);
		rx_buffer[i] = spi_read(SPIx);

		if (TIMES_UP(timeout)) {
			res = ETIMEOUT;
			goto END;
		}
	}
	rx_buffer[i] = spi_read(SPIx);

END:
	return res;
}
err_t spi_transmit_block(const uint8_t *tx_buffer, uint32_t tx_size, utime_t timeout) {
	err_t res;
	uint32_t i;

	assert(tx_buffer != NULL);
	assert(tx_size > 0);

	res = EOK;
	timeout = SET_TIMEOUT(timeout);

	// The tx buffer stays one step ahead of the rx buffer throughout the loop
	// in order to keep the transfer continuous
	spi_write(SPIx, tx_buffer[0]);
	for (i = 1; i < tx_size; ++i) {
		spi_send(SPIx, tx_buffer[i]);
		spi_read(SPIx);

		if (TIMES_UP(timeout)) {
			res = ETIMEOUT;
			goto END;
		}
	}
	spi_read(SPIx);

END:
	return res;
}

#endif // USE_SPI

#ifdef __cplusplus
 }
#endif
