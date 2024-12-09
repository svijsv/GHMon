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

static utime_t set_alarms(bool force);
static void check_warnings(void);
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
	utime_t next_wakeup;

	platform_init();
	early_init_hook();

#if USE_CTRL_BUTTON
	input_pin_on(CTRL_BUTTON_PIN);
	input_pin_listen_init(&ctrl_button_listen_handle, CTRL_BUTTON_PIN);
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
			hibernate_s(next_wakeup - now, HIBERNATE_DEEP, uHAL_CFG_ALLOW_INTERRUPTS);
			// Clear the status *after* hibernation so that we wake instantly if we
			// recieved an interrupt while something besides sleep was happening
			uHAL_CLEAR_STATUS(uHAL_FLAG_IRQ);
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
		switch (ctrl_button_pressed) {
		case 0:
			break;

		case 1:
			led_flash(1, STATUS_LED_ACK_DELAY_MS);
			if (CONTROLLER_CHECK_MINUTES == 0) {
				do_controllers = true;
				LOGGER("Running controllers");
			}
			if (USE_LOGGING && LOG_APPEND_MINUTES == 0) {
				do_log = true;
				LOGGER("Appending to log");
			}
			LOGGER("Updating status");
			do_status = true;
			break;

		case 2:
			led_flash(2, STATUS_LED_LONG_DELAY_MS);
			if (USE_LOGGING) {
				LOGGER("Forcing log sync");
				force_sync = true;
			} else {
				LOGGER("No action taken, logging disabled");
			}
			break;

		case 3:
			led_flash(3, STATUS_LED_LONG_DELAY_MS);
			LOGGER("Forcing controller run");
			do_controllers = true;
			force_controllers = true;
			break;

		case 4: {
			uint time_h = RESET_TIME_OFFSET_MINUTES / 60U;
			uint time_m = RESET_TIME_OFFSET_MINUTES % 60U;

			led_flash(4, STATUS_LED_LONG_DELAY_MS);
			LOGGER("Setting system time to %u:%02u:00", (uint )time_h, (uint )time_m);
			set_time(time_h, time_m, 0);
			next_wakeup = set_alarms(true);
			break;
		}

		default:
			led_flash(2, STATUS_LED_ACK_DELAY_MS);
			LOGGER("No action taken");
			break;
		}
		ctrl_button_pressed = 0;
#endif // USE_CTRL_BUTTON

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
			check_common_actuator_warnings();
			check_common_sensor_warnings();
			check_common_controller_warnings();
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
				write_log_to_storage();
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

	LOGGER("Next alarm in %u seconds: %s", (uint )(next - now), reason);
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

void led_on(void) {
	if (USE_STATUS_LED) {
		output_pin_on(STATUS_LED_PIN);
	}
	return;
}
void led_off(void) {
	if (USE_STATUS_LED) {
		output_pin_off(STATUS_LED_PIN);
	}
	return;
}
void led_toggle(void) {
	if (USE_STATUS_LED) {
		output_pin_toggle(STATUS_LED_PIN);
	}
	return;
}
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

void error_state_hook(void) {
	led_toggle();
	return;
}
