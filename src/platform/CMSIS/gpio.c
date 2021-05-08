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


/*
* Static values
*/
// MODE bits for setting pin direction in GPIOx_CR[LH]
#define OUTPUT_SLOW   0b10 // Slow speed,   2MHZ
#define OUTPUT_MEDIUM 0b01 // Medium speed, 10MHZ
#define OUTPUT_HIGH   0b11 // High speed,   50MHZ
#define OUTPUT OUTPUT_SLOW
#define INPUT  0x00


/*
* Types
*/


/*
* Variables
*/


/*
* Local function prototypes
*/
static bool is_output(pin_t pin);


/*
* Interrupt handlers
*/


/*
* Functions
*/
void gpio_init(void) {
	// Enable GPIO port clocks
	// Only enable GPIO ports A and B since none of the rest are broken out
	clock_init(&RCC->APB2ENR, &RCC->APB2RSTR, RCC_APB2ENR_IOPAEN|RCC_APB2ENR_IOPBEN);

	// Set all pins to analog mode, which should be the lowest-power-drawing
	// mode
	// Analog mode corresponds to MODE == 0b00 and CNF == 0b00
	// Set the output low first just in case
	// Port A
	GPIOA->ODR = 0;
	GPIOA->CRL = 0;
	GPIOA->CRH = 0;
	// Port B
	GPIOB->ODR = 0;
	GPIOB->CRL = 0;
	GPIOB->CRH = 0;

	return;
}

void gpio_set_state(pin_t pin, gpio_state_t new_state) {
	GPIO_TypeDef* port;
	uint32_t pinmask;

	assert(pin != 0);

	port = GPIO_GET_PORT(pin);
	pinmask = GPIO_GET_PINMASK(pin);

	switch (new_state) {
	case GPIO_HIGH:
		port->BSRR = pinmask;
		break;
	case GPIO_LOW:
		port->BRR = pinmask;
		break;
	case GPIO_FLOAT:
		break;
	}

	return;
}
void gpio_toggle_state(pin_t pin) {
	GPIO_TypeDef* port;
	uint32_t pinmask, dr;

	assert(pin != 0);

	port = GPIO_GET_PORT(pin);
	pinmask = GPIO_GET_PINMASK(pin);
	dr = (is_output(pin)) ? port->ODR : port->IDR;

	port->BSRR = ((pinmask & dr) == 0) ? pinmask : pinmask << 16;

	return;
}

gpio_state_t gpio_get_state(pin_t pin) {
	GPIO_TypeDef* port;
	uint32_t pinmask;

	assert(pin != 0);

	port = GPIO_GET_PORT(pin);
	pinmask = GPIO_GET_PINMASK(pin);

	if (is_output(pin)) {
		switch (BIT_IS_SET(port->ODR, pinmask)) {
		case 0:
			return GPIO_LOW;
			break;
		default:
			return GPIO_HIGH;
			break;
		}
	} else {
		switch (BIT_IS_SET(port->IDR, pinmask)) {
		case 0:
			return GPIO_LOW;
			break;
		default:
			return GPIO_HIGH;
			break;
		}
	}

	// Won't actually reach down here
	return GPIO_FLOAT;
}

void gpio_quickread_prepare(volatile gpio_quick_t *qpin, pin_t pin) {
	assert(qpin != NULL);
	assert(pin != 0);

	qpin->mask = GPIO_GET_PINMASK(pin);
	qpin->idr  = &(GPIO_GET_PORT(pin)->IDR);

	return;
}

void gpio_set_mode(pin_t pin, gpio_mode_t mode, gpio_state_t istate) {
	uint32_t mask;
	GPIO_TypeDef* port;
	uint32_t pinmask, pinno, mpinno;

	assert(pin != 0);

	port = GPIO_GET_PORT(pin);
	pinmask = GPIO_GET_PINMASK(pin);
	pinno = GPIO_GET_PINNO(pin);

	// Determine the CNF and MODE bits for the pin
	switch (mode) {
	case GPIO_MODE_RESET:
		mask = 0x04; // Reference-defined reset state
		break;
	case GPIO_MODE_PP:
		mask = 0b0000|OUTPUT;
		break;
	case GPIO_MODE_PP_AF:
		mask = 0b1000|OUTPUT;
		break;
	case GPIO_MODE_OD:
		mask = 0b0100|OUTPUT;
		break;
	case GPIO_MODE_OD_AF:
		mask = 0b1100|OUTPUT;
		break;
	case GPIO_MODE_IN:
		switch (istate) {
		case GPIO_HIGH:
		case GPIO_LOW:
			mask = 0b1000|INPUT;
			break;
		default:
			mask = 0b0100|INPUT;
			break;
		}
		break;
	case GPIO_MODE_AIN:
	case GPIO_MODE_HiZ:
	default:
		mask = 0b0000|INPUT;
		break;
	}

	// I don't know why they designed it this way...would have made more sense
	// to use one register for modes and the other for configuration
	if (pinno < 8) {
		mpinno = pinno * 4;
		MODIFY_BITS(port->CRL, (0b1111 << mpinno),
			(mask << mpinno)
			);
	} else {
		mpinno = (pinno - 8) * 4;
		MODIFY_BITS(port->CRH, (0b1111 << mpinno),
			(mask << mpinno)
			);
	}

	// For normal outputs, set the initial pin state
	// For inputs with a bias, set the pull direction
	// For analog and floating inputs this does nothing
	// Ignore AF outputs, let the controlling peripherals handle them
	switch (mode) {
	case GPIO_MODE_RESET:
	case GPIO_MODE_AIN:
	case GPIO_MODE_HiZ:
		port->BRR = pinmask;
		break;
	case GPIO_MODE_PP_AF:
	case GPIO_MODE_OD_AF:
		break;
	default:
		switch (istate) {
		case GPIO_HIGH:
			port->BSRR = pinmask;
			break;
		case GPIO_LOW:
			port->BRR = pinmask;
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
	GPIO_TypeDef* port;
	uint32_t pinno, mpinno;

	assert(pin != 0);

	port = GPIO_GET_PORT(pin);
	pinno = GPIO_GET_PINNO(pin);

	if (pinno < 8) {
		mpinno = pinno * 4;
		mask = GATHER_BITS(port->CRL, 0b1111, mpinno);
	} else {
		mpinno = (pinno - 8) * 4;
		mask = GATHER_BITS(port->CRH, 0b1111, mpinno);
	}

	switch (mask) {
	case (0b0000|OUTPUT):
		return GPIO_MODE_PP;
		break;
	case (0b1000|OUTPUT):
		return GPIO_MODE_PP_AF;
		break;
	case (0b0100|OUTPUT):
		return GPIO_MODE_OD;
		break;
	case (0b1100|OUTPUT):
		return GPIO_MODE_OD_AF;
		break;
	case (0b1000|INPUT):
		return GPIO_MODE_IN;
		break;
	case (0b0100|INPUT):
		return GPIO_MODE_IN;
		break;
	case (0b0000|INPUT):
		return GPIO_MODE_AIN;
		break;
	}

	// Shouldn't actually reach this point, all the possibilities are handled
	// except other output speeds, which aren't used anywhere
	return GPIO_MODE_HiZ;
}

static bool is_output(pin_t pin) {
	uint32_t selected;
	GPIO_TypeDef* port;
	uint32_t pinno, mpinno;

	assert(pin != 0);

	port = GPIO_GET_PORT(pin);
	pinno = GPIO_GET_PINNO(pin);

	if (pinno < 8) {
		mpinno = pinno * 4;
		selected = SELECT_BITS(port->CRL, 0x03 << mpinno);
	} else {
		mpinno = (pinno - 8) * 4;
		selected = SELECT_BITS(port->CRH, 0x03 << mpinno);
	}

	return (selected != 0);
}


#ifdef __cplusplus
 }
#endif
