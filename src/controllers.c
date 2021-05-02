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
// controllers.c
// Manage external device control
// NOTES:
//

#ifdef __cplusplus
 extern "C" {
#endif

//#define DEBUG 1
/*
* Includes
*/
#include "controllers.h"

#include "sensors.h"
#include "power.h"
#include "serial.h"


// Need to check this *after* the headers are included
#if USE_CONTROLLERS

#if CONTROLLER_COUNT > 255 || CONTROLLER_COUNT < 1
# error "CONTROLLER_COUNT not between 1 and 255"
#endif
#if CONTROLLER_SENS_COUNT < 1 || CONTROLLER_SENS_COUNT > 255
# error "CONTROLLER_SENS_COUNT not between 1 and 127"
#endif


/*
* Static values
*/


/*
* Types
*/


/*
* Variables
*/
#if USE_SMALL_CONTROLLERS < 1
static utime_t last_check[CONTROLLER_COUNT];
#endif // USE_SMALL_CONTROLLERS < 1

controller_t G_controllers[CONTROLLER_COUNT];

/*
* Local function prototypes
*/
#if USE_SMALL_CONTROLLERS < 2
static gpio_state_t read_stop(_FLASH const controller_static_t *cfg);
#endif
static void update_runtime(controller_t *c);


/*
 * Macros
*/
#define CHECK_ENGAGED(c) (BIT_IS_SET((c)->iflags, CTRL_FLAG_ENGAGED))
#define SET_ENGAGED(c)   (SET_BIT(   (c)->iflags, CTRL_FLAG_ENGAGED))
#define UNSET_ENGAGED(c) (CLEAR_BIT( (c)->iflags, CTRL_FLAG_ENGAGED))

#define ENGAGE(c, cfg) \
	do { \
		LOGGER("Engaging %s", FROM_FSTR((cfg)->name)); \
		SET_ENGAGED((c)); \
		power_on_output((cfg)->control_pin); \
	} while (0);

#define DISENGAGE(c, cfg, msg) \
	do { \
		LOGGER("Halting %s: " msg, FROM_FSTR((cfg)->name)); \
		UNSET_ENGAGED((c)); \
		power_off_output((cfg)->control_pin); \
	} while (0);


/*
* Interrupt handlers
*/


/*
* Functions
*/
void controllers_init(void) {
	// controller_t *c;
	_FLASH const controller_static_t *cfg;

	for (uiter_t i = 0; i < CONTROLLER_COUNT; ++i) {
		// c = &G_controllers[i];
		cfg = &CONTROLLERS[i];

		if (cfg->name[0] == 0) {
			LOGGER("Unset name in CONTROLLERS[%u]; is CONTROLLER_COUNT correct?", (uint )i);
			ERROR_STATE("Unset name in CONTROLLERS[]; is CONTROLLER_COUNT correct?");
		}

		for (uiter_t j = 0; j < CONTROLLER_SENS_COUNT; ++j) {
			if (cfg->inputs[j].si >= SENSOR_COUNT) {
				LOGGER("Controller %u input %u index >= SENSOR_COUNT", (uint )i, (uint )j);
				ERROR_STATE("Controller input index >= SENSOR_COUNT");
			} else if (cfg->inputs[j].si >= 0) {
				SET_BIT(G_controllers[i].iflags, CTRL_FLAG_USES_SENSORS);
			}
		}

		//G_controllers[i].i = i;
		power_off_output(cfg->control_pin);
	}

	return;
}

void check_controller(controller_t *c) {
	_FLASH const controller_sens_t *s;
	_FLASH const controller_static_t *cfg;
	status_t status;
	bool do_engage, is_engaged;

	cfg = &CONTROLLERS[GET_CONTROLLER_I(c)];
	if (BIT_IS_SET(G_warnings, WARN_BATTERY_LOW|WARN_VCC_LOW) && !BIT_IS_SET(cfg->cflags, CTRL_FLAG_IGNORE_POWER)) {
		is_engaged = CHECK_ENGAGED(c);
		if (is_engaged) {
			DISENGAGE(c, cfg, "battery or Vcc low");
#if USE_SMALL_CONTROLLERS < 1
			c->try_count = 0;
#endif
			update_runtime(c);
		} else {
			LOGGER("Skipping %s check, battery or Vcc low", FROM_FSTR(cfg->name));
		}

		SET_BIT(G_warnings, WARN_CONTROLLER);
		SET_BIT(c->iflags, CTRL_FLAG_WARNING);
		return;
	}

	LOGGER("Checking %s", FROM_FSTR(cfg->name));

	do_engage = false;
	is_engaged = false;
	CLEAR_BIT(c->iflags, CTRL_FLAG_WARNING);
	CLEAR_BIT(c->iflags, CTRL_FLAG_INVALIDATE);

	do_engage = true;
	if (BIT_IS_SET(c->iflags, CTRL_FLAG_USES_SENSORS)) {
		bool any_met = false, any_unmet = false;
		bool high = false, low = false, inside = false;

		// It simplifies things greatly to always check all the sensors here
		// and trust in the cooldown timer to keep us from doing so too often
		check_sensors();

		for (uiter_t i = 0; i < CONTROLLER_SENS_COUNT; ++i) {
			s = &cfg->inputs[i];
			if (s->si < 0) {
				continue;
			}
			status = G_sensors[s->si].status;

			high = (s->above == SENS_THRESHOLD_IGNORE) ? false : (status > s->above);
			low  = (s->below == SENS_THRESHOLD_IGNORE) ? false : (status < s->below);
			inside = ((s->above != SENS_THRESHOLD_IGNORE) && (s->below != SENS_THRESHOLD_IGNORE) && (s->above < s->below));

			if (inside) {
				if (high && low) {
					any_met = true;
				} else {
					any_unmet = true;
				}
			} else if (high || low) {
				any_met = true;
			} else {
				any_unmet = true;
			}
		}

		if (!any_met || (any_unmet && !BIT_IS_SET(cfg->cflags, CTRL_FLAG_TRIGGER_ANY))) {
			do_engage = false;
		}
	}
	is_engaged = CHECK_ENGAGED(c);

	if ((do_engage) && (BIT_IS_SET(cfg->cflags, CTRL_FLAG_WARN_WHEN_ON))) {
		SET_BIT(c->iflags, CTRL_FLAG_WARNING);
	}

// There's some duplication but overall readability is better when
// USE_SMALL_CONTROLLERS is handled in bigger blocks
#if USE_SMALL_CONTROLLERS < 1 // Have stop pin & individual check period
	if (do_engage && !is_engaged) {
		utime_t now;

		if (read_stop(cfg) == GPIO_HIGH) {
			LOGGER("Skipping %s: stop-pin is high", FROM_FSTR(cfg->name));
			goto END;
		}

		ENGAGE(c, cfg);
		now = NOW();
		++c->try_count;
		c->run_start = now;
		++c->run_count;

		if (cfg->run_timeout_seconds > 0) {
			if (cfg->stop_pin != 0) {
				c->next_check = now + ((cfg->run_timeout_seconds > CONTROLLER_STOP_CHECK_SECONDS) ? CONTROLLER_STOP_CHECK_SECONDS : cfg->run_timeout_seconds);
			} else {
				c->next_check = now + cfg->run_timeout_seconds;
			}
		}
		last_check[GET_CONTROLLER_I(c)] = now;
		SET_BIT(c->iflags, CTRL_FLAG_INVALIDATE);

	} else if (do_engage && is_engaged) {
		utime_t now, timeout;

		update_runtime(c);
		if (cfg->run_timeout_seconds == 0) {
			goto END;
		}

		now = NOW();
		// The time may have been changed between when we started and now
		if (now > c->run_start) {
			timeout = (now - c->run_start);
			timeout = (cfg->run_timeout_seconds > timeout) ? cfg->run_timeout_seconds - timeout : 0;
		} else {
			// Not much else we can do in this case but assume the timeout has
			// been reached
			LOGGER("%s start time is in the future; assuming it's timed out", FROM_FSTR(cfg->name));
			timeout = 0;
		}

		if (timeout == 0) {
			DISENGAGE(c, cfg, "run timeout");
			if (BIT_IS_SET(cfg->cflags, CTRL_FLAG_RETRY)) {
				if (c->try_count <= CONTROLLER_RETRY_MAX) {
					c->next_check = now + ((cfg->run_timeout_seconds > CONTROLLER_RETRY_DELAY_SECONDS) ? CONTROLLER_RETRY_DELAY_SECONDS : cfg->run_timeout_seconds);
					LOGGER("Re-checking %s in %us", FROM_FSTR(cfg->name), (uint )(c->next_check - now));
					SET_BIT(c->iflags, CTRL_FLAG_INVALIDATE);
				} else {
					LOGGER("Aborting %s: exceeded max retries", FROM_FSTR(cfg->name));
					SET_BIT(c->iflags, CTRL_FLAG_WARNING);
					c->try_count = 0;
				}
			}

		} else if (read_stop(cfg) == GPIO_HIGH) {
			DISENGAGE(c, cfg, "stop pin is high");
			c->try_count = 0;

		} else {
			if (cfg->stop_pin != 0) {
				c->next_check = now + ((timeout > CONTROLLER_STOP_CHECK_SECONDS) ? CONTROLLER_STOP_CHECK_SECONDS : timeout);
			} else {
				c->next_check = now + timeout;
			}
		}

	} else if (!do_engage && is_engaged) {
		DISENGAGE(c, cfg, "conditions not met");
		update_runtime(c);
		c->try_count = 0;

	}

#else // !USE_SMALL_CONTROLLERS < 1 // Don't have check period but may have stop
	if (do_engage && !is_engaged) {
#if USE_SMALL_CONTROLLERS < 2 // Have stop pin
		if (read_stop(cfg) == GPIO_HIGH) {
			LOGGER("Skipping %s: stop-pin is high", FROM_FSTR(cfg->name));
			goto END;
		}
#endif // USE_SMALL_CONTROLLERS < 2

		ENGAGE(c, cfg);
		if (cfg->run_timeout_seconds > 0) {
			utime_t timeout;
			timeout = cfg->run_timeout_seconds;

#if USE_SMALL_CONTROLLERS < 2
			utime_t wakeup;

			if (cfg->stop_pin != 0) {
				while (timeout > 0) {
					wakeup = ((timeout > CONTROLLER_STOP_CHECK_SECONDS) ? CONTROLLER_STOP_CHECK_SECONDS : timeout);
					timeout -= wakeup;
					hibernate_s(wakeup, 0);
					if (read_stop(cfg) == GPIO_HIGH) {
						DISENGAGE(c, cfg, "stop pin is high");
						break;
					}
				}
			} else
#endif // USE_SMALL_CONTROLLERS < 2
			{
				hibernate_s(timeout, 0);
				timeout = 0;
			}
			if (timeout == 0) {
				DISENGAGE(c, cfg, "run timeout");
			}
			power_off_output(cfg->control_pin);
		}

	} else if (!do_engage && is_engaged) {
		DISENGAGE(c, cfg, "conditions not met");
		update_runtime(c);

	} else if (do_engage && is_engaged) {
#if USE_SMALL_CONTROLLERS < 2
		if (read_stop(cfg) == GPIO_HIGH) {
			DISENGAGE(c, cfg, "stop pin is high");
		}
		update_runtime(c);
#endif // USE_SMALL_CONTROLLERS < 2
	}
#endif // USE_SMALL_CONTROLLERS < 1

END:
	return;
}

void check_controller_warnings(void) {
	controller_t *c;

	CLEAR_BIT(G_warnings, (WARN_CONTROLLER));

	for (uiter_t i = 0; i < CONTROLLER_COUNT; ++i) {
		c = &G_controllers[i];

		if (BIT_IS_SET(c->iflags, CTRL_FLAG_WARNING)) {
			SET_BIT(G_warnings, WARN_CONTROLLER);
			// No need to check the rest; there's only one warning flag
			break;
		}
	}

	return;
}

static void update_runtime(controller_t *c) {
#if USE_SMALL_CONTROLLERS < 1
	_FLASH const controller_static_t *cfg;

	cfg = &CONTROLLERS[GET_CONTROLLER_I(c)];

	utime_t now;
	utime_t *last;

	now = NOW();
	last = &last_check[GET_CONTROLLER_I(c)];

	if (*last < now) {
		if ((now - *last) > (HOURS*4)) {
			LOGGER("Traveled %u seconds into the future, not updating %s run time", (uint )(now - *last), FROM_FSTR(cfg->name));
		} else {
			LOGGER("%s has been running %us", FROM_FSTR(cfg->name), (uint )(now - *last));
			c->run_time_seconds += now - *last;
		}
	} else {
		LOGGER("Traveled %u seconds into the past, not updating %s run time", (uint )(*last - now), FROM_FSTR(cfg->name));
	}
	*last = now;

#else
	UNUSED(c);
#endif // USE_SMALL_CONTROLLERS < 1

	return;
}

#if USE_SMALL_CONTROLLERS < 2
/*
static void stop_on(_FLASH const controller_static_t *cfg) {
	if (cfg->stop_pin != 0) {
		gpio_set_mode(cfg->stop_pin, GPIO_MODE_IN, GPIO_LOW);
	}

	return;
}
static void stop_off(_FLASH const controller_static_t *cfg) {
	if (cfg->stop_pin != 0) {
		gpio_set_mode(cfg->stop_pin, GPIO_MODE_HiZ, GPIO_FLOAT);
	}

	return;
}
*/
static gpio_state_t read_stop(_FLASH const controller_static_t *cfg) {
	gpio_state_t pstate;

	pstate = GPIO_FLOAT;
	if (cfg->stop_pin != 0) {
		power_on_input(cfg->stop_pin);
		pstate = gpio_get_state(cfg->stop_pin);
		power_off_input(cfg->stop_pin);
	}

	return pstate;
}
#endif // USE_SMALL_CONTROLLERS < 2

/*
static bool check_engaged(pin_t pin) {
	bool is_engaged;

	is_engaged = false;
	if (gpio_get_mode(pin) == GPIO_MODE_PP) {
		gpio_state_t pstate;

		pstate = gpio_get_state(pin);
		switch (GPIO_GET_BIAS(pin)) {
		case BIAS_HIGH:
			is_engaged = (pstate != GPIO_HIGH);
			break;
		default:
			is_engaged = (pstate == GPIO_HIGH);
			break;
		}
	}

	return is_engaged;
}
*/
/*
static void disengage(controller_t *c, const char *msg) {
	uint8_t ci;
	_FLASH const char *shared = NULL;

	ci = GET_CONTROLLER_I(c);
	for (uiter_t i = 0; i < SENSOR_COUNT; ++i) {
		if ((i != ci) && (PINID(CONTROLLERS[i].control_pin) == PINID(CONTROLLERS[ci].control_pin)) && (BIT_IS_SET(G_controllers[i].iflags, CTRL_FLAG_ENGAGED))) {
			shared = CONTROLLERS[i].name;
			break;
		}
	}
	if (shared != NULL) {
		LOGGER("Halting %s: %s", FROM_FSTR(CONTROLLERS[ci].name), msg);
		UNSET_ENGAGED(c);
		power_off_output(CONTROLLERS[ci].control_pin);
	} else {
		LOGGER("Holding %s on: shares pin with %s", FROM_FSTR(CONTROLLERS[ci].name), FROM_FSTR2(CONTROLLERS[ci].name));
	}

	return;
}
*/


#endif // USE_CONTROLLERS

#ifdef __cplusplus
 }
#endif
