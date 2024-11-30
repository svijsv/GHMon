// SPDX-License-Identifier: GPL-3.0-only
/***********************************************************************
*                                                                      *
*                                                                      *
* Copyright 2024 svijsv                                                *
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
// terminal.h
// Configure the UART terminal
// NOTES:
//
#ifndef _TERMINAL_H
#define _TERMINAL_H

#include "ulib/include/cstrings.h" // For cstring_next_token()
#include <stdlib.h> // For atoi()


void uart_rx_irq_hook(uart_port_t *p) {
	UNUSED(p);
	uHAL_SET_STATUS(uHAL_FLAG_IRQ);
	SET_BIT(ghmon_IRQs, TERMINAL_IRQ_FLAG);

	return;
}

#define NEXT_TOK(_cs_, _sep_) (cstring_next_token((_cs_), (_sep_)))

#if USE_STATUS_LED
static int terminalcmd_led_flash(const char *line_in) {
	int n;

	n = atoi(NEXT_TOK(line_in, ' '));

	PRINTF("Flashing LED %u times.\r\n", (uint )n);

	led_flash(n, 300);

	return 0;
}
static int terminalcmd_led_on(const char *line_in) {
	UNUSED(line_in);

	PUTS("Turning LED on\r\n", 0);
	led_on();
	return 0;
}
static int terminalcmd_led_off(const char *line_in) {
	UNUSED(line_in);

	PUTS("Turning LED off\r\n", 0);
	led_off();
	return 0;
}
static int terminalcmd_led_toggle(const char *line_in) {
	UNUSED(line_in);

	PUTS("Toggling LED\r\n", 0);
	led_toggle();
	return 0;
}
#endif // USE_STATUS_LED

#if USE_LOGGING && LOG_LINE_BUFFER_COUNT > 0
static int terminalcmd_play_log(const char *line_in) {
	UNUSED(line_in);

	print_log(serial_printf);
	return 0;
}
static int terminalcmd_write_log(const char *line_in) {
	UNUSED(line_in);

	write_log_to_storage();
	return 0;
}
#endif

static int terminalcmd_reset(const char *line_in) {
	UNUSED(line_in);

	PUTS("Resetting device\r\n", 0);
	platform_reset();
	return 0;
}


FMEM_STORAGE const terminal_cmd_t terminal_extra_cmds[] = {
#if USE_STATUS_LED
	{ terminalcmd_led_flash,   "led_flash",   9 },
	{ terminalcmd_led_on,      "led_on",      6 },
	{ terminalcmd_led_off,     "led_off",     7 },
	{ terminalcmd_led_toggle,  "led_toggle", 10 },
#endif
#if USE_LOGGING && LOG_LINE_BUFFER_COUNT > 0
	{ terminalcmd_play_log,    "play_log",    8 },
	{ terminalcmd_write_log,   "write_log",   9 },
#endif
	{ terminalcmd_reset,       "reset",       5 },
	{ NULL, {0}, 0 },
};
FMEM_STORAGE const char terminal_extra_help[] =
#if USE_STATUS_LED
"   led_flash <N>     - Flash the LED <N> times\r\n"
"   led_on            - Turn the LED on\r\n"
"   led_off           - Turn the LED off\r\n"
"   led_toggle        - Toggle the LED\r\n"
#endif
#if USE_LOGGING && LOG_LINE_BUFFER_COUNT > 0
"   play_log          - Print the log buffer"
"   write_log         - Write the log buffer to storage"
#endif
"   reset             - Reset the device\r\n"
;

#endif // _TERMINAL_H
