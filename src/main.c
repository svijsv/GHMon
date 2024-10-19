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

#if RTC_CORRECTION_PERIOD_MINUTES < 0
# error "RTC_CORRECTION_PERIOD_MINUTES must be >= 0"
#endif
#if RTC_FINE_CORRECTION_PERIOD_MINUTES < 0
# error "RTC_FINE_CORRECTION_PERIOD_MINUTES must be >= 0"
#endif

#if RTC_CORRECTION_PERIOD_MINUTES && RTC_CORRECTION_SECONDS
# define DO_CLOCK_DESCEW 1
#else
# define DO_CLOCK_DESCEW 0
#endif
#if RTC_FINE_CORRECTION_PERIOD_MINUTES && RTC_FINE_CORRECTION_SECONDS
# define DO_FINE_CLOCK_DESCEW 1
#else
# define DO_FINE_CLOCK_DESCEW 0
#endif

typedef enum {
	BUTTON_IRQ_FLAG = 0x01U,
	TERMINAL_IRQ_FLAG = 0x02U,
} ghmon_IRQ_t;


static volatile uint_fast8_t ghmon_IRQs = 0;
uint_fast8_t ghmon_warnings = 0;

static utime_t log_alarm = 0;
static utime_t status_alarm = 0;
static utime_t descew_alarm = 0;
static utime_t fine_descew_alarm = 0;

static utime_t set_alarms(bool force);
static void check_warnings(void);
static void descew_clock(int_fast16_t correction);

#if USE_CTRL_BUTTON
# include "ctrl_button.h"
#endif
#if USE_UART_TERMINAL
# include "terminal.h"
#endif


//
// Program entry point
//
int main(void) {
	platform_init();

	utime_t next_wakeup = set_alarms(false);
#if USE_CTRL_BUTTON
	input_pin_on(CTRL_BUTTON_PIN);
	input_pin_listen_init(&ctrl_button_listen_handle, CTRL_BUTTON_PIN);
#endif
#if USE_UART_TERMINAL
	uart_listen_on(UART_COMM_PORT);
#endif

	while (true) {
		utime_t now_sec = NOW();
		utime_t now_min = now_sec / SECONDS_PER_MINUTE;

#if USE_CTRL_BUTTON
		input_pin_listen_on(&ctrl_button_listen_handle);
#endif
		if ((next_wakeup > now_min) && (ghmon_IRQs == 0)) {
			next_wakeup = (next_wakeup * SECONDS_PER_MINUTE);
			next_wakeup = SNAP_TO_FACTOR(next_wakeup, 60);
			next_wakeup -= now_sec;

			hibernate_s(next_wakeup, HIBERNATE_DEEP, uHAL_CFG_ALLOW_INTERRUPTS);
			// Clear the status *after* hibernation so that we wake instantly if we
			// recieved an interrupt while something besides sleep was happening
			uHAL_CLEAR_STATUS(uHAL_FLAG_IRQ);
		} else {
			// This message would have saved me many hours tracking down a bug in
			// set_alarms(); don't remove without a good reason.
			LOGGER("Skipping hibernate_s()");
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

			now_sec = NOW();
			now_min = now_sec / SECONDS_PER_MINUTE;
			// Reset the alarms if there's reason to believe the time was changed
			if ((now_sec < entry_time) || ((now_sec - entry_time) > (SECONDS_PER_HOUR / 4U))) {
				next_wakeup = set_alarms(true);
			}
		}
#endif
		if (ghmon_IRQs != 0) {
			LOGGER("Uncleared IRQ(s): 0x%02X", ghmon_IRQs);
		}
		ghmon_IRQs = 0;

		bool do_controllers = false;
		bool do_log         = false;
		bool do_status      = false;
		bool force_sync     = false;
		bool force_controllers = false;

#if USE_CTRL_BUTTON
		switch (ctrl_button_pressed) {
		case 0:
			break;

		case 1:
			led_flash(1, STATUS_LED_ACK_DELAY_MS);
			if (USE_CONTROLLERS && CONTROLLER_CHECK_MINUTES == 0) {
				do_controllers = true;
				LOGGER("Checking controllers");
			}
			if (USE_LOGGING && LOG_APPEND_MINUTES == 0) {
				do_log = true;
				LOGGER("Appending to log");
			}
			if (USE_SENSORS) {
				LOGGER("Forcing sensor status check");
/*
				invalidate_sensors();
*/
			}
			LOGGER("Updating status");
			do_status = true;
			break;

		case 2:
			led_flash(2, STATUS_LED_LONG_DELAY_MS);
			if (USE_LOGGING) {
				LOGGER("Forcing log sync");
				do_log = true;
				force_sync = true;
			} else {
				LOGGER("No action taken, logging disabled");
			}
			break;

		case 3:
			led_flash(3, STATUS_LED_LONG_DELAY_MS);
			if (USE_CONTROLLERS) {
				LOGGER("Forcing controller checks");
				do_controllers = true;
				force_controllers = true;
			} else {
				LOGGER("No action taken, controllers disabled");
			}
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

		now_sec = NOW();
		now_min = now_sec / SECONDS_PER_MINUTE;
		if (DO_CLOCK_DESCEW && (descew_alarm > 0) && (now_min >= descew_alarm)) {
			descew_alarm = 0;
			descew_clock(RTC_CORRECTION_SECONDS);
			now_sec = NOW();
			now_min = now_sec / SECONDS_PER_MINUTE;
		}
		if (DO_FINE_CLOCK_DESCEW && (fine_descew_alarm > 0) && (now_min >= fine_descew_alarm)) {
			fine_descew_alarm = 0;
			descew_clock(RTC_FINE_CORRECTION_SECONDS);
			now_sec = NOW();
			now_min = now_sec / SECONDS_PER_MINUTE;
		}

		// Checking the status only updates warning blinks
		if (do_status || ((status_alarm > 0) && (now_min >= status_alarm))) {
			status_alarm = 0;
/*
			if (USE_SENSORS) {
				LOGGER("Checking sensor status");
				check_sensors();
				now_sec = NOW();
				now_min = now_sec / SECONDS_PER_MINUTE;
				check_sensor_warnings();
			}
			if (USE_CONTROLLERS) {
				check_controller_warnings();
			}
*/
			check_warnings();

/*
			if (DEBUG && USE_SENSORS) {
				PRINTF("   VCC: %d\r\n\tIndex\tName\tStatus\r\n", (int )G_vcc_voltage);
				for (uiter_t i = 0; i < SENSOR_COUNT; ++i) {
					PRINTF("\t%u\t%s\t%d\r\n", (uint )i, FROM_FSTR(SENSORS[i].name), (int )G_sensors[i].status);
				}
			}
*/
		}

		if (USE_LOGGING && (do_log || ((log_alarm > 0) && (now_min >= log_alarm)))) {
			log_alarm = 0;
/*
			log_status(force_sync);
*/
		}

/*
		if (USE_CONTROLLERS) {
			controller_t *c;
			utime_t alarm;
			bool invalidated;

			invalidated = false;
			for (uiter_t i = 0; i < CONTROLLER_COUNT; ++i) {
				c = &G_controllers[i];
				alarm = c->next_check;
				now_sec = NOW();
				now_min = now_sec / SECONDS_PER_MINUTE;

				if (force_controllers || ((alarm > 0) && (now_min >= alarm)) || (do_controllers && (alarm == 0))) {
					if (!invalidated && BITS_ARE_SET(c->iflags, CTRL_FLAG_INVALIDATE|CTRL_FLAG_USES_SENSORS)) {
						invalidated = true;
						invalidate_sensors();
					}
					c->next_check = 0;
					check_controller(c);
				}
			}
		}
*/
		next_wakeup = set_alarms(false);
	}

	// Should never reach here:
	//ERROR_STATE("Reached the end of main()?!");
	return 1;
}

static utime_t set_alarms(bool force) {
	utime_t now_min, now_sec, next;
	char *reason = "Unknown";

	// Zeroing out alarms when they're executed and only setting them if they've
	// been zeroed allows us to catch missed alarms, such as when a controller
	// ran longer than the wait period would have been
	now_sec = NOW();
	now_min = now_sec / SECONDS_PER_MINUTE;
	if (USE_LOGGING && (LOG_APPEND_MINUTES > 0) && ((log_alarm == 0) || force)) {
		next = now_min + LOG_APPEND_MINUTES;
		log_alarm = SNAP_TO_FACTOR(next, LOG_APPEND_MINUTES);
	}
	if ((STATUS_CHECK_MINUTES > 0) && ((status_alarm == 0) || force)) {
		next = now_min + STATUS_CHECK_MINUTES;
		status_alarm = SNAP_TO_FACTOR(next, STATUS_CHECK_MINUTES);
	}

	if (DO_CLOCK_DESCEW && ((descew_alarm == 0) || force)) {
		next = now_min + RTC_CORRECTION_PERIOD_MINUTES;
		descew_alarm = SNAP_TO_FACTOR(next, RTC_CORRECTION_PERIOD_MINUTES);
		// Without this check, a negative correction may be applied repeatedly.
		if ((RTC_CORRECTION_SECONDS < 0) && (descew_alarm <= ((now_sec + -RTC_CORRECTION_SECONDS) / SECONDS_PER_MINUTE))) {
			descew_alarm += RTC_CORRECTION_PERIOD_MINUTES;
		}
	}
	if (DO_FINE_CLOCK_DESCEW && ((fine_descew_alarm == 0) || force)) {
		next = now_min + RTC_FINE_CORRECTION_PERIOD_MINUTES;
		fine_descew_alarm = SNAP_TO_FACTOR(next, RTC_FINE_CORRECTION_PERIOD_MINUTES);
		// Without this check, a negative correction may be applied repeatedly.
		if ((RTC_CORRECTION_SECONDS < 0) && (descew_alarm <= ((now_sec + -RTC_FINE_CORRECTION_SECONDS) / SECONDS_PER_MINUTE))) {
			fine_descew_alarm += RTC_FINE_CORRECTION_PERIOD_MINUTES;
		}
	}

/*
	if (USE_CONTROLLERS) {
		controller_t *c;
		FMEM_STORAGE const controller_static_t *cfg;
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
					next = now_min + CONTROLLER_CHECK_MINUTES;
					c->next_check = SNAP_TO_FACTOR(next, CONTROLLER_CHECK_MINUTES);
				//
				// If the alarm is set but there's no default scheduled time, that
				// means it was set somewhere other than here and if we've made it to
				// this point then we're in a forced update so check on the next round
				} else if (alarm != 0) {
					c->next_check = now_min;
				}
			//
			// Controller-specific polling frequency
			} else {
				//
				// Time of day
				if (BIT_IS_SET(cfg->cflags, CTRL_FLAG_USE_TIME_OF_DAY)) {
					utime_t sm;

					sm = cfg->schedule_minutes;
					next = SNAP_TO_FACTOR(now_min, MINUTES_PER_DAY) + sm;
					//
					// If the alarm is set but not for the normal time, that means it
					// was set somewhere other than here and if we've made it to this
					// point then we're in a forced update so check on the next round
					if ((alarm != 0) && ((alarm % MINUTES_PER_DAY) != sm)) {
						c->next_check = now_min;
					//
					// If the scheduled time would be earlier than now and we're outside
					// the clock scew window, wait until tomorrow
					} else if ((next + CONTROLLER_SCHEDULE_SCEW_WINDOW_MINUTES) < now_min) {
						c->next_check = next + MINUTES_PER_DAY;

					} else {
						c->next_check = next;
					}
				//
				// Periodic
				} else {
					utime_t sm;

					sm = cfg->schedule_minutes;
					next = now_min + sm;
					next = SNAP_TO_FACTOR(next, sm);

					// If the alarm is set but not for the normal time, that means it
					// was set somewhere other than here and if we've made it to this
					// point then we're in a forced update so check on the next round
					if ((alarm != 0) && (next != alarm)) {
						c->next_check = now_min;

					} else {
						c->next_check = next;
					}
				}
			}
		}
	}
*/

	// Check for the next alarm in a separate loop so that I don't have to keep
	// track of what has or hasn't been set and when
	// 12 hours should be more than long enough a default sleep period
	next = now_min + (12U * MINUTES_PER_HOUR);
	if ((log_alarm != 0) && (next > log_alarm)) {
		next = log_alarm;
		reason = "Write log";
	}
	if ((status_alarm != 0) && (next > status_alarm)) {
		next = status_alarm;
		reason = "Update status";
	}
	if (DO_CLOCK_DESCEW && (descew_alarm != 0) && (next > descew_alarm)) {
		next = descew_alarm;
		reason = "Descew clock (coarse)";
	}
	if (DO_FINE_CLOCK_DESCEW && (fine_descew_alarm != 0) && (next > fine_descew_alarm)) {
		next = fine_descew_alarm;
		reason = "Descew clock (fine)";
	}
/*
	if (USE_CONTROLLERS) {
		for (uiter_t i = 0; i < CONTROLLER_COUNT; ++i) {
			controller_t *c = &G_controllers[i];

			if ((c->next_check != 0) && (next > c->next_check)) {
				next = c->next_check;
				reason = "Run controller";
			}
		}
	}
*/

	LOGGER("Next alarm %u minutes for: %s", (uint )(next - now_min), reason);
	return next;
}

static void descew_clock(int_fast16_t correction) {
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
	const ghmon_warning_flags_t warn_SD         = (WARN_SD_SKIPPED|WARN_SD_SKIPPED);
	const ghmon_warning_flags_t warn_power      = (WARN_BATTERY_LOW|WARN_VCC_LOW);
	const ghmon_warning_flags_t warn_sensor     = (WARN_SENSOR);
	const ghmon_warning_flags_t warn_controller = (WARN_CONTROLLER);

	// Delay briefly first so that it doesn't blend into any
	// acknowledgement flashes
	if (ghmon_warnings & warn_power) {
		sleep_ms(STATUS_LED_LONG_DELAY_MS);
		led_flash(1, STATUS_LED_ERR_DELAY_MS);
	} else {
		if (USE_SENSORS && (ghmon_warnings & warn_sensor)) {
			sleep_ms(STATUS_LED_LONG_DELAY_MS);
			led_flash(2, STATUS_LED_ERR_DELAY_MS);
		}
		if (USE_CONTROLLERS && (ghmon_warnings & warn_controller)) {
			sleep_ms(STATUS_LED_LONG_DELAY_MS);
			led_flash(3, STATUS_LED_ERR_DELAY_MS);
		}
		if (USE_SD && (ghmon_warnings & warn_SD)) {
			sleep_ms(STATUS_LED_LONG_DELAY_MS);
			led_flash(4, STATUS_LED_ERR_DELAY_MS);
		}
	}

	return;
}

void error_state_hook(void) {
	led_toggle();
	return;
}
