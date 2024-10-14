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
//

#include "i2c.h"
#include "system.h"
// For bug workaround using gpio_clear_outbit():
#include "gpio.h"

#include <avr/io.h>
#include <avr/power.h>


#if uHAL_USE_I2C

// There's only one possible TWI device, but to make future expansion easier
// let's do this
#if PINID(I2C_SCL_PIN) == PINID_I2C0_SCL && PINID(I2C_SDA_PIN) == PINID_I2C0_SDA
# define TWIx TWI0
  //DEBUG_CPP_MACRO(TWIx)
  DEBUG_CPP_MSG("TWIx == TWI0")
#else
# error "Unsupported I2C device"
#endif
// Minimum baud for fast mode:
#define FM_MIN_BAUD  (125000)
// Minimum baud for fast mode plus:
#define FMP_MIN_BAUD (425000)
// 50NS is the backward-compatible hold time, see reference manual for other
// options
#define HOLD_TIME TWI_SDAHOLD_50NS_gc


// Clear the status flags by writing '1' to them
#define CLEAR_STATUS() (TWIx.MSTATUS = TWI_BUSERR_bm | TWI_ARBLOST_bm | TWI_CLKHOLD_bm | TWI_WIF_bm | TWI_RIF_bm)

// The formulas for calculating the different possible baud rates are both
// derived from formulas given in the manual for the ATmega3208
#define CALC_BAUD_MIN(rise_nS) (((G_freq_TWICLK) / (2U * I2C_FREQUENCY_HZ)) - (5U + ((G_freq_TWICLK/1000000U * rise_nS) / 2000U)))
#define CALC_BAUD_MAX(min_nS, fall_nS) (((G_freq_TWICLK/1000000U) * (min_nS + fall_nS))/1000U - 5U)

#define ADDRESS_OK(_addr_) ((_addr_) <= 0x7FU)
#define BUFFER_OK(_name_) (_name_ ## _buffer != NULL && _name_ ## _size > 0)
#define TWIx_INIT_OK(_TWIx_) ((_TWIx_).MBAUD != 0 && BIT_IS_SET((_TWIx_).MCTRLA, TWI_ENABLE_bm) && SELECT_BITS((_TWIx_).MSTATUS, TWI_BUSSTATE_gm) != TWI_BUSSTATE_UNKNOWN_gc)

void i2c_init(void) {
	uint16_t baud_min, baud_max, baud;
	uint8_t reg;

	// Make sure the slave interface is disabled
	TWIx.SCTRLA = 0;

#if I2C_FREQUENCY_HZ >= FMP_MIN_BAUD
	reg = (TWI_SDASETUP_4CYC_gc | HOLD_TIME | TWI_FMPEN_bm);
#else
	reg = (TWI_SDASETUP_4CYC_gc | HOLD_TIME);
#endif
	TWIx.CTRLA = reg;
	// The register description in the manual suggests that the timeout values
	// are frequency-dependent and the default names are only true for 100KHz
	// clock rates, but I don't see anything confirming this
	TWIx.MCTRLA = (TWI_TIMEOUT_DISABLED_gc | TWI_SMEN_bm);

	TWIx.MCTRLB = 0;

	// The reference manual gives the formula for calculating the frequency
	// as:
	//    F_scl == (F_clk_per) / (10 + 2*BAUD + F_clk_per * T_rise)
	// so to calculate BAUD we do:
	//    BAUD = (F_clk_per / (2*F_scl)) - (5 + ((F_clk_per * T_rise) / 2))
	// or to work with nS for the T_rise (and thereby avoid a float):
	//    BAUD = (F_clk_per / (2*F_scl)) - (5 + ((F_clk_per/1000000 * T_rise) / 2000))
	//
	// The electrical characteristics section of the manual lists the maximum
	// T_rise for the internal pullups, which is as good a generic time to use
	// as any. However, there's also a minimum low time (also listed in the
	// electrical characteristics section) so it's helpful to calculate a baud
	// value based on the maximum rise times and then also on the minimum low
	// time and just use the higher of the two (which will give the slower of
	// the two frequencies)
#if I2C_FREQUENCY_HZ >= FMP_MIN_BAUD
	baud_min = CALC_BAUD_MIN(120);
	baud_max = CALC_BAUD_MAX(500, 120);
#elif I2C_FREQUENCY_HZ >= FM_MIN_BAUD
	baud_min = CALC_BAUD_MIN(300);
	baud_max = CALC_BAUD_MAX(1300, 250);
#else
	baud_min = CALC_BAUD_MIN(1000);
	baud_max = CALC_BAUD_MAX(4700, 250);
#endif
	baud = MAX(baud_min, baud_max);
	if (baud > 255) {
		baud = 255;
	}
	TWIx.MBAUD = baud;

	CLEAR_STATUS();
	i2c_off();

	return;
}
err_t i2c_on(void) {
#if ! uHAL_SKIP_OTHER_CHECKS
	if (BIT_IS_SET(TWIx.MCTRLA, TWI_ENABLE_bm)) {
		return ERR_OK;
	}
#endif

	gpio_set_mode(I2C_SCL_PIN, GPIO_MODE_IN, GPIO_FLOAT);
	gpio_set_mode(I2C_SDA_PIN, GPIO_MODE_IN, GPIO_FLOAT);
	// There's apparently a hardware bug that results in the peripheral pulling
	// the pins up instead of down if the OUT bit is set. I can't find the
	// original source but it's mentioned here:
	// https://github.com/SpenceKonde/megaTinyCore/blob/master/megaavr/libraries/Wire/README.md#errata-warning
	gpio_clear_outbit(I2C_SCL_PIN);
	gpio_clear_outbit(I2C_SDA_PIN);

	CLEAR_STATUS();
	SET_BIT(TWIx.MCTRLA, TWI_ENABLE_bm);
	// The bus state is unknown when first enabled, let's hope it really is
	// idle...
	MODIFY_BITS(TWIx.MSTATUS, TWI_BUSSTATE_gm, TWI_BUSSTATE_IDLE_gc);

	return ERR_OK;
}
err_t i2c_off(void) {
#if ! uHAL_SKIP_OTHER_CHECKS
	if (!BIT_IS_SET(TWIx.MCTRLA, TWI_ENABLE_bm)) {
		return ERR_OK;
	}
#endif

	// This probably isn't needed, the only time it might matter is if the
	// peripheral is disabled before the stop condition has been fully
	// broadcast
	utime_t timeout = SET_TIMEOUT_MS(100);
	while (SELECT_BITS(TWIx.MSTATUS, TWI_BUSSTATE_gm) == TWI_BUSSTATE_OWNER_gc) {
		if (TIMES_UP(timeout)) {
			break;
		}
	}

	CLEAR_BIT(TWIx.MCTRLA, TWI_ENABLE_bm);

	gpio_set_mode(I2C_SCL_PIN, GPIO_MODE_RESET, GPIO_FLOAT);
	gpio_set_mode(I2C_SDA_PIN, GPIO_MODE_RESET, GPIO_FLOAT);

	return ERR_OK;
}
bool i2c_is_on(void) {
	return (BIT_IS_SET(TWIx.MCTRLA, TWI_ENABLE_bm));
}

err_t i2c_receive_block(uint8_t addr, uint8_t *rx_buffer, txsize_t rx_size, utime_t timeout) {
	err_t res = ERR_OK;
	txsize_t i;

	assert(ADDRESS_OK(addr));
	assert(BUFFER_OK(rx));
	assert(TWIx_INIT_OK(TWIx));

#if ! uHAL_SKIP_INIT_CHECKS
	if (!TWIx_INIT_OK(TWIx)) {
		return ERR_INIT;
	}
#endif
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (!ADDRESS_OK(addr) || !BUFFER_OK(rx)) {
		return ERR_BADARG;
	}
#endif

	timeout = SET_TIMEOUT_MS(timeout);

	// Make sure the bus is ready
	/*
	if (SELECT_BITS(TWIx.MSTATUS, TWI_BUSSTATE_gm) != TWI_BUSSTATE_IDLE_gc) {
		return ERR_RETRY;
	}
	*/
	/*
	// This is handled by the hardware
	while (SELECT_BITS(TWIx.MSTATUS, TWI_BUSSTATE_gm) != TWI_BUSSTATE_IDLE_gc) {
		if (TIMES_UP(timeout)) {
			res = ERR_TIMEOUT;
			goto END;
		}
	}
	*/

	//
	// Send start condition and slave address
	// Setting MADDR resets any bus error flags
	TWIx.MADDR = (addr << 1U) | 0x01U;
	// WIF will be set if there's an error, RIF will be set if the address
	// was transmitted successfully and a data packet returned
	while (!BIT_IS_SET(TWIx.MSTATUS, TWI_WIF_bm|TWI_RIF_bm)) {
		if (TIMES_UP(timeout)) {
			res = ERR_TIMEOUT;
			goto END;
		}
	}
	if (BIT_IS_SET(TWIx.MSTATUS, TWI_ARBLOST_bm)) {
		res = ERR_RETRY;
		goto END;
	}
	// RXACK should be cleared after the response is recieved
	if (BIT_IS_SET(TWIx.MSTATUS, TWI_BUSERR_bm|TWI_RXACK_bm)) {
		res = ERR_UNKNOWN;
		goto END;
	}

	//
	// Receive the data packets
	// The first byte was recieved automatically after sending the address
	// The last byte is handled separately because it needs to send a NACK
	--rx_size;
	TWIx.MCTRLB = TWI_ACKACT_ACK_gc;
	for (i = 0; i < rx_size; ++i) {
		while (!BIT_IS_SET(TWIx.MSTATUS, TWI_RIF_bm)) {
			if (TIMES_UP(timeout)) {
				res = ERR_TIMEOUT;
				goto END;
			}
		}
		// The value of ACKACT in MCTRLB is automatcally sent when MDATA is read
		// and smart mode is enabled
		rx_buffer[i] = TWIx.MDATA;
	}

	while (!BIT_IS_SET(TWIx.MSTATUS, TWI_RIF_bm)) {
		if (TIMES_UP(timeout)) {
			res = ERR_TIMEOUT;
			goto END;
		}
	}
	// Read the last byte
	TWIx.MCTRLB = TWI_ACKACT_NACK_gc;
	rx_buffer[i] = TWIx.MDATA;

END:
	// Stop the transmission
	if (SELECT_BITS(TWIx.MSTATUS, TWI_BUSSTATE_gm) == TWI_BUSSTATE_OWNER_gc) {
		// We *could* wait around here for the stop condition to be broadcast, but
		// there's really no reason to so that's handled in i2c_off()
		TWIx.MCTRLB = TWI_ACKACT_NACK_gc | TWI_MCMD_STOP_gc;
	}

	return res;
}

static err_t _i2c_transmit_block_begin(uint8_t addr, utime_t timeout) {
	err_t res = ERR_OK;

	//
	// Send start condition and slave address
	// Setting MADDR resets any bus error flags
	TWIx.MADDR = (addr << 1U) | 0x00U;
	while (!BIT_IS_SET(TWIx.MSTATUS, TWI_WIF_bm)) {
		if (TIMES_UP(timeout)) {
			res = ERR_TIMEOUT;
			goto END;
		}
	}
	if (BIT_IS_SET(TWIx.MSTATUS, TWI_ARBLOST_bm)) {
		res = ERR_RETRY;
		goto END;
	}
	// RXACK should be cleared after the response is recieved
	if (BIT_IS_SET(TWIx.MSTATUS, TWI_BUSERR_bm|TWI_RXACK_bm)) {
		res = ERR_UNKNOWN;
		goto END;
	}

END:
	return res;
}
err_t i2c_transmit_block_begin(uint8_t addr, utime_t timeout) {
	assert(ADDRESS_OK(addr));
	assert(TWIx_INIT_OK(TWIx));

#if ! uHAL_SKIP_INIT_CHECKS
	if (!TWIx_INIT_OK(TWIx)) {
		return ERR_INIT;
	}
#endif
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (!ADDRESS_OK(addr)) {
		return ERR_BADARG;
	}
#endif

	timeout = SET_TIMEOUT_MS(timeout);

	return _i2c_transmit_block_begin(addr, timeout);
}

static err_t _i2c_transmit_block_continue(const uint8_t *tx_buffer, txsize_t tx_size, utime_t timeout) {
	err_t res = ERR_OK;

	//
	// Send the data packets
	for (txsize_t i = 0; i < tx_size; ++i) {
		TWIx.MDATA = tx_buffer[i];
		while (!BIT_IS_SET(TWIx.MSTATUS, TWI_WIF_bm)) {
			if (TIMES_UP(timeout)) {
				res = ERR_TIMEOUT;
				goto END;
			}
		}

		if (BIT_IS_SET(TWIx.MSTATUS, TWI_ARBLOST_bm|TWI_BUSERR_bm)) {
			res = ERR_UNKNOWN;
			goto END;
		}

		// NACK indicates the slave couldn't or doesn't need to recieve more
		// data
		if (BIT_IS_SET(TWIx.MSTATUS, TWI_RXACK_bm)) {
			res = ERR_INTERRUPT;
			goto END;
		}
	}

END:
	return res;
}
err_t i2c_transmit_block_continue(const uint8_t *tx_buffer, txsize_t tx_size, utime_t timeout) {
	assert(BUFFER_OK(tx));
	assert(TWIx_INIT_OK(TWIx));

#if ! uHAL_SKIP_INIT_CHECKS
	if (!TWIx_INIT_OK(TWIx)) {
		return ERR_INIT;
	}
#endif
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (!BUFFER_OK(tx)) {
		return ERR_BADARG;
	}
#endif

	timeout = SET_TIMEOUT_MS(timeout);

	return _i2c_transmit_block_continue(tx_buffer, tx_size, timeout);
}

// i2c_transmit_block_end() is called by _i2c_transmit_block_end() instead
// of the other way around because that's 4 bytes smaller while still allowing
// the interface to remain consistent
static err_t _i2c_transmit_block_end(void) {
	return i2c_transmit_block_end();
}
err_t i2c_transmit_block_end(void) {
	// Stop the transmission
	if (SELECT_BITS(TWIx.MSTATUS, TWI_BUSSTATE_gm) == TWI_BUSSTATE_OWNER_gc) {
		// We *could* wait around here for the stop condition to be broadcast, but
		// there's really no reason to so that's handled in i2c_off()
		TWIx.MCTRLB = TWI_ACKACT_NACK_gc | TWI_MCMD_STOP_gc;
	}

	return ERR_OK;
}

// We can save program space just by using an alternative function that uses
// a new timout for each call instead of a shared timeout, but that's probably
// not what's normally wanted so only use it when desparate
#if uHAL_USE_SMALL_CODE < 2
err_t i2c_transmit_block(uint8_t addr, const uint8_t *tx_buffer, txsize_t tx_size, utime_t timeout) {
	err_t res = ERR_OK;

	assert(ADDRESS_OK(addr));
	assert(BUFFER_OK(tx));
	assert(TWIx_INIT_OK(TWIx));

#if ! uHAL_SKIP_INIT_CHECKS
	if (!TWIx_INIT_OK(TWIx)) {
		return ERR_INIT;
	}
#endif
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (!ADDRESS_OK(addr) || !BUFFER_OK(tx)) {
		return ERR_BADARG;
	}
#endif

	timeout = SET_TIMEOUT_MS(timeout);

	if ((res = _i2c_transmit_block_begin(addr, timeout)) != ERR_OK) {
		goto END;
	}
	res = _i2c_transmit_block_continue(tx_buffer, tx_size, timeout);

END:
	_i2c_transmit_block_end();
	return res;
}
#else // uHAL_USE_SMALL_CODE < 2
err_t i2c_transmit_block(uint8_t addr, const uint8_t *tx_buffer, txsize_t tx_size, utime_t timeout) {
	err_t res = ERR_OK;

	if ((res = i2c_transmit_block_begin(addr, timeout)) != ERR_OK) {
		goto END;
	}
	res = i2c_transmit_block_continue(tx_buffer, tx_size, timeout);

END:
	i2c_transmit_block_end();
	return res;
}
#endif // uHAL_USE_SMALL_CODE < 2


#endif // uHAL_USE_I2C
