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
// gpio.c
// Manage the GPIO peripheral
// NOTES:
//    Per DM00315319, pins use less power in analog input mode than in any
//    other (especially compared to floating input) so that's the default
//    mode
//
//    The STM32F401 manual says reset for port A MODER is 0x0C00 0000 but
//    the other manuals I've checked all give it as 0xA800 0000 which
//    makes more sense since that would make the JTAG pins AF mode rather
//    than leaving 2 of them as GP and one as analog
//
#include "gpio.h"
#include "system.h"
#include "spi.h"


// If 1, setting an output pin to GPIO_FLOAT will toggle it
#if defined(uHAL_TOGGLE_GPIO_OUTPUT_WITH_FLOAT) && uHAL_TOGGLE_GPIO_OUTPUT_WITH_FLOAT > 0
# define FLOAT_TOGGLES_OUTPUT 1
#else
# define FLOAT_TOGGLES_OUTPUT 0
#endif

#if HAVE_GPIO_PORTA
 DEBUG_CPP_MSG("Have GPIO port A")
#endif
#if HAVE_GPIO_PORTB
 DEBUG_CPP_MSG("Have GPIO port B")
#endif
#if HAVE_GPIO_PORTC
 DEBUG_CPP_MSG("Have GPIO port C")
#endif
#if HAVE_GPIO_PORTD
 DEBUG_CPP_MSG("Have GPIO port D")
#endif
#if HAVE_GPIO_PORTE
 DEBUG_CPP_MSG("Have GPIO port E")
#endif
#if HAVE_GPIO_PORTF
 DEBUG_CPP_MSG("Have GPIO port F")
#endif
#if HAVE_GPIO_PORTG
 DEBUG_CPP_MSG("Have GPIO port G")
#endif
#if HAVE_GPIO_PORTH
 DEBUG_CPP_MSG("Have GPIO port H")
#endif
#if HAVE_GPIO_PORTI
 DEBUG_CPP_MSG("Have GPIO port I")
#endif
#if HAVE_GPIO_PORTJ
 DEBUG_CPP_MSG("Have GPIO port J")
#endif
#if HAVE_GPIO_PORTK
 DEBUG_CPP_MSG("Have GPIO port K")
#endif
#if HAVE_GPIO_PORTL
 DEBUG_CPP_MSG("Have GPIO port L")
#endif
#if HAVE_GPIO_PORTM
 DEBUG_CPP_MSG("Have GPIO port M")
#endif
#if HAVE_GPIO_PORTN
 DEBUG_CPP_MSG("Have GPIO port N")
#endif
#if HAVE_GPIO_PORTO
 DEBUG_CPP_MSG("Have GPIO port O")
#endif


// According to the STM32F4 errata sheet, the last bit received in SPI master
// mode can be corrupted if the GPIO clock is set too slow for the APB bus
// clock. Bus speeds below 28MHz would seem to be unaffected by this.
// OUTPUT_x is defined in the platform-specific GPIO files
#if uHAL_USE_SPI && ! defined(uHAL_GPIO_SPEED)
# if SPIx_BUSFREQ <= 28000000
#  define OUTPUT OUTPUT_SLOW
# elif SPIx_BUSFREQ <= (28000000 * 2)
#  define OUTPUT OUTPUT_MEDIUM
# elif SPIx_BUSFREQ <= (28000000 * 3)
#  define OUTPUT OUTPUT_FAST
# else
#  define OUTPUT OUTPUT_VERY_FAST
# endif
#elif defined(uHAL_GPIO_SPEED)
# define OUTPUT uHAL_GPIO_SPEED
#else
# define OUTPUT OUTPUT_SLOW
#endif


#define LISTEN_HANDLE_IS_OK(_lh_) (((_lh_) != NULL && GPIO_PIN_IS_VALID((_lh_)->pin)))


static void port_reset(GPIO_TypeDef *port);
static void gpio_platform_init(void);


#define INCLUDED_BY_GPIO_C 1
#if HAVE_STM32F1_GPIO
# include "gpio_STM32F1.c"
#else
# include "gpio_STM32Fx.c"
#endif

void gpio_init(void) {
	rcc_periph_t ports = 0;

#if HAVE_GPIO_PORTA
	ports |= RCC_PERIPH_GPIOA;
#endif
#if HAVE_GPIO_PORTB
	ports |= RCC_PERIPH_GPIOB;
#endif
#if HAVE_GPIO_PORTC
	ports |= RCC_PERIPH_GPIOC;
#endif
#if HAVE_GPIO_PORTD
	ports |= RCC_PERIPH_GPIOD;
#endif
#if HAVE_GPIO_PORTE
	ports |= RCC_PERIPH_GPIOE;
#endif
#if HAVE_GPIO_PORTF
	ports |= RCC_PERIPH_GPIOF;
#endif
#if HAVE_GPIO_PORTG
	ports |= RCC_PERIPH_GPIOG;
#endif
#if HAVE_GPIO_PORTH
	ports |= RCC_PERIPH_GPIOH;
#endif
#if HAVE_GPIO_PORTI
	ports |= RCC_PERIPH_GPIOI;
#endif
#if HAVE_GPIO_PORTJ
	ports |= RCC_PERIPH_GPIOJ;
#endif
#if HAVE_GPIO_PORTK
	ports |= RCC_PERIPH_GPIOK;
#endif
#if HAVE_GPIO_PORTL
	ports |= RCC_PERIPH_GPIOL;
#endif
#if HAVE_GPIO_PORTM
	ports |= RCC_PERIPH_GPIOM;
#endif
#if HAVE_GPIO_PORTN
	ports |= RCC_PERIPH_GPION;
#endif
#if HAVE_GPIO_PORTO
	ports |= RCC_PERIPH_GPIOO;
#endif

	clock_init(ports);

#if HAVE_GPIO_PORTA
	port_reset(GPIOA);
#endif
#if HAVE_GPIO_PORTB
	port_reset(GPIOB);
#endif
#if HAVE_GPIO_PORTC
	port_reset(GPIOC);
#endif
#if HAVE_GPIO_PORTD
	port_reset(GPIOD);
#endif
#if HAVE_GPIO_PORTE
	port_reset(GPIOE);
#endif
#if HAVE_GPIO_PORTF
	port_reset(GPIOF);
#endif
#if HAVE_GPIO_PORTG
	port_reset(GPIOG);
#endif
#if HAVE_GPIO_PORTH
	port_reset(GPIOH);
#endif
#if HAVE_GPIO_PORTI
	port_reset(GPIOI);
#endif
#if HAVE_GPIO_PORTJ
	port_reset(GPIOJ);
#endif
#if HAVE_GPIO_PORTK
	port_reset(GPIOK);
#endif
#if HAVE_GPIO_PORTL
	port_reset(GPIOL);
#endif
#if HAVE_GPIO_PORTM
	port_reset(GPIOM);
#endif
#if HAVE_GPIO_PORTN
	port_reset(GPION);
#endif
#if HAVE_GPIO_PORTO
	port_reset(GPIOO);
#endif

	gpio_platform_init();

	return;
}

static uint32_t get_pinno_irqn(gpio_pin_t pinno) {
	uint32_t irqn;

	switch (pinno) {
	case 0:
		irqn = EXTI0_IRQn;
		break;
	case 1:
		irqn = EXTI1_IRQn;
		break;
	case 2:
		irqn = EXTI2_IRQn;
		break;
	case 3:
		irqn = EXTI3_IRQn;
		break;
	case 4:
		irqn = EXTI4_IRQn;
		break;
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
		irqn = EXTI9_5_IRQn;
		break;
	/*
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
	case 15:
	*/
	default:
		irqn = EXTI15_10_IRQn;
		break;

	/*
	default:
		return ERR_NOTSUP;
	*/
	}

	return irqn;
}
static err_t _gpio_listen_off(gpio_pin_t pin, uint32_t irqn) {
	NVIC_DisableIRQ(irqn);
	// The EXTI pending bit is cleared by writing 1
	SET_BIT(EXTI->PR, GPIO_GET_PINMASK(pin));
	NVIC_ClearPendingIRQ(irqn);

	return ERR_OK;
}
err_t gpio_listen_init(gpio_listen_t *handle, const gpio_listen_cfg_t *conf) {

	gpio_pin_t pin, pinno, pin_mask;
	uint32_t port_mask;
	uint32_t irqn;
	__IO uint32_t *exticr = NULL;
	bool redisable_clock;

	assert(handle != NULL);
	assert(conf != NULL);
	assert(GPIO_PIN_IS_VALID(conf->pin));
	assert(BIT_IS_SET(conf->trigger, GPIO_TRIGGER_RISING|GPIO_TRIGGER_FALLING));

#if ! uHAL_SKIP_INIT_CHECKS
#endif
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (handle == NULL || conf == NULL || !GPIO_PIN_IS_VALID(conf->pin) || !BIT_IS_SET(conf->trigger, GPIO_TRIGGER_RISING|GPIO_TRIGGER_FALLING)) {
		return ERR_BADARG;
	}
#endif

	pin = conf->pin;
	handle->pin = pin;

	port_mask = GPIO_GET_PORTNO(pin) - 1U;
	pinno = GPIO_GET_PINNO(pin);
	pin_mask = GPIO_GET_PINMASK(pin);

	redisable_clock = !clock_is_enabled(EXTI_PREG_CLOCKEN);
	if (redisable_clock) {
		clock_enable(EXTI_PREG_CLOCKEN);
	}

	// Do this *before* modifying pinno below
	irqn = get_pinno_irqn(pinno);
	_gpio_listen_off(pin, irqn);

	switch (pinno) {
	case 0:
	case 1:
	case 2:
	case 3:
		exticr = &(EXTI_PREG->EXTICR[0]);
		break;
	case 4:
	case 5:
	case 6:
	case 7:
		pinno -= 4;
		exticr = &(EXTI_PREG->EXTICR[1]);
		break;
	case 8:
	case 9:
	case 10:
	case 11:
		pinno -= 8;
		exticr = &(EXTI_PREG->EXTICR[2]);
		break;
	/*
	case 12:
	case 13:
	case 14:
	case 15:
	*/
	default:
		pinno -= 12;
		exticr = &(EXTI_PREG->EXTICR[3]);
		break;

	/*
	default:
		return ERR_NOTSUP;
	*/
	}

	pinno *= 4U;
	port_mask <<= pinno;
	// Set the interrupt on line 'pinno' to the pin's port
	MODIFY_BITS(*exticr, 0b1111U << pinno, port_mask);

	// Make sure there's no currently-enabled interrupt
	CLEAR_BIT(EXTI->RTSR, pin_mask);
	CLEAR_BIT(EXTI->FTSR, pin_mask);

	// Unmask the interrupt
	SET_BIT(EXTI->IMR, pin_mask);

	// Set the rising and/or falling edge trigger
	if (BIT_IS_SET(conf->trigger, GPIO_TRIGGER_RISING)) {
		SET_BIT(EXTI->RTSR, pin_mask);
	}
	if (BIT_IS_SET(conf->trigger, GPIO_TRIGGER_FALLING)) {
		SET_BIT(EXTI->FTSR, pin_mask);
	}

	if (redisable_clock) {
		clock_disable(EXTI_PREG_CLOCKEN);
	}

	NVIC_SetPriority(irqn, GPIO_IRQp);

	_gpio_listen_off(pin, irqn);

	return ERR_OK;
}
err_t gpio_listen_on(gpio_listen_t *handle) {
	gpio_pin_t pin;
	uint32_t irqn;

	assert(LISTEN_HANDLE_IS_OK(handle));
#if ! uHAL_SKIP_INIT_CHECKS
#endif
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (!LISTEN_HANDLE_IS_OK(handle)) {
		return ERR_INIT;
	}
#endif

	pin = handle->pin;

	irqn = get_pinno_irqn(GPIO_GET_PINNO(pin));

	NVIC_ClearPendingIRQ(irqn);
	// The EXTI pending bit is cleared by writing 1
	SET_BIT(EXTI->PR, GPIO_GET_PINMASK(pin));
	NVIC_EnableIRQ(irqn);

	return ERR_OK;
}
err_t gpio_listen_off(gpio_listen_t *handle) {
	assert(LISTEN_HANDLE_IS_OK(handle));
#if ! uHAL_SKIP_INIT_CHECKS
#endif
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (!LISTEN_HANDLE_IS_OK(handle)) {
		return ERR_INIT;
	}
#endif

	return _gpio_listen_off(handle->pin, get_pinno_irqn(GPIO_GET_PINNO(handle->pin)));
}
bool gpio_is_listening(gpio_pin_t pin) {
	uint32_t irqn;

	assert(GPIO_PIN_IS_VALID(pin));
#if ! uHAL_SKIP_INIT_CHECKS
#endif
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (!GPIO_PIN_IS_VALID(pin)) {
		return ERR_BADARG;
	}
#endif

	irqn = get_pinno_irqn(GPIO_GET_PINNO(pin));

	return (NVIC_GetEnableIRQ(irqn) != 0);
}

err_t gpio_quickread_prepare(gpio_quick_t *qpin, gpio_pin_t pin) {
	assert(qpin != NULL);
	assert(GPIO_PIN_IS_VALID(pin));

#if ! uHAL_SKIP_INIT_CHECKS
#endif
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (qpin == NULL || !GPIO_PIN_IS_VALID(pin)) {
		return ERR_BADARG;
	}
#endif

	qpin->mask = GPIO_GET_PINMASK(pin);
	qpin->idr  = &(GPIO_GET_PORT(pin)->IDR);

	return ERR_OK;
}
