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
// gpio_STM32F1.h
// Manage the GPIO peripheral
// NOTES:
//    This was split off from gpio.c because the GPIO peripherals differ so
//    much between the STM32F1 and the other lines that code can't really be
//    reused
//
//    This file should only be included by gpio.c
//
#ifdef INCLUDED_BY_GPIO_C


#define OUTPUT_SLOW      0b10U // Slow speed,   2MHZ
#define OUTPUT_MEDIUM    0b01U // Medium speed, 10MHZ
#define OUTPUT_FAST      0b11U // High speed,   50MHZ
#define OUTPUT_VERY_FAST 0b11U // Same as high speed, here for compatibility
//#define OUTPUT OUTPUT_SLOW
#define INPUT  0b00U

// This is the hardware RESET value which is floating digital input. We want to
// use analog input mode instead because it uses less power.
//#define MODE_RESET (0x04U)
#define MODE_RESET MODE_AN
#define MODE_AF    (0b1000U)
#define MODE_PP    (0b0000U|OUTPUT)
#define MODE_PP_AF (MODE_AF|MODE_PP)
#define MODE_OD    (0b0100U|OUTPUT)
#define MODE_OD_AF (MODE_AF|MODE_OD)
#define MODE_INP   (0b1000U|INPUT)
#define MODE_INF   (0b0100U|INPUT)
#define MODE_AN    (0b0000U|INPUT)

// According to the datasheet, pins C13-C15 shouldn't be used at any speed
// faster than 2MHz and must not be used as a current source because they're
// supplied through the power switch
// We use these modes specifically for those pins
#define OUTPUT_PWRSWITCH     (OUTPUT_SLOW)
#define MODE_PP_PWRSWITCH    (MODE_OD_PWRSWITCH)
#define MODE_PP_AF_PWRSWITCH (MODE_AF|MODE_PP_PWRSWITCH)
#define MODE_OD_PWRSWITCH    (0b0100U|OUTPUT_PWRSWITCH)
#define MODE_OD_AF_PWRSWITCH (MODE_AF|MODE_OD_PWRSWITCH)

// Unlike everything else, JTAG is enabled on reset and needs to be explicitly
// disabled
// Unlike everything else, reading the JTAG part of AFIO_MAPR will always
// return '0' (fully-enabled) so it needs to be set everytime.
// NOJNTRST would have full JTAG and SWD but no JNTRST
// JTAGDISABLE disables JTAG while leaving SWD
// DISABLE would disable everything
#ifndef uHAL_SWD_DEBUG
# define uHAL_SWD_DEBUG 1
#endif
#if DEBUG
# if uHAL_JTAG_DEBUG
#  define JTAG_STATE 0
# elif uHAL_SWD_DEBUG
#  define JTAG_STATE AFIO_MAPR_SWJ_CFG_JTAGDISABLE
# endif
#else
# define JTAG_STATE AFIO_MAPR_SWJ_CFG_DISABLE
#endif


static void gpio_platform_init(void) {
	uint32_t remaps = JTAG_STATE;

	// RCC_PERIPH_AFIO was enabled in platform_init() before calling gpio_init()
	// because it's needed there too
	//clock_init(RCC_PERIPH_AFIO);

	// Remap CAN to PD[01] (per the errata sheet) so it doesn't interfere with
	// UART1 (even though we don't use RTS...)
	remaps |= AFIO_MAPR_CAN_REMAP_REMAP3;
	DEBUG_CPP_MSG("Remapping CAN pins")
#if DO_SPI1_GPIO_REMAP
	DEBUG_CPP_MSG("Remapping SPI1 pins")
	remaps |= AFIO_MAPR_SPI1_REMAP;
#endif
#if DO_I2C1_GPIO_REMAP
	DEBUG_CPP_MSG("Remapping I2C1 pins")
	remaps |= AFIO_MAPR_I2C1_REMAP;
#endif
#if DO_UART1_GPIO_REMAP
	DEBUG_CPP_MSG("Remapping UART1 pins")
	remaps |= AFIO_MAPR_USART1_REMAP;
#endif
#if DO_UART2_GPIO_REMAP
	remaps |= AFIO_MAPR_USART2_REMAP;
	DEBUG_CPP_MSG("Remapping UART2 pins")
#endif
#if DO_UART3_GPIO_REMAP
	DEBUG_CPP_MSG("Remapping UART3 pins")
# if DO_UART3_GPIO_REMAP == GPIO_REMAP_PARTIAL
	remaps |= AFIO_MAPR_USART3_REMAP_PARTIALREMAP;
# else
	remaps |= AFIO_MAPR_USART3_REMAP_FULLREMAP;
# endif
#endif
#if DO_TIM1_GPIO_REMAP
	DEBUG_CPP_MSG("Remapping TIM1 PWM outputs")
# if DO_TIM1_GPIO_REMAP == GPIO_REMAP_PARTIAL
	remaps |= AFIO_MAPR_TIM1_REMAP_PARTIALREMAP;
# else
	remaps |= AFIO_MAPR_TIM1_REMAP_FULLREMAP;
# endif
#endif
#if DO_TIM2_GPIO_REMAP
	DEBUG_CPP_MSG("Remapping TIM2 PWM outputs")
# if DO_TIM2_GPIO_REMAP == GPIO_REMAP_PARTIAL_1
	remaps |= AFIO_MAPR_TIM2_REMAP_PARTIALREMAP1;
# elif DO_TIM2_GPIO_REMAP == GPIO_REMAP_PARTIAL_2
	remaps |= AFIO_MAPR_TIM2_REMAP_PARTIALREMAP2;
# else
	remaps |= AFIO_MAPR_TIM2_REMAP_FULLREMAP;
# endif
#endif
#if DO_TIM3_GPIO_REMAP
	DEBUG_CPP_MSG("Remapping TIM3 PWM outputs")
# if DO_TIM3_GPIO_REMAP == GPIO_REMAP_PARTIAL
	remaps |= AFIO_MAPR_TIM3_REMAP_PARTIALREMAP;
# else
	remaps |= AFIO_MAPR_TIM3_REMAP_FULLREMAP;
# endif
#endif
#if DO_TIM4_GPIO_REMAP
	DEBUG_CPP_MSG("Remapping TIM4 PWM outputs")
	remaps |= AFIO_MAPR_TIM4_REMAP;
#endif
#if DO_PD01_GPIO_REMAP
	DEBUG_CPP_MSG("Remapping OSC_IN/OSC_OUT to PD0/PD1")
	remaps |= AFIO_MAPR_PD01_REMAP;
#endif
	AFIO->MAPR = remaps;

	remaps = 0;
#if DO_TIM9_GPIO_REMAP
	DEBUG_CPP_MSG("Remapping TIM9 PWM outputs")
	remaps |= AFIO_MAPR2_TIM9_REMAP;
#endif
#if DO_TIM10_GPIO_REMAP
	DEBUG_CPP_MSG("Remapping TIM10 PWM outputs")
	remaps |= AFIO_MAPR2_TIM10_REMAP;
#endif
#if DO_TIM11_GPIO_REMAP
	DEBUG_CPP_MSG("Remapping TIM11 PWM outputs")
	remaps |= AFIO_MAPR2_TIM11_REMAP;
#endif
#if DO_TIM13_GPIO_REMAP
	DEBUG_CPP_MSG("Remapping TIM13 PWM outputs")
	remaps |= AFIO_MAPR2_TIM13_REMAP;
#endif
#if DO_TIM14_GPIO_REMAP
	DEBUG_CPP_MSG("Remapping TIM14 PWM outputs")
	remaps |= AFIO_MAPR2_TIM14_REMAP;
#endif
	AFIO->MAPR2 = remaps;

	return;
}
static void port_reset(GPIO_TypeDef *port) {
#if DEBUG && uHAL_JTAG_DEBUG
	if (port == GPIOA) {
		uint32_t mask;

		mask = ~(
			(0b1111U << ((GPIO_GET_PINNO(PINID_JTMS)-8U) * 4U)) |
			(0b1111U << ((GPIO_GET_PINNO(PINID_JTCK)-8U) * 4U)) |
			(0b1111U << ((GPIO_GET_PINNO(PINID_JTDI)-8U) * 4U)) |
			0);
		CLEAR_BIT(GPIOA->CRH, mask);
		GPIOA->CRL = 0;

		mask = ~(GPIO_GET_PINMASK(PINID_JTMS) | GPIO_GET_PINMASK(PINID_JTCK) | GPIO_GET_PINMASK(PINID_JTDI));
		//CLEAR_BIT(GPIOA->ODR, mask);
		GPIOA->BRR = mask;
	} else if (port == GPIOB) {
		uint32_t mask;

		GPIOB->CRH = 0;
		mask = ~(
			(0b1111U << ((GPIO_GET_PINNO(PINID_JTDO  )) * 4U)) |
			(0b1111U << ((GPIO_GET_PINNO(PINID_JNTRST)) * 4U)) |
			0);
		CLEAR_BIT(GPIOB->CRL, mask);

		mask = ~(GPIO_GET_PINMASK(PINID_JTDO) | GPIO_GET_PINMASK(PINID_JNTRST));
		//CLEAR_BIT(GPIOB->ODR, mask);
		GPIOB->BRR = mask;
	} else

#elif DEBUG && uHAL_SWD_DEBUG
	if (port == GPIOA) {
		uint32_t mask;

		mask = ~(
			(0b1111U << ((GPIO_GET_PINNO(PINID_SWDIO)-8) * 4)) |
			(0b1111U << ((GPIO_GET_PINNO(PINID_SWCLK)-8) * 4)) |
			0);
		CLEAR_BIT(GPIOA->CRH, mask);
		GPIOA->CRL = 0;

		mask = ~(GPIO_GET_PINMASK(PINID_SWDIO) | GPIO_GET_PINMASK(PINID_SWCLK));
		//CLEAR_BIT(GPIOA->ODR, mask);
		GPIOA->BRR = mask;
	} else
# endif

	{
		port->CRL = 0;
		port->CRH = 0;
		port->ODR = 0;
	}

	return;
}
void gpio_set_AF(gpio_pin_t pin, gpio_af_t af) {
	assert(GPIO_PIN_IS_VALID(pin));

	UNUSED(af);

	return;
}

/*
// There should be no case where this would be useful, enabling it could cause
// confusion if someone doesn't know it's meaningless on F1s.
gpio_af_t gpio_get_AF(gpio_pin_t pin) {
	assert(GPIO_PIN_IS_VALID(pin));

	UNUSED(af);

	return 0;
}
*/
void gpio_remap_uart1(void) {
	bool redisable_clock;

	redisable_clock = !clock_is_enabled(RCC_PERIPH_AFIO);
	if (redisable_clock) {
		clock_enable(RCC_PERIPH_AFIO);
	}

	MODIFY_BITS(AFIO->MAPR, AFIO_MAPR_SWJ_CFG|AFIO_MAPR_USART1_REMAP,
		JTAG_STATE | AFIO_MAPR_USART1_REMAP);

	if (redisable_clock) {
		clock_disable(RCC_PERIPH_AFIO);
	}

	return;
}
#ifdef TIM5
bool gpio_remap_tim5ch4(void) {
	bool redisable_clock, ret;

	redisable_clock = !clock_is_enabled(RCC_PERIPH_AFIO);
	if (redisable_clock) {
		clock_enable(RCC_PERIPH_AFIO);
	}

	if ((ret = BIT_IS_SET(AFIO->MAPR, AFIO_MAPR_TIM5CH4_IREMAP)) == 0) {
		MODIFY_BITS(AFIO->MAPR, AFIO_MAPR_SWJ_CFG|AFIO_MAPR_TIM5CH4_IREMAP,
			JTAG_STATE | AFIO_MAPR_TIM5CH4_IREMAP);
	}

	if (redisable_clock) {
		clock_disable(RCC_PERIPH_AFIO);
	}

	return ret;
}
bool gpio_unremap_tim5ch4(void) {
	bool redisable_clock, ret;

	redisable_clock = !clock_is_enabled(RCC_PERIPH_AFIO);
	if (redisable_clock) {
		clock_enable(RCC_PERIPH_AFIO);
	}

	if ((ret = BIT_IS_SET(AFIO->MAPR, AFIO_MAPR_TIM5CH4_IREMAP)) != 0) {
		MODIFY_BITS(AFIO->MAPR, AFIO_MAPR_SWJ_CFG|AFIO_MAPR_TIM5CH4_IREMAP,
			JTAG_STATE | 0);
	}

	if (redisable_clock) {
		clock_disable(RCC_PERIPH_AFIO);
	}

	return ret;
}
#endif

err_t gpio_set_mode(gpio_pin_t pin, gpio_mode_t mode, gpio_state_t istate) {
	uint32_t mask;
	GPIO_TypeDef* port;
	uint32_t pinmask, pinno, mpinno;

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

	// Determine the CNF and MODE bits for the pin
	switch (mode) {
	case GPIO_MODE_RESET:
		mask = MODE_RESET;
		break;
	case GPIO_MODE_PP:
#if HAVE_GPIO_PORTC
		mask = 0;
		if (port == GPIOC) {
			switch (pinno) {
			case 13:
			case 14:
			case 15:
				mask = MODE_PP_PWRSWITCH;
				break;
			}
		}
		if (mask == 0)
#endif
		mask = MODE_PP;
		break;
	case GPIO_MODE_PP_AF:
#if HAVE_GPIO_PORTC
		mask = 0;
		if (port == GPIOC) {
			switch (pinno) {
			case 13:
			case 14:
			case 15:
				mask = MODE_PP_AF_PWRSWITCH;
				break;
			}
		}
		if (mask == 0)
#endif
		mask = MODE_PP_AF;
		break;
	case GPIO_MODE_OD:
#if HAVE_GPIO_PORTC
		mask = 0;
		if (port == GPIOC) {
			switch (pinno) {
			case 13:
			case 14:
			case 15:
				mask = MODE_OD_PWRSWITCH;
				break;
			}
		}
		if (mask == 0)
#endif
		mask = MODE_OD;
		break;
	case GPIO_MODE_OD_AF:
#if HAVE_GPIO_PORTC
		mask = 0;
		if (port == GPIOC) {
			switch (pinno) {
			case 13:
			case 14:
			case 15:
				mask = MODE_OD_AF_PWRSWITCH;
				break;
			}
		}
		if (mask == 0)
#endif
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
	//case GPIO_MODE_AIN:
	//case GPIO_MODE_HiZ:
	default:
		mask = MODE_AN;
		break;
	}

	if (pinno < 8) {
		mpinno = pinno * 4;
		MODIFY_BITS(port->CRL, (0b1111U << mpinno),
			(mask << mpinno)
			);
	} else {
		mpinno = (pinno - 8) * 4;
		MODIFY_BITS(port->CRH, (0b1111U << mpinno),
			(mask << mpinno)
			);
	}

	// For normal outputs, set the pin state
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

	return ERR_OK;
}
gpio_mode_t gpio_get_mode(gpio_pin_t pin) {
	uint32_t mask;
	GPIO_TypeDef* port;
	uint32_t pinno, mpinno;

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

	if (pinno < 8) {
		mpinno = pinno * 4;
		mask = GATHER_BITS(port->CRL, 0b1111U, mpinno);
	} else {
		mpinno = (pinno - 8) * 4;
		mask = GATHER_BITS(port->CRH, 0b1111U, mpinno);
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

	// Shouldn't actually reach this point
	return GPIO_MODE_RESET;
}

err_t gpio_set_state(gpio_pin_t pin, gpio_state_t new_state) {
	GPIO_TypeDef* port;
	uint32_t pinmask, pinno, modemask, mpinno;

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

	if (pinno < 8) {
		mpinno = pinno * 4U;
		modemask = GATHER_BITS(port->CRL, 0b1111U, mpinno);
	} else {
		mpinno = (pinno - 8U) * 4U;
		modemask = GATHER_BITS(port->CRH, 0b1111U, mpinno);
	}

	switch (modemask) {
	case (MODE_PP):
	case (MODE_PP_AF):
	case (MODE_OD):
	case (MODE_OD_AF):
		switch (new_state) {
		case GPIO_HIGH:
			port->BSRR = pinmask;
			break;
		case GPIO_LOW:
			port->BRR = pinmask;
			break;
		case GPIO_FLOAT:
#if FLOAT_TOGGLES_OUTPUT
			if ((port->ODR & pinmask) == 0) {
				port->BSRR = pinmask;
			} else {
				port->BRR = pinmask;
			}
#endif
			break;
		}
		break;

	case (MODE_INF):
	case (MODE_INP): {
		uint32_t nmask;

		switch (new_state) {
		case GPIO_HIGH:
			port->BSRR = pinmask;
			nmask = MODE_INP;
			break;
		case GPIO_LOW:
			port->BRR = pinmask;
			nmask = MODE_INP;
			break;
		//case GPIO_FLOAT:
		default:
			nmask = MODE_INF;
			break;
		}
		// Is there any benefit to not re-setting unecessarily? The check adds
		// a branch but cuts out register read/modify/write in the common case
		if (nmask != modemask) {
			if (pinno < 8) {
				MODIFY_BITS(port->CRL, (0b1111U << mpinno),
					(nmask << mpinno)
					);
			} else {
				MODIFY_BITS(port->CRH, (0b1111U << mpinno),
					(nmask << mpinno)
					);
			}
		}
		break;
	}

	case (MODE_AN):
		break;
	}

	return ERR_OK;
}
err_t gpio_set_input_state(gpio_pin_t pin, gpio_state_t new_state) {
	GPIO_TypeDef* port;
	uint32_t modemask, pinmask, pinno, mpinno, nmask;

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

	if (pinno < 8) {
		mpinno = pinno * 4U;
	} else {
		mpinno = (pinno - 8U) * 4U;
	}

	if (pinno < 8) {
		modemask = GATHER_BITS(port->CRL, 0b1111U, mpinno);
	} else {
		modemask = GATHER_BITS(port->CRH, 0b1111U, mpinno);
	}

#if ! uHAL_SKIP_INIT_CHECKS
	switch (modemask) {
	case (MODE_INF):
	case (MODE_INP):
		break;

	default:
		return ERR_INIT;
	}
#endif

	switch (new_state) {
	case GPIO_HIGH:
		port->BSRR = pinmask;
		nmask = MODE_INP;
		break;
	case GPIO_LOW:
		port->BRR = pinmask;
		nmask = MODE_INP;
		break;
	//case GPIO_FLOAT:
	default:
		nmask = MODE_INF;
		break;
	}
	// Is there any benefit to not re-setting unecessarily? The check adds
	// a branch but cuts out register read/modify/write in the common case
	if (nmask != modemask) {
		if (pinno < 8) {
			MODIFY_BITS(port->CRL, (0b1111U << mpinno),
				(nmask << mpinno)
				);
		} else {
			MODIFY_BITS(port->CRH, (0b1111U << mpinno),
				(nmask << mpinno)
				);
		}
	}

	return ERR_OK;
}
err_t gpio_set_output_state(gpio_pin_t pin, gpio_state_t new_state) {
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
	pinmask = GPIO_GET_PINMASK(pin);

#if ! uHAL_SKIP_INIT_CHECKS
	uint32_t modemask, mpinno, pinno;

	pinno = GPIO_GET_PINNO(pin);
	if (pinno < 8) {
		mpinno = pinno * 4U;
	} else {
		mpinno = (pinno - 8U) * 4U;
	}

	if (pinno < 8) {
		modemask = GATHER_BITS(port->CRL, 0b1111U, mpinno);
	} else {
		modemask = GATHER_BITS(port->CRH, 0b1111U, mpinno);
	}

	switch (modemask) {
	case (MODE_PP):
	case (MODE_PP_AF):
	case (MODE_OD):
	case (MODE_OD_AF):
		break;

	default:
		return ERR_INIT;
	}
#endif

	switch (new_state) {
	case GPIO_HIGH:
		port->BSRR = pinmask;
		break;
	case GPIO_LOW:
		port->BRR = pinmask;
		break;
	case GPIO_FLOAT:
#if FLOAT_TOGGLES_OUTPUT
		if ((port->ODR & pinmask) == 0) {
			port->BSRR = pinmask;
		} else {
			port->BRR = pinmask;
		}
#endif
		break;
	}

	return ERR_OK;
}

err_t gpio_toggle_state(gpio_pin_t pin) {
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
	pinmask = GPIO_GET_PINMASK(pin);

	// This works for all output modes as well as the pullup/pulldown on inputs
	// with those enabled and has no effect on analog inputs and unbiased inputs,
	// so there's no need to go through all the stuff we do when setting a state
	//port->BSRR = ((pinmask & port->ODR) == 0) ? pinmask : pinmask << GPIO_BSRR_BR0_Pos;
	if ((pinmask & port->ODR) == 0) {
		port->BSRR = pinmask;
	} else {
		port->BRR = pinmask;
	}

	return ERR_OK;
}
err_t gpio_toggle_input_state(gpio_pin_t pin) {
	return gpio_toggle_state(pin);
}
err_t gpio_toggle_output_state(gpio_pin_t pin) {
	return gpio_toggle_state(pin);
}

gpio_state_t gpio_get_state(gpio_pin_t pin) {
	GPIO_TypeDef* port;
	uint32_t pinmask, pinno, modemask, mpinno;
	uint_fast16_t check;

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
	pinno = GPIO_GET_PINNO(pin);

	if (pinno < 8) {
		mpinno = pinno * 4U;
		modemask = GATHER_BITS(port->CRL, 0b1111U, mpinno);
	} else {
		mpinno = (pinno - 8U) * 4U;
		modemask = GATHER_BITS(port->CRH, 0b1111U, mpinno);
	}

	switch (modemask) {
	case (MODE_PP):
	case (MODE_PP_AF):
	case (MODE_OD):
	case (MODE_OD_AF):
		check = port->ODR;
		break;

	case (MODE_INF):
	case (MODE_INP):
		check = port->IDR;
		break;

	//case (MODE_AN):
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
	uint32_t modemask, pinno, mpinno;

	pinno = GPIO_GET_PINNO(pin);

	if (pinno < 8) {
		mpinno = pinno * 4U;
	} else {
		mpinno = (pinno - 8U) * 4U;
	}

	if (pinno < 8) {
		modemask = GATHER_BITS(port->CRL, 0b1111U, mpinno);
	} else {
		modemask = GATHER_BITS(port->CRH, 0b1111U, mpinno);
	}

	switch (modemask) {
	case (MODE_INF):
	case (MODE_INP):
	// Reading the input register on an OD pin will correctly read it's present
	// state, which may be desired
	case (MODE_OD):
	case (MODE_OD_AF):
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
		return ERR_BADARG;
	}
#endif
	pinmask = GPIO_GET_PINMASK(pin);

#if ! uHAL_SKIP_INIT_CHECKS
	uint32_t modemask, pinno, mpinno;

	pinno = GPIO_GET_PINNO(pin);

	if (pinno < 8) {
		mpinno = pinno * 4U;
	} else {
		mpinno = (pinno - 8U) * 4U;
	}

	if (pinno < 8) {
		modemask = GATHER_BITS(port->CRL, 0b1111U, mpinno);
	} else {
		modemask = GATHER_BITS(port->CRH, 0b1111U, mpinno);
	}

	switch (modemask) {
	case (MODE_PP):
	case (MODE_PP_AF):
	case (MODE_OD):
	case (MODE_OD_AF):
		break;

	default:
		return GPIO_FLOAT;
	}
#endif

	return (BIT_IS_SET(port->ODR, pinmask)) ? GPIO_HIGH : GPIO_LOW;
}

#endif // INCLUDED_BY_GPIO_C
