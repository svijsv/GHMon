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

#include <avr/io.h>
#include <avr/power.h>


#if USE_I2C

#if (PINID(I2C_SCL_PIN) != PINID_C5) || (PINID(I2C_SDA_PIN) != PINID_C4)
# error "Incorrect pin(s) set for I2C"
#endif

// To simplify things the I2C prescaler is always '1', which limits the core
// clock:I2C clock ratio
#if (((G_freq_CPUCLK/(I2C_SPEED*2)) - 8) < 0) || (((G_freq_CPUCLK/(I2C_SPEED*2)) - 8) > 0xFF)
# error "This combination of core clock frequency and I2C clock frequency won't work"
#endif

/*
* Static values
* (And macros, I guess)
*/
// The bits written in TWCR when setting up a transfer
#define XFER_MASK  (_BV(TWINT)|_BV(TWSTA)|_BV(TWSTO)|_BV(TWEA))
#define XFER_START() MODIFY_BITS(TWCR, XFER_MASK, _BV(TWINT)|_BV(TWSTA)|_BV(TWEA))
#define XFER_STOP()  MODIFY_BITS(TWCR, XFER_MASK, _BV(TWINT)|_BV(TWSTO)|_BV(TWEA))
#define XFER_NEXT()  MODIFY_BITS(TWCR, XFER_MASK, _BV(TWINT)|_BV(TWEA))
#define XFER_FINAL() MODIFY_BITS(TWCR, XFER_MASK, _BV(TWINT))
#define XFER_RESET() (TWCR = (_BV(TWINT)|_BV(TWEN)))

#define XFER_STATUS() (TWSR & 0xF8)


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
// The I2C peripheral needs to be re-configured if it's powered off so
// initialization is just a stub; everything happens in i2c_on()
void i2c_init(void) {
	i2c_off();
	return;
}
void i2c_on(void) {
	power_twi_enable();

	//
	// Set the bit rate generator division factor
	// From section 22.5.2 in the reference manual:
	//    freq = cpu_clk / (16 + (2*twbr) * prescaler)
	// Assuming a prescaler of 1 to simplify things:
	//    1/freq = (16 + (2*twbr) * 1) / cpu_clk
	//    cpu_clk/freq = (16 + 2*twbr)
	//    2*twbr = cpu_clk/freq - 16
	//    twbr = cpu_clk/(freq*2) - 8
	TWBR = ((G_freq_CPUCLK/(I2C_SPEED*2)) - 8);
	//
	// Set the bit rate prescaler
	MODIFY_BITS(TWSR, (0b11 << TWPS0),
		(0b00 << TWPS0) // Prescaler of '1'
		);
	//
	// Enable the I2C peripheral
	SET_BIT(TWCR, _BV(TWEN));
	// The peripheral controls the pins, we don't need to set anything
	//gpio_set_mode(I2C_SCL_PIN, GPIO_MODE_OD, GPIO_HIGH);
	//gpio_set_mode(I2C_SDA_PIN, GPIO_MODE_OD, GPIO_HIGH);

	return;
}
void i2c_off(void) {
	//gpio_set_mode(I2C_SCL_PIN, GPIO_MODE_HiZ, GPIO_FLOAT);
	//gpio_set_mode(I2C_SDA_PIN, GPIO_MODE_HiZ, GPIO_FLOAT);

	CLEAR_BIT(TWCR, _BV(TWEN));
	power_twi_disable();

	return;
}

err_t i2c_receive_block(uint8_t addr, uint8_t *rx_buffer, txsize_t rx_size, utime_t timeout) {
	err_t res;
	txsize_t i;

	assert(rx_buffer != NULL);
	assert(rx_size > 0);

	res = EOK;
	timeout = SET_TIMEOUT(timeout);

	//
	// Send start condition
	XFER_START();

	while (!BIT_IS_SET(TWCR, _BV(TWINT))) {
		if (TIMES_UP(timeout)) {
			res = ETIMEOUT;
			goto END;
		}
	}
	//
	// Send slave address
	// Clearing TWINT (by writing '1' to it) will start the next operation of
	// the TWI so any accesses to TWAR, TWSR, and TWDR must be completed
	// beforehand
	// LSB of address is 1 for RX
	TWDR = (addr << 1) | 0x01;
	// Initiate address transmission
	XFER_NEXT();
	while (!BIT_IS_SET(TWCR, _BV(TWINT))) {
		if (TIMES_UP(timeout)) {
			res = ETIMEOUT;
			goto END;
		}
	}
	//
	// Receive the data packets
	// The last data byte is handled separately than the rest because it needs
	// to return a NACK
	--rx_size;
	for (i = 0; i < rx_size; ++i) {
		// Tell the slave to send a byte
		XFER_NEXT();
		while (!BIT_IS_SET(TWCR, _BV(TWINT))) {
			if (TIMES_UP(timeout)) {
				res = ETIMEOUT;
				goto END;
			}
		}
		rx_buffer[i] = TWDR;
	}
	// Tell the slave to send a byte, but don't ACK it
	XFER_FINAL();
	while (!BIT_IS_SET(TWCR, _BV(TWINT))) {
		if (TIMES_UP(timeout)) {
			res = ETIMEOUT;
			goto END;
		}
	}
	// Read the last byte
	rx_buffer[i] = TWDR;

END:
	// Stop the transmission
	if (!BIT_IS_SET(TWCR, _BV(TWSTA))) {
		XFER_STOP();
		timeout = SET_TIMEOUT(100);
		while (BIT_IS_SET(TWCR, _BV(TWSTO))) {
			if (TIMES_UP(timeout)) {
				res = ETIMEOUT;
				break;
			}
		}
	}
	XFER_RESET();

	return res;
}
err_t i2c_transmit_block(uint8_t addr, const uint8_t *tx_buffer, txsize_t tx_size, utime_t timeout) {
	err_t res;
	txsize_t i;

	assert(tx_buffer != NULL);
	assert(tx_size > 0);

	res = EOK;
	timeout = SET_TIMEOUT(timeout);

	//
	// Send start condition
	XFER_START();
	while (!BIT_IS_SET(TWCR, _BV(TWINT))) {
		if (TIMES_UP(timeout)) {
			res = ETIMEOUT;
			goto END;
		}
	}
	//
	// Send slave address
	// Clearing TWINT (by writing '1' to it) will start the next operation of
	// the TWI so any accesses to TWAR, TWSR, and TWDR must be completed
	// beforehand
	// LSB of address is 0 for TX
	TWDR = (addr << 1) | 0x00;
	// Initiate address transmission
	XFER_NEXT();
	while (!BIT_IS_SET(TWCR, _BV(TWINT))) {
		if (TIMES_UP(timeout)) {
			res = ETIMEOUT;
			goto END;
		}
	}
	//
	// Send the data packets
	for (i = 0; i < tx_size; ++i) {
		TWDR = tx_buffer[i];
		// Send the next byte to the slave
		XFER_NEXT();
		while (!BIT_IS_SET(TWCR, _BV(TWINT))) {
			if (TIMES_UP(timeout)) {
				res = ETIMEOUT;
				goto END;
			}
		}
	}

END:
	// Stop the transmission
	if (!BIT_IS_SET(TWCR, _BV(TWSTA))) {
		XFER_STOP();
		timeout = SET_TIMEOUT(100);
		while (BIT_IS_SET(TWCR, _BV(TWSTO))) {
			if (TIMES_UP(timeout)) {
				res = ETIMEOUT;
				break;
			}
		}
	}
	XFER_RESET();

	return res;
}


#endif // USE_I2C

#ifdef __cplusplus
 }
#endif
