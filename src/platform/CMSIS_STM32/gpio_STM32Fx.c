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
#ifdef INCLUDED_BY_GPIO_C


#define OUTPUT_SLOW      0b00U // Slow speed
#define OUTPUT_MEDIUM    0b01U // Medium speed
#define OUTPUT_FAST      0b10U // High speed
#define OUTPUT_VERY_FAST 0b11U // Very High speed
//#define OUTPUT OUTPUT_SLOW
// The value of a whole register set to OUTPUT:
#if OUTPUT == OUTPUT_SLOW
# define OUTPUT_REGISTER 0U
#elif OUTPUT == OUTPUT_MEDIUM
# define OUTPUT_REGISTER 0x55555555U
#elif OUTPUT == OUTPUT_FAST
# define OUTPUT_REGISTER 0xAAAAAAAAU
#elif OUTPUT == OUTPUT_VERY_FAST
# define OUTPUT_REGISTER 0xFFFFFFFFU
#endif

#define MODE_INPUT  0b00U
#define MODE_OUTPUT 0b01U
#define MODE_AF     0b10U
#define MODE_ANALOG 0b11U

#define OUTPUT_PP 0b0U
#define OUTPUT_OD 0b1U

#define NO_PULL   0b00U
#define PULL_UP   0b01U
#define PULL_DOWN 0b10U


static void gpio_platform_init(void) {
	return;
}
static void port_reset(GPIO_TypeDef *port) {
#if DEBUG && uHAL_JTAG_DEBUG
	if (port == GPIOA) {
		uint32_t mask;

		mask = ~(
			(0b11U << (GPIO_GET_PINNO(PINID_JTMS) * 2)) |
			(0b11U << (GPIO_GET_PINNO(PINID_JTCK) * 2)) |
			(0b11U << (GPIO_GET_PINNO(PINID_JTDI) * 2)) |
			0);
		SET_BIT(  GPIOA->MODER, mask);
		CLEAR_BIT(GPIOA->PUPDR, mask);

		mask = ~(GPIO_GET_PINMASK(PINID_JTMS) | GPIO_GET_PINMASK(PINID_JTCK) | GPIO_GET_PINMASK(PINID_JTDI));
		MODIFY_BITS(GPIOA->OSPEEDR, mask, OUTPUT_REGISTER);
		GPIOA->BSRR = (mask << GPIO_BSRR_BR0_Pos);
	} else if (port == GPIOB) {
		uint32_t mask;

		mask = ~(
			(0b11U << (GPIO_GET_PINNO(PINID_JTDO  ) * 2)) |
			(0b11U << (GPIO_GET_PINNO(PINID_JNTRST) * 2)) |
			0);
		SET_BIT(  GPIOB->MODER, mask);
		CLEAR_BIT(GPIOB->PUPDR, mask);

		mask = ~(GPIO_GET_PINMASK(PINID_JTDO) | GPIO_GET_PINMASK(PINID_JNTRST));
		MODIFY_BITS(GPIOB->OSPEEDR, mask, OUTPUT_REGISTER);
		GPIOB->BSRR = (mask << GPIO_BSRR_BR0_Pos);
	} else

#elif DEBUG && uHAL_SWD_DEBUG
	if (port == GPIOA) {
		uint32_t mask;

		mask = ~(
			(0b11U << (GPIO_GET_PINNO(PINID_SWDIO) * 2)) |
			(0b11U << (GPIO_GET_PINNO(PINID_SWCLK) * 2)) |
			0);
		SET_BIT(  GPIOA->MODER, mask);
		CLEAR_BIT(GPIOA->PUPDR, mask);

		mask = ~(GPIO_GET_PINMASK(PINID_SWDIO) | GPIO_GET_PINMASK(PINID_SWCLK));
		MODIFY_BITS(GPIOA->OSPEEDR, mask, OUTPUT_REGISTER);
		GPIOA->BSRR = (mask << GPIO_BSRR_BR0_Pos);
	} else
# endif

	{
		port->MODER   = 0xFFFFFFFFU;
		port->PUPDR   = 0;
		port->OSPEEDR = OUTPUT_REGISTER;
		port->ODR     = 0;
	}

	return;
}
void gpio_set_AF(gpio_pin_t pin, gpio_af_t af) {
	GPIO_TypeDef *port;
	__IO uint32_t *reg;
	uint_fast8_t pos;
	uint32_t mask, naf;

	assert(GPIO_PIN_IS_VALID(pin));
	assert((af & 0b1111U) == af);
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (!GPIO_PIN_IS_VALID(pin) || (af & 0x0FU) != af) {
		return;
	}
#endif

	// Some AF mappings don't play nice
#if PINID_I2C3_SDA && defined(GPIOAF_I2C3_ALT)
	if ((af == GPIOAF_I2C3) && (PINID(pin) == PINID_I2C3_SDA)) {
		af = GPIOAF_I2C3_ALT;
	} else
#endif
#if PINID_I2C2_SDA_ALT && defined(GPIOAF_I2C2_ALT)
	if ((af == GPIOAF_I2C2) && (PINID(pin) == PINID_I2C2_SDA_ALT)) {
		af = GPIOAF_I2C2_ALT;
	} else
#endif
#if PINID_SPI3_MOSI_ALT && defined(GPIOAF_SPI3_ALT)
	if ((af == GPIOAF_SPI3) && (PINID(pin) == PINID_SPI3_MOSI_ALT)) {
		af = GPIOAF_SPI3_ALT;
	} else
#endif
	{
		((void )0U);
	}

	port = GPIO_GET_PORT(pin);
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (port == NULL) {
		return;
	}
#endif
	pos = GPIO_GET_PINNO(pin);

	if (pos < 8) {
		reg = &port->AFR[0];
	} else {
		pos -= 8;
		reg = &port->AFR[1];
	}
	pos *= 4U;
	mask = (uint32_t )0b1111U << pos;
	naf = (uint32_t )af << pos;
	MODIFY_BITS(*reg, mask, naf);

	return;
}
gpio_af_t gpio_get_AF(gpio_pin_t pin) {
	GPIO_TypeDef *port;
	__IO uint32_t *reg;
	uint_fast8_t pos;
	uint32_t mask;
	gpio_af_t af;

	assert(GPIO_PIN_IS_VALID(pin));
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (!GPIO_PIN_IS_VALID(pin)) {
		return 0;
	}
#endif

	port = GPIO_GET_PORT(pin);
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (port == NULL) {
		return 0;
	}
#endif
	pos = GPIO_GET_PINNO(pin);

	if (pos < 8) {
		reg = &port->AFR[0];
	} else {
		pos -= 8;
		reg = &port->AFR[1];
	}
	pos *= 4U;
	mask = (uint32_t )0b1111U << pos;
	af = GATHER_BITS(*reg, mask, pos);

	// Some AF mappings don't play nice
#if PINID_I2C3_SDA && defined(GPIOAF_I2C3_ALT)
	if ((af == GPIOAF_I2C3_ALT) && (PINID(pin) == PINID_I2C3_SDA)) {
		af = GPIOAF_I2C3;
	} else
#endif
#if PINID_I2C2_SDA_ALT && defined(GPIOAF_I2C2_ALT)
	if ((af == GPIOAF_I2C2_ALT) && (PINID(pin) == PINID_I2C2_SDA_ALT)) {
		af = GPIOAF_I2C2;
	} else
#endif
#if PINID_SPI3_MOSI_ALT && defined(GPIOAF_SPI3_ALT)
	if ((af == GPIOAF_SPI3_ALT) && (PINID(pin) == PINID_SPI3_MOSI_ALT)) {
		af = GPIOAF_SPI3;
	} else
#endif
	{
		((void )0U);
	}

	return af;
}

err_t gpio_set_mode(gpio_pin_t pin, gpio_mode_t mode, gpio_state_t istate) {
	GPIO_TypeDef* port;
	uint_fast8_t pinno, pos2;
	uint32_t pinmask, mask2, bsrr = 0;
	uint32_t cfg, otype = 0, pull;

	assert(GPIO_PIN_IS_VALID(pin));
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (!GPIO_PIN_IS_VALID(pin)) {
		return ERR_BADARG;
	}
#endif

	port = GPIO_GET_PORT(pin);
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (port == NULL) {
		return ERR_BADARG;
	}
#endif
	pinmask = GPIO_GET_PINMASK(pin);
	pinno = GPIO_GET_PINNO(pin);

	pos2 = pinno * 2U;
	mask2 = (uint32_t )0b11U << pos2;

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
		case GPIO_LOW:
			bsrr = pinmask << GPIO_BSRR_BR0_Pos;
			break;
		case GPIO_FLOAT:
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
	//case GPIO_MODE_AIN:
	//case GPIO_MODE_HiZ:
	default:
		pull = NO_PULL;
		break;
	}

	switch (mode) {
	case GPIO_MODE_PP:
		cfg = MODE_OUTPUT;
		otype = OUTPUT_PP;
		break;
	case GPIO_MODE_OD:
		cfg = MODE_OUTPUT;
		otype = OUTPUT_OD;
		break;
	case GPIO_MODE_IN:
		cfg = MODE_INPUT;
		break;
	case GPIO_MODE_IN_AF:
	case GPIO_MODE_PP_AF:
		cfg = MODE_AF;
		otype = OUTPUT_PP;
		break;
	case GPIO_MODE_OD_AF:
		cfg = MODE_AF;
		otype = OUTPUT_OD;
		break;
	case GPIO_MODE_RESET:
	case GPIO_MODE_AIN:
	case GPIO_MODE_HiZ:
	default:
		cfg = MODE_ANALOG;
		break;
	}
	cfg = cfg << pos2;
	otype = otype << pinno;
	pull = pull << pos2;

	port->BSRR = bsrr;
	MODIFY_BITS(port->PUPDR, mask2, pull);
	MODIFY_BITS(port->OTYPER, pinmask, otype);
	// Per the data sheet, MODER should be set after configuration (at least
	// for alternate functions)
	MODIFY_BITS(port->MODER,  mask2, cfg);

	return ERR_OK;
}
gpio_mode_t gpio_get_mode(gpio_pin_t pin) {
	GPIO_TypeDef* port;
	uint32_t pinno;
	uint_fast8_t mode, otype, pos2;

	assert(GPIO_PIN_IS_VALID(pin));
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (!GPIO_PIN_IS_VALID(pin)) {
		return GPIO_MODE_RESET;
	}
#endif

	port = GPIO_GET_PORT(pin);
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (port == NULL) {
		return GPIO_MODE_RESET;
	}
#endif
	pinno = GPIO_GET_PINNO(pin);

	pos2 = pinno * 2U;
	mode  = GATHER_BITS(port->MODER,  0b11U, pos2);
	otype = GATHER_BITS(port->OTYPER, 0b1U,  pinno);

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

	// Shouldn't actually reach this point
	return GPIO_MODE_HiZ;
}

err_t gpio_set_state(gpio_pin_t pin, gpio_state_t new_state) {
	GPIO_TypeDef* port;
	uint32_t pinno, pinmask;
	uint_fast8_t mode, pos2;
	uint32_t reg = 0;

	assert(GPIO_PIN_IS_VALID(pin));
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (!GPIO_PIN_IS_VALID(pin)) {
		return ERR_BADARG;
	}
#endif

	port = GPIO_GET_PORT(pin);
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (port == NULL) {
		return ERR_BADARG;
	}
#endif
	pinno = GPIO_GET_PINNO(pin);

	pos2 = pinno * 2U;
	mode  = GATHER_BITS(port->MODER,  0b11U, pos2);

	switch (mode) {
	case MODE_OUTPUT:
		reg = 0;
		pinmask = GPIO_GET_PINMASK(pin);
		switch (new_state) {
		case GPIO_HIGH:
			reg = pinmask;
			break;
		case GPIO_LOW:
			reg = (pinmask << GPIO_BSRR_BR0_Pos);
			break;
		case GPIO_FLOAT:
#if FLOAT_TOGGLES_OUTPUT
			reg = ((pinmask & port->ODR) == 0) ? pinmask : pinmask << GPIO_BSRR_BR0_Pos;
#endif
			break;
		}
		port->BSRR = reg;
		break;

	case MODE_AF:
	case MODE_INPUT:
		switch (new_state) {
		case GPIO_HIGH:
			reg = PULL_UP;
			break;
		case GPIO_LOW:
			reg = PULL_DOWN;
			break;
		default:
			reg = NO_PULL;
			break;
		}
		reg <<= pos2;
		MODIFY_BITS(port->PUPDR, (0b11U << pos2), reg);
		break;

	default:
		break;
	}

	return ERR_OK;
}
err_t gpio_set_input_state(gpio_pin_t pin, gpio_state_t new_state) {
	GPIO_TypeDef* port;
	uint32_t pinno;
	uint_fast8_t pos2;
	uint32_t reg = 0;

	assert(GPIO_PIN_IS_VALID(pin));
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (!GPIO_PIN_IS_VALID(pin)) {
		return ERR_BADARG;
	}
#endif

	port = GPIO_GET_PORT(pin);
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (port == NULL) {
		return ERR_BADARG;
	}
#endif
	pinno = GPIO_GET_PINNO(pin);
	pos2 = pinno * 2U;

#if ! uHAL_SKIP_INIT_CHECKS
	uint_fast8_t mode  = GATHER_BITS(port->MODER,  0b11U, pos2);

	switch (mode) {
	case MODE_AF:
	case MODE_INPUT:
		break;

	default:
		return ERR_INIT;
	}
#endif

	switch (new_state) {
	case GPIO_HIGH:
		reg = PULL_UP;
		break;
	case GPIO_LOW:
		reg = PULL_DOWN;
		break;
	default:
		reg = NO_PULL;
		break;
	}
	reg <<= pos2;
	MODIFY_BITS(port->PUPDR, (0b11U << pos2), reg);

	return ERR_OK;
}
err_t gpio_set_output_state(gpio_pin_t pin, gpio_state_t new_state) {
	GPIO_TypeDef* port;
	uint32_t pinmask;
	uint32_t reg = 0;

	assert(GPIO_PIN_IS_VALID(pin));
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (!GPIO_PIN_IS_VALID(pin)) {
		return ERR_BADARG;
	}
#endif

	port = GPIO_GET_PORT(pin);
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (port == NULL) {
		return ERR_BADARG;
	}
#endif

#if ! uHAL_SKIP_INIT_CHECKS
	uint_fast8_t pos2 = GPIO_GET_PINNO(pin) * 2U;
	uint_fast8_t mode  = GATHER_BITS(port->MODER,  0b11U, pos2);

	switch (mode) {
	case MODE_OUTPUT:
		break;

	default:
		return ERR_INIT;
	}
#endif

	reg = 0;
	pinmask = GPIO_GET_PINMASK(pin);
	switch (new_state) {
	case GPIO_HIGH:
		reg = pinmask;
		break;
	case GPIO_LOW:
		reg = (pinmask << GPIO_BSRR_BR0_Pos);
		break;
	case GPIO_FLOAT:
#if FLOAT_TOGGLES_OUTPUT
		reg = ((pinmask & port->ODR) == 0) ? pinmask : pinmask << GPIO_BSRR_BR0_Pos;
#endif
		break;
	}
	port->BSRR = reg;

	return ERR_OK;
}

err_t gpio_toggle_state(gpio_pin_t pin) {
	GPIO_TypeDef* port;
	uint32_t pinno, pinmask;
	uint_fast8_t mode, pos2;
	uint32_t reg = 0;

	assert(GPIO_PIN_IS_VALID(pin));
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (!GPIO_PIN_IS_VALID(pin)) {
		return ERR_BADARG;
	}
#endif

	port = GPIO_GET_PORT(pin);
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (port == NULL) {
		return ERR_BADARG;
	}
#endif
	pinno = GPIO_GET_PINNO(pin);

	pos2 = pinno * 2U;
	mode  = GATHER_BITS(port->MODER,  0b11U, pos2);

	switch (mode) {
	case MODE_OUTPUT:
		pinmask = GPIO_GET_PINMASK(pin);
		port->BSRR = ((pinmask & port->ODR) == 0) ? pinmask : pinmask << GPIO_BSRR_BR0_Pos;
		break;

	case MODE_AF:
	case MODE_INPUT:
		switch (GATHER_BITS(port->PUPDR, 0b11U, pos2)) {
		case PULL_UP:
			reg = PULL_DOWN;
			break;
		case PULL_DOWN:
			reg = PULL_UP;
			break;
		default:
			reg = NO_PULL;
			break;
		}
		reg <<= pos2;
		MODIFY_BITS(port->PUPDR, (0b11U << pos2), reg);
		break;

	default:
		break;
	}

	return ERR_OK;
}
err_t gpio_toggle_input_state(gpio_pin_t pin) {
	GPIO_TypeDef* port;
	uint32_t pinno;
	uint_fast8_t pos2;
	uint32_t reg = 0;

	assert(GPIO_PIN_IS_VALID(pin));
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (!GPIO_PIN_IS_VALID(pin)) {
		return ERR_BADARG;
	}
#endif

	port = GPIO_GET_PORT(pin);
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (port == NULL) {
		return ERR_BADARG;
	}
#endif
	pinno = GPIO_GET_PINNO(pin);
	pos2 = pinno * 2U;

#if ! uHAL_SKIP_INIT_CHECKS
	switch (GATHER_BITS(port->MODER,  0b11U, pos2)) {
	case MODE_AF:
	case MODE_INPUT:
		break;
	default:
		return ERR_INIT;
	}
#endif

	switch (GATHER_BITS(port->PUPDR, 0b11U, pos2)) {
	case PULL_UP:
		reg = PULL_DOWN;
		break;
	case PULL_DOWN:
		reg = PULL_UP;
		break;
	default:
		reg = NO_PULL;
		break;
	}
	reg <<= pos2;
	MODIFY_BITS(port->PUPDR, (0b11U << pos2), reg);

	return ERR_OK;
}
err_t gpio_toggle_output_state(gpio_pin_t pin) {
	GPIO_TypeDef* port;
	uint32_t pinmask;

	assert(GPIO_PIN_IS_VALID(pin));
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (!GPIO_PIN_IS_VALID(pin)) {
		return ERR_BADARG;
	}
#endif

	port = GPIO_GET_PORT(pin);
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (port == NULL) {
		return ERR_BADARG;
	}
#endif

#if ! uHAL_SKIP_INIT_CHECKS
	uint_fast8_t pos2 = GPIO_GET_PINNO(pin) * 2U;

	switch (GATHER_BITS(port->MODER,  0b11U, pos2)) {
	case MODE_OUTPUT:
		break;
	default:
		return ERR_INIT;
	}
#endif

	pinmask = GPIO_GET_PINMASK(pin);
	port->BSRR = ((pinmask & port->ODR) == 0) ? pinmask : pinmask << GPIO_BSRR_BR0_Pos;

	return ERR_OK;
}

gpio_state_t gpio_get_state(gpio_pin_t pin) {
	GPIO_TypeDef* port;
	uint32_t pinno, pinmask;
	uint_fast8_t mode, pos2;
	uint32_t check;

	assert(GPIO_PIN_IS_VALID(pin));
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (!GPIO_PIN_IS_VALID(pin)) {
		return GPIO_FLOAT;
	}
#endif

	port = GPIO_GET_PORT(pin);
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (port == NULL) {
		return GPIO_FLOAT;
	}
#endif
	pinno = GPIO_GET_PINNO(pin);
	pinmask = GPIO_GET_PINMASK(pin);

	pos2 = pinno * 2U;
	mode  = GATHER_BITS(port->MODER,  0b11U, pos2);

	switch (mode) {
	case MODE_OUTPUT:
		check = port->ODR;
		break;

	case MODE_AF:
	case MODE_INPUT:
		check = port->IDR;
		break;

	default:
		return GPIO_FLOAT;
	}

	return (BIT_IS_SET(check, pinmask)) ? GPIO_HIGH : GPIO_LOW;
}
gpio_state_t gpio_get_input_state(gpio_pin_t pin) {
	GPIO_TypeDef* port;
	uint32_t pinmask;

	assert(GPIO_PIN_IS_VALID(pin));
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (!GPIO_PIN_IS_VALID(pin)) {
		return GPIO_FLOAT;
	}
#endif

	port = GPIO_GET_PORT(pin);
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (port == NULL) {
		return GPIO_FLOAT;
	}
#endif
	pinmask = GPIO_GET_PINMASK(pin);

#if ! uHAL_SKIP_INIT_CHECKS
	uint_fast8_t pos2 = GPIO_GET_PINNO(pin) * 2U;

	switch (GATHER_BITS(port->MODER,  0b11U, pos2)) {
	case MODE_AF:
	case MODE_INPUT:
		break;

	default:
		return GPIO_FLOAT;
	}
#endif

	return (BIT_IS_SET(port->IDR, pinmask)) ? GPIO_HIGH : GPIO_LOW;
}
gpio_state_t gpio_get_output_state(gpio_pin_t pin) {
	GPIO_TypeDef* port;
	uint32_t pinmask;

	assert(GPIO_PIN_IS_VALID(pin));
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (!GPIO_PIN_IS_VALID(pin)) {
		return GPIO_FLOAT;
	}
#endif

	port = GPIO_GET_PORT(pin);
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (port == NULL) {
		return GPIO_FLOAT;
	}
#endif
	pinmask = GPIO_GET_PINMASK(pin);

#if ! uHAL_SKIP_INIT_CHECKS
	uint_fast8_t pos2 = GPIO_GET_PINNO(pin) * 2U;

	switch (GATHER_BITS(port->MODER,  0b11U, pos2)) {
	case MODE_OUTPUT:
		break;

	default:
		return GPIO_FLOAT;
	}
#endif

	return (BIT_IS_SET(port->ODR, pinmask)) ? GPIO_HIGH : GPIO_LOW;
}


#endif // INCLUDED_BY_GPIO_C
