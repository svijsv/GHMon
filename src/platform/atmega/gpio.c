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

#include <avr/io.h>
#include <avr/power.h>


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
* Macros
*/
// F() doesn't deduplicate, so do this instead
static _FLASH const char u_port_fmt[] = "Unhandled GPIO port 0x%02u at " __FILE__ ":%u";
#define UNHANDLED_PORT(pin) LOGGER_NOF(FROM_FSTR(u_port_fmt), (uint )GPIO_GET_PORT(pin), (uint )__LINE__)


/*
* Interrupt handlers
*/


/*
* Functions
*/
void gpio_init(void) {
	// Default all digital and analog pins to HiZ (input without pullups)
	DDRB  = 0; DDRC  = 0; DDRD  = 0;
	PORTB = 0; PORTC = 0; PORTD = 0;

	// Disable digital buffers on analog inputs
	// The datasheet recommends doing this with analog inputs because inputs
	// hovering around Vcc/2 can cause significant current use.
	// Pins ADC6 and ADC7 do not have digital input buffers.
	power_adc_enable();
	DIDR0 = 0x3F;
	// AIN0 and AIN1 (the analog comparator) are Arduino pins 6 and 7.
	DIDR1 = 0x03;
	power_adc_disable();

	return;
}

void gpio_set_state(pin_t pin, gpio_state_t new_state) {
	uint8_t pinmask;

	assert(pin != 0);

	pinmask = GPIO_GET_PINMASK(pin);
	switch (new_state) {
	case GPIO_HIGH:
		switch (GPIO_GET_PORT(pin)) {
		case GPIO_PORTB:
			SET_BIT(PORTB, pinmask);
			break;
		case GPIO_PORTC:
			SET_BIT(PORTC, pinmask);
			break;
		case GPIO_PORTD:
			SET_BIT(PORTD, pinmask);
			break;
		default:
			UNHANDLED_PORT(pin);
			break;
		}
		break;
	case GPIO_LOW:
		switch (GPIO_GET_PORT(pin)) {
		case GPIO_PORTB:
			CLEAR_BIT(PORTB, pinmask);
			break;
		case GPIO_PORTC:
			CLEAR_BIT(PORTC, pinmask);
			break;
		case GPIO_PORTD:
			CLEAR_BIT(PORTD, pinmask);
			break;
		default:
			UNHANDLED_PORT(pin);
			break;
		}
		break;
	case GPIO_FLOAT:
		switch (GPIO_GET_PORT(pin)) {
		case GPIO_PORTB:
			CLEAR_BIT(PORTB, pinmask & (PORTB ^ pinmask));
			break;
		case GPIO_PORTC:
			CLEAR_BIT(PORTC, pinmask & (PORTC ^ pinmask));
			break;
		case GPIO_PORTD:
			CLEAR_BIT(PORTD, pinmask & (PORTD ^ pinmask));
			break;
		default:
			UNHANDLED_PORT(pin);
			break;
		}
		break;
	}

	return;
}
void gpio_toggle_state(pin_t pin) {
	assert(pin != 0);

	switch (GPIO_GET_PORT(pin)) {
	case GPIO_PORTB:
		SET_BIT(PINB, GPIO_GET_PINMASK(pin));
		break;
	case GPIO_PORTC:
		SET_BIT(PINC, GPIO_GET_PINMASK(pin));
		break;
	case GPIO_PORTD:
		SET_BIT(PIND, GPIO_GET_PINMASK(pin));
		break;
	default:
		UNHANDLED_PORT(pin);
		break;
	}

	return;
}
gpio_state_t gpio_get_state(pin_t pin) {
	uint8_t st = 0;

	assert(pin != 0);

	switch (GPIO_GET_PORT(pin)) {
	case GPIO_PORTB:
		st = SELECT_BITS(PINB, GPIO_GET_PINMASK(pin));
		break;
	case GPIO_PORTC:
		st = SELECT_BITS(PINC, GPIO_GET_PINMASK(pin));
		break;
	case GPIO_PORTD:
		st = SELECT_BITS(PIND, GPIO_GET_PINMASK(pin));
		break;
	default:
		UNHANDLED_PORT(pin);
		break;
	}

	return (st != 0) ? GPIO_HIGH : GPIO_LOW;
}

void gpio_set_mode(pin_t pin, gpio_mode_t mode, gpio_state_t istate) {
	uint8_t pinmask;

	assert(pin != 0);

	pinmask = GPIO_GET_PINMASK(pin);

	if ((mode != GPIO_MODE_AIN) && (mode != GPIO_MODE_HiZ) && (mode != GPIO_MODE_RESET)) {
		uint8_t DDR_mask, PORT_mask;

		DDR_mask = (mode == GPIO_MODE_IN) ? 0 : pinmask;
		PORT_mask = (istate == GPIO_HIGH) ? pinmask : 0;

		// When switching between input and output, it's not possible to set
		// both the direction and the state at the same time. Setting the state
		// first and then the direction is least likely to cause problems; see
		// section 14.2.3 in the manual.
		switch (GPIO_GET_PORT(pin)) {
		case GPIO_PORTB:
			MODIFY_BITS(PORTB, pinmask, PORT_mask);
			MODIFY_BITS(DDRB, pinmask, DDR_mask);
			break;

		case GPIO_PORTC:
			// Port C pins 0-5 are the ADC pins, their digital input buffers are
			// normally disabled
			// The input buffers are needed to read the PINx registers, which
			// are used to determine the pin state in output modes too
			if (GPIO_GET_PINNO(pin) <= 5) {
				CLEAR_BIT(DIDR0, pinmask);
			}
			MODIFY_BITS(PORTC, pinmask, PORT_mask);
			MODIFY_BITS(DDRC, pinmask, DDR_mask);
			break;

		case GPIO_PORTD:
			// Port D pins 6 and 7 are the AIN pins, their digital buffers are
			// normally disabled
			switch (GPIO_GET_PINNO(pin)) {
			case 6:
				CLEAR_BIT(DIDR1, 0x01);
				break;
			case 7:
				CLEAR_BIT(DIDR1, 0x02);
				break;
			}
			MODIFY_BITS(PORTD, pinmask, PORT_mask);
			MODIFY_BITS(DDRD, pinmask, DDR_mask);
			break;

		default:
			UNHANDLED_PORT(pin);
			break;
		}

	} else { // GPIO_MODE_AIN, GPIO_MODE_HiZ, or GPIO_MODE_RESET
		switch (GPIO_GET_PORT(pin)) {
		case GPIO_PORTB:
			CLEAR_BIT(DDRB, pinmask);
			CLEAR_BIT(PORTB, pinmask);
			break;
		case GPIO_PORTC:
			CLEAR_BIT(DDRC, pinmask);
			CLEAR_BIT(PORTC, pinmask);
			if (GPIO_GET_PINNO(pin) <= 5) {
				SET_BIT(DIDR0, pinmask);
			}
			break;
		case GPIO_PORTD:
			CLEAR_BIT(DDRD, pinmask);
			CLEAR_BIT(PORTD, pinmask);
			switch (GPIO_GET_PINNO(pin)) {
			case 6:
				SET_BIT(DIDR1, 0x01);
				break;
			case 7:
				SET_BIT(DIDR1, 0x02);
				break;
			}
			break;
		case GPIO_PORTZ:
			//Nothing to do here, they don't have digital capabilities
			break;
		default:
			UNHANDLED_PORT(pin);
			break;
		}
	}

	return;
}
gpio_mode_t gpio_get_mode(pin_t pin) {
	uint8_t pinmask;

	assert(pin != 0);

	pinmask = GPIO_GET_PINMASK(pin);
	// TODO: This should check for AF modes
	switch (GPIO_GET_PORT(pin)) {
	case GPIO_PORTB:
		return BIT_IS_SET(DDRB, pinmask) ? GPIO_MODE_PP : GPIO_MODE_IN;
		break;

	case GPIO_PORTC:
		return BIT_IS_SET(DDRC, pinmask) ? GPIO_MODE_PP : BIT_IS_SET(DIDR0, pinmask) ? GPIO_MODE_AIN : GPIO_MODE_IN;
		break;

	case GPIO_PORTD:
		switch (GPIO_GET_PINNO(pin)) {
		case 6:
			return BIT_IS_SET(DDRD, pinmask) ? GPIO_MODE_PP : BIT_IS_SET(DIDR1, 0x01) ? GPIO_MODE_HiZ : GPIO_MODE_IN;
			break;
		case 7:
			return BIT_IS_SET(DDRD, pinmask) ? GPIO_MODE_PP : BIT_IS_SET(DIDR1, 0x02) ? GPIO_MODE_HiZ : GPIO_MODE_IN;
			break;
		default:
			return BIT_IS_SET(DDRD, pinmask) ? GPIO_MODE_PP : GPIO_MODE_IN;
			break;
		}
		break;

	case GPIO_PORTZ:
		return GPIO_MODE_AIN;
		break;

	default:
		UNHANDLED_PORT(pin);
		break;
	}

	// Shouldn't actually reach this point, all the possibilities are handled
	return GPIO_MODE_RESET;
}


#ifdef __cplusplus
 }
#endif
