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
//    Per DM00315319, pins use less power in analog input mode than in any
//    other (especially compared to floating input) so that's the default
//    mode
//
//    The STM32F401 manual says reset for port A MODER is 0x0C00 0000 but
//    the other manuals I've checked all give it as 0xA800 0000 which
//    makes more sense since that would make the JTAG pins AF mode rather
//    than leaving 2 of them as GP and one as analog
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
// OUTPUT_SLOW is defined in the platform-specific GPIO files
#define OUTPUT OUTPUT_SLOW


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
static void port_reset(GPIO_TypeDef *port);
static void gpio_platform_init(void);


/*
* Interrupt handlers
*/


/*
* Functions
*/
#define INCLUDED_BY_GPIO_C 1
#if USE_STM32F1_GPIO
# include "gpio_STM32F1.c"
#else
# include "gpio_STM32Fx.c"
#endif

void gpio_init(void) {
	rcc_periph_t ports;

	ports = RCC_PERIPH_GPIOA|RCC_PERIPH_GPIOB;
#if defined(GPIOC)
	ports |= RCC_PERIPH_GPIOC;
#endif
#if defined(GPIOH)
	ports |= RCC_PERIPH_GPIOH;
#endif
	clock_init(ports);

	port_reset(GPIOA);
	port_reset(GPIOB);
#if defined(GPIOC)
	port_reset(GPIOC);
#endif
#if defined(GPIOH)
	port_reset(GPIOH);
#endif

	gpio_platform_init();

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
		port->BSRR = pinmask << GPIO_BSRR_BR0_Pos;
		break;
	case GPIO_FLOAT:
		break;
	}

	return;
}
void gpio_toggle_state(pin_t pin) {
	GPIO_TypeDef* port;
	uint32_t pinmask;

	assert(pin != 0);

	port = GPIO_GET_PORT(pin);
	pinmask = GPIO_GET_PINMASK(pin);

	port->BSRR = ((pinmask & port->ODR) == 0) ? pinmask : pinmask << GPIO_BSRR_BR0_Pos;

	return;
}
gpio_state_t gpio_get_state(pin_t pin) {
	GPIO_TypeDef* port;
	uint32_t pinmask;

	assert(pin != 0);

	port = GPIO_GET_PORT(pin);
	pinmask = GPIO_GET_PINMASK(pin);

	return (BIT_IS_SET(port->IDR, pinmask)) ? GPIO_HIGH : GPIO_LOW;
}
void gpio_quickread_prepare(volatile gpio_quick_t *qpin, pin_t pin) {
	assert(qpin != NULL);
	assert(pin != 0);

	qpin->mask = GPIO_GET_PINMASK(pin);
	qpin->idr  = &(GPIO_GET_PORT(pin)->IDR);

	return;
}

#ifdef __cplusplus
 }
#endif
