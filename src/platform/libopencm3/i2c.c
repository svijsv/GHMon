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

#include <libopencm3/stm32/i2c.h>
#include <libopencm3/stm32/rcc.h>


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
	uint16_t pclk_MHz;

	pclk_MHz = I2Cx_BUSFREQ/1000000;

	// Start the clock and reset the peripheral
	rcc_periph_clock_enable(I2Cx_RCC);
	rcc_periph_reset_pulse(I2Cx_RST);

	i2c_set_standard_mode(I2Cx);
	//
	// Set the frequency (in MHz) of the RCC bus the I2C peripheral is on so
	// that clock timing can be generated accurately
	i2c_set_clock_frequency(I2Cx, pclk_MHz);
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
	i2c_set_ccr(I2Cx, I2Cx_BUSFREQ/(I2C_FREQUENCY*2));
	//
	// Configure the I2C maximum rise time
	// The reference manual gives the formula (1000ns/Tpclk)+1, which becomes
	// (Fpclk/10^6)+1 when adapted from ns to MHz
	i2c_set_trise(I2Cx, (pclk_MHz)+1);

	i2c_off();

	return;
}

void i2c_on(void) {
	// Reference manual section 9.1.11 lists pin configuration for peripherals.
	gpio_set_mode(I2C_SCL_PIN, GPIO_MODE_OD_AF, GPIO_HIGH);
	gpio_set_mode(I2C_SDA_PIN, GPIO_MODE_OD_AF, GPIO_HIGH);

	rcc_periph_clock_enable(I2Cx_RCC);
	i2c_peripheral_enable(I2Cx);

	return;
}
void i2c_off(void) {
	i2c_peripheral_disable(I2Cx);
	rcc_periph_clock_disable(I2Cx_RCC);

	gpio_set_mode(I2C_SCL_PIN, GPIO_MODE_HiZ, GPIO_LOW);
	gpio_set_mode(I2C_SDA_PIN, GPIO_MODE_HiZ, GPIO_LOW);

	return;
}

err_t i2c_receive_block(uint8_t addr, uint8_t *rx_buffer, txsize_t rx_size, utime_t timeout) {
	assert(rx_buffer != NULL);
	assert(rx_size > 0);
	UNUSED(timeout);

	i2c_transfer7(I2Cx, addr, NULL, 0, rx_buffer, rx_size);

	return EOK;
}
err_t i2c_transmit_block(uint8_t addr, const uint8_t *tx_buffer, txsize_t tx_size, utime_t timeout) {
	assert(tx_buffer != NULL);
	assert(tx_size > 0);
	UNUSED(timeout);

	i2c_transfer7(I2Cx, addr, (uint8_t *)tx_buffer, tx_size, NULL, 0);

	return EOK;
}


#endif // USE_I2C

#ifdef __cplusplus
 }
#endif
