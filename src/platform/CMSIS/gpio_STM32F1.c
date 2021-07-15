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
// gpio_STM32F1.h
// Manage the GPIO peripheral
// NOTES:
//    This was split off from gpio.c because the GPIO peripherals differ so
//    much between the STM32F1 and the other lines that code can't really be
//    reused
//
//    This file should only be included by gpio.c
//
#if INCLUDED_BY_GPIO_C


/*
* Includes
*/


/*
* Static values
*/
// MODE bits for setting pin direction in GPIOx_CR[LH]
#define OUTPUT_SLOW      0b10 // Slow speed,   2MHZ
#define OUTPUT_MEDIUM    0b01 // Medium speed, 10MHZ
#define OUTPUT_FAST      0b11 // High speed,   50MHZ
#define OUTPUT_VERY_FAST 0b11 // Same as high speed, here for compatibility
//#define OUTPUT OUTPUT_SLOW
#define INPUT  0b00

// MODE bits for setting mode
#define MODE_RESET (0x04)
#define MODE_AF    (0b1000)
#define MODE_PP    (0b0000|OUTPUT)
#define MODE_PP_AF (MODE_AF|MODE_PP)
#define MODE_OD    (0b0100|OUTPUT)
#define MODE_OD_AF (MODE_AF|MODE_OD)
#define MODE_INP   (0b1000|INPUT)
#define MODE_INF   (0b0100|INPUT)
#define MODE_AN    (0b0000|INPUT)


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
	uint32_t remaps = 0;

	// RCC_PERIPH_AFIO was enabled in platform_init() before calling gpio_init()
	// because it's needed there too
	//clock_init(RCC_PERIPH_AFIO);

	// Remap CAN to PD[01] (per the errata sheet) so it doesn't interfere with
	// USART1 (even though we don't use RTS...)
	remaps |= AFIO_MAPR_CAN_REMAP_REMAP3;
#if SPI1_DO_REMAP
	remaps |= AFIO_MAPR_SPI1_REMAP;
#endif
#if UART1_DO_REMAP
	remaps |= AFIO_MAPR_USART1_REMAP;
#endif
#if I2C1_DO_REMAP
	remaps |= AFIO_MAPR_I2C1_REMAP;
#endif
	// Unlike everything else, JTAG is enabled on reset and needs to be explicitly
	// disabled
	// Unlike everything else, reading the JTAG part of AFIO_MAPR will always
	// return '0' (fully-enabled) so it needs to be set everytime.
	// NOJNTRST would have full JTAG and SWD but no JNTRST
	// JTAGDISABLE disables JTAG while leaving SWD
	// DISABLE would disable everything
#if DEBUG
	remaps |= AFIO_MAPR_SWJ_CFG_JTAGDISABLE;
#else
	remaps |= AFIO_MAPR_SWJ_CFG_DISABLE;
#endif
	MODIFY_BITS(AFIO->MAPR, AFIO_MAPR_SWJ_CFG|AFIO_MAPR_USART1_REMAP|AFIO_MAPR_SPI1_REMAP|AFIO_MAPR_I2C1_REMAP|AFIO_MAPR_CAN_REMAP,
		remaps);

	return;
}
static void port_reset(GPIO_TypeDef *port) {
# if DEBUG
	// Pins A13 and A14 are left alone because those are the SWD pins and
	// the rest are set to analog mode with pullups/downs disabled
	if (port == GPIOA) {
		uint32_t mask;

		// We can assume the SWD pins are > 7 because they are
		mask = ~(
			(0b1111 << ((GPIO_GET_PINNO(PIN_SWDIO)-8) * 4)) |
			(0b1111 << ((GPIO_GET_PINNO(PIN_SWCLK)-8) * 4)) |
			0);
		CLEAR_BIT(port->CRH, mask);
		port->CRL = 0;

		mask = ~(GPIO_GET_PINMASK(PIN_SWDIO) | GPIO_GET_PINMASK(PIN_SWCLK));
		CLEAR_BIT(port->ODR, mask);
	} else
# endif
	{
		port->CRL = 0;
		port->CRH = 0;
		port->ODR = 0;
	}

	return;
}
void gpio_set_AF(pin_t pin, gpio_af_t af) {
	assert(pin != 0);

	UNUSED(af);

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
		mask = MODE_RESET;
		break;
	case GPIO_MODE_PP:
		mask = MODE_PP;
		break;
	case GPIO_MODE_PP_AF:
		mask = MODE_PP_AF;
		break;
	case GPIO_MODE_OD:
		mask = MODE_OD;
		break;
	case GPIO_MODE_OD_AF:
		mask = MODE_OD_AF;
		break;
	case GPIO_MODE_IN:
	case GPIO_MODE_IN_AF:
		switch (istate) {
		case GPIO_HIGH:
		case GPIO_LOW:
			mask = MODE_INP;
			break;
		default:
			mask = MODE_INF;
			break;
		}
		break;
	case GPIO_MODE_AIN:
	case GPIO_MODE_HiZ:
	default:
		mask = MODE_AN;
		break;
	}

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
	// Ignore AF outputs, the pullups/downs are disabled and the output is
	// driven by the peripheral
	switch (mode) {
	case GPIO_MODE_RESET:
	case GPIO_MODE_AIN:
	case GPIO_MODE_HiZ:
		port->BRR = pinmask;
		break;
	case GPIO_MODE_PP_AF:
	case GPIO_MODE_OD_AF:
		break;
	//case GPIO_MODE_IN_AF:
	//case GPIO_MODE_IN:
	//case GPIO_MODE_PP:
	//case GPIO_MODE_OD:
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
	case (MODE_PP):
		return GPIO_MODE_PP;
		break;
	case (MODE_PP_AF):
		return GPIO_MODE_PP_AF;
		break;
	case (MODE_OD):
		return GPIO_MODE_OD;
		break;
	case (MODE_OD_AF):
		return GPIO_MODE_OD_AF;
		break;
	case (MODE_INF):
	case (MODE_INP):
		return GPIO_MODE_IN;
	case (MODE_AN):
		return GPIO_MODE_AIN;
		break;
	}

	// Shouldn't actually reach this point, all the possibilities are handled
	// except other output speeds, which aren't used anywhere
	return GPIO_MODE_HiZ;
}

#endif // INCLUDED_BY_GPIO_C
