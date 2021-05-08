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
// i2c.c
// Manage the I2C peripheral
// NOTES:
//

#ifdef __cplusplus
 extern "C" {
#endif

//#define DEBUG 1
/*
* Includes
*/
#include "i2c.h"
#include "system.h"


#if USE_I2C

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
void i2c_init(void) {
	uint32_t pclk_MHz, tmp;

	pclk_MHz = I2Cx_BUSFREQ/1000000;

	// Start the clock and reset the peripheral
	clock_init(&I2Cx_APBxENR, &I2Cx_APBxRSTR, I2Cx_CLOCKEN);

	//
	// Set the frequency (in MHz) of the RCC bus the I2C peripheral is on so
	// that clock timing can be generated accurately
	MODIFY_BITS(I2Cx->CR2, I2C_CR2_FREQ,
		(pclk_MHz << I2C_CR2_FREQ_Pos)
		);
	//
	// Configure the I2C clock timing
	// The reference manual formula uses the cycle *time* of the target clock
	// and peripheral clock to calculate the prescaler, but the time is just
	// the reciprocol of the frequency (i.e. a period of 1/10^9 seconds (1ns)
	// corresponds to a frequency of 10^9Hz) so that CCR = i2c_ns/pclk_ns can
	// be rewritten as CCR = pclk_hz/i2c_hz and we can spare ourselves a bit
	// of math calculating the time periods.
	// The desired frequency is multiplied by 2 because we're actually
	// calculating the period of a half-cycle
	tmp = I2Cx_BUSFREQ/(I2C_FREQUENCY*2);
	MODIFY_BITS(I2Cx->CCR, I2C_CCR_FS|I2C_CCR_CCR,
		(0b0 << I2C_CCR_FS_Pos ) | // Use Sm mode
		(tmp << I2C_CCR_CCR_Pos) | // Set prescaler
		0
		);
	/*
	// Dead code left here in case it's useful:
	//
	// There's 10^9 ns per second and our target rate is for half a cycle
	//    F*1000 cycles = 10^9 ns
	//    F*1 cycles = 10^6 ns
	//    1 cycle = 10^6 ns / F
	//    1/2 cycle = (10^6 ns / F)/2 = (10^6) / (F*2)
	i2c_ns = (1000000 / (I2C_FREQUENCY*2));
	//    F*10^6 cycles = 10^9 ns
	//    F*1 cycles = 10^3 ns
	//    1 cycle = 10^3 ns / F
	//pclk_ns = 1000/(G_freq_PCLK1/1000000);
	pclk_ns = 1000000000/(G_freq_PCLK1);
	//
	// From the reference manual:
	//    Tlow = CCR * Tpclk1
	//    Thigh = CCR * Tpclk1
	// so:
	//    Tlow/Tpclk1 = CCR
	ccr = i2c_ns / pclk_ns;
	*/
	//
	// Configure the I2C maximum rise time
	// The reference manual gives the formula (1000ns/Tpclk)+1, which becomes
	// (Fpclk/10^6)+1 when adapted from ns to MHz
	tmp = (pclk_MHz)+1;
	MODIFY_BITS(I2Cx->TRISE, I2C_TRISE_TRISE,
		(tmp << I2C_TRISE_TRISE_Pos)
		);

	i2c_off();

	return;
}

void i2c_on(void) {
	// Enable the peripheral before switching the pin GPIO mode to prevent the
	// pins from briefly pulling low
	clock_enable(&I2Cx_APBxENR, I2Cx_CLOCKEN);
	SET_BIT(I2Cx->CR1, I2C_CR1_PE);

	// Reference manual section 9.1.11 lists pin configuration for peripherals.
	gpio_set_mode(I2C_SCL_PIN, GPIO_MODE_OD_AF, GPIO_FLOAT);
	gpio_set_mode(I2C_SDA_PIN, GPIO_MODE_OD_AF, GPIO_FLOAT);

	return;
}
void i2c_off(void) {
	// Switch the pin GPIO mode before disabling the peripheral to prevent the
	// pins from briefly pulling low
	gpio_set_mode(I2C_SCL_PIN, GPIO_MODE_HiZ, GPIO_FLOAT);
	gpio_set_mode(I2C_SDA_PIN, GPIO_MODE_HiZ, GPIO_FLOAT);

	CLEAR_BIT(I2Cx->CR1, I2C_CR1_PE);
	clock_disable(&I2Cx_APBxENR, I2Cx_CLOCKEN);

	return;
}

err_t i2c_receive_block(uint8_t addr, uint8_t *rx_buffer, txsize_t rx_size, utime_t timeout) {
	err_t res;
	txsize_t i;
	volatile uint32_t tmp;

	assert(rx_buffer != NULL);
	assert(rx_size > 0);

	res = EOK;
	timeout = SET_TIMEOUT(timeout);

	// LSB of address is 1 for RX
	addr = (addr << 1) | 0x01;

	if (rx_size == 2) {
		SET_BIT(I2Cx->CR1, I2C_CR1_POS);
	} else {
		CLEAR_BIT(I2Cx->CR1, I2C_CR1_POS);
	}
	SET_BIT(I2Cx->CR1, I2C_CR1_START|I2C_CR1_ACK);
	while (!BIT_IS_SET(I2Cx->SR1, I2C_SR1_SB)) {
		if (TIMES_UP(timeout)) {
			res = ETIMEOUT;
			goto END;
		}
	}

	I2Cx->DR = addr;
	while (!BIT_IS_SET(I2Cx->SR1, I2C_SR1_ADDR)) {
		if (TIMES_UP(timeout)) {
			res = ETIMEOUT;
			goto END;
		}
	}

	// I don't know why I2C is so convoluted when SPI and UART are both so
	// simple
	// The following stop procedure is taken from the reference manual
	switch (rx_size) {
	case 1:
		CLEAR_BIT(I2Cx->CR1, I2C_CR1_ACK);
		// Read SR2 to clear the ADDR flag
		tmp = I2Cx->SR2;
		SET_BIT(I2Cx->CR1, I2C_CR1_STOP);

		while (!BIT_IS_SET(I2Cx->SR1, I2C_SR1_RXNE)) {
			if (TIMES_UP(timeout)) {
				res = ETIMEOUT;
				goto END;
			}
		}
		rx_buffer[0] = I2Cx->DR;
		break;

	case 2:
		// Read SR2 to clear the ADDR flag
		tmp = I2Cx->SR2;
		CLEAR_BIT(I2Cx->CR1, I2C_CR1_ACK);

		while (!BITS_ARE_SET(I2Cx->SR1, I2C_SR1_RXNE|I2C_SR1_BTF)) {
			if (TIMES_UP(timeout)) {
				res = ETIMEOUT;
				goto END;
			}
		}
		SET_BIT(I2Cx->CR1, I2C_CR1_STOP);
		rx_buffer[0] = I2Cx->DR;
		rx_buffer[1] = I2Cx->DR;
		break;

	default:
		// Read SR2 to clear the ADDR flag
		tmp = I2Cx->SR2;

		// Read all but the last 3 bytes
		rx_size -= 3;
		for (i = 0; i < rx_size; ++i) {
			while (!BIT_IS_SET(I2Cx->SR1, I2C_SR1_RXNE)) {
				if (TIMES_UP(timeout)) {
					res = ETIMEOUT;
					goto END;
				}
			}
			rx_buffer[i] = I2Cx->DR;
		}
		// Wait for both RXNE and BTF, which will mean two bytes ready (one in
		// DR and one in shift register)
		while (!BITS_ARE_SET(I2Cx->SR1, I2C_SR1_RXNE|I2C_SR1_BTF)) {
			if (TIMES_UP(timeout)) {
				res = ETIMEOUT;
				goto END;
			}
		}
		CLEAR_BIT(I2Cx->CR1, I2C_CR1_ACK);
		// Read n-2th bit
		rx_buffer[i++] = I2Cx->DR;
		SET_BIT(I2Cx->CR1, I2C_CR1_STOP);
		// Read n-1th bit
		rx_buffer[i++] = I2Cx->DR;
		// Wait for last byte
		while (!BIT_IS_SET(I2Cx->SR1, I2C_SR1_RXNE)) {
			if (TIMES_UP(timeout)) {
				res = ETIMEOUT;
				goto END;
			}
		}
		rx_buffer[i] = I2Cx->DR;
		break;
	}
	while (BIT_IS_SET(I2Cx->SR2, I2C_SR2_BUSY)) {
		if (TIMES_UP(timeout)) {
			res = ETIMEOUT;
			goto END;
		}
	}

END:
	// If the busy flag is set, something went wrong (most immediately probably
	// a timeout, but that indicates something else happened too)
	//
	// https://www.i2c-bus.org/i2c-primer/analysing-obscure-problems/blocked-bus/
	// Suggests toggling the clock 16 times and then sending stop
	if (BIT_IS_SET(I2Cx->SR2, I2C_SR2_BUSY)) {
		SET_BIT(I2Cx->CR1, I2C_CR1_STOP);
		timeout = SET_TIMEOUT(100);
		while (BIT_IS_SET(I2Cx->SR2, I2C_SR2_BUSY)) {
			if (TIMES_UP(timeout)) {
				NOTIFY("I2C RX Timed out waiting for busy to end");
				res = ETIMEOUT;
				break;
			}
		}
	}

#if DEBUG
	if ((I2Cx->SR1 & ~(I2C_SR1_TXE|I2C_SR1_RXNE)) != 0) {
		LOGGER("RX I2Cx_SR1: 0x%04X", (uint )I2Cx->SR1);
	}
	if ((I2Cx->SR2 & 0xFF03) != 0) {
		LOGGER("RX I2Cx_SR2: 0x%04X", (uint )I2Cx->SR2);
	}
#endif

	UNUSED(tmp);
	return res;
}
err_t i2c_transmit_block(uint8_t addr, const uint8_t *tx_buffer, txsize_t tx_size, utime_t timeout) {
	err_t res;
	txsize_t i;
	volatile uint32_t tmp;

	assert(tx_buffer != NULL);
	assert(tx_size > 0);

	res = EOK;
	timeout = SET_TIMEOUT(timeout);

	// LSB of address is 0 for TX
	addr = (addr << 1) | 0x00;

	CLEAR_BIT(I2Cx->CR1, I2C_CR1_POS);
	SET_BIT(I2Cx->CR1, I2C_CR1_START|I2C_CR1_ACK);
	while (!BIT_IS_SET(I2Cx->SR1, I2C_SR1_SB)) {
		if (TIMES_UP(timeout)) {
			res = ETIMEOUT;
			goto END;
		}
	}

	I2Cx->DR = addr;
	while (!BIT_IS_SET(I2Cx->SR1, I2C_SR1_ADDR)) {
		if (TIMES_UP(timeout)) {
			res = ETIMEOUT;
			goto END;
		}
	}
	// Read SR2 to clear the ADDR flag
	tmp = I2Cx->SR2;

	I2Cx->DR = tx_buffer[0];
	for (i = 1; i < tx_size; ++i) {
		while (!BIT_IS_SET(I2Cx->SR1, I2C_SR1_TXE)) {
			if (TIMES_UP(timeout)) {
				res = ETIMEOUT;
				goto END;
			}
		}
		I2Cx->DR = tx_buffer[i];
	}
	while (!BIT_IS_SET(I2Cx->SR1, I2C_SR1_BTF)) {
		if (TIMES_UP(timeout)) {
			res = ETIMEOUT;
			goto END;
		}
	}
	SET_BIT(I2Cx->CR1, I2C_CR1_STOP);
	while (BIT_IS_SET(I2Cx->SR2, I2C_SR2_BUSY)) {
		if (TIMES_UP(timeout)) {
			res = ETIMEOUT;
			goto END;
		}
	}

END:
	// If the busy flag is set, something went wrong (most immediately probably
	// a timeout, but that indicates something else happened too)
	//
	// https://www.i2c-bus.org/i2c-primer/analysing-obscure-problems/blocked-bus/
	// Suggests toggling the clock 16 times and then sending stop
	if (BIT_IS_SET(I2Cx->SR2, I2C_SR2_BUSY)) {
		SET_BIT(I2Cx->CR1, I2C_CR1_STOP);
		timeout = SET_TIMEOUT(100);
		while (BIT_IS_SET(I2Cx->SR2, I2C_SR2_BUSY)) {
			if (TIMES_UP(timeout)) {
				NOTIFY("I2C TX Timed out waiting for busy to end");
				res = ETIMEOUT;
				break;
			}
		}
	}

#if DEBUG
	if ((I2Cx->SR1 & ~(I2C_SR1_TXE|I2C_SR1_RXNE)) != 0) {
		LOGGER("TX I2Cx_SR1: 0x%04X", (uint )I2Cx->SR1);
	}
	if ((I2Cx->SR2 & 0xFF03) != 0) {
		LOGGER("TX I2Cx_SR2: 0x%04X", (uint )I2Cx->SR2);
	}
#endif

	UNUSED(tmp);
	return res;
}


#endif // USE_I2C

#ifdef __cplusplus
 }
#endif
