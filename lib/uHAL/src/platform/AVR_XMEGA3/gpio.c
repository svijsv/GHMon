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
//

#include "gpio.h"
#include "system.h"

#include <avr/io.h>
#include <avr/power.h>

// If 1, setting an output pin to GPIO_FLOAT will toggle it
#if defined(uHAL_TOGGLE_GPIO_OUTPUT_WITH_FLOAT) && uHAL_TOGGLE_GPIO_OUTPUT_WITH_FLOAT > 0
# define FLOAT_TOGGLES_OUTPUT 1
#else
# define FLOAT_TOGGLES_OUTPUT 0
#endif

// Turn the input buffer off in reset state to save power
#define PINCTRL_RESET (PORT_ISC_INPUT_DISABLE_gc)

#define PINx_CTRL(_PORTx, _pinno) ((&(_PORTx)->PIN0CTRL)[_pinno])
//
// Default all digital and analog pins to HiZ (input without pullups) and
// turn off the digital input buffer to save power
static void INIT_PORT(PORT_t *port) {
	port->DIR = 0;
	port->OUT = 0;
	port->PIN0CTRL = PINCTRL_RESET;
	port->PIN1CTRL = PINCTRL_RESET;
	port->PIN2CTRL = PINCTRL_RESET;
	port->PIN3CTRL = PINCTRL_RESET;
	port->PIN4CTRL = PINCTRL_RESET;
	port->PIN5CTRL = PINCTRL_RESET;
	port->PIN6CTRL = PINCTRL_RESET;
	port->PIN7CTRL = PINCTRL_RESET;
	port->INTFLAGS = 0xFFU;

	return;
}
static void INIT_PIN(PORT_t *port, uint8_t pinmask, uint8_t pinno) {
	port->DIRCLR = pinmask;
	port->OUTCLR = pinmask;
	PINx_CTRL(port, pinno) = PINCTRL_RESET;
	port->INTFLAGS = pinmask;
}

static PORT_t* gpio_get_port(gpio_pin_t pin) {
	gpio_pin_t port;

	port = GPIO_GET_PORTMASK(pin);

	switch(port) {
#if HAVE_GPIO_PORTA
	case GPIO_PORTA_MASK:
		return &PORTA;
		break;
#endif
#if HAVE_GPIO_PORTB
	case GPIO_PORTB_MASK:
		return &PORTB;
		break;
#endif
#if HAVE_GPIO_PORTC
	case GPIO_PORTC_MASK:
		return &PORTC;
		break;
#endif
#if HAVE_GPIO_PORTD
	case GPIO_PORTD_MASK:
		return &PORTD;
		break;
#endif
#if HAVE_GPIO_PORTE
	case GPIO_PORTE_MASK:
		return &PORTE;
		break;
#endif
#if HAVE_GPIO_PORTF
	case GPIO_PORTF_MASK:
		return &PORTF;
		break;
#endif
	}

	return NULL;
}
/*
// In theory the virtual ports are useful but between the PINxCTRL registers (which
// don't have virtual versions) and the SET/CLR/TGL registers there isn't really a
// use case at present.
static VPORT_t* gpio_get_vport(gpio_pin_t pin) {
	gpio_pin_t port;

	port = GPIO_GET_PORTMASK(pin);

	switch(port) {
#if HAVE_GPIO_PORTA
	case GPIO_PORTA_MASK:
		return &VPORTA;
		break;
#endif
#if HAVE_GPIO_PORTB
	case GPIO_PORTB_MASK:
		return &VPORTB;
		break;
#endif
#if HAVE_GPIO_PORTC
	case GPIO_PORTC_MASK:
		return &VPORTC;
		break;
#endif
#if HAVE_GPIO_PORTD
	case GPIO_PORTD_MASK:
		return &VPORTD;
		break;
#endif
#if HAVE_GPIO_PORTE
	case GPIO_PORTE_MASK:
		return &VPORTE;
		break;
#endif
#if HAVE_GPIO_PORTF
	case GPIO_PORTF_MASK:
		return &VPORTF;
		break;
#endif
	}

	return NULL;
}
*/
void gpio_clear_outbit(gpio_pin_t pin) {
	uint8_t pinmask;
	PORT_t *PORTx;

	pinmask = GPIO_GET_PINMASK(pin);
	PORTx = gpio_get_port(pin);
	SET_BIT(PORTx->OUTCLR, pinmask);

	return;
}

void gpio_init(void) {
	uint8_t reg;

#if HAVE_GPIO_PORTA
	INIT_PORT(&PORTA);
#endif
#if HAVE_GPIO_PORTB
	INIT_PORT(&PORTB);
#endif
#if HAVE_GPIO_PORTC
	INIT_PORT(&PORTC);
#endif
#if HAVE_GPIO_PORTD
	INIT_PORT(&PORTD);
#endif
#if HAVE_GPIO_PORTE
	INIT_PORT(&PORTE);
#endif
#if HAVE_GPIO_PORTF
	INIT_PORT(&PORTF);
#endif

//
// Remap PORTMUX.CTRLB pins
	reg = 0;
#if GPIO_REMAP_I2C0 && defined(PORTMUX_TWI0_bm)
	DEBUG_CPP_MSG("Remapping I2C0")
	SET_BIT(reg, PORTMUX_TWI0_bm);
#endif
#if GPIO_REMAP_UART0 && defined(PORTMUX_USART0_bm)
	DEBUG_CPP_MSG("Remapping UART0")
	SET_BIT(reg, PORTMUX_USART0_bm);
#endif
#if GPIO_REMAP_SPI0 && defined(PORTMUX_SPI0_bm)
	DEBUG_CPP_MSG("Remapping SPI0")
	SET_BIT(reg, PORTMUX_SPI0_bm);
#endif
	PORTMUX.CTRLB = reg;

//
// Remap PORTMUX.CTRLC pins
	reg = 0;
#if GPIO_REMAP_TCA0W0 && defined(PORTMUX_TCA00_bm)
	DEBUG_CPP_MSG("Remapping TCA0W0")
	SET_BIT(reg, PORTMUX_TCA00_bm);
#endif
#if GPIO_REMAP_TCA0W1 && defined(PORTMUX_TCA01_bm)
	DEBUG_CPP_MSG("Remapping TCA0W1")
	SET_BIT(reg, PORTMUX_TCA01_bm);
#endif
#if GPIO_REMAP_TCA0W2 && defined(PORTMUX_TCA02_bm)
	DEBUG_CPP_MSG("Remapping TCA0W2")
	SET_BIT(reg, PORTMUX_TCA02_bm);
#endif
#if GPIO_REMAP_TCA0W3 && defined(PORTMUX_TCA03_bm)
	DEBUG_CPP_MSG("Remapping TCA0W3")
	SET_BIT(reg, PORTMUX_TCA03_bm);
#endif
	PORTMUX.CTRLC = reg;

//
// Remap PORTMUX.CTRLD pins
	reg = 0;
#if GPIO_REMAP_TCB0 && defined(PORTMUX_TCB0_bm)
	DEBUG_CPP_MSG("Remapping TCB0")
	SET_BIT(reg, PORTMUX_TCB0_bm);
#endif
	PORTMUX.CTRLD = reg;

	return;
}

err_t gpio_set_state(gpio_pin_t pin, gpio_state_t new_state) {
	uint8_t pinmask;
	PORT_t *PORTx;

	assert(GPIO_PIN_IS_VALID(pin));
	if (!uHAL_SKIP_INVALID_ARG_CHECKS) {
		if (!GPIO_PIN_IS_VALID(pin)) {
			return ERR_BADARG;
		}
	}

	pinmask = GPIO_GET_PINMASK(pin);
	PORTx = gpio_get_port(pin);
	if (!uHAL_SKIP_INVALID_ARG_CHECKS) {
		if (PORTx == NULL) {
			return ERR_BADARG;
		}
	}
	if (BIT_IS_SET(PORTx->DIR, pinmask)) {
		switch (new_state) {
		case GPIO_HIGH:
			PORTx->OUTSET = pinmask;
			break;
		case GPIO_LOW:
			PORTx->OUTCLR = pinmask;
			break;
		case GPIO_FLOAT:
#if FLOAT_TOGGLES_OUTPUT
			PORTx->OUTTGL = pinmask;
#endif
			break;
		}
	} else {
		uint8_t pinno = GPIO_GET_PINNO(pin);

		switch (new_state) {
		case GPIO_HIGH:
			SET_BIT(PINx_CTRL(PORTx, pinno), PORT_PULLUPEN_bm);
			break;
		//case GPIO_FLOAT:
		//case GPIO_LOW:
		default:
			CLEAR_BIT(PINx_CTRL(PORTx, pinno), PORT_PULLUPEN_bm);
			break;
		}
	}

	return ERR_OK;
}
err_t gpio_set_input_state(gpio_pin_t pin, gpio_state_t new_state) {
	uint8_t pinno;
	PORT_t *PORTx;

	assert(GPIO_PIN_IS_VALID(pin));
	if (!uHAL_SKIP_INVALID_ARG_CHECKS) {
		if (!GPIO_PIN_IS_VALID(pin)) {
			return ERR_BADARG;
		}
	}

	pinno = GPIO_GET_PINNO(pin);
	PORTx = gpio_get_port(pin);
	if (!uHAL_SKIP_INVALID_ARG_CHECKS) {
		if (PORTx == NULL) {
			return ERR_BADARG;
		}
	}
	if (!uHAL_SKIP_INIT_CHECKS) {
		uint8_t pinmask = GPIO_GET_PINMASK(pin);

	if (BIT_IS_SET(PORTx->DIR, pinmask)) {
			return ERR_INIT;
		}
	}

	switch (new_state) {
	case GPIO_HIGH:
		SET_BIT(PINx_CTRL(PORTx, pinno), PORT_PULLUPEN_bm);
		break;
	//case GPIO_FLOAT:
	//case GPIO_LOW:
	default:
		CLEAR_BIT(PINx_CTRL(PORTx, pinno), PORT_PULLUPEN_bm);
		break;
	}

	return ERR_OK;
}
err_t gpio_set_output_state(gpio_pin_t pin, gpio_state_t new_state) {
	uint8_t pinmask;
	PORT_t *PORTx;

	assert(GPIO_PIN_IS_VALID(pin));
	if (!uHAL_SKIP_INVALID_ARG_CHECKS) {
		if (!GPIO_PIN_IS_VALID(pin)) {
			return ERR_BADARG;
		}
	}

	pinmask = GPIO_GET_PINMASK(pin);
	PORTx = gpio_get_port(pin);
	if (!uHAL_SKIP_INVALID_ARG_CHECKS) {
		if (PORTx == NULL) {
			return ERR_BADARG;
		}
	}
	if (!uHAL_SKIP_INIT_CHECKS) {
		if (!BIT_IS_SET(PORTx->DIR, pinmask)) {
			return ERR_INIT;
		}
	}

	switch (new_state) {
	case GPIO_HIGH:
		PORTx->OUTSET = pinmask;
		break;
	case GPIO_LOW:
		PORTx->OUTCLR = pinmask;
		break;
	case GPIO_FLOAT:
#if FLOAT_TOGGLES_OUTPUT
		PORTx->OUTTGL = pinmask;
#endif
		break;
	}

	return ERR_OK;
}

err_t gpio_toggle_state(gpio_pin_t pin) {
	uint8_t pinmask;
	PORT_t *PORTx;

	assert(GPIO_PIN_IS_VALID(pin));
	if (!uHAL_SKIP_INVALID_ARG_CHECKS) {
		if (!GPIO_PIN_IS_VALID(pin)) {
			return ERR_BADARG;
		}
	}

	pinmask = GPIO_GET_PINMASK(pin);
	PORTx = gpio_get_port(pin);
	if (!uHAL_SKIP_INVALID_ARG_CHECKS) {
		if (PORTx == NULL) {
			return ERR_BADARG;
		}
	}
	if (BIT_IS_SET(PORTx->DIR, pinmask)) {
		PORTx->OUTTGL = pinmask;
	} else {
		uint8_t pinno = GPIO_GET_PINNO(pin);
		TOGGLE_BIT(PINx_CTRL(PORTx, pinno), PORT_PULLUPEN_bm);
	}

	return ERR_OK;
}
err_t gpio_toggle_input_state(gpio_pin_t pin) {
	uint8_t pinno;
	PORT_t *PORTx;

	assert(GPIO_PIN_IS_VALID(pin));
	if (!uHAL_SKIP_INVALID_ARG_CHECKS) {
		if (!GPIO_PIN_IS_VALID(pin)) {
			return ERR_BADARG;
		}
	}

	pinno = GPIO_GET_PINNO(pin);
	PORTx = gpio_get_port(pin);
	if (!uHAL_SKIP_INVALID_ARG_CHECKS) {
		if (PORTx == NULL) {
			return ERR_BADARG;
		}
	}
	if (!uHAL_SKIP_INIT_CHECKS) {
		uint8_t pinmask = GPIO_GET_PINMASK(pin);

	if (BIT_IS_SET(PORTx->DIR, pinmask)) {
			return ERR_INIT;
		}
	}

	TOGGLE_BIT(PINx_CTRL(PORTx, pinno), PORT_PULLUPEN_bm);

	return ERR_OK;
}
err_t gpio_toggle_output_state(gpio_pin_t pin) {
	uint8_t pinmask;
	PORT_t *PORTx;

	assert(GPIO_PIN_IS_VALID(pin));
	if (!uHAL_SKIP_INVALID_ARG_CHECKS) {
		if (!GPIO_PIN_IS_VALID(pin)) {
			return ERR_BADARG;
		}
	}

	pinmask = GPIO_GET_PINMASK(pin);
	PORTx = gpio_get_port(pin);
	if (!uHAL_SKIP_INVALID_ARG_CHECKS) {
		if (PORTx == NULL) {
			return ERR_BADARG;
		}
	}
	if (!uHAL_SKIP_INIT_CHECKS) {
		if (!BIT_IS_SET(PORTx->DIR, pinmask)) {
			return ERR_INIT;
		}
	}

	PORTx->OUTTGL = pinmask;

	return ERR_OK;
}

gpio_state_t gpio_get_state(gpio_pin_t pin) {
	uint8_t pinmask;
	PORT_t *PORTx;

	assert(GPIO_PIN_IS_VALID(pin));
	if (!uHAL_SKIP_INVALID_ARG_CHECKS) {
		if (!GPIO_PIN_IS_VALID(pin)) {
			return GPIO_FLOAT;
		}
	}

	pinmask = GPIO_GET_PINMASK(pin);
	PORTx = gpio_get_port(pin);
	if (!uHAL_SKIP_INVALID_ARG_CHECKS) {
		if (PORTx == NULL) {
			return GPIO_FLOAT;
		}
	}

	if (BIT_IS_SET(PORTx->DIR, pinmask)) {
		return BIT_IS_SET(PORTx->OUT, pinmask) ? GPIO_HIGH : GPIO_LOW;
	} else {
		uint8_t tmp = PINx_CTRL(PORTx, GPIO_GET_PINNO(pin));
		tmp = SELECT_BITS(tmp, PORT_ISC_gm);
		if (tmp != PORT_ISC_INPUT_DISABLE_gc) {
			return BIT_IS_SET(PORTx->IN, pinmask) ? GPIO_HIGH : GPIO_LOW;
		}
	}
	return GPIO_FLOAT;
}
gpio_state_t gpio_get_input_state(gpio_pin_t pin) {
	uint8_t pinmask, tmp;
	PORT_t *PORTx;

	assert(GPIO_PIN_IS_VALID(pin));
	if (!uHAL_SKIP_INVALID_ARG_CHECKS) {
		if (!GPIO_PIN_IS_VALID(pin)) {
			return GPIO_FLOAT;
		}
	}

	pinmask = GPIO_GET_PINMASK(pin);
	PORTx = gpio_get_port(pin);
	if (!uHAL_SKIP_INVALID_ARG_CHECKS) {
		if (PORTx == NULL) {
			return GPIO_FLOAT;
		}
	}
	if (!uHAL_SKIP_INIT_CHECKS) {
		if (BIT_IS_SET(PORTx->DIR, pinmask)) {
			return ERR_INIT;
		}
	}

	tmp = PINx_CTRL(PORTx, GPIO_GET_PINNO(pin));
	tmp = SELECT_BITS(tmp, PORT_ISC_gm);
	if (tmp == PORT_ISC_INPUT_DISABLE_gc) {
		return GPIO_FLOAT;
	}
	return BIT_IS_SET(PORTx->IN, pinmask) ? GPIO_HIGH : GPIO_LOW;
}
gpio_state_t gpio_get_output_state(gpio_pin_t pin) {
	uint8_t pinmask;
	PORT_t *PORTx;

	assert(GPIO_PIN_IS_VALID(pin));
	if (!uHAL_SKIP_INVALID_ARG_CHECKS) {
		if (!GPIO_PIN_IS_VALID(pin)) {
			return GPIO_FLOAT;
		}
	}

	pinmask = GPIO_GET_PINMASK(pin);
	PORTx = gpio_get_port(pin);
	if (!uHAL_SKIP_INVALID_ARG_CHECKS) {
		if (PORTx == NULL) {
			return GPIO_FLOAT;
		}
	}
	if (!uHAL_SKIP_INIT_CHECKS) {
		if (!BIT_IS_SET(PORTx->DIR, pinmask)) {
			return ERR_INIT;
		}
	}

	return BIT_IS_SET(PORTx->OUT, pinmask) ? GPIO_HIGH : GPIO_LOW;
}

err_t gpio_quickread_prepare(gpio_quick_t *qpin, gpio_pin_t pin) {
	PORT_t *PORTx;

	assert(qpin != NULL);
	assert(GPIO_PIN_IS_VALID(pin));
	if (!uHAL_SKIP_INVALID_ARG_CHECKS) {
		if ((!GPIO_PIN_IS_VALID(pin)) || (qpin == NULL)) {
			return ERR_BADARG;
		}
	}

	qpin->mask = GPIO_GET_PINMASK(pin);

	PORTx = gpio_get_port(pin);
	if (!uHAL_SKIP_INVALID_ARG_CHECKS) {
		if (PORTx == NULL) {
			return ERR_BADARG;
		}
	}
	qpin->port = &PORTx->IN;

	return ERR_OK;
}

err_t gpio_set_mode(gpio_pin_t pin, gpio_mode_t mode, gpio_state_t istate) {
	uint8_t pinmask, pinno, reg;
	PORT_t *PORTx;

	assert(GPIO_PIN_IS_VALID(pin));
	if (!uHAL_SKIP_INVALID_ARG_CHECKS) {
		if (!GPIO_PIN_IS_VALID(pin)) {
			return ERR_BADARG;
		}
	}

	pinmask = GPIO_GET_PINMASK(pin);
	pinno   = GPIO_GET_PINNO(pin);

	PORTx = gpio_get_port(pin);
	if (!uHAL_SKIP_INVALID_ARG_CHECKS) {
		if (PORTx == NULL) {
			return ERR_BADARG;
		}
	}

	switch (mode) {
	case GPIO_MODE_IN:
		reg = PORT_ISC_INTDISABLE_gc;
		if (istate == GPIO_HIGH) {
			SET_BIT(reg, PORT_PULLUPEN_bm);
		}
		PINx_CTRL(PORTx, pinno) = reg;
		PORTx->DIRCLR = pinmask;
		break;

	case GPIO_MODE_PP:
	//case GPIO_MODE_OD:
		PINx_CTRL(PORTx, pinno) = PORT_ISC_INPUT_DISABLE_gc;
		PORTx->DIRSET = pinmask;
		if (istate == GPIO_HIGH) {
			PORTx->OUTSET = pinmask;
		} else {
			PORTx->OUTCLR = pinmask;
		}
		break;

	case GPIO_MODE_AIN:
	case GPIO_MODE_HiZ:
		PINx_CTRL(PORTx, pinno) = PORT_ISC_INPUT_DISABLE_gc;
		PORTx->DIRCLR = pinmask;
		PORTx->OUTCLR = pinmask;
		break;

	case GPIO_MODE_RESET:
		INIT_PIN(PORTx, pinmask, pinno);
		break;
	}
	//PORTx->INTFLAGS = pinmask;

	return ERR_OK;
}
gpio_mode_t gpio_get_mode(gpio_pin_t pin) {
	uint8_t pinmask, pinno;
	PORT_t *PORTx;

	assert(GPIO_PIN_IS_VALID(pin));
	if (!uHAL_SKIP_INVALID_ARG_CHECKS) {
		if (!GPIO_PIN_IS_VALID(pin)) {
			return GPIO_MODE_RESET;
		}
	}

	pinmask = GPIO_GET_PINMASK(pin);
	PORTx = gpio_get_port(pin);
	if (!uHAL_SKIP_INVALID_ARG_CHECKS) {
		if (PORTx == NULL) {
			return GPIO_MODE_RESET;
		}
	}

	if (BIT_IS_SET(PORTx->DIR, pinmask)) {
		return GPIO_MODE_PP;
	}
	pinno = GPIO_GET_PINNO(pin);
	if (SELECT_BITS(PINx_CTRL(PORTx, pinno), PORT_ISC_gm) == PORT_ISC_INPUT_DISABLE_gc) {
		return GPIO_MODE_AIN;
	}

	return GPIO_MODE_IN;
}

err_t gpio_listen_init(gpio_listen_t *handle, const gpio_listen_cfg_t *conf) {
	PORT_t *portx;
	gpio_pin_t pin;

	assert(handle != NULL);
	assert(conf != NULL);
	assert(GPIO_PIN_IS_VALID(conf->pin));

	if (!uHAL_SKIP_INVALID_ARG_CHECKS) {
		if (handle == NULL || conf == NULL || !GPIO_PIN_IS_VALID(conf->pin)) {
			return ERR_BADARG;
		}
	}

	pin = conf->pin;
	portx = gpio_get_port(pin);
	if (!uHAL_SKIP_INVALID_ARG_CHECKS) {
		if (portx == NULL) {
			return ERR_BADARG;
		}
	}
	handle->pinctrl = &PINx_CTRL(portx, GPIO_GET_PINNO(pin));
	handle->old_trigger = SELECT_BITS(*handle->pinctrl, PORT_ISC_gm);

	switch (SELECT_BITS(conf->trigger, GPIO_TRIGGER_RISING|GPIO_TRIGGER_FALLING)) {
	case GPIO_TRIGGER_RISING|GPIO_TRIGGER_FALLING:
		handle->trigger = PORT_ISC_BOTHEDGES_gc;
		break;
	case GPIO_TRIGGER_RISING:
		handle->trigger = PORT_ISC_RISING_gc;
		break;
	case GPIO_TRIGGER_FALLING:
		handle->trigger = PORT_ISC_FALLING_gc;
		break;
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	default:
		return ERR_BADARG;
#endif
	}

	return ERR_OK;
}
err_t gpio_listen_on(gpio_listen_t *handle) {
	assert(handle != NULL);
	assert(handle->pinctrl != NULL);

	if (!uHAL_SKIP_INVALID_ARG_CHECKS) {
		if (handle == NULL) {
			return ERR_BADARG;
		}
		if (handle->pinctrl == NULL) {
			return ERR_INIT;
		}
	}

	MODIFY_BITS(*handle->pinctrl, PORT_ISC_gm, handle->trigger);
	return ERR_OK;
}
err_t gpio_listen_off(gpio_listen_t *handle) {
	assert(handle != NULL);
	assert(handle->pinctrl != NULL);

	if (!uHAL_SKIP_INVALID_ARG_CHECKS) {
		if (handle == NULL) {
			return ERR_BADARG;
		}
		if (handle->pinctrl == NULL) {
			return ERR_INIT;
		}
	}

	MODIFY_BITS(*handle->pinctrl, PORT_ISC_gm, handle->old_trigger);
	return ERR_OK;
}
bool gpio_is_listening(gpio_pin_t pin) {
	PORT_t *portx;

	assert(GPIO_PIN_IS_VALID(pin));

	portx = gpio_get_port(pin);
	if (!uHAL_SKIP_INVALID_ARG_CHECKS) {
		if (portx == NULL) {
			return false;
		}
	}
	switch (SELECT_BITS(PINx_CTRL(portx, GPIO_GET_PINNO(pin)), PORT_ISC_gm)) {
	case PORT_ISC_BOTHEDGES_gc:
	case PORT_ISC_RISING_gc:
	case PORT_ISC_FALLING_gc:
		return true;
	}

	return false;
}
