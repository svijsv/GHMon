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
// main.c
// Main program logic
// NOTES:
//

#ifdef __cplusplus
 extern "C" {
#endif

//#define DEBUG 1
/*
* Includes
*/
#include "common.h"

#include "sensors.h"
#include "controllers.h"
#include "log.h"
#include "serial.h"
#include "terminal.h"
#include "calendar.h"
#include "power.h"

#include "ulib/cstrings.h"

#if RTC_CORRECTION_PERIOD_MINUTES < 0
# error "RTC_CORRECTION_PERIOD_MINUTES must be >= 0"
#endif
#if RTC_FINE_CORRECTION_PERIOD_MINUTES < 0
# error "RTC_FINE_CORRECTION_PERIOD_MINUTES must be >= 0"
#endif

/*
* Static values
*/
#define RTC_CORRECTION_PERIOD_SECONDS ((RTC_CORRECTION_PERIOD_MINUTES)*SECONDS_PER_MINUTE)
#if RTC_CORRECTION_PERIOD_SECONDS && RTC_CORRECTION_SECONDS && (USE_SMALL_CODE < 2)
# define DO_CLOCK_DESCEW 1
#endif
#define RTC_FINE_CORRECTION_PERIOD_SECONDS ((RTC_FINE_CORRECTION_PERIOD_MINUTES)*SECONDS_PER_MINUTE)
#if RTC_FINE_CORRECTION_PERIOD_SECONDS && RTC_FINE_CORRECTION_SECONDS && (USE_SMALL_CODE < 2)
# define DO_FINE_CLOCK_DESCEW 1
#endif


/*
* Types
*/


/*
* Variables
*/
volatile uint8_t  G_IRQs           = 0;
volatile uint8_t  G_warnings       = 0;
volatile uint8_t  G_button_pressed = 0;

static utime_t log_alarm = 0;
static utime_t status_alarm = 0;
#if DO_CLOCK_DESCEW
static utime_t descew_alarm = 0;
#endif
#if DO_FINE_CLOCK_DESCEW
static utime_t fine_descew_alarm = 0;
#endif
#if USE_SMALL_CONTROLLERS >= 1
static utime_t controller_alarm = 0;
#endif // USE_SMALL_CONTROLLERS >= 1


/*
* Local function prototypes
*/
// Calculate all the wakeup alarms and return the time of the next alarm
static uint32_t set_alarms(bool force);
static void check_warnings(void);
static void descew_clock(int16_t correction);


/*
* Interrupt handlers
*/
// IRQ handler for the control button
// The actual handler will be in the platform-specific code
static void button_IRQHandler(void) {
#if BUTTON_PIN
	utime_t timeout;

	G_button_pressed = 1;
	// First timeout is shorter to account for time taken to wake up
	timeout = SET_TIMEOUT(CTRL_PRESS-50);

#if BUTTON_DEBOUNCE_MS
	delay_ms(BUTTON_DEBOUNCE_MS);
#endif // BUTTON_DEBOUNCE_MS

	// Use a delay of 10ms to limit the influence of delay_ms() overhead
	do {
		if (TIMES_UP(timeout)) {
			++G_button_pressed;
			led_flash(1, DELAY_ACK);
			timeout = SET_TIMEOUT(CTRL_PRESS);
		}
		delay_ms(10);
	} while (gpio_get_state(BUTTON_PIN) == GPIO_HIGH);
#endif // BUTTON_PIN

	return;
}


/*
* Functions
*/
//
// Program entry point
int main(void) {
	bool do_controllers, do_log, do_status, force_sync, force_controllers;
	utime_t now, next_wakeup;

	platform_init();
	led_flash(1, DELAY_SHORT);

	// This doesn't warrant a flash
	power_init();
	//led_flash(1, DELAY_SHORT);

#if USE_SERIAL
	serial_init();
	led_flash(1, DELAY_SHORT);
#endif

	log_init();
	led_flash(1, DELAY_SHORT);

	sensors_init();
	led_flash(1, DELAY_SHORT);

	controllers_init();
	led_flash(1, DELAY_SHORT);

	next_wakeup = set_alarms(false);

	while (true) {
		now = NOW();

		if ((next_wakeup > now) && (G_IRQs == 0)) {
			hibernate_s(next_wakeup - now, CFG_ALLOW_INTERRUPTS);
		} else {
			// This message would have saved me many hours tracking down a bug in
			// set_alarms(); don't remove without a good reason.
			LOGGER("Skipping hibernate_s()");
		}
		if (BIT_IS_SET(G_IRQs, BUTTON_IRQf)) {
			CLEAR_BIT(G_IRQs, BUTTON_IRQf);
			button_IRQHandler();
		}
		if (BIT_IS_SET(G_IRQs, UART_IRQf)) {
			utime_t entry_time;
			CLEAR_BIT(G_IRQs, UART_IRQf);

			entry_time = NOW();
			terminal();

			now = NOW();
			// Reset the alarms if there's reason to believe the time was changed
			if ((now < entry_time) || ((now - entry_time) > HOURS)) {
				next_wakeup = set_alarms(true);
			}
		}
		if (G_IRQs != 0) {
			LOGGER("Uncleared IRQ(s): 0x%02X", G_IRQs);
		}
		G_IRQs = 0;

		do_controllers = false;
		do_log         = false;
		do_status      = false;
		force_sync     = false;
		force_controllers = false;

		if (G_button_pressed != 0) {
			switch (G_button_pressed-1) {
				case 0:
					led_flash(1, DELAY_ACK);
					if (CONTROLLER_CHECK_MINUTES == 0) {
						do_controllers = true;
						LOGGER("Checking controllers");
					}
					if (LOG_APPEND_MINUTES == 0) {
						do_log = true;
						LOGGER("Appending to log");
					}
					do_status = true;
					LOGGER("Forcing sensor status check");
					invalidate_sensors();
					break;
				case 1:
					led_flash(2, DELAY_LONG);
					LOGGER("Forcing log sync");
					do_log = true;
					force_sync = true;
					break;
				case 2:
					led_flash(3, DELAY_LONG);
					LOGGER("Forcing controller checks");
					do_controllers = true;
					force_controllers = true;
					break;
				case 3: {
					uint time_h = RESET_TIME_OFFSET_MINUTES / 60;
					uint time_m = RESET_TIME_OFFSET_MINUTES % 60;

					led_flash(4, DELAY_LONG);
					LOGGER("Setting system time to %u:%02u:00", (uint )time_h, (uint )time_m);
					set_time(time_h, time_m, 0);
					next_wakeup = set_alarms(true);
					break;
				}
				default:
					led_flash(2, DELAY_ACK);
					LOGGER("Command canceled");
					break;
			}
			G_button_pressed = 0;
		}

#if DO_CLOCK_DESCEW
		if ((descew_alarm > 0) && (NOW() >= descew_alarm)) {
			descew_alarm = 0;
			descew_clock(RTC_CORRECTION_SECONDS);
		}
#endif
#if DO_FINE_CLOCK_DESCEW
		if ((fine_descew_alarm > 0) && (NOW() >= fine_descew_alarm)) {
			fine_descew_alarm = 0;
			descew_clock(RTC_FINE_CORRECTION_SECONDS);
		}
#endif

		// Checking the status only updates warning blinks
		if (do_status || ((status_alarm > 0) && (NOW() >= status_alarm))) {
			LOGGER("Checking sensor status");
			status_alarm = 0;
			check_sensors();
			check_sensor_warnings();
			check_controller_warnings();
			check_warnings();

#if DEBUG && USE_SERIAL
			PRINTF("   VCC: %d\r\n\n   I  Name  Status\r\n",
				(int )G_vcc_voltage);
			for (uiter_t i = 0; i < SENSOR_COUNT; ++i) {
				PRINTF("   %u  %s   %d\r\n", (uint )i, FROM_FSTR(SENSORS[i].name), (int )G_sensors[i].status);
			}
#endif
		}

		if ((do_log) || ((log_alarm > 0) && (NOW() >= log_alarm))) {
			log_alarm = 0;
			log_status(force_sync);
		}

#if USE_CONTROLLERS
#if USE_SMALL_CONTROLLERS < 1
		controller_t *c;
		utime_t alarm;
		bool invalidated;

		invalidated = false;
		for (uiter_t i = 0; i < CONTROLLER_COUNT; ++i) {
			c = &G_controllers[i];
			alarm = c->next_check;

			if (force_controllers || ((alarm > 0) && (NOW() >= alarm)) || (do_controllers && (alarm == 0))) {
				if (!invalidated && BITS_ARE_SET(c->iflags, CTRL_FLAG_INVALIDATE|CTRL_FLAG_USES_SENSORS)) {
					invalidated = true;
					invalidate_sensors();
				}
				c->next_check = 0;
				check_controller(c);
			}
		}
#else // !USE_SMALL_CONTROLLERS < 1
		if (force_controllers || do_controllers || ((controller_alarm > 0) && (NOW() >= controller_alarm))) {
			controller_alarm = 0;

			for (uiter_t i = 0; i < CONTROLLER_COUNT; ++i) {
				check_controller(&G_controllers[i]);
			}
		}
#endif // USE_SMALL_CONTROLLERS < 1
#endif // USE_CONTROLLERS

		next_wakeup = set_alarms(false);
	}

	// Should never reach here:
	ERROR_STATE("Reached the end of main()?!");
	return 1;
}
static uint32_t set_alarms(bool force) {
	utime_t now, tmp, next;

	// Zeroing out alarms when they're executed and only setting them if they've
	// been zeroed allows us to catch missed alarms, such as when a controller
	// ran longer than the wait period would have been
	now = NOW();
	if ((LOG_APPEND_MINUTES > 0) && ((log_alarm == 0) || force)) {
		tmp = LOG_APPEND_MINUTES * MINUTES;
		log_alarm = SNAP_TO_FACTOR(now + tmp, tmp);
	}
	if ((STATUS_CHECK_MINUTES > 0) && ((status_alarm == 0) || force)) {
		tmp = STATUS_CHECK_MINUTES * MINUTES;
		status_alarm = SNAP_TO_FACTOR(now + tmp, tmp);
	}
#if DO_CLOCK_DESCEW
	if ((descew_alarm == 0) || force) {
		tmp = RTC_CORRECTION_PERIOD_SECONDS;
		descew_alarm = SNAP_TO_FACTOR(now + tmp, tmp);
		// Without this check, a negative correction will be applied repeatedly.
		if ((RTC_CORRECTION_SECONDS < 0) && (descew_alarm <= (now + -(RTC_CORRECTION_SECONDS)))) {
			descew_alarm += RTC_CORRECTION_PERIOD_SECONDS;
		}
	}
#endif
#if DO_FINE_CLOCK_DESCEW
	if ((fine_descew_alarm == 0) || force) {
		tmp = RTC_FINE_CORRECTION_PERIOD_SECONDS;
		fine_descew_alarm = SNAP_TO_FACTOR(now + tmp, tmp);
		// Without this check, a negative correction will be applied repeatedly.
		if ((RTC_FINE_CORRECTION_SECONDS < 0) && (fine_descew_alarm <= (now + -(RTC_FINE_CORRECTION_SECONDS)))) {
			fine_descew_alarm += RTC_FINE_CORRECTION_PERIOD_SECONDS;
		}
	}
#endif

#if USE_CONTROLLERS
#if USE_SMALL_CONTROLLERS >= 1
	if ((CONTROLLER_CHECK_MINUTES > 0) && ((controller_alarm == 0) || force)) {
		tmp = CONTROLLER_CHECK_MINUTES * MINUTES;
		controller_alarm = SNAP_TO_FACTOR(now + tmp, tmp);
	}
#else // !USE_SMALL_CONTROLLERS >= 1
	controller_t *c;
	_FLASH const controller_static_t *cfg;
	utime_t alarm;

	for (uiter_t i = 0; i < CONTROLLER_COUNT; ++i) {
		c = &G_controllers[i];
		cfg = &CONTROLLERS[i];
		alarm = c->next_check;

		if ((alarm != 0) && (!force)) {
			continue;
		}

		//
		// Default polling frequency
		if (cfg->schedule_minutes == 0) {
			if (CONTROLLER_CHECK_MINUTES > 0) {
				tmp = (CONTROLLER_CHECK_MINUTES * MINUTES);
				c->next_check = SNAP_TO_FACTOR(now + tmp, tmp);
			//
			// If the alarm is set but there's no default scheduled time, that
			// means it was set somewhere other than here and if we've made it to
			// this point then we're in a forced update so check on the next round
			} else if (alarm != 0) {
				c->next_check = now;
			}
		//
		// Controller-specific polling frequency
		} else {
			//
			// Time of day
			if (BIT_IS_SET(cfg->cflags, CTRL_FLAG_USE_TIME_OF_DAY)) {
				utime_t sm;

				sm = cfg->schedule_minutes * MINUTES;
				tmp = SNAP_TO_FACTOR(now, DAYS) + sm;
				//
				// If the alarm is set but not for the normal time, that means it
				// was set somewhere other than here and if we've made it to this
				// point then we're in a forced update so check on the next round
				if ((alarm != 0) && ((alarm % DAYS) != sm)) {
					c->next_check = now;
				//
				// If the scheduled time would be earlier than now, wait until
				// tomorrow
				} else if (tmp < now) {
					c->next_check = tmp + DAYS;

				} else {
					c->next_check = tmp;
				}
			//
			// Periodic
			} else {
				tmp = (cfg->schedule_minutes * MINUTES);
				tmp = SNAP_TO_FACTOR(now + tmp, tmp);

				// If the alarm is set but not for the normal time, that means it
				// was set somewhere other than here and if we've made it to this
				// point then we're in a forced update so check on the next round
				if ((alarm != 0) && (tmp != alarm)) {
					c->next_check = now;

				} else {
					c->next_check = tmp;
				}
			}
		}
	}
#endif // USE_SMALL_CONTROLLERS >= 1
#endif // USE_CONTROLLERS

	// Check for the next alarm in a separate loop so that I don't have to keep
	// track of what has or hasn't been set and when
	// 18 hours should be more than long enough a default sleep period
	next = now + 0xFFFF;
	if ((log_alarm != 0) && (next > log_alarm)) {
		next = log_alarm;
	}
	if ((status_alarm != 0) && (next > status_alarm)) {
		next = status_alarm;
	}
#if DO_CLOCK_DESCEW
	if ((descew_alarm != 0) && (next > descew_alarm)) {
		next = descew_alarm;
	}
#endif
#if DO_FINE_CLOCK_DESCEW
	if ((fine_descew_alarm != 0) && (next > fine_descew_alarm)) {
		next = fine_descew_alarm;
	}
#endif
#if USE_CONTROLLERS
#if USE_SMALL_CONTROLLERS >= 1
	if ((controller_alarm != 0) && (next > controller_alarm)) {
		next = controller_alarm;
	}
#else // !USE_SMALL_CONTROLLERS >= 1
	for (uiter_t i = 0; i < CONTROLLER_COUNT; ++i) {
		c = &G_controllers[i];

		if ((c->next_check != 0) && (next > c->next_check)) {
			next = c->next_check;
		}
	}
#endif // USE_SMALL_CONTROLLERS >= 1
#endif // USE_CONTROLLERS

	return next;
}

static void descew_clock(int16_t correction) {
#if DO_CLOCK_DESCEW || DO_FINE_CLOCK_DESCEW
	err_t res = EOK;
	utime_t now;

	now = get_RTC_seconds() + correction;
	if ((res = set_RTC_seconds(now)) != EOK) {
		LOGGER("Error %u while adjusting clock scew", (uint )res);
	}
	LOGGER("Clock adjusted %d seconds", (int )correction);

#endif // DO_CLOCK_DESCEW || DO_FINE_CLOCK_DESCEW
	return;
}

void led_on(void) {
	gpio_set_state(LED_PIN, GPIO_HIGH);
	return;
}
void led_off(void) {
	gpio_set_state(LED_PIN, GPIO_LOW);
	return;
}
void led_toggle(void) {
	gpio_toggle_state(LED_PIN);
	return;
}
void led_flash(uint8_t count, uint16_t ms) {
	for (uiter_t i = 0; i < count; ++i) {
		led_toggle();
		sleep_ms(ms);
		led_toggle();
		// Slight delay to keep separate flashes distinct
		sleep_ms(75);
	}

	return;
}
void issue_warning(void) {
	led_flash(3, DELAY_ERR);

	return;
}
static void check_warnings(void) {
	static _FLASH const uint8_t warn_SD         = (WARN_SD_SKIPPED|WARN_SD_SKIPPED);
	static _FLASH const uint8_t warn_power      = (WARN_BATTERY_LOW|WARN_VCC_LOW);
	static _FLASH const uint8_t warn_sensor     = (WARN_SENSOR);
	static _FLASH const uint8_t warn_controller = (WARN_CONTROLLER);

	// Delay briefly first so that it doesn't blend into any
	// acknowledgement flashes
	sleep_ms(DELAY_LONG);
	if (G_warnings & warn_power) {
		led_flash(1, DELAY_ERR);
	} else {
		if (G_warnings & warn_sensor) {
			led_flash(2, DELAY_ERR);
			delay_ms(DELAY_LONG);
		}
		if (G_warnings & warn_controller) {
			led_flash(3, DELAY_ERR);
			delay_ms(DELAY_LONG);
		}
		if (G_warnings & warn_SD) {
			led_flash(4, DELAY_ERR);
			delay_ms(DELAY_LONG);
		}
	}

	return;
}

void error_state_crude(void) {
	while (1) {
		led_toggle();
		dumb_delay_ms(DELAY_ERR);
	}
	return;
}
#if USE_SERIAL
void error_state(const char *file_path, uint32_t lineno, const char *func_name, const char *msg) {
	const char *basename;
	utime_t msg_timeout;

	msg_timeout = 0;
	basename = cstring_basename(file_path, '/');
	while (1) {
		if (G_serial_is_up && TIMES_UP(msg_timeout)) {
			if (msg_timeout == 0) {
				// Another message at 10s should give enough time to get the serial console up if it's not already
				msg_timeout = SET_TIMEOUT(10000);
			} else {
				msg_timeout = SET_TIMEOUT(5 * 60000);
			}
			// F() and F1() were likely both used for arguments and using any of
			// the other F() macros would cause another buffer to be used, so
			// using a plain string for the format saves RAM in this particular
			// case
			NOTIFY_NOF("Err %s:%u in %s(): %s\r\n", basename, (uint )lineno, func_name, msg);
		}

		led_toggle();
		dumb_delay_ms(DELAY_ERR);
	}
	return;
}
#else // !USE_SERIAL
void error_state(const char *file_path, uint32_t lineno, const char *func_name, const char *msg) {
	UNUSED(file_path);
	UNUSED(lineno);
	UNUSED(func_name);
	UNUSED(msg);
	error_state_crude();
	return;
}
#endif // USE_SERIAL

void _assert_failed(const char *file_path, uint32_t lineno, const char *func_name, const char *expr) {
	error_state(file_path, lineno, func_name, expr);

	return;
}


#ifdef __cplusplus
 }
#endif
