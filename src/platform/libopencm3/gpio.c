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
// gpio.c
// Manage the GPIO peripheral
// NOTES:
//

#ifdef __cplusplus
 extern "C" {
#endif

//#define DEBUG 1
/*
* Includes
*/
#include "gpio.h"
#include "system.h"

// This is the only file which should need to access the library's
// gpio_set_mode(); it should be undef'd after all local headers but before
// libopencm3/stm32/gpio.h
#undef gpio_set_mode

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>


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
void gpio_init(void) {
	// Enable GPIO port clocks
	// Only enable GPIO ports A and B since none of the rest are broken out
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_reset_pulse(RST_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOB);
	rcc_periph_reset_pulse(RST_GPIOB);

	// Set all pins to analog mode, which should be the lowest-power-drawing
	// mode
	// Port A
	gpio_set_mode(GPIOA, GPIO_MODE_INPUT, GPIO_CNF_INPUT_ANALOG, GPIO_ALL);
	gpio_set_mode(GPIOB, GPIO_MODE_INPUT, GPIO_CNF_INPUT_ANALOG, GPIO_ALL);

	return;
}

void gpio_set_state(pin_t pin, gpio_state_t new_state) {
	uint32_t port;
	uint32_t pinmask;

	assert(pin != 0);

	port = GPIO_GET_PORT(pin);
	pinmask = GPIO_GET_PINMASK(pin);

	switch (new_state) {
	case GPIO_HIGH:
		gpio_set(port, pinmask);
		break;
	case GPIO_LOW:
		gpio_clear(port, pinmask);
		break;
	default:
		break;
	}

	return;
}
void gpio_toggle_state(pin_t pin) {
	uint32_t port;
	uint32_t pinmask;

	assert(pin != 0);

	port = GPIO_GET_PORT(pin);
	pinmask = GPIO_GET_PINMASK(pin);

	gpio_toggle(port, pinmask);

	return;
}

gpio_state_t gpio_get_state(pin_t pin) {
	uint32_t port;
	uint32_t pinmask;

	assert(pin != 0);

	port = GPIO_GET_PORT(pin);
	pinmask = GPIO_GET_PINMASK(pin);

	switch (gpio_get(port, pinmask)) {
	case 0:
		return GPIO_LOW;
		break;
	default:
		return GPIO_HIGH;
		break;
	}

	// Won't actually reach down here
	return GPIO_FLOAT;
}

void gpio_quickread_prepare(volatile gpio_quick_t *qpin, pin_t pin) {
	assert(qpin != NULL);
	assert(pin != 0);

	qpin->mask = GPIO_GET_PINMASK(pin);
	qpin->idr = &(GPIO_IDR(GPIO_GET_PORT(pin)));

	return;
}

void gpio_set_mode_OVERRIDE(pin_t pin, gpio_mode_t mode, gpio_state_t istate) {
	uint32_t lmode, cnf;
	uint32_t port;
	uint32_t pinmask;

	assert(pin != 0);

	port = GPIO_GET_PORT(pin);
	pinmask = GPIO_GET_PINMASK(pin);

	switch (mode) {
	case GPIO_MODE_RESET:
		cnf   = GPIO_CNF_INPUT_FLOAT;
		lmode = GPIO_MODE_INPUT;
		break;
	case GPIO_MODE_PP:
		cnf   = GPIO_CNF_OUTPUT_PUSHPULL;
		lmode = GPIO_MODE_OUTPUT_2_MHZ;
		break;
	case GPIO_MODE_PP_AF:
		cnf   = GPIO_CNF_OUTPUT_ALTFN_PUSHPULL;
		lmode = GPIO_MODE_OUTPUT_2_MHZ;
		break;
	case GPIO_MODE_OD:
		cnf   = GPIO_CNF_OUTPUT_OPENDRAIN;
		lmode = GPIO_MODE_OUTPUT_2_MHZ;
		break;
	case GPIO_MODE_OD_AF:
		cnf   = GPIO_CNF_OUTPUT_ALTFN_OPENDRAIN;
		lmode = GPIO_MODE_OUTPUT_2_MHZ;
		break;
	case GPIO_MODE_IN:
		lmode = GPIO_MODE_INPUT;
		switch (istate) {
		case GPIO_HIGH:
		case GPIO_LOW:
			cnf   = GPIO_CNF_INPUT_PULL_UPDOWN;
			break;
		default:
			cnf   = GPIO_CNF_INPUT_FLOAT;
			break;
		}
		break;
	case GPIO_MODE_AIN:
	case GPIO_MODE_HiZ:
	default:
		cnf   = GPIO_CNF_INPUT_ANALOG;
		lmode = GPIO_MODE_INPUT;
		break;
	}

	gpio_set_mode(port, lmode, cnf, pinmask);

	// For normal outputs, set the initial pin state
	// For inputs with a bias, set the pull direction
	// For analog and floating inputs this does nothing
	// Ignore AF outputs, let the controlling peripherals handle them
	switch (mode) {
	case GPIO_MODE_RESET:
	case GPIO_MODE_AIN:
	case GPIO_MODE_HiZ:
		gpio_clear(port, pinmask);
		break;
	case GPIO_MODE_PP_AF:
	case GPIO_MODE_OD_AF:
		break;
	default:
		switch (istate) {
		case GPIO_HIGH:
			gpio_set(port, pinmask);
			break;
		case GPIO_LOW:
			gpio_clear(port, pinmask);
			break;
		default:
			break;
		}
		break;
	}

	return;
}
gpio_mode_t gpio_get_mode(pin_t pin) {
	uint32_t mask;
	uint32_t port;
	uint32_t pinno, mpinno;
	gpio_mode_t rmode;

	assert(pin != 0);

	port = GPIO_GET_PORT(pin);
	pinno = GPIO_GET_PINNO(pin);

	// libopencm3 doesn't seem to have a convenient way to get the mode of a
	// pin, so do it the old-fashioned way
	if (pinno < 8) {
		mpinno = pinno * 4;
		mask = GATHER_BITS(GPIO_CRL(port), 0b1111, mpinno);
	} else {
		mpinno = (pinno - 8) * 4;
		mask = GATHER_BITS(GPIO_CRH(port), 0b1111, mpinno);
	}

	rmode = GPIO_MODE_RESET;
	switch (mask & 0x0F) {
	case GPIO_MODE_INPUT:
		switch (mask >> 2) {
		case GPIO_CNF_INPUT_ANALOG:
			rmode = GPIO_MODE_AIN;
			break;
		case GPIO_CNF_INPUT_FLOAT:
		case GPIO_CNF_INPUT_PULL_UPDOWN:
			rmode = GPIO_MODE_IN;
			break;
		}
		break;

	case GPIO_MODE_OUTPUT_10_MHZ:
	case GPIO_MODE_OUTPUT_2_MHZ:
	case GPIO_MODE_OUTPUT_50_MHZ:
		switch (mask >> 2) {
		case GPIO_CNF_OUTPUT_PUSHPULL:
			rmode = GPIO_MODE_PP;
			break;
		case GPIO_CNF_OUTPUT_OPENDRAIN:
			rmode = GPIO_MODE_OD;
			break;
		case GPIO_CNF_OUTPUT_ALTFN_PUSHPULL:
			rmode = GPIO_MODE_PP_AF;
			break;
		case GPIO_CNF_OUTPUT_ALTFN_OPENDRAIN:
			rmode = GPIO_MODE_OD_AF;
			break;
		}
		break;
	}

	return rmode;
}


#ifdef __cplusplus
 }
#endif
