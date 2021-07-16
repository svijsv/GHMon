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
// gpio_STM32Fx.h
// Manage the GPIO peripheral
// NOTES:
//    This was split off from gpio.c because the GPIO peripherals differ so
//    much between the STM32F1 and the other lines that code can't really be
//    reused
//
//    This file should only be included by gpio.c
//
//    The STM32F401 manual says reset for port A MODER is 0x0C00 0000 but
//    the other manuals I've checked all give it as 0xA800 0000 which
//    makes more sense since that would make the JTAG pins AF mode rather
//    than leaving 2 of them as GP and one as analog
//
#if INCLUDED_BY_GPIO_C


/*
* Includes
*/


/*
* Static values
*/
#define OUTPUT_SLOW      0b00 // Slow speed
#define OUTPUT_MEDIUM    0b01 // Medium speed
#define OUTPUT_FAST      0b10 // High speed
#define OUTPUT_VERY_FAST 0b11 // Very High speed
//#define OUTPUT OUTPUT_SLOW
// The value of a whole register set to OUTPUT:
#if OUTPUT == OUTPUT_SLOW
# define OUTPUT_REGISTER 0
#elif OUTPUT == OUTPUT_MEDIUM
# define OUTPUT_REGISTER 0x55555555
#elif OUTPUT == OUTPUT_FAST
# define OUTPUT_REGISTER 0xAAAAAAAA
#elif OUTPUT == OUTPUT_VERY_FAST
# define OUTPUT_REGISTER 0xFFFFFFFF
#endif

#define MODE_INPUT  0b00
#define MODE_OUTPUT 0b01
#define MODE_AF     0b10
#define MODE_ANALOG 0b11

#define OUTPUT_PP 0b0
#define OUTPUT_OD 0b1

#define NO_PULL   0b00
#define PULL_UP   0b01
#define PULL_DOWN 0b10

/*
* Types
*/


/*
* Variables
*/


/*
* Macros
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
static void gpio_platform_init(void) {
	return;
}
static void port_reset(GPIO_TypeDef *port) {
# if DEBUG
	// Pins A13 and A14 are left alone because those are the SWD pins and
	// the rest are set to analog mode with pullups/downs disabled
	if (port == GPIOA) {
		uint32_t mask;

		mask = ~(
			(0b11 << (GPIO_GET_PINNO(PIN_SWDIO) * 2)) |
			(0b11 << (GPIO_GET_PINNO(PIN_SWCLK) * 2)) |
			0);
		SET_BIT(  port->MODER, mask);
		CLEAR_BIT(port->PUPDR, mask);

		mask = ~(GPIO_GET_PINMASK(PIN_SWDIO) | GPIO_GET_PINMASK(PIN_SWCLK));
		MODIFY_BITS(port->ODR, mask, OUTPUT_REGISTER);
	} else
# endif
	{
		port->MODER = 0xFFFFFFFF;
		port->PUPDR = 0;
		port->ODR   = OUTPUT_REGISTER;
	}

	return;
}
void gpio_set_AF(pin_t pin, gpio_af_t af) {
	GPIO_TypeDef *port;
	__IO uint32_t *reg;
	uint8_t pos;
	uint32_t mask, naf;

	assert(pin != 0);

	// Some AF mappings don't play nice
	if ((af == AF_I2C3) && (PINID(pin) == PIN_I2C3_SDA)) {
		af = AF_I2C3_ALT;
	} else if ((af == AF_I2C2) && (PINID(pin) == PIN_I2C2_SDA_REMAP)) {
		af = AF_I2C2_ALT;
	}

	port = GPIO_GET_PORT(pin);
	pos = GPIO_GET_PINNO(pin);

	if (pos < 8) {
		reg = &port->AFR[0];
	} else {
		reg = &port->AFR[1];
		pos -= 8;
	}
	pos *= 4;
	mask = (uint32_t )0b1111 << pos;
	naf = (uint32_t )af << pos;
	MODIFY_BITS(*reg, mask, naf);

	return;
}

void gpio_set_mode(pin_t pin, gpio_mode_t mode, gpio_state_t istate) {
	GPIO_TypeDef* port;
	uint8_t pinno, pos2;
	uint32_t pinmask, mask2, bsrr = 0;
	uint32_t cfg, otype = 0, pull;

	assert(pin != 0);

	port = GPIO_GET_PORT(pin);
	pinmask = GPIO_GET_PINMASK(pin);
	pinno = GPIO_GET_PINNO(pin);

	pos2 = pinno * 2;
	mask2 = (uint32_t )0b11 << pos2;

	// For normal outputs, set the initial pin state
	// For inputs and alternate function pins, set the pull direction
	// For analog and floating inputs this does nothing
	switch (mode) {
	case GPIO_MODE_PP:
	case GPIO_MODE_OD:
		pull = NO_PULL;
		switch (istate) {
		case GPIO_HIGH:
			bsrr = pinmask;
			break;
		case GPIO_FLOAT: // Shut the compiler up
		case GPIO_LOW:
			bsrr = pinmask << GPIO_BSRR_BR0_Pos;
			break;
		}
		break;
	case GPIO_MODE_PP_AF:
	case GPIO_MODE_OD_AF:
	case GPIO_MODE_IN_AF:
	case GPIO_MODE_IN:
		switch (istate) {
		case GPIO_HIGH:
			pull = PULL_UP;
			break;
		case GPIO_LOW:
			pull = PULL_DOWN;
			break;
		default:
			pull = NO_PULL;
			break;
		}
		break;
	case GPIO_MODE_RESET:
	case GPIO_MODE_AIN:
	case GPIO_MODE_HiZ:
	default:
		pull = NO_PULL;
		break;
	}

	switch (mode) {
	case GPIO_MODE_PP:
		cfg = MODE_OUTPUT;
		otype = OUTPUT_PP;
		break;
	case GPIO_MODE_IN_AF:
	case GPIO_MODE_PP_AF:
		cfg = MODE_AF;
		otype = OUTPUT_PP;
		break;
	case GPIO_MODE_OD:
		cfg = MODE_OUTPUT;
		otype = OUTPUT_OD;
		break;
	case GPIO_MODE_OD_AF:
		cfg = MODE_AF;
		otype = OUTPUT_OD;
		break;
	case GPIO_MODE_IN:
		cfg = MODE_INPUT;
		break;
	case GPIO_MODE_RESET:
	case GPIO_MODE_AIN:
	case GPIO_MODE_HiZ:
	default:
		cfg = MODE_ANALOG;
		break;
	}
	port->BSRR = bsrr;
	pull = pull << pos2;
	MODIFY_BITS(port->PUPDR, mask2, pull);
	otype = otype << pinno;
	MODIFY_BITS(port->OTYPER, pinmask, otype);
	// Per the data sheet, MODER should be set after configuration (at least
	// for alternate functions)
	cfg = cfg << pos2;
	MODIFY_BITS(port->MODER,  mask2, cfg);

	return;
}
gpio_mode_t gpio_get_mode(pin_t pin) {
	GPIO_TypeDef* port;
	uint32_t pinno;
	uint8_t mode, otype, pos;

	assert(pin != 0);

	port = GPIO_GET_PORT(pin);
	pinno = GPIO_GET_PINNO(pin);

	pos = pinno * 2;
	mode  = GATHER_BITS(port->MODER,  0b11, pos);
	otype = GATHER_BITS(port->OTYPER, 0b1,  pinno);
LOGGER("Mode 0x%02X for pin 0x%02X", (uint )mode, (uint )pin);
	switch (mode) {
	case MODE_OUTPUT:
		switch(otype) {
		case OUTPUT_PP:
			return GPIO_MODE_PP;
			break;
		default:
			return GPIO_MODE_OD;
			break;
		}
		break;
	case MODE_AF:
		switch(otype) {
		case OUTPUT_PP:
			return GPIO_MODE_PP_AF;
			break;
		default:
			return GPIO_MODE_OD_AF;
			break;
		}
		break;
	case MODE_INPUT:
		return GPIO_MODE_IN;
		break;
	default:
		return GPIO_MODE_AIN;
		break;
	}
}

#endif // INCLUDED_BY_GPIO_C
