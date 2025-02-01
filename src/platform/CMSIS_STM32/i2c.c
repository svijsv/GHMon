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
// i2c.c
// Manage the I2C peripheral
// NOTES:
//   Arbitration loss is detected when a master releases SDA but it remains
//   high, it therefore can only occur during transmissions
//
#include "i2c.h"
#include "system.h"
#include "gpio.h"


#if uHAL_USE_I2C
#define INCLUDED_BY_I2C_C 1

#include "i2c_find_periph.h"

#if ! defined(I2Cx)
# error "Can't determine I2C peripheral"
#endif

#if (I2Cx_CLOCKEN & RCC_BUS_MASK) == RCC_BUS_APB1
# define I2Cx_BUSFREQ G_freq_PCLK1
#elif (I2Cx_CLOCKEN & RCC_BUS_MASK) == RCC_BUS_APB2
# define I2Cx_BUSFREQ G_freq_PCLK2
#elif (I2Cx_CLOCKEN & RCC_BUS_MASK) == RCC_BUS_AHB1
# define I2Cx_BUSFREQ G_freq_HCLK
#else
# error "Can't determine I2C bus clock"
#endif

#if I2C_FREQUENCY_HZ > 100000
# define USE_FAST_MODE 1
  // I have no fucking idea when or why to use the 9/16 duty cycle mode,
  // the manual only says it's needed to reach 400KHz
# if I2C_FREQUENCY_HZ >= 350000
#  define USE_FAST_DUTY_MODE 1
# else
#  define USE_FAST_DUTY_MODE 0
# endif
#else
# define USE_FAST_MODE 0
# define USE_FAST_DUTY_MODE 0
#endif

#if (I2Cx_BUSFREQ/(I2C_FREQUENCY_HZ*2)) > (I2C_CCR_CCR >> I2C_CCR_CCR_Pos)
# error "I2C frequency is too low for the bus clock"
#endif
#if (I2Cx_BUSFREQ < 2000000)
# error "I2C bus frequency is too low, must be at least 2MHz"
#endif
#if (I2Cx_BUSFREQ < 4000000) && USE_FAST_MODE
# error "I2C bus frequency must be at least 4MHz for >100KHz operation"
#endif
#if (I2Cx_BUSFREQ % 10000000) != 0 && USE_FAST_DUTY_MODE
# error "I2C bus frequency must be a multiple of 10MHz for 400KHz operation"
#endif

DEBUG_CPP_MACRO(USE_FAST_MODE)
DEBUG_CPP_MACRO(USE_FAST_DUTY_MODE)

#define BUS_IS_OWNED(_if_) (BITS_ARE_SET((_if_)->SR2, I2C_SR2_BUSY|I2C_SR2_MSL))
#define PERIPH_IS_INITIALIZED(_if_) ((_if_)->CCR != 0 && BIT_IS_SET((_if_)->CR1, I2C_CR1_PE))

void i2c_init(void) {
	uint32_t pclk_MHz, reg;

#if ! uHAL_SKIP_INIT_CHECKS
#endif
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
#endif

	pclk_MHz = I2Cx_BUSFREQ/1000000U;

	// Start the clock and reset the peripheral
	clock_init(I2Cx_CLOCKEN);

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
	// of math calculating the time periods
	//
	// The formula for standard mode is:
	//   T = CCR * Tpclk * 2
	// For fast mode:
	//   T = CCR * Tpclk * 3
	// For fast duty mode
	//   T = CCR * Tpclk * 25
	// And converted to our desired frequency-based format and re-arranged:
	//   CCR = Fpclk / (F * X)
	// where 'X' is 2, 3, or 25 as needed
	// They could have made this all clearer.
	if (USE_FAST_MODE) {
		if (USE_FAST_DUTY_MODE) {
			reg = (I2Cx_BUSFREQ / (I2C_FREQUENCY_HZ * 25U)) << I2C_CCR_CCR_Pos;
		} else {
			reg = (I2Cx_BUSFREQ / (I2C_FREQUENCY_HZ * 3U)) << I2C_CCR_CCR_Pos;
			reg |= (1U << I2C_CCR_DUTY_Pos);
		}
		reg |= (1U << I2C_CCR_FS_Pos);
	} else {
		reg = (I2Cx_BUSFREQ / (I2C_FREQUENCY_HZ * 2U)) << I2C_CCR_CCR_Pos;
	}
	MODIFY_BITS(I2Cx->CCR, I2C_CCR_FS|I2C_CCR_CCR, reg);

	//
	// Configure the I2C maximum rise time
	// The reference manual gives the formula (Trise/Tpclk)+1, which becomes
	// (Fpclk/(1/Trise))+1 when adapted from ns to MHz Trise is defined by
	// the I2C specificiation, for standard mode it's 1000nS (1uS) and for
	// fast mode its 300nS (0.3uS)
	if (USE_FAST_MODE) {
		reg = ((I2Cx_BUSFREQ / 3333333U) + 1U) << I2C_TRISE_TRISE_Pos;
	} else {
		reg = ((pclk_MHz) + 1U) << I2C_TRISE_TRISE_Pos;
	}
	MODIFY_BITS(I2Cx->TRISE, I2C_TRISE_TRISE, reg);

	i2c_off();

	return;
}
static void pins_on(void) {
	gpio_set_AF(I2C_SCL_PIN, I2Cx_AF);
	gpio_set_AF(I2C_SDA_PIN, I2Cx_AF);

	// Peripheral pin modes specified in the STM32F1 reference manual section
	// 9.1.11
	// I can't find specifications for the other devices, I'm assuming they
	// just need to be AF
	gpio_set_mode(I2C_SCL_PIN, GPIO_MODE_OD_AF, GPIO_FLOAT);
	gpio_set_mode(I2C_SDA_PIN, GPIO_MODE_OD_AF, GPIO_FLOAT);

	return;
}
static void pins_off(void) {
	gpio_set_mode(I2C_SCL_PIN, GPIO_MODE_RESET, GPIO_FLOAT);
	gpio_set_mode(I2C_SDA_PIN, GPIO_MODE_RESET, GPIO_FLOAT);

	return;
}
err_t i2c_on(void) {
#if ! uHAL_SKIP_INIT_CHECKS
#endif
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
#endif

	// Enable the peripheral before switching the pin GPIO mode to prevent the
	// pins from briefly pulling low
	clock_enable(I2Cx_CLOCKEN);
	SET_BIT(I2Cx->CR1, I2C_CR1_PE);
	while (!BIT_IS_SET(I2Cx->CR1, I2C_CR1_PE)) {
		// Nothing to do here
	}
	pins_on();

	return ERR_OK;
}
err_t i2c_off(void) {
#if ! uHAL_SKIP_INIT_CHECKS
#endif
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
#endif

	if (!clock_is_enabled(I2Cx_CLOCKEN)) {
		return ERR_OK;
	}

	// This probably isn't needed, the only time it might matter is if the
	// peripheral is disabled before the stop condition has been fully
	// broadcast
	utime_t timeout = SET_TIMEOUT_MS(100);
	while (BUS_IS_OWNED(I2Cx)) {
		if (TIMES_UP(timeout)) {
			break;
		}
	}

	// Switch the pin GPIO mode before disabling the peripheral to prevent the
	// pins from briefly pulling low
	pins_off();
	CLEAR_BIT(I2Cx->CR1, I2C_CR1_PE);
	while (BIT_IS_SET(I2Cx->CR1, I2C_CR1_PE)) {
		// Nothing to do here
	}
	clock_disable(I2Cx_CLOCKEN);

	return ERR_OK;
}
bool i2c_is_on(void) {
#if ! uHAL_SKIP_INIT_CHECKS
#endif
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
#endif

	return (clock_is_enabled(I2Cx_CLOCKEN) && BIT_IS_SET(I2Cx->CR1, I2C_CR1_PE));
}

err_t i2c_receive_block(uint8_t addr, uint8_t *rx_buffer, txsize_t rx_size, utime_t timeout) {
	err_t res;
	txsize_t i;
	volatile uint32_t tmp;

	uHAL_assert(PERIPH_IS_INITIALIZED(I2Cx));
	uHAL_assert(addr <= 0x7FU);
	uHAL_assert(rx_buffer != NULL);
	uHAL_assert(rx_size > 0);
#if ! uHAL_SKIP_INIT_CHECKS
	if (!PERIPH_IS_INITIALIZED(I2Cx)) {
		return ERR_INIT;
	}
#endif
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if ((addr > 0x7FU) || (rx_buffer == NULL) || (rx_size <= 0)) {
		return ERR_BADARG;
	}
#endif

	res = ERR_OK;
	timeout = SET_TIMEOUT_MS(timeout);

	/*
	if (BIT_IS_SET(I2Cx->SR2, I2C_SR2_BUSY) && !BIT_IS_SET(I2Cx->SR2, I2C_SR2_MSL)) {
		return ERR_RETRY;
	}
	*/
	while (BIT_IS_SET(I2Cx->SR2, I2C_SR2_BUSY)) {
		if (TIMES_UP(timeout)) {
			return ERR_TIMEOUT;
		}
	}

	// LSB of address is 1 for RX
	addr = (addr << 1U) | 0x01U;

	if (rx_size == 2) {
		SET_BIT(I2Cx->CR1, I2C_CR1_POS);
	} else {
		CLEAR_BIT(I2Cx->CR1, I2C_CR1_POS);
	}
	CLEAR_BIT(I2Cx->SR1, I2C_SR1_BERR|I2C_SR1_ARLO);
	SET_BIT(I2Cx->CR1, I2C_CR1_START|I2C_CR1_ACK);
	while (!BIT_IS_SET(I2Cx->SR1, I2C_SR1_SB)) {
		if (TIMES_UP(timeout)) {
			res = ERR_TIMEOUT;
			goto END;
		}
		if (BIT_IS_SET(I2Cx->SR1, I2C_SR1_ARLO)) {
			res = ERR_RETRY;
			goto END;
		}
		if (BIT_IS_SET(I2Cx->SR1, I2C_SR1_BERR)) {
			res = ERR_UNKNOWN;
			goto END;
		}
	}

	I2Cx->DR = addr;
	while (!BIT_IS_SET(I2Cx->SR1, I2C_SR1_ADDR)) {
		if (TIMES_UP(timeout)) {
			res = ERR_TIMEOUT;
			goto END;
		}
		if (BIT_IS_SET(I2Cx->SR1, I2C_SR1_ARLO)) {
			res = ERR_RETRY;
			goto END;
		}
		if (BIT_IS_SET(I2Cx->SR1, I2C_SR1_BERR)) {
			res = ERR_UNKNOWN;
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
				res = ERR_TIMEOUT;
				goto END;
			}
			if (BIT_IS_SET(I2Cx->SR1, I2C_SR1_BERR)) {
				res = ERR_UNKNOWN;
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
				res = ERR_TIMEOUT;
				goto END;
			}
			if (BIT_IS_SET(I2Cx->SR1, I2C_SR1_BERR)) {
				res = ERR_UNKNOWN;
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
					res = ERR_TIMEOUT;
					goto END;
				}
				if (BIT_IS_SET(I2Cx->SR1, I2C_SR1_BERR)) {
					res = ERR_UNKNOWN;
					goto END;
				}
			}
			rx_buffer[i] = I2Cx->DR;
		}
		// Wait for both RXNE and BTF, which will mean two bytes ready (one in
		// DR and one in shift register)
		while (!BITS_ARE_SET(I2Cx->SR1, I2C_SR1_RXNE|I2C_SR1_BTF)) {
			if (TIMES_UP(timeout)) {
				res = ERR_TIMEOUT;
				goto END;
			}
			if (BIT_IS_SET(I2Cx->SR1, I2C_SR1_BERR)) {
				res = ERR_UNKNOWN;
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
				res = ERR_TIMEOUT;
				goto END;
			}
			if (BIT_IS_SET(I2Cx->SR1, I2C_SR1_BERR)) {
				res = ERR_UNKNOWN;
				goto END;
			}
		}
		rx_buffer[i] = I2Cx->DR;
		break;
	}
	while (BIT_IS_SET(I2Cx->SR2, I2C_SR2_BUSY)) {
		if (TIMES_UP(timeout)) {
			res = ERR_TIMEOUT;
			goto END;
		}
		if (BIT_IS_SET(I2Cx->SR1, I2C_SR1_ARLO)) {
			res = ERR_RETRY;
			goto END;
		}
		if (BIT_IS_SET(I2Cx->SR1, I2C_SR1_BERR)) {
			res = ERR_UNKNOWN;
			goto END;
		}
	}

END:
	if (BUS_IS_OWNED(I2Cx)) {
		// We *could* wait around here for the stop condition to be broadcast, but
		// there's really no reason to so that's handled in i2c_off()
		SET_BIT(I2Cx->CR1, I2C_CR1_STOP);
	}

#if DEBUG && 0
	if ((I2Cx->SR1 & ~(I2C_SR1_TXE|I2C_SR1_RXNE|I2C_SR1_BTF)) != 0) {
		LOGGER("RX I2Cx_SR1: 0x%04X", (uint )I2Cx->SR1);
	}
	// MSL and BUSY will prbably be set because we dont wait after setting
	// stop bit
	//if ((I2Cx->SR2 & 0xFF03U) != 0) {
	if ((I2Cx->SR2 & 0xFF00U) != 0) {
		LOGGER("RX I2Cx_SR2: 0x%04X", (uint )I2Cx->SR2);
	}
#endif

	UNUSED(tmp);
	return res;
}

static err_t _i2c_transmit_block_begin(uint8_t addr, utime_t timeout) {
	err_t res = ERR_OK;
	volatile uint32_t tmp;

	/*
	if (BIT_IS_SET(I2Cx->SR2, I2C_SR2_BUSY) && !BIT_IS_SET(I2Cx->SR2, I2C_SR2_MSL)) {
		return ERR_RETRY;
	}
	*/
	while (BIT_IS_SET(I2Cx->SR2, I2C_SR2_BUSY)) {
		if (TIMES_UP(timeout)) {
			return ERR_TIMEOUT;
		}
	}

	// LSB of address is 0 for TX
	addr = (addr << 1) | 0x00U;

	CLEAR_BIT(I2Cx->CR1, I2C_CR1_POS);
	CLEAR_BIT(I2Cx->SR1, I2C_SR1_BERR|I2C_SR1_ARLO|I2C_SR1_AF);
	SET_BIT(I2Cx->CR1, I2C_CR1_START|I2C_CR1_ACK);
	while (!BIT_IS_SET(I2Cx->SR1, I2C_SR1_SB)) {
		if (TIMES_UP(timeout)) {
			res = ERR_TIMEOUT;
			goto END;
		}
		if (BIT_IS_SET(I2Cx->SR1, I2C_SR1_ARLO)) {
			res = ERR_RETRY;
			goto END;
		}
		if (BIT_IS_SET(I2Cx->SR1, I2C_SR1_BERR)) {
			res = ERR_UNKNOWN;
			goto END;
		}
	}

	I2Cx->DR = addr;
	while (!BIT_IS_SET(I2Cx->SR1, I2C_SR1_ADDR)) {
		if (TIMES_UP(timeout)) {
			res = ERR_TIMEOUT;
			goto END;
		}
		if (BIT_IS_SET(I2Cx->SR1, I2C_SR1_ARLO)) {
			res = ERR_RETRY;
			goto END;
		}
		if (BIT_IS_SET(I2Cx->SR1, I2C_SR1_BERR)) {
			res = ERR_UNKNOWN;
			goto END;
		}
	}
	// Read SR2 to clear the ADDR flag
	tmp = I2Cx->SR2;

END:
	UNUSED(tmp);
	return res;
}
err_t i2c_transmit_block_begin(uint8_t addr, utime_t timeout) {
	uHAL_assert(PERIPH_IS_INITIALIZED(I2Cx));
	uHAL_assert(addr <= 0x7FU);
#if ! uHAL_SKIP_INIT_CHECKS
	if (!PERIPH_IS_INITIALIZED(I2Cx)) {
		return ERR_INIT;
	}
#endif
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (addr > 0x7FU) {
		return ERR_BADARG;
	}
#endif

	timeout = SET_TIMEOUT_MS(timeout);

	return _i2c_transmit_block_begin(addr, timeout);
}

static err_t _i2c_transmit_block_continue(const uint8_t *tx_buffer, txsize_t tx_size, utime_t timeout) {
	err_t res = ERR_OK;

	for (txsize_t i = 0; i < tx_size; ++i) {
		I2Cx->DR = tx_buffer[i];
		while (!BIT_IS_SET(I2Cx->SR1, I2C_SR1_TXE)) {
			if (TIMES_UP(timeout)) {
				res = ERR_TIMEOUT;
				goto END;
			}
			if (BIT_IS_SET(I2Cx->SR1, I2C_SR1_AF)) {
				res = ERR_INTERRUPT;
				goto END;
			}
			if (BIT_IS_SET(I2Cx->SR1, I2C_SR1_ARLO|I2C_SR1_BERR)) {
				res = ERR_UNKNOWN;
				goto END;
			}
		}
	}

END:
	return res;
}
err_t i2c_transmit_block_continue(const uint8_t *tx_buffer, txsize_t tx_size, utime_t timeout) {
	uHAL_assert(PERIPH_IS_INITIALIZED(I2Cx));
	uHAL_assert(tx_buffer != NULL);
	uHAL_assert(tx_size > 0);
#if ! uHAL_SKIP_INIT_CHECKS
	if (!PERIPH_IS_INITIALIZED(I2Cx) || !BUS_IS_OWNED(I2Cx)) {
		return ERR_INIT;
	}
#endif
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if ((tx_buffer == NULL) || (tx_size <= 0)) {
		return ERR_BADARG;
	}
#endif

	timeout = SET_TIMEOUT_MS(timeout);

	return _i2c_transmit_block_continue(tx_buffer, tx_size, timeout);
}

err_t i2c_transmit_block_end(void) {
	utime_t timeout = SET_TIMEOUT_MS(100);

	uHAL_assert(PERIPH_IS_INITIALIZED(I2Cx));
#if ! uHAL_SKIP_INIT_CHECKS
	if (!PERIPH_IS_INITIALIZED(I2Cx)) {
		return ERR_INIT;
	}
#endif

	while (!BIT_IS_SET(I2Cx->SR1, I2C_SR1_BTF|I2C_SR1_TXE)) {
		if (TIMES_UP(timeout)) {
			break;
		}
	}
	if (BUS_IS_OWNED(I2Cx)) {
		// We *could* wait around here for the stop condition to be broadcast, but
		// there's really no reason to so that's handled in i2c_off()
		SET_BIT(I2Cx->CR1, I2C_CR1_STOP);
	}

#if DEBUG && 0
	if ((I2Cx->SR1 & ~(I2C_SR1_TXE|I2C_SR1_RXNE|I2C_SR1_BTF)) != 0) {
		LOGGER("TX I2Cx_SR1: 0x%04X", (uint )I2Cx->SR1);
	}
	// MSL and BUSY will prbably be set because we dont wait after setting
	// stop bit
	//if ((I2Cx->SR2 & 0xFF03U) != 0) {
	if ((I2Cx->SR2 & 0xFF00U) != 0) {
		LOGGER("TX I2Cx_SR2: 0x%04X", (uint )I2Cx->SR2);
	}
#endif

	return ERR_OK;
}

err_t i2c_transmit_block(uint8_t addr, const uint8_t *tx_buffer, txsize_t tx_size, utime_t timeout) {
	err_t res = ERR_OK;

	uHAL_assert(PERIPH_IS_INITIALIZED(I2Cx));
	uHAL_assert(addr <= 0x7FU);
	uHAL_assert(tx_buffer != NULL);
	uHAL_assert(tx_size > 0);
#if ! uHAL_SKIP_INIT_CHECKS
	if (!PERIPH_IS_INITIALIZED(I2Cx)) {
		return ERR_INIT;
	}
#endif
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if ((addr > 0x7FU) || (tx_buffer == NULL) || (tx_size <= 0)) {
		return ERR_BADARG;
	}
#endif

	timeout = SET_TIMEOUT_MS(timeout);

	if ((res = _i2c_transmit_block_begin(addr, timeout)) != ERR_OK) {
		goto END;
	}
	if ((res = _i2c_transmit_block_continue(tx_buffer, tx_size, timeout)) != ERR_OK) {
		goto END;
	}

END:
	i2c_transmit_block_end();
	return res;
}


/*
err_t i2c_transmit_block(uint8_t addr, const uint8_t *tx_buffer, txsize_t tx_size, utime_t timeout) {
	err_t res;
	txsize_t i;
	volatile uint32_t tmp;

	uHAL_assert(tx_buffer != NULL);
	uHAL_assert(tx_size > 0);

	res = ERR_OK;
	timeout = SET_TIMEOUT_MS(timeout);

	//if (BIT_IS_SET(I2Cx->SR2, I2C_SR2_BUSY) && !BIT_IS_SET(I2Cx->SR2, I2C_SR2_MSL)) {
	//	return ERR_RETRY;
	//}
	while (BIT_IS_SET(I2Cx->SR2, I2C_SR2_BUSY)) {
		if (TIMES_UP(timeout)) {
			return ERR_TIMEOUT;
		}
	}

	// LSB of address is 0 for TX
	addr = (addr << 1) | 0x00U;

	CLEAR_BIT(I2Cx->CR1, I2C_CR1_POS);
	CLEAR_BIT(I2Cx->SR1, I2C_SR1_BERR|I2C_SR1_ARLO|I2C_SR1_AF);
	SET_BIT(I2Cx->CR1, I2C_CR1_START|I2C_CR1_ACK);
	while (!BIT_IS_SET(I2Cx->SR1, I2C_SR1_SB)) {
		if (TIMES_UP(timeout)) {
			res = ERR_TIMEOUT;
			goto END;
		}
		if (BIT_IS_SET(I2Cx->SR1, I2C_SR1_ARLO)) {
			res = ERR_RETRY;
			goto END;
		}
		if (BIT_IS_SET(I2Cx->SR1, I2C_SR1_BERR)) {
			res = ERR_UNKNOWN;
			goto END;
		}
	}

	I2Cx->DR = addr;
	while (!BIT_IS_SET(I2Cx->SR1, I2C_SR1_ADDR)) {
		if (TIMES_UP(timeout)) {
			res = ERR_TIMEOUT;
			goto END;
		}
		if (BIT_IS_SET(I2Cx->SR1, I2C_SR1_ARLO)) {
			res = ERR_RETRY;
			goto END;
		}
		if (BIT_IS_SET(I2Cx->SR1, I2C_SR1_BERR)) {
			res = ERR_UNKNOWN;
			goto END;
		}
	}
	// Read SR2 to clear the ADDR flag
	tmp = I2Cx->SR2;

	I2Cx->DR = tx_buffer[0];
	for (i = 1; i < tx_size; ++i) {
		while (!BIT_IS_SET(I2Cx->SR1, I2C_SR1_TXE)) {
			if (TIMES_UP(timeout)) {
				res = ERR_TIMEOUT;
				goto END;
			}
			if (BIT_IS_SET(I2Cx->SR1, I2C_SR1_ARLO)) {
				res = ERR_RETRY;
				goto END;
			}
			if (BIT_IS_SET(I2Cx->SR1, I2C_SR1_BERR|I2C_SR1_AF)) {
				res = ERR_UNKNOWN;
				goto END;
			}
		}
		I2Cx->DR = tx_buffer[i];
	}
	while (!BIT_IS_SET(I2Cx->SR1, I2C_SR1_BTF)) {
		if (TIMES_UP(timeout)) {
			res = ERR_TIMEOUT;
			goto END;
		}
		if (BIT_IS_SET(I2Cx->SR1, I2C_SR1_ARLO)) {
			res = ERR_RETRY;
			goto END;
		}
		if (BIT_IS_SET(I2Cx->SR1, I2C_SR1_BERR)) {
			res = ERR_UNKNOWN;
			goto END;
		}
	}

END:
	if (BUS_IS_OWNED(I2Cx)) {
		// We *could* wait around here for the stop condition to be broadcast, but
		// there's really no reason to so that's handled in i2c_off()
		SET_BIT(I2Cx->CR1, I2C_CR1_STOP);
	}

#if DEBUG && 0
	if ((I2Cx->SR1 & ~(I2C_SR1_TXE|I2C_SR1_RXNE|I2C_SR1_BTF)) != 0) {
		LOGGER("TX I2Cx_SR1: 0x%04X", (uint )I2Cx->SR1);
	}
	// MSL and BUSY will prbably be set because we dont wait after setting
	// stop bit
	//if ((I2Cx->SR2 & 0xFF03U) != 0) {
	if ((I2Cx->SR2 & 0xFF00U) != 0) {
		LOGGER("TX I2Cx_SR2: 0x%04X", (uint )I2Cx->SR2);
	}
#endif

	UNUSED(tmp);
	return res;
}
*/


#endif // uHAL_USE_I2C
