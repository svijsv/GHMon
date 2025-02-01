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

#if uHAL_USE_EXPERIMENTAL_GPIO_INTERFACE

#if ! uHAL_SKIP_INVALID_ARG_CHECKS
static bool verify_handle(const pinctrl_handle_t *handle) {
	return ((handle != NULL) && GPIO_PIN_IS_VALID(handle->pin));
}
static bool verify_cfg(const pinctrl_cfg_t *cfg) {
	return ((cfg != NULL) && GPIO_PIN_IS_VALID(cfg->pin));
}
#else
# define verify_handle(_p_) (true)
# define verify_cfg(_p_) (true)
#endif

#if defined(GPIO_MODE_OD)
static bool is_OD_cfg(const pinctrl_cfg_t *cfg) {
	return (!cfg->hi_active && cfg->lo_active);
}
#else
// It's easier to make sure GPIO_MODE_OD is defined so we don't have to keep
// checking with the preprocessor, but no code path that depends on the value
// should ever be taken
# define GPIO_MODE_OD 0
# define is_OD_cfg(_p_) (false)
#endif

err_t pinctrl_init(pinctrl_handle_t *handle, const pinctrl_cfg_t *cfg) {
	pinctrl_handle_t th = {
		.pin = 0,
		.on_gpio_mode = GPIO_MODE_RESET,
		.off_gpio_mode = GPIO_MODE_RESET,
		.on_gpio_state = GPIO_FLOAT,
		.off_gpio_state = GPIO_FLOAT,
		.is_input = 0,
		.set_state = 0,
		.on_state = 0,
	};

	uHAL_assert(handle != NULL);
	uHAL_assert(verify_cfg(cfg));

#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (!verify_cfg(cfg) || handle == NULL) {
		return ERR_BADARG;
	}
#endif

	th.pin = cfg->pin;
	if (cfg->turn_on) {
		th.set_state = 1;
	}
	if (cfg->hi_is_on) {
		th.on_state = 1;
	}

	if (cfg->as_input) {
		th.is_input = true;
		th.on_gpio_mode = GPIO_MODE_IN;
		th.off_gpio_mode = GPIO_MODE_IN;
		if (th.on_state == 1) {
			if (cfg->hi_active) {
				th.on_gpio_state = GPIO_HIGH;
			}
			if (cfg->lo_active) {
				th.off_gpio_state = GPIO_LOW;
			}
		} else {
			if (cfg->lo_active) {
				th.on_gpio_state = GPIO_LOW;
			}
			if (cfg->hi_active) {
				th.off_gpio_state = GPIO_HIGH;
			}
		}

	} else {
		if (is_OD_cfg(cfg)) {
			th.on_gpio_mode = GPIO_MODE_OD;
			th.off_gpio_mode = GPIO_MODE_OD;
			if (th.on_state == 1) {
				th.on_gpio_state = GPIO_HIGH;
				th.off_gpio_state = GPIO_LOW;
			} else {
				th.on_gpio_state = GPIO_LOW;
				th.off_gpio_state = GPIO_HIGH;
			}

		} else if (th.on_state == 1) {
			if (cfg->hi_active) {
				th.on_gpio_mode = GPIO_MODE_PP;
				th.on_gpio_state = GPIO_HIGH;
			}
			if (cfg->lo_active) {
				th.off_gpio_mode = GPIO_MODE_PP;
				th.off_gpio_state = GPIO_LOW;
			}

		} else {
			if (cfg->lo_active) {
				th.on_gpio_mode = GPIO_MODE_PP;
				th.on_gpio_state = GPIO_LOW;
			}
			if (cfg->hi_active) {
				th.off_gpio_mode = GPIO_MODE_PP;
				th.off_gpio_state = GPIO_HIGH;
			}
		}
	}

	*handle = th;

	if (th.set_state == 1) {
		return gpio_set_mode(th.pin, th.on_gpio_mode, th.on_gpio_state);
	} else {
		return gpio_set_mode(th.pin, th.off_gpio_mode, th.off_gpio_state);
	}
}
err_t pinctrl_init2_input(pinctrl_handle_t *handle, gpio_pin_t pin) {
	pinctrl_cfg_t cfg = {
		.pin = pin,
		.as_input = true,
		.hi_is_on = true,
		.hi_active = false,
		.lo_active = false,
		.turn_on = false
	};

	if (BIT_IS_SET(pin, GPIO_CTRL_INVERT)) {
		cfg.hi_is_on = false;
	}

	switch (SELECT_BITS(pin, GPIO_CTRL_BIAS_INPUT|GPIO_CTRL_BIAS_LOW)) {
	case GPIO_CTRL_BIAS_INPUT|GPIO_CTRL_BIAS_LOW:
		cfg.lo_active = 1;
		break;
	case GPIO_CTRL_BIAS_INPUT:
		cfg.hi_active = 1;
		break;
	}

	return pinctrl_init(handle, &cfg);
}
err_t pinctrl_init2_output(pinctrl_handle_t *handle, gpio_pin_t pin) {
	pinctrl_cfg_t cfg = {
		.pin = pin,
		.as_input = true,
		.hi_is_on = true,
		.hi_active = false,
		.lo_active = false,
		.turn_on = false
	};

	if (BIT_IS_SET(pin, GPIO_CTRL_INVERT)) {
		cfg.hi_is_on = false;
	}
	if (BIT_IS_SET(pin, GPIO_CTRL_TRISTATE_LOW)) {
		cfg.lo_active = 1;
	}
	if (BIT_IS_SET(pin, GPIO_CTRL_TRISTATE_HIGH)) {
		cfg.hi_active = 1;
	}

	return pinctrl_init(handle, &cfg);
}

bool pinctrl_is_on(pinctrl_handle_t *handle) {
	gpio_state_t state = GPIO_FLOAT;

	uHAL_assert(verify_handle(handle));
	if (!verify_handle(handle)) {
		return false;
	}

	if ((handle->set_state && handle->on_gpio_mode == GPIO_MODE_IN) || (!handle->set_state && handle->off_gpio_mode == GPIO_MODE_IN)) {
		state = gpio_get_input_state(handle->pin);
	}

	if (state != GPIO_FLOAT) {
		return ((handle->on_state == 1 && state == GPIO_HIGH) || (handle->on_state == 0 && state == GPIO_LOW));
	}
	return false;
}

static err_t _pinctrl_on(pinctrl_handle_t *handle) {
	handle->set_state = 1;

	if (handle->on_gpio_mode != handle->off_gpio_mode) {
		return gpio_set_mode(handle->pin, handle->on_gpio_mode, handle->on_gpio_state);
	}
	return gpio_set_state(handle->pin, handle->on_gpio_state);
}
err_t pinctrl_on(pinctrl_handle_t *handle) {
	uHAL_assert(verify_handle(handle));
	if (!verify_handle(handle)) {
		return ERR_BADARG;
	}
	return _pinctrl_on(handle);
}

static err_t _pinctrl_off(pinctrl_handle_t *handle) {
	handle->set_state = 0;

	if (handle->on_gpio_mode != handle->off_gpio_mode) {
		return gpio_set_mode(handle->pin, handle->off_gpio_mode, handle->off_gpio_state);
	}
	return gpio_set_state(handle->pin, handle->off_gpio_state);
}
err_t pinctrl_off(pinctrl_handle_t *handle) {
	uHAL_assert(verify_handle(handle));
	if (!verify_handle(handle)) {
		return ERR_BADARG;
	}

	return _pinctrl_off(handle);
}

err_t pinctrl_toggle(pinctrl_handle_t *handle) {
	uHAL_assert(verify_handle(handle));
	if (!verify_handle(handle)) {
		return ERR_BADARG;
	}

	return (handle->set_state == 1) ? _pinctrl_off(handle) : _pinctrl_on(handle);
}

err_t pinctrl_suspend(pinctrl_handle_t *handle) {
	uHAL_assert(verify_handle(handle));
	if (!verify_handle(handle)) {
		return ERR_BADARG;
	}

	return gpio_set_mode(handle->pin, GPIO_MODE_RESET, GPIO_FLOAT);
}
err_t pinctrl_resume(pinctrl_handle_t *handle) {
	uHAL_assert(verify_handle(handle));
	if (!verify_handle(handle)) {
		return ERR_BADARG;
	}

	if (handle->set_state == 1) {
		return gpio_set_mode(handle->pin, handle->on_gpio_mode, handle->on_gpio_state);
	} else {
		return gpio_set_mode(handle->pin, handle->off_gpio_mode, handle->off_gpio_state);
	}
}


#endif // uHAL_USE_EXPERIMENTAL_GPIO_INTERFACE
