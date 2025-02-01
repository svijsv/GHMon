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
// NOTES:
//

#include "common.h"

#if uHAL_USE_HIGH_LEVEL_GPIO

err_t input_pin_on(gpio_pin_t pin) {
	gpio_state_t bias = GPIO_FLOAT;

	uHAL_assert(GPIO_PIN_IS_VALID(pin));
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (!GPIO_PIN_IS_VALID(pin)) {
		return ERR_BADARG;
	}
#endif

	if (BIT_IS_SET(pin, GPIO_CTRL_BIAS_INPUT)) {
		if (BIT_IS_SET(pin, GPIO_CTRL_BIAS_LOW)) {
			bias = GPIO_LOW;
		} else {
			bias = GPIO_HIGH;
		}
	}

	return gpio_set_mode(pin, GPIO_MODE_IN, bias);
}
err_t input_pin_off(gpio_pin_t pin) {
	uHAL_assert(GPIO_PIN_IS_VALID(pin));
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (!GPIO_PIN_IS_VALID(pin)) {
		return ERR_BADARG;
	}
#endif

	return gpio_set_mode(pin, GPIO_MODE_RESET, GPIO_FLOAT);
}
err_t input_pin_toggle(gpio_pin_t pin) {
	uHAL_assert(GPIO_PIN_IS_VALID(pin));
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (!GPIO_PIN_IS_VALID(pin)) {
		return ERR_BADARG;
	}
#endif

	if (gpio_get_mode(pin) == GPIO_MODE_IN) {
		return input_pin_off(pin);
	}
	return input_pin_on(pin);
}
bool input_pin_is_on(gpio_pin_t pin) {
	gpio_state_t state;
	bool is_on = false;

	uHAL_assert(GPIO_PIN_IS_VALID(pin));
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (!GPIO_PIN_IS_VALID(pin)) {
		return ERR_BADARG;
	}
#endif

	state = gpio_get_input_state(pin);
	if (BIT_IS_SET(pin, GPIO_CTRL_INVERT)) {
		if (state == GPIO_LOW) {
			is_on = true;
		}
	} else {
		if (state == GPIO_HIGH) {
			is_on = true;
		}
	}
	return is_on;
}

err_t output_pin_on(gpio_pin_t pin) {
	gpio_state_t dir = GPIO_FLOAT;
	gpio_mode_t mode = GPIO_MODE_RESET;

	uHAL_assert(GPIO_PIN_IS_VALID(pin));
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (!GPIO_PIN_IS_VALID(pin)) {
		return ERR_BADARG;
	}
#endif

	if (BIT_IS_SET(pin, GPIO_CTRL_INVERT)) {
		if (!BIT_IS_SET(pin, GPIO_CTRL_TRISTATE_LOW)) {
			mode = GPIO_MODE_PP;
			dir = GPIO_LOW;
		}
	} else {
		if (!BIT_IS_SET(pin, GPIO_CTRL_TRISTATE_HIGH)) {
			mode = GPIO_MODE_PP;
			dir = GPIO_HIGH;
		}
	}

	return gpio_set_mode(pin, mode, dir);
}
err_t output_pin_off(gpio_pin_t pin) {
	gpio_state_t dir = GPIO_FLOAT;
	gpio_mode_t mode = GPIO_MODE_RESET;

	uHAL_assert(GPIO_PIN_IS_VALID(pin));
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (!GPIO_PIN_IS_VALID(pin)) {
		return ERR_BADARG;
	}
#endif

	if (BIT_IS_SET(pin, GPIO_CTRL_INVERT)) {
		if (!BIT_IS_SET(pin, GPIO_CTRL_TRISTATE_HIGH)) {
			mode = GPIO_MODE_PP;
			dir = GPIO_HIGH;
		}
	} else {
		if (!BIT_IS_SET(pin, GPIO_CTRL_TRISTATE_LOW)) {
			mode = GPIO_MODE_PP;
			dir = GPIO_LOW;
		}
	}

	return gpio_set_mode(pin, mode, dir);
}
err_t output_pin_toggle(gpio_pin_t pin) {
	bool is_on = false;

	uHAL_assert(GPIO_PIN_IS_VALID(pin));
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (!GPIO_PIN_IS_VALID(pin)) {
		return ERR_BADARG;
	}
#endif

	switch (gpio_get_mode(pin)) {
	//case GPIO_MODE_RESET:
	//case GPIO_MODE_AIN:
	//case GPIO_MODE_HiZ:
	case GPIO_MODE_HiZ_ALIAS:
		if (BIT_IS_SET(pin, GPIO_CTRL_TRISTATE_HIGH)) {
			is_on = true;
		}
		break;
	case GPIO_MODE_PP:
		if (gpio_get_output_state(pin) == GPIO_HIGH) {
			is_on = true;
		}
		break;
	// Assume an unexpected pin state is 'OFF'
	default:
		break;
	}
	if (BIT_IS_SET(pin, GPIO_CTRL_INVERT)) {
		is_on = !is_on;
	}

	if (is_on) {
		return output_pin_off(pin);
	}
	return output_pin_on(pin);
}
bool output_pin_is_on(gpio_pin_t pin) {
	bool is_on = false;

	uHAL_assert(GPIO_PIN_IS_VALID(pin));
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (!GPIO_PIN_IS_VALID(pin)) {
		return false;
	}
#endif

	switch (gpio_get_mode(pin)) {
	//case GPIO_MODE_RESET:
	//case GPIO_MODE_AIN:
	//case GPIO_MODE_HiZ:
	case GPIO_MODE_HiZ_ALIAS:
		if (BIT_IS_SET(pin, GPIO_CTRL_TRISTATE_HIGH)) {
			is_on = true;
		}
		break;
	case GPIO_MODE_PP:
		if (gpio_get_output_state(pin) == GPIO_HIGH) {
			is_on = true;
		}
		break;
	// Assume an unexpected pin state is 'OFF'
	default:
		break;
	}
	if (BIT_IS_SET(pin, GPIO_CTRL_INVERT)) {
		is_on = !is_on;
	}

	return is_on;
}

err_t input_pin_listen_init(gpio_listen_t *handle, gpio_pin_t pin) {
	gpio_listen_cfg_t conf;

	conf.pin = pin;
	switch (SELECT_BITS(pin, GPIO_CTRL_BIAS_INPUT|GPIO_CTRL_BIAS_LOW)) {
	case GPIO_CTRL_BIAS_INPUT|GPIO_CTRL_BIAS_LOW:
		conf.trigger = GPIO_TRIGGER_RISING;
		break;
	case GPIO_CTRL_BIAS_INPUT:
		conf.trigger = GPIO_TRIGGER_FALLING;
		break;
	default:
		conf.trigger = GPIO_TRIGGER_RISING | GPIO_TRIGGER_FALLING;
		break;
	}

	return gpio_listen_init(handle, &conf);
}
err_t input_pin_listen_on(gpio_listen_t *handle) {
	return gpio_listen_on(handle);
}
err_t input_pin_listen_off(gpio_listen_t *handle) {
	return gpio_listen_off(handle);
}

#endif // uHAL_USE_HIGH_LEVEL_GPIO
