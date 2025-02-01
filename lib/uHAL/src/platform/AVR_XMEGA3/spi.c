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
// spi.c
// Manage the SPI peripheral
// NOTES:
//

#include "spi.h"
#include "system.h"

#include <avr/io.h>
#include <avr/power.h>


#if uHAL_USE_SPI

// Don't check the SS pin, that's not used
#if PINID(SPI_SCK_PIN) == PINID_SPI0_SCK && PINID(SPI_MISO_PIN) == PINID_SPI0_MISO && PINID(SPI_MOSI_PIN) == PINID_SPI0_MOSI
# define SPIx SPI0
#elif PINID(SPI_SCK_PIN) == PINID_SPI0_REMAP_SCK && PINID(SPI_MISO_PIN) == PINID_SPI0_REMAP_MISO && PINID(SPI_MOSI_PIN) == PINID_SPI0_REMAP_MOSI
# error "SPI pin remapping is not currently supported"
#else
# error "Unsupported SPI device"
#endif

#define DORD_MSB_bm 0
#define DORD_LSB_bm SPI_DORD_bm
#define DORD_bm DORD_MSB_bm

// Clear the interrupt flags by writing '1' to them
#define CLEAR_INTERRUPTS() (SPIx.INTFLAGS = SPI_SSIF_bm | SPI_TXCIF_bm | SPI_RXCIF_bm)

void spi_init(void) {
	uint8_t reg;

	reg = (DORD_bm | SPI_MASTER_bm);

	// Allow the SPI frequency to be up to 1/20 (5%) slower than the desired
	// speed to keep it from instead being way too high
#if ((G_freq_SPICLK/128) >= (SPI_FREQUENCY_HZ - (SPI_FREQUENCY_HZ/20)))
	SET_BIT(reg, SPI_PRESC_DIV128_gc);
#elif ((G_freq_SPICLK/64) >= (SPI_FREQUENCY_HZ - (SPI_FREQUENCY_HZ/20)))
	SET_BIT(reg, SPI_PRESC_DIV64_gc);
#elif ((G_freq_SPICLK/32) >= (SPI_FREQUENCY_HZ - (SPI_FREQUENCY_HZ/20)))
	SET_BIT(reg, SPI_PRESC_DIV64_gc | SPI_CLK2X_bm);
#elif ((G_freq_SPICLK/16) >= (SPI_FREQUENCY_HZ - (SPI_FREQUENCY_HZ/20)))
	SET_BIT(reg, SPI_PRESC_DIV16_gc);
#elif ((G_freq_SPICLK/8) >= (SPI_FREQUENCY_HZ - (SPI_FREQUENCY_HZ/20)))
	SET_BIT(reg, SPI_PRESC_DIV16_gc | SPI_CLK2X_bm);
#elif ((G_freq_SPICLK/4) >= (SPI_FREQUENCY_HZ - (SPI_FREQUENCY_HZ/20)))
	SET_BIT(reg, SPI_PRESC_DIV4_gc);
//#elif ((G_freq_SPICLK/2) >= (SPI_FREQUENCY_HZ - (SPI_FREQUENCY_HZ/20)))
#else
	SET_BIT(reg, SPI_PRESC_DIV4_gc | SPI_CLK2X_bm);
#endif

	SPIx.INTCTRL = 0;
	SPIx.CTRLA = reg;
	SPIx.CTRLB = (SPI_SSD_bm | SPI_MODE_0_gc);

	CLEAR_INTERRUPTS();
	spi_off();

	return;
}
err_t spi_on(void) {
#if ! uHAL_SKIP_OTHER_CHECKS
	if (BIT_IS_SET(SPIx.CTRLA, SPI_ENABLE_bm)) {
		return ERR_OK;
	}
#endif

	// MOSI, SCK, and SS are managed by software but MISO is forced into input
	// by the peripheral
	gpio_set_mode(SPI_SCK_PIN,  GPIO_MODE_PP, GPIO_HIGH);
	gpio_set_mode(SPI_MOSI_PIN, GPIO_MODE_PP, GPIO_HIGH);

	// The SS pin is disabled during configuration by setting the SSD bit in
	// SPIx.CTRLB so there's nothing to do to it here
	/*
	gpio_set_mode(SPI_SS_PIN, GPIO_MODE_IN, GPIO_HIGH);
	*/
	// MISO will be forced into input when SPI is enabled, all this does is
	// turn on the pullup when the pin's bias is high
	gpio_set_mode(SPI_MISO_PIN, GPIO_MODE_IN, GPIO_FLOAT);

	CLEAR_INTERRUPTS();

	// Enable the SPI peripheral
	SET_BIT(SPIx.CTRLA, SPI_ENABLE_bm);

	return ERR_OK;
}
err_t spi_off(void) {
#if ! uHAL_SKIP_OTHER_CHECKS
	if (!BIT_IS_SET(SPIx.CTRLA, SPI_ENABLE_bm)) {
		return ERR_OK;
	}
#endif

	CLEAR_BIT(SPIx.CTRLA, SPI_ENABLE_bm);

	gpio_set_mode(SPI_SCK_PIN,  GPIO_MODE_RESET, GPIO_FLOAT);
	gpio_set_mode(SPI_MOSI_PIN, GPIO_MODE_RESET, GPIO_FLOAT);
	gpio_set_mode(SPI_MISO_PIN, GPIO_MODE_RESET, GPIO_FLOAT);
	// The SS pin is disabled during configuration by setting the SSD bit in
	// SPIx.CTRLB so there's nothing to do to it here
	//gpio_set_mode(SPI_SS_PIN, GPIO_MODE_RESET, GPIO_FLOAT);

	return ERR_OK;
}
bool spi_is_on(void) {
	return BIT_IS_SET(SPIx.CTRLA, SPI_ENABLE_bm);
}

err_t spi_exchange_byte(uint8_t tx, uint8_t *rx, utime_t timeout) {
	err_t res;

	uHAL_assert(rx != NULL);

#if ! uHAL_SKIP_INIT_CHECKS
	if (!BIT_IS_SET(SPIx.CTRLA, SPI_ENABLE_bm) || !BIT_IS_SET(SPIx.CTRLA, SPI_MASTER_bm)) {
		return ERR_INIT;
	}
#endif
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (rx == NULL) {
		return ERR_BADARG;
	}
#endif

	res = ERR_OK;
	timeout = SET_TIMEOUT_MS(timeout);

	// This is just done to clear the IF flag in case it's already set for
	// some reason (an earlier timed-out transmission perhaps?)
	if (BIT_IS_SET(SPIx.INTFLAGS, SPI_IF_bm)) {
		*rx = SPIx.DATA;
	}

	SPIx.DATA = tx;
	while (!BIT_IS_SET(SPIx.INTFLAGS, SPI_IF_bm)) {
		if (TIMES_UP(timeout)) {
			res = ERR_TIMEOUT;
			break;
		}
	}
	*rx = SPIx.DATA;

	return res;
}
err_t spi_receive_block(uint8_t *rx_buffer, txsize_t rx_size, uint8_t tx, utime_t timeout) {
	err_t res;
	txsize_t i;

	uHAL_assert(rx_buffer != NULL);
	uHAL_assert(rx_size > 0);

#if ! uHAL_SKIP_INIT_CHECKS
	if (!BIT_IS_SET(SPIx.CTRLA, SPI_ENABLE_bm) || !BIT_IS_SET(SPIx.CTRLA, SPI_MASTER_bm)) {
		return ERR_INIT;
	}
#endif
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if ((rx_buffer == NULL) || (rx_size <= 0)) {
		return ERR_BADARG;
	}
#endif

	// This is just done to clear the IF flag in case it's already set for
	// some reason (an earlier timed-out transmission perhaps?)
	if (BIT_IS_SET(SPIx.INTFLAGS, SPI_IF_bm)) {
		rx_buffer[0] = SPIx.DATA;
	}

	res = ERR_OK;
	timeout = SET_TIMEOUT_MS(timeout);

	for (i = 0; i < rx_size; ++i) {
		SPIx.DATA = tx;
		while (!BIT_IS_SET(SPIx.INTFLAGS, SPI_IF_bm)) {
			if (TIMES_UP(timeout)) {
				res = ERR_TIMEOUT;
				goto END;
			}
		}
		rx_buffer[i] = SPIx.DATA;
	}

END:
	return res;
}
err_t spi_transmit_block(const uint8_t *tx_buffer, txsize_t tx_size, utime_t timeout) {
	err_t res;
	uint8_t rx;
	txsize_t i;

	uHAL_assert(tx_buffer != NULL);
	uHAL_assert(tx_size > 0);

#if ! uHAL_SKIP_INIT_CHECKS
	if (!BIT_IS_SET(SPIx.CTRLA, SPI_ENABLE_bm) || !BIT_IS_SET(SPIx.CTRLA, SPI_MASTER_bm)) {
		return ERR_INIT;
	}
#endif
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if ((tx_buffer == NULL) || (tx_size <= 0)) {
		return ERR_BADARG;
	}
#endif

	res = ERR_OK;
	timeout = SET_TIMEOUT_MS(timeout);

	for (i = 0; i < tx_size; ++i) {
		SPIx.DATA = tx_buffer[i];
		while (!BIT_IS_SET(SPIx.INTFLAGS, SPI_IF_bm)) {
			if (TIMES_UP(timeout)) {
				res = ERR_TIMEOUT;
				goto END;
			}
		}
		rx = SPIx.DATA;
	}
	UNUSED(rx);

END:
	return res;
}


#endif // uHAL_USE_SPI
