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
// main.c
// Main program logic
// NOTES:
//

#include "common.h"
#include "actuators.h"
#include "sensors.h"
#include "controllers.h"
#include "log.h"

#if RTC_CORRECTION_PERIOD_MINUTES < 0
# error "RTC_CORRECTION_PERIOD_MINUTES must be >= 0"
#endif
#if RTC_FINE_CORRECTION_PERIOD_MINUTES < 0
# error "RTC_FINE_CORRECTION_PERIOD_MINUTES must be >= 0"
#endif

#if RTC_CORRECTION_PERIOD_MINUTES && RTC_CORRECTION_SECONDS
# define DO_CLOCK_DESKEW 1
#else
# define DO_CLOCK_DESKEW 0
#endif
#if RTC_FINE_CORRECTION_PERIOD_MINUTES && RTC_FINE_CORRECTION_SECONDS
# define DO_FINE_CLOCK_DESKEW 1
#else
# define DO_FINE_CLOCK_DESKEW 0
#endif

typedef enum {
	BUTTON_IRQ_FLAG = 0x01U,
	TERMINAL_IRQ_FLAG = 0x02U,
} ghmon_IRQ_t;


static volatile uint_fast8_t ghmon_IRQs = 0;
uint_fast8_t ghmon_warnings = 0;

static utime_t log_alarm = 0;
static utime_t status_alarm = 0;
static utime_t deskew_alarm = 0;
static utime_t fine_deskew_alarm = 0;
static utime_t next_wakeup;

static utime_t set_alarms(bool force);
static void check_warnings(void);
static void update_warnings(void);
static void deskew_clock(int_fast16_t correction);
static inline utime_t calculate_alarm(const utime_t now, const utime_t period);
static inline utime_t correct_deskew_alarm(const utime_t now, const utime_t alarm, const utime_t period, const int_fast16_t correction);

#if USE_CTRL_BUTTON
# include "ctrl_button.h"
#endif
#if USE_UART_TERMINAL
# include "terminal.h"
#endif

#include GHMON_INCLUDE_CONFIG_HEADER(main_hooks.h)

//
// Program entry point
//
int main(void) {
	platform_init();
	early_init_hook();

#if USE_STATUS_LED
# if uHAL_USE_HIGH_LEVEL_GPIO
	output_pin_off(STATUS_LED_PIN);
# else
	gpio_set_mode(STATUS_LED_PIN, GPIO_MODE_PP, GPIO_LOW);
# endif
#endif
#if USE_CTRL_BUTTON
	button_pin_on(CTRL_BUTTON_PIN);
#endif
#if USE_UART_TERMINAL
	uart_listen_on(UART_COMM_PORT);
#endif
	//
	// Initialize actuators first because the other two may use them and
	// sensors before controller because controllers may depend on sensors
	init_common_actuators();
	init_common_sensors();
	init_common_controllers();
#if USE_LOGGING
	log_init();
#endif
	next_wakeup = set_alarms(false);

	late_init_hook();
	while (true) {
		utime_t now = NOW();

#if USE_CTRL_BUTTON
		input_pin_listen_on(&ctrl_button_listen_handle);
#endif
		if ((next_wakeup > now) && (ghmon_IRQs == 0)) {
			if (STATUS_LED_LIGHTS_ON_WARNING) {
				update_warnings();
				if (ghmon_warnings != 0) {
					led_on();
				}
			}
			// Due to small innaccuracies in how uHAL RTC emulation and AVR timers work,
			// we will often awaken just before we really want to and end up sleeping for
			// a very short period to compensate. It's not a big problem, but just the
			// same deliberately setting the wake alarm for 1 second later will mostly
			// fix that at the cost of a few (~10?) bytes of program memory.
			hibernate_s((next_wakeup - now), HIBERNATE_DEEP, uHAL_CFG_ALLOW_INTERRUPTS);
			//hibernate_s((next_wakeup - now)+1, HIBERNATE_DEEP, uHAL_CFG_ALLOW_INTERRUPTS);

			// Clear the status *after* hibernation so that we wake instantly if we
			// recieved an interrupt while something besides sleep was happening
			uHAL_CLEAR_STATUS(uHAL_FLAG_IRQ);
			if (STATUS_LED_LIGHTS_ON_WARNING) {
				led_off();
			}
		} else {
			// This message would have saved me many hours tracking down a bug in
			// set_alarms(); don't remove without a good reason.
			LOGGER("Skipping hibernation");
		}
#if USE_CTRL_BUTTON
		if (BIT_IS_SET(ghmon_IRQs, BUTTON_IRQ_FLAG)) {
			CLEAR_BIT(ghmon_IRQs, BUTTON_IRQ_FLAG);
			button_IRQHandler();
		}
#endif
#if USE_UART_TERMINAL
		if (BIT_IS_SET(ghmon_IRQs, TERMINAL_IRQ_FLAG)) {
			utime_t entry_time;
			CLEAR_BIT(ghmon_IRQs, TERMINAL_IRQ_FLAG);

			entry_time = NOW();
			terminal();

			now = NOW();
			// Reset the alarms if there's reason to believe the time was changed
			if ((now < entry_time) || ((now - entry_time) > (SECONDS_PER_HOUR / 4U))) {
				next_wakeup = set_alarms(true);
			}
		}
#endif
		if (ghmon_IRQs != 0) {
			LOGGER("Uncleared IRQ(s): 0x%02X", ghmon_IRQs);
		}
		ghmon_IRQs = 0;

		do_controllers = false;
		do_log         = false;
		do_status      = false;
		force_sync     = false;
		force_controllers = false;

#if USE_CTRL_BUTTON
		if (ctrl_button_pressed != 0) {
			ctrl_button_hook(ctrl_button_pressed - 1);
			ctrl_button_pressed = 0;
		}
#endif

		early_loop_hook();

		now = NOW();
		if (DO_CLOCK_DESKEW && (deskew_alarm > 0) && (now >= deskew_alarm)) {
			deskew_clock(RTC_CORRECTION_SECONDS);
			now = NOW();
			deskew_alarm = calculate_alarm(now, RTC_CORRECTION_PERIOD_MINUTES * SECONDS_PER_MINUTE);
			deskew_alarm = correct_deskew_alarm(now, deskew_alarm, RTC_CORRECTION_PERIOD_MINUTES * SECONDS_PER_MINUTE, RTC_CORRECTION_SECONDS);
		}
		if (DO_FINE_CLOCK_DESKEW && (fine_deskew_alarm > 0) && (now >= fine_deskew_alarm)) {
			deskew_clock(RTC_FINE_CORRECTION_SECONDS);
			now = NOW();
			fine_deskew_alarm = calculate_alarm(now, RTC_FINE_CORRECTION_PERIOD_MINUTES * SECONDS_PER_MINUTE);
			fine_deskew_alarm = correct_deskew_alarm(now, fine_deskew_alarm, RTC_FINE_CORRECTION_PERIOD_MINUTES * SECONDS_PER_MINUTE, RTC_FINE_CORRECTION_SECONDS);
		}

		run_common_controllers(do_controllers, force_controllers);

		if (do_status || ((status_alarm > 0) && (now >= status_alarm))) {
			check_warnings();

			status_alarm = calculate_alarm(now+1, STATUS_CHECK_MINUTES * SECONDS_PER_MINUTE);
		}

		if (USE_LOGGING) {
			if (do_log || ((log_alarm > 0) && (now >= log_alarm))) {
				log_status();
				log_alarm = calculate_alarm(now+1, LOG_APPEND_MINUTES * SECONDS_PER_MINUTE);
			}

			if (force_sync) {
				led_on();
				write_log_to_storage(0);
				led_off();
			}
		}

		late_loop_hook();
		next_wakeup = set_alarms(false);
	}

	// Should never reach here:
	//ERROR_STATE("Reached the end of main()?!");
	return 1;
}
static void update_warnings(void) {
	check_common_actuator_warnings();
	check_common_sensor_warnings();
	check_common_controller_warnings();
	return;
}
static inline utime_t calculate_alarm(const utime_t now, const utime_t period) {
	if (period == 0) {
		return 0;
	}

	utime_t next = now + period;
	next = SNAP_TO_FACTOR(next, period);
	return next;
}
static inline utime_t correct_deskew_alarm(const utime_t now, const utime_t alarm, const utime_t period, const int_fast16_t correction) {
	//
	// Without this check, a negative correction may be applied repeatedly.
	if ((correction < 0) && (alarm <= (now + -correction))) {
		return alarm + period;
	}
	return alarm;
}
static utime_t set_alarms(bool force) {
	utime_t now, next, test;
	char *reason = "Unknown";

	now = NOW();
	if (USE_LOGGING && (LOG_APPEND_MINUTES > 0) && ((log_alarm == 0) || force)) {
		log_alarm = calculate_alarm(now, LOG_APPEND_MINUTES * SECONDS_PER_MINUTE);
	}
	if ((STATUS_CHECK_MINUTES > 0) && ((status_alarm == 0) || force)) {
		status_alarm = calculate_alarm(now, STATUS_CHECK_MINUTES * SECONDS_PER_MINUTE);
	}

	if (DO_CLOCK_DESKEW && ((deskew_alarm == 0) || force)) {
		deskew_alarm = calculate_alarm(now, RTC_CORRECTION_PERIOD_MINUTES * SECONDS_PER_MINUTE);
		deskew_alarm = correct_deskew_alarm(now, deskew_alarm, RTC_CORRECTION_PERIOD_MINUTES * SECONDS_PER_MINUTE, RTC_CORRECTION_SECONDS);
	}
	if (DO_FINE_CLOCK_DESKEW && ((fine_deskew_alarm == 0) || force)) {
		fine_deskew_alarm = calculate_alarm(now, RTC_FINE_CORRECTION_PERIOD_MINUTES * SECONDS_PER_MINUTE);
		fine_deskew_alarm = correct_deskew_alarm(now, fine_deskew_alarm, RTC_FINE_CORRECTION_PERIOD_MINUTES * SECONDS_PER_MINUTE, RTC_FINE_CORRECTION_SECONDS);
	}
	calculate_common_controller_alarms(force);

	// 12 hours should be more than long enough a default sleep period
	next = now + (12U * SECONDS_PER_HOUR);
	if (wake_alarm != 0 && next > wake_alarm) {
		next = wake_alarm;
		reason = "General wake alarm";
	}
	if ((log_alarm != 0) && (next > log_alarm)) {
		next = log_alarm;
		reason = "Write log";
	}
	if ((status_alarm != 0) && (next > status_alarm)) {
		next = status_alarm;
		reason = "Update status";
	}
	if (DO_CLOCK_DESKEW && (deskew_alarm != 0) && (next > deskew_alarm)) {
		next = deskew_alarm;
		reason = "Deskew clock (coarse)";
	}
	if (DO_FINE_CLOCK_DESKEW && (fine_deskew_alarm != 0) && (next > fine_deskew_alarm)) {
		next = fine_deskew_alarm;
		reason = "Deskew clock (fine)";
	}
	test = find_next_common_controller_alarm();
	if (test > 0 && test < next) {
		next = test;
		reason = "Run controllers";
	}

	long diff = (next > now) ? (long )(next - now) : -((long )(now - next));
	LOGGER("Next alarm in %ld seconds: %s", diff, reason);
	// Get rid of compiler warnings when LOGGER() isn't used
	UNUSED(diff);
	UNUSED(reason);

	return next;
}

static void deskew_clock(int_fast16_t correction) {
	err_t res = ERR_OK;
	utime_t now;

	now = get_RTC_seconds();
	if (SKIP_SAFETY_CHECKS || (correction < 0 || now > (uint_fast16_t )correction)) {
		now += correction;
		if ((res = set_RTC_seconds(now)) != ERR_OK) {
			LOGGER("Error %d while adjusting clock", (int )res);
		}
	}
	LOGGER("Clock adjusted %d seconds", (int )correction);

	return;
}

#if USE_STATUS_LED
# if uHAL_USE_HIGH_LEVEL_GPIO
static void led_pin_on(void) {
	output_pin_on(STATUS_LED_PIN);
	return;
}
static void led_pin_off(void) {
	output_pin_off(STATUS_LED_PIN);
	return;
}
# else
static void led_pin_on(void) {
	gpio_set_output_state(STATUS_LED_PIN, GPIO_HIGH);
	return;
}
static void led_pin_off(void) {
	gpio_set_output_state(STATUS_LED_PIN, GPIO_LOW);
	return;
}
# endif

void led_on(void) {
	output_pin_on(STATUS_LED_PIN);
	return;
}
void led_off(void) {
	output_pin_off(STATUS_LED_PIN);
	return;
}
void led_toggle(void) {
	gpio_toggle_output_state(STATUS_LED_PIN);
	return;
}
#else
void led_on(void) {
	return;
}
void led_off(void) {
	return;
}
void led_toggle(void) {
	return;
}
#endif
void led_flash(uint8_t count, uint16_t ms) {
	if (USE_STATUS_LED) {
		for (uiter_t i = 0; i < count; ++i) {
			led_toggle();
			sleep_ms(ms);
			led_toggle();
			// Small delay to keep separate flashes distinct
			sleep_ms(100);
		}
	}

	return;
}
void issue_warning(void) {
	led_flash(3, STATUS_LED_ERR_DELAY_MS);

	return;
}
static void check_warnings(void) {
	const ghmon_warning_flags_t warn_log        = (WARN_LOG_SKIPPED|WARN_LOG_ERROR);
	const ghmon_warning_flags_t warn_power      = (WARN_BATTERY_LOW|WARN_VCC_LOW);
	const ghmon_warning_flags_t warn_actuator   = (WARN_ACTUATOR);
	const ghmon_warning_flags_t warn_sensor     = (WARN_SENSOR);
	const ghmon_warning_flags_t warn_controller = (WARN_CONTROLLER);

	update_warnings();

	// Delay briefly first so that it doesn't blend into any
	// acknowledgement flashes
	if (ghmon_warnings & warn_power) {
		sleep_ms(STATUS_LED_LONG_DELAY_MS);
		led_flash(1, STATUS_LED_ERR_DELAY_MS);
	} else {
		if (ghmon_warnings & warn_actuator) {
			sleep_ms(STATUS_LED_LONG_DELAY_MS);
			led_flash(2, STATUS_LED_ERR_DELAY_MS);
		}
		if (ghmon_warnings & warn_sensor) {
			sleep_ms(STATUS_LED_LONG_DELAY_MS);
			led_flash(3, STATUS_LED_ERR_DELAY_MS);
		}
		if (ghmon_warnings & warn_controller) {
			sleep_ms(STATUS_LED_LONG_DELAY_MS);
			led_flash(4, STATUS_LED_ERR_DELAY_MS);
		}
		if (USE_LOGGING && (ghmon_warnings & warn_log)) {
			sleep_ms(STATUS_LED_LONG_DELAY_MS);
			led_flash(5, STATUS_LED_ERR_DELAY_MS);
		}
	}

	return;
}

err_t set_system_datetime(datetime_t *dt) {
	err_t ret;

	ret = set_RTC_datetime(dt);
	next_wakeup = set_alarms(true);

	return ret;
}


void error_state_hook(void) {
	led_toggle();
	return;
}
