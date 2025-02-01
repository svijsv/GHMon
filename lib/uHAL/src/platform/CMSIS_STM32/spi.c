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
//    According to the STM32F4 errata sheet, the last bit received in master
//    mode can be corrupted if the GPIO clock is set too slow for the APB bus
//    clock. Bus speeds below 28MHz would seem to be unaffected by this.
//
#include "spi.h"
#include "system.h"
#include "gpio.h"


#if uHAL_USE_SPI
#define INCLUDED_BY_SPI_C 1

#include "spi_find_periph.h"

#if ! defined(SPIx)
# error "Can't determine SPI peripheral"
#endif

#if (SPIx_CLOCKEN & RCC_BUS_MASK) == RCC_BUS_APB1
# define SPIx_BUSFREQ G_freq_PCLK1
#elif (SPIx_CLOCKEN & RCC_BUS_MASK) == RCC_BUS_APB2
# define SPIx_BUSFREQ G_freq_PCLK2
#elif (SPIx_CLOCKEN & RCC_BUS_MASK) == RCC_BUS_AHB1
# define SPIx_BUSFREQ G_freq_HCLK
#else
# error "Can't determine SPI bus clock"
#endif

#define DIV_256 (0b111U << SPI_CR1_BR_Pos)
#define DIV_128 (0b110U << SPI_CR1_BR_Pos)
#define DIV_64  (0b101U << SPI_CR1_BR_Pos)
#define DIV_32  (0b100U << SPI_CR1_BR_Pos)
#define DIV_16  (0b011U << SPI_CR1_BR_Pos)
#define DIV_8   (0b010U << SPI_CR1_BR_Pos)
#define DIV_4   (0b001U << SPI_CR1_BR_Pos)
#define DIV_2   (0b000U << SPI_CR1_BR_Pos)

#define BUS_IS_FREE(_spix_) (SELECT_BITS((_spix_)->SR, SPI_SR_TXE|SPI_SR_BSY) == SPI_SR_TXE)


static uint32_t calculate_prescaler(uint32_t goal);


void spi_init(void) {
	// Start the clock and reset the peripheral
	clock_init(SPIx_CLOCKEN);

	// Set SPI parameters
	// Per http:// elm-chan.org/docs/mmc/mmc_e.html, this needs to correspond
	// to SPI mode 0 for use with SD cards but mode 3 sometimes works too.
	// https:// www.electronicshub.org/basics-serial-peripheral-interface-spi/#Mode_0
	MODIFY_BITS(SPIx->CR1,
		SPI_CR1_CPHA|SPI_CR1_CPOL|SPI_CR1_MSTR|SPI_CR1_LSBFIRST|SPI_CR1_SSI|SPI_CR1_SSM|SPI_CR1_DFF|SPI_CR1_BR,
		(0b0U << SPI_CR1_CPHA_Pos    )  | // First clock transition (rising edge when polarity is low)
		(0b0U << SPI_CR1_CPOL_Pos    )  | // Keep clock low when idle
		(0b1U << SPI_CR1_MSTR_Pos    )  | // Master mode enabled
		(0b0U << SPI_CR1_LSBFIRST_Pos)  | // Transmit MSB first
		(0b1U << SPI_CR1_SSI_Pos     )  | // Keep SS pin high internally
		(0b1U << SPI_CR1_SSM_Pos     )  | // Enable software slave management
		(0b0U << SPI_CR1_DFF_Pos     )  | // 8 bit frames
		calculate_prescaler(SPI_FREQUENCY_HZ) | // Baud rate prescaler
		0);

	spi_off();

	return;
}
static uint32_t calculate_prescaler(uint32_t goal) {
	uint32_t scaler;

	// Allow speed up to 10% slower than requested
	goal -= (goal / 10U);

	if ((SPIx_BUSFREQ / 256U) >= goal) {
		scaler = DIV_256;
	} else
	if ((SPIx_BUSFREQ / 128U) >= goal) {
		scaler = DIV_128;
	} else
	if ((SPIx_BUSFREQ / 64U) >= goal) {
		scaler = DIV_64;
	} else
	if ((SPIx_BUSFREQ / 32U) >= goal) {
		scaler = DIV_32;
	} else
	if ((SPIx_BUSFREQ / 16U) >= goal) {
		scaler = DIV_16;
	} else
	if ((SPIx_BUSFREQ / 8U) >= goal) {
		scaler = DIV_8;
	} else
	if ((SPIx_BUSFREQ / 4U) >= goal) {
		scaler = DIV_4;
	} else {
		scaler = DIV_2;
	}

	return scaler;
}
static void pins_on(void) {
	gpio_set_AF(SPI_SCK_PIN,  SPIx_AF);
	gpio_set_AF(SPI_MISO_PIN, SPIx_AF);
	gpio_set_AF(SPI_MOSI_PIN, SPIx_AF);

	// Peripheral pin modes specified in the STM32F1 reference manual section
	// 9.1.11
	// I can't find specifications for the other devices, I'm assuming they
	// just need to be AF
	gpio_set_mode(SPI_SCK_PIN,  GPIO_MODE_PP_AF, GPIO_FLOAT);
	gpio_set_mode(SPI_MOSI_PIN, GPIO_MODE_PP_AF, GPIO_FLOAT);
	gpio_set_mode(SPI_MISO_PIN, GPIO_MODE_IN_AF, GPIO_FLOAT);

	return;
}
static void pins_off(void) {
	gpio_set_mode(SPI_SCK_PIN,  GPIO_MODE_RESET, GPIO_FLOAT);
	gpio_set_mode(SPI_MOSI_PIN, GPIO_MODE_RESET, GPIO_FLOAT);
	gpio_set_mode(SPI_MISO_PIN, GPIO_MODE_RESET, GPIO_FLOAT);

	return;
}
err_t spi_on(void) {
	clock_enable(SPIx_CLOCKEN);
	SET_BIT(SPIx->CR1, SPI_CR1_SPE);
	while (!BIT_IS_SET(SPIx->CR1, SPI_CR1_SPE)) {
		// Nothing to do here
	}

	pins_on();

	return ERR_OK;
}
err_t spi_off(void) {
	uint16_t rx;

	uHAL_assert(clock_is_enabled(SPIx_CLOCKEN));
#if ! uHAL_SKIP_INIT_CHECKS
#endif
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
#endif
#if ! uHAL_SKIP_OTHER_CHECKS
	if (!clock_is_enabled(SPIx_CLOCKEN)) {
		return ERR_UNKNOWN;
	}
#endif

	// If the SPI peripheral clock is already disabled but the status flags
	// for whatever reason haven't been cleared, this would become an infinite
	// loop
	if (BIT_IS_SET(SPIx->CR1, SPI_CR1_SPE)) {
		// Section 25.3.8 of the reference manual says the BSY flag may become
		// unreliable if the proper procedure isn't followed before shutting an
		// SPI interface down
		while (BIT_IS_SET(SPIx->SR, SPI_SR_RXNE)) {
			rx = SPIx->DR;
			// If for whatever reason there are still bytes coming in we have no
			// way of knowing how many so just wait until we've gone an arbitrary
			// period of time without seeing any
			delay_ms(1);
		}
		rx = rx; // Shut the compiler up
		while (!BUS_IS_FREE(SPIx)) {
			// Nothing to do here
		}
	}

	pins_off();

	CLEAR_BIT(SPIx->CR1, SPI_CR1_SPE);
	while (BIT_IS_SET(SPIx->CR1, SPI_CR1_SPE)) {
		// Nothing to do here
	}
	clock_disable(SPIx_CLOCKEN);

	return ERR_OK;
}
bool spi_is_on(void) {
	return (clock_is_enabled(SPIx_CLOCKEN) && BIT_IS_SET(SPIx->CR1, SPI_CR1_SPE));
}

err_t spi_exchange_byte(uint8_t tx, uint8_t *rx, utime_t timeout) {
	err_t res;

	uHAL_assert(rx != NULL);
#if ! uHAL_SKIP_INIT_CHECKS
#endif
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (rx == NULL) {
		return ERR_BADARG;
	}
#endif

	res = ERR_OK;
	timeout = SET_TIMEOUT_MS(timeout);

	/*
	while (!BUS_IS_FREE(SPIx)) {
		if (TIMES_UP(timeout)) {
			res = ERR_TIMEOUT;
			goto END;
		}
	}
	*/

	SPIx->DR = tx;
	while (!BIT_IS_SET(SPIx->SR, SPI_SR_RXNE)) {
		if (TIMES_UP(timeout)) {
			res = ERR_TIMEOUT;
			goto END;
		}
	}
	*rx = SPIx->DR;

END:
	return res;
}
err_t spi_receive_block(uint8_t *rx_buffer, txsize_t rx_size, uint8_t tx, utime_t timeout) {
	err_t res;
	txsize_t i;

	uHAL_assert(rx_buffer != NULL);
	uHAL_assert(rx_size > 0);
#if ! uHAL_SKIP_INIT_CHECKS
#endif
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (rx_buffer == NULL || rx_size == 0) {
		return ERR_BADARG;
	}
#endif

	res = ERR_OK;
	timeout = SET_TIMEOUT_MS(timeout);

	/*
	while (!BUS_IS_FREE(SPIx)) {
		if (TIMES_UP(timeout)) {
			res = ERR_TIMEOUT;
			goto END;
		}
	}
	*/

	// The tx buffer stays one step ahead of the rx buffer throughout the loop
	// in order to keep the transfer continuous
	SPIx->DR = tx;
	rx_size -= 1;
	for (i = 0; i < rx_size; ++i) {
		while (!BIT_IS_SET(SPIx->SR, SPI_SR_TXE)) {
			if (TIMES_UP(timeout)) {
				res = ERR_TIMEOUT;
				goto END;
			}
		}
		SPIx->DR = tx;

		while (!BIT_IS_SET(SPIx->SR, SPI_SR_RXNE)) {
			if (TIMES_UP(timeout)) {
				res = ERR_TIMEOUT;
				goto END;
			}
		}
		rx_buffer[i] = SPIx->DR;
	}
	while (!BIT_IS_SET(SPIx->SR, SPI_SR_RXNE)) {
		if (TIMES_UP(timeout)) {
			res = ERR_TIMEOUT;
			break;
		}
	}
	rx_buffer[i] = SPIx->DR;

END:
	return res;
}
err_t spi_transmit_block(const uint8_t *tx_buffer, txsize_t tx_size, utime_t timeout) {
	err_t res;
	uint16_t rx;
	txsize_t i;

	uHAL_assert(tx_buffer != NULL);
	uHAL_assert(tx_size > 0);
#if ! uHAL_SKIP_INIT_CHECKS
#endif
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (tx_buffer == NULL || tx_size == 0) {
		return ERR_BADARG;
	}
#endif

	res = ERR_OK;
	timeout = SET_TIMEOUT_MS(timeout);

	/*
	while (!BUS_IS_FREE(SPIx)) {
		if (TIMES_UP(timeout)) {
			res = ERR_TIMEOUT;
			goto END;
		}
	}
	*/

	// The tx buffer stays one step ahead of the rx buffer throughout the loop
	// in order to keep the transfer continuous
	SPIx->DR = tx_buffer[0];
	for (i = 1; i < tx_size; ++i) {
		while (!BIT_IS_SET(SPIx->SR, SPI_SR_TXE)) {
			if (TIMES_UP(timeout)) {
				res = ERR_TIMEOUT;
				goto END;
			}
		}
		SPIx->DR = tx_buffer[i];

		while (!BIT_IS_SET(SPIx->SR, SPI_SR_RXNE)) {
			if (TIMES_UP(timeout)) {
				res = ERR_TIMEOUT;
				goto END;
			}
		}
		rx = SPIx->DR;
	}
	while (!BIT_IS_SET(SPIx->SR, SPI_SR_RXNE)) {
		if (TIMES_UP(timeout)) {
			res = ERR_TIMEOUT;
			break;
		}
	}
	rx = SPIx->DR;
	UNUSED(rx);

END:
	return res;
}

#endif // uHAL_USE_SPI
