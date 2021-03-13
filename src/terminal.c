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
// terminal.c
// An interactive serial terminal
// NOTES:
//

#ifdef __cplusplus
 extern "C" {
#endif

//#define DEBUG 1
/*
* Includes
*/
#include "terminal.h"

#include "serial.h"
#include "sensors.h"
#include "controllers.h"
#include "fdisk.h"
#include "log.h"

#if USE_TERMINAL

#if YEAR_0 < 2000
# error "YEAR_0 must be >= 2000"
#endif // YEAR_0 < 2000

/*
* Static values
*/
// Use const pointers instead of #define for these to prevent using extra
// memory for each place where they occur
static const char *TERMINAL_INTRO =
	"\r\nEntering command terminal\r\nType 'help' for a list of commands.\r\n";
static const char *TERMINAL_OUTRO =
	"Leaving command terminal until input is received.\r\n";
// Begin with '\r' to make sure the prompt is at the start of the line
static const char *TERMINAL_PROMPT =
	"\r$ ";
static const char *TERMINAL_HELP =
"Accepted commands:\r\n"
"   set_time YY.MM.DD hh:mm:ss - Set system time, clock is 24-hour\r\n"
"   info                       - Print system info\r\n"
"   status                     - Display sensor status\r\n"
#if USE_CONTROLLERS
"   controllers                - Check the controllers for met conditions\r\n"
#endif // USE_CONTROLLERS
#if USE_LOGGING
"   synclog                    - Write any buffered log data to the SD card\r\n"
#endif // USE_LOGGING
"   led_on\r\n"
"   led_off\r\n"
"   led_toggle\r\n"
#if USE_FDISK
"   fdisk                      - Format the SD card\r\n"
#endif // USE_FDISK
"   test                       - Stub command for testing new functionality\r\n"
"   help                       - Display this help\r\n"
"   exit                       - Exit the command terminal\r\n"
;


/*
* Types
*/


/*
* Variables
*/


/*
* Local function prototypes
*/
static uiter_t terminal_gets(char *line_in, uiter_t size);

static int  read_number(char *token);
static void terminalcmd_print_sensor_status(void);
static void terminalcmd_set_time(char *token);
static void terminalcmd_led_flash(char *token);
static void terminalcmd_test(char *token);
#if USE_LOGGING
static void terminalcmd_sync_log(void);
#endif // USE_LOGGING
#if USE_CONTROLLERS
static void terminalcmd_run_controllers(void);
#endif // USE_CONTROLLERS
#if USE_FDISK
static void terminalcmd_fdisk(char *line_in, uiter_t size);
#endif // USE_FDISK


/*
* Interrupt handlers
*/


/*
* Functions
*/
void terminal(void) {
	char line_in[TERMINAL_BUFFER_SIZE];

	serial_print(TERMINAL_INTRO, 0);
	led_off();
	led_flash(1, DELAY_SHORT);

	while (terminal_gets(line_in, TERMINAL_BUFFER_SIZE) > 0) {
		if (cstring_ncmp(line_in, "set_time ", 9) == 0) {
			terminalcmd_set_time(line_in);

		} else if (cstring_ncmp(line_in, "led_flash ", 10) == 0) {
			terminalcmd_led_flash(line_in);

		} else if (cstring_cmp(line_in, "led_on") == 0) {
			serial_print("Turning LED on\r\n", 0);
			led_on();

		} else if (cstring_cmp(line_in, "led_off") == 0) {
			serial_print("Turning LED off\r\n", 0);
			led_off();

		} else if (cstring_cmp(line_in, "led_toggle") == 0) {
			led_toggle();

		} else if (cstring_cmp(line_in, "exit") == 0) {
			goto END;

		} else if (cstring_cmp(line_in, "info") == 0) {
			print_system_info();

		} else if (cstring_cmp(line_in, "status") == 0) {
			terminalcmd_print_sensor_status();

#if USE_CONTROLLERS
		} else if (cstring_cmp(line_in, "controllers") == 0) {
			terminalcmd_run_controllers();
#endif // USE_CONTROLLERS

#if USE_LOGGING
		} else if (cstring_cmp(line_in, "synclog") == 0) {
			terminalcmd_sync_log();
#endif // USE_LOGGING

		} else if (cstring_ncmp(line_in, "help", 4) == 0) {
			serial_print(TERMINAL_HELP, 0);

#if USE_FDISK
		} else if (cstring_cmp(line_in, "format") == 0) {
			terminalcmd_fdisk(line_in, TERMINAL_BUFFER_SIZE);
#endif // USE_FDISK

		} else if (cstring_ncmp(line_in, "test", 4) == 0) {
			terminalcmd_test(line_in);

		} else {
			serial_printf("Unknown command '%s'\r\n", line_in);
		}
	}

END:
	serial_print(TERMINAL_OUTRO, 0);

	return;
}

static uiter_t terminal_gets(char *line_in, uiter_t size) {
	uint8_t c;
	uiter_t i;
	bool done, started_line;

	done = false;
	started_line = false;
	i = 0;
	while (1) {
		if (!started_line) {
			serial_print(TERMINAL_PROMPT, 0);
		}
		if (uart_receive_block(&c, 1, TERMINAL_TIMEOUT_S*1000) == ETIMEOUT) {
			serial_print("Timed out waiting for input\r\n", 0);
			line_in[0] = 0;
			return 0;
		}

		// TODO: ASCII escape sequences, especially CTRL-C
		// TODO: Command history?
		switch (c) {
			case '\n':
			case '\r':
				if (started_line) {
					done = true;
					if (i < size) {
						line_in[i] = 0;
					} else {
						line_in[size-1] = 0;
					}
				}
				break;
			case 0x7F: // ASCII DEL
			case '\b':
				if (i > 0) {
					--i;
				}
				break;
			case ' ':
			case '\t':
				if (started_line) {
					if (i < size) {
						line_in[i] = (char )c;
					}
					++i;
				}
				break;
			default:
				started_line = true;
				if (i < size) {
					line_in[i] = (char )c;
				}
				++i;
				break;
		}

		if (done) {
			uiter_t newlines, other;
			utime_t timeout;

			other = 0;
			newlines = 0;
			timeout = SET_TIMEOUT(10);
			// Eat any remaining input
			while (!TIMES_UP(timeout) && (uart_receive_block(&c, 1, 1) != ETIMEOUT)) {
				switch (c) {
				case '\n':
					++newlines;
					break;
				default:
					++other;
					break;
				}
			}
			if ((newlines != 0) || (other != 0)) {
				serial_printf("Ignoring %u newlines and %u other characters after the linebreak", (uint )newlines, (uint )other);
			}
			break;
		}
	}

	return i;
}

static void terminalcmd_print_sensor_status(void) {
	serial_print("Checking sensor status...\r\n", 0);
	invalidate_sensors();
	check_sensors();

	serial_printf("   VCC: %u\r\n   CPU_temp: %u\r\n\n   I  Name  Status\r\n",
		(uint )G_vcc_voltage, (uint16_t )G_mcu_temp);
	for (uiter_t i = 0; i < SENSOR_COUNT; ++i) {
		serial_printf("   %u  %s   %d\r\n", (uint )i, SENSORS[i].name, (int )G_sensors[i].status);
	}

	return;
}
#if USE_CONTROLLERS
static void terminalcmd_run_controllers(void) {
	serial_print("Running controller checks...\r\n", 0);

	for (uiter_t i = 0; i < CONTROLLER_COUNT; ++i) {
		check_controller(&G_controllers[i]);
	}

	return;
}
#endif // USE_CONTROLLERS
#if USE_LOGGING
static void terminalcmd_sync_log(void) {
	log_status(true);
}
#endif // USE_LOGGING
static int read_number(char *token) {
	char num[5];
	uiter_t i;

	for (i = 0; ((token[i] >= '0') && (token[i] <= '9') && (i < 4)); ++i) {
		num[i] = token[i];
	}
	num[i] = 0;
	return cstring_to_int(num);
}
// Format: 'set_time YY.MM.DD hh:mm:ss'
static void terminalcmd_set_time(char *token) {
	uiter_t i;
	int year, month, day, hour, minute, second;
	bool format_ok;
	err_t err;

	// Year
	// Two-digit years won't be an issue until the distant year 2000.
	token = (char *)cstring_next_token(token, ' ');
	year = read_number(token);
	// Month
	token = (char *)cstring_next_token(token, '.');
	month = read_number(token);
	// Day
	token = (char *)cstring_next_token(token, '.');
	day = read_number(token);

	// Hour
	token = (char *)cstring_next_token(token, ' ');
	hour = read_number(token);
	// Minute
	token = (char *)cstring_next_token(token, ':');
	minute = read_number(token);
	// Second
	token = (char *)cstring_next_token(token, ':');
	second = read_number(token);

	for (i = 0; ((token[i] >= '0') && (token[i] <= '9')); ++i) {
		// Nothing to do here
	}
	if (token[i] != 0) {
		serial_printf("Unexpected token(s): %s'\r\n", token);
		goto END;
	}

	format_ok = true;
	if (!IS_BETWEEN(hour, 0, 23)) {
		format_ok = false;
	}
	if (!IS_BETWEEN(minute, 0, 59)) {
		format_ok = false;
	}
	if (!IS_BETWEEN(second, 0, 59)) {
		format_ok = false;
	}
	if ((!IS_BETWEEN(year, 0, 99)) && (year < 2000)) {
		format_ok = false;
	}
	if (!IS_BETWEEN(month, 1, 12)) {
		format_ok = false;
	}
	if (!IS_BETWEEN(day, 1, 31)) {
		format_ok = false;
	}

	if (!format_ok) {
		serial_print("Invalid format; use 'YY.MM.DD hh:mm:ss'\r\n", 0);
		goto END;
	}

	if (year > 2000) {
		year -= 2000;
	}
	serial_printf("Setting time to 20%02u.%02u.%02u %02u:%02u:%02u\r\n",
		(uint )year, (uint )month, (uint )day, (uint )hour, (uint )minute, (uint )second);

	year -= (YEAR_0 - 2000);
	if (year < 0) {
		year = 0;
	}
	if (((err = set_date(year, month, day)) != EOK) || ((err = set_time(hour, minute, second)) != EOK)) {
		serial_printf("   Error %u while setting time\r\n", (uint )err);
		goto END;
	}

END:
	return;
}
// Format: 'led_flash <N>'
static void terminalcmd_led_flash(char *token) {
	int n;

	token = (char *)cstring_next_token(token, ' ');
	n = read_number(token);

	serial_printf("Flashing LED %u times.\r\n", (uint )n);

	led_flash(n, DELAY_SHORT);

	return;
}
#if USE_FDISK
static void terminalcmd_fdisk(char *line_in, uiter_t size) {
	static const char *confirm_string = "ERASE MY CARD!";
	uiter_t len;

	serial_printf("Format SD card? Type '%s' to confirm:\r\n", confirm_string);
	terminal_gets(line_in, size);

	len = cstring_len(confirm_string);
	for (uiter_t i = 0; i < len; ++i) {
		if (line_in[i] != confirm_string[i]) {
			serial_print("Aborting format", 0);
			return;
		}
	}

	format_SD();

	return;
}
#endif // USE_FDISK

//#include "../temp/terminal_X_test.h"
//#include "../temp/terminal_sd_test.h"
#ifndef TERMINAL_TEST_CMD
static void terminalcmd_test(char *token) {
	UNUSED(token);
#if DEBUG

	serial_printf("sensor struct sizes:\r\n\tsensor_t: %u\r\n\tsensor_static_t: %u\r\n\tsensor_devcfg_t: %u\r\n\tSENSORS: %u\r\n\tG_sensors: %u\r\n",
		(uint )sizeof(sensor_t), (uint )sizeof(sensor_static_t), (uint )sizeof(sensor_devcfg_t), (uint )sizeof(SENSORS), (uint )sizeof(G_sensors));
#if USE_CONTROLLERS
	serial_printf("controller struct sizes:\r\n\tcontroller_t: %u\r\n\tcontroller_static_t: %u\r\n\tCONTROLLERS: %u\r\n\tG_controllers: %u\r\n",
		(uint )sizeof(controller_t), (uint )sizeof(controller_static_t), (uint )sizeof(CONTROLLERS), (uint )sizeof(G_controllers));
#endif

#if USE_CONTROLLERS && USE_SMALL_CONTROLLERS < 1
	for (uiter_t i = 0; i < CONTROLLER_COUNT; ++i) {
		serial_printf("controller %u: next check in %d minutes\r\n", (uint )i, (int )(((int32_t )G_controllers[i].next_check - (int32_t )NOW())/60));
	}
#endif // USE_SMALL_CONTROLLERS < 1

#if USE_LOGGING
	serial_printf("Log print buffer size: %u\r\n", (uint )PRINT_BUFFER_SIZE);
#endif

#endif // DEBUG
	return;
}
#endif // TERMINAL_TEST_CMD

#endif // USE_TERMINAL

#ifdef __cplusplus
 }
#endif
