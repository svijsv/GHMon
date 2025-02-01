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
// terminal.c
// An interactive serial terminal
// NOTES:
//

#include "common.h"

#include "ulib/include/cstrings.h"
#include "ulib/include/ascii.h"

// Needed for atoi()
#include <stdlib.h>

//#define atoi(_n_) strtol((_n_), NULL, 10)

#if uHAL_USE_TERMINAL

#if TIME_YEAR_0 < 2000
# error "TIME_YEAR_0 must be >= 2000"
#endif

DEBUG_CPP_MACRO(TERMINAL_BUFFER_BYTES)


#if ! uHAL_USE_SMALL_MESSAGES
static FMEM_STORAGE const char TERMINAL_INTRO[] =
	"\r\nEntering command terminal\r\nType 'help' for a list of commands.\r\n";
static FMEM_STORAGE const char TERMINAL_OUTRO[] =
	"Leaving command terminal until input is received.\r\n";

#else
static FMEM_STORAGE const char TERMINAL_INTRO[] =
	"\r\nType 'help' for commands\r\n";
static FMEM_STORAGE const char TERMINAL_OUTRO[] =
	"Exiting\r\n";
#endif

static FMEM_STORAGE const char TERMINAL_PROMPT[] =
	"\r$ ";

static txsize_t terminal_gets(char *line_in, txsize_t size);

static int terminalcmd_exit(const char *line_in);
static int terminalcmd_show_info(const char *line_in);
static int terminalcmd_show_help(const char *line_in);
static int terminalcmd_set_time(const char *line_in);
static int terminalcmd_delay_S(const char *line_in);
#if uHAL_USE_FDISK
static int terminalcmd_fdisk(const char *line_in);
#endif // uHAL_USE_FDISK

static FMEM_STORAGE const terminal_cmd_t default_cmds[] = {
#if uHAL_USE_RTC
	{ terminalcmd_set_time,    "set_time",    8 },
#endif
#if uHAL_USE_RTC || uHAL_USE_UPTIME
	{ terminalcmd_delay_S,     "delay",       5 },
#endif
	{ terminalcmd_show_info,   "info",        4 },
	{ terminalcmd_show_help,   "help",        4 },
	{ terminalcmd_exit,        "exit",        4 },
#if uHAL_USE_FDISK
	{ terminalcmd_fdisk,       "format",      6 },
#endif
	{ NULL, {0}, 0 }
};

static FMEM_STORAGE const char default_help[] =
#if ! uHAL_USE_SMALL_MESSAGES
"Accepted commands:\r\n"
#if uHAL_USE_RTC
"   set_time YY.MM.DD hh:mm:ss - Set system time, clock is 24-hour\r\n"
#endif
"   info                       - Print system information\r\n"
#if uHAL_USE_RTC || uHAL_USE_UPTIME
"   delay <seconds>            - Pause the system\r\n"
#endif
# if uHAL_USE_FDISK
"   fdisk                      - Format the SD card\r\n"
# endif
"   help                       - Display this help\r\n"
"   exit                       - Exit the command terminal\r\n"

#else
"Commands: set_time info delay fdisk help exit\r\n"
#endif
;

#define NEXT_TOK(_cs, _sep) (cstring_next_token((_cs), (_sep)))

void terminal(void) {
	char line_in[TERMINAL_BUFFER_BYTES];

#if ! uHAL_SKIP_INIT_CHECKS
#endif
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
#endif

	PUTS_NOF(FROM_FSTR(TERMINAL_INTRO), 0);

	while (terminal_gets(line_in, TERMINAL_BUFFER_BYTES) > 0) {
		FMEM_STORAGE const terminal_cmd_t *cmd;
		bool found = false;

#if TERMINAL_HAVE_EXTRA_CMDS
		for (uiter_t i = 0; terminal_extra_cmds[i].cmd[0] != 0; ++i) {
			cmd = &terminal_extra_cmds[i];
			if (cstring_eqn(line_in, FROM_FSTR(cmd->cmd), cmd->length)) {
				//if (cmd->cmd_handler(NEXT_TOK(line_in, ' ')) != 0) {
				if (cmd->cmd_handler(line_in) != 0) {
					goto END;
				}
				found = true;
				break;
			}
		}
		if (found) {
			continue;
		}
#endif

		for (uiter_t i = 0; default_cmds[i].cmd[0] != 0; ++i) {
			cmd = &default_cmds[i];
			if (cstring_eqn(line_in, FROM_FSTR(cmd->cmd), cmd->length)) {
				//if (cmd->cmd_handler(NEXT_TOK(line_in, ' ')) != 0) {
				if (cmd->cmd_handler(line_in) != 0) {
					goto END;
				}
				found = true;
				break;
			}
		}

		if (!found) {
			PRINTF("Unknown command '%s'\r\n", line_in);
		}
	}

END:
	PUTS_NOF(FROM_FSTR(TERMINAL_OUTRO), 0);

	return;
}

static txsize_t terminal_gets(char *line_in, txsize_t size) {
	uint8_t c;
	txsize_t i;
	bool done, started_line;

	done = false;
	started_line = false;
	i = 0;
	while (1) {
		if (!started_line) {
			PUTS_NOF(FROM_FSTR(TERMINAL_PROMPT), 0);
		}
		if (uart_receive_block(NULL, &c, 1, (uint32_t )TERMINAL_TIMEROUT_S*1000) == ERR_TIMEOUT) {
			PUTS("Timed out waiting for input\r\n", 0);
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
			uint newlines, other;
			utime_t timeout;

			other = 0;
			newlines = 0;
			timeout = SET_TIMEOUT_MS(10);
			// Eat any remaining input
			while (!TIMES_UP(timeout) && (uart_receive_block(NULL, &c, 1, 5) != ERR_TIMEOUT)) {
				switch (c) {
				case '\n':
				case '\r':
					++newlines;
					break;
				default:
					++other;
					break;
				}
			}
			if ((newlines != 0) || (other != 0)) {
#if ! uHAL_USE_SMALL_MESSAGES
				PRINTF("Ignoring %u newlines and %u other characters after the linebreak\r\n", (uint )newlines, (uint )other);
#else
				PRINTF("%u extra bytes\r\n", (uint_t )(newlines + other));
#endif
			}
			break;
		}
	}

	return i;
}

static int terminalcmd_exit(const char *line_in) {
	UNUSED(line_in);

	return 1;
}
static int terminalcmd_show_info(const char *line_in) {
	UNUSED(line_in);
	print_system_info();

	return 0;
}
static int terminalcmd_show_help(const char *line_in) {
	UNUSED(line_in);
	PUTS_NOF(FROM_FSTR(default_help        ), 0);
#if TERMINAL_HAVE_EXTRA_CMDS
	PUTS_NOF(FROM_FSTR(terminal_extra_help ), 0);
#endif

	return 0;
}

#if uHAL_USE_RTC
// Format: 'set_time [[20]YY.MM.DD] [hh:mm[:ss]]'
static int terminalcmd_set_time(const char *line_in) {
	err_t err = ERR_OK;
	time_year_t year;
	uint_fast8_t month = 0, day = 0, hour = 0, second = 0, minute = 0;
	bool bad_format = false, use_date = false, use_time = false;
	const char *nt = NEXT_TOK(line_in, ' ');

	// We need to skip over the digits here in order to determine whether we're
	// reading the date or time so don't bother with atoi() for this first number.
	for (year = 0; ASCII_IS_DIGIT(*nt); ++nt) {
		year *= 10;
		year += ASCII_TO_DIGIT(*nt);
	}
	// '.' means we have a date rather than just a time
	if (*nt == '.') {
		use_date = true;

		++nt;
		month = (uint_fast8_t )atoi(nt);

		nt = NEXT_TOK(nt, '.');
		day = (uint_fast8_t )atoi(nt);

		nt = NEXT_TOK(nt, ' ');
		hour = (uint_fast8_t )atoi(nt);

	} else if (*nt == ':') {
		use_time = true;
		hour = (uint_fast8_t )year;
		year = 0;

	} else {
		bad_format = true;
		goto END;
	}

	nt = NEXT_TOK(nt, ':');
	if (*nt != 0) {
		use_time = true;
		minute = (uint_fast8_t )atoi(nt);

		nt = NEXT_TOK(nt, ':');
		second = (uint_fast8_t )atoi(nt);
	}

	if (!uHAL_SKIP_OTHER_CHECKS) {
		// nt is still at the start of the second if all the possible fields
		// were present, but if not this won't catch junk at the end.
		for (; *nt != 0; ++nt) {
			if (!ASCII_IS_DIGIT(*nt)) {
				PRINTF("Unexpected token(s): %s'\r\n", nt);
				bad_format = true;
				goto END;
			}
		}

		if (use_time) {
			// All types are unsigned so we don't need to check if they're >= 0
			if (hour > 23 || minute > 59 || second > 59) {
				bad_format = true;
			}
		}
		if (use_date) {
			//if ((!IS_IN_RANGE(year, 0, 99)) && (!IS_IN_RANGE(year, 2000, 2099))) {
			if ((year > 99) && (!IS_IN_RANGE(year, 2000, 2099))) {
				bad_format = true;
			}
			if (!IS_IN_RANGE(month, 1, 12)) {
				bad_format = true;
			}
			if (!IS_IN_RANGE(day, 1, 31)) {
				bad_format = true;
			}
		}
		if (bad_format) {
			goto END;
		}
	}

	datetime_t new_datetime = { 0 };
	if (use_date) {
		if (year < 100) {
			year += 2000;
		}

		if (!uHAL_USE_SMALL_MESSAGES) {
			PRINTF("Setting date to %u.%02u.%02u\r\n", (uint )(year), (uint )month, (uint )day);
		}
		new_datetime.year  = year;
		new_datetime.month = month;
		new_datetime.day   = day;
	}
	if (use_time) {
		if (!uHAL_USE_SMALL_MESSAGES) {
			PRINTF("Setting time to %02u:%02u:%02u\r\n", (uint )hour, (uint )minute, (uint )second);
		}
		new_datetime.hour   = hour;
		new_datetime.minute = minute;
		new_datetime.second = second;
	}
	err = set_RTC_datetime(&new_datetime);
	if (err != ERR_OK) {
		if (!uHAL_USE_SMALL_MESSAGES) {
			PRINTF("   Error %d while setting date/time\r\n", (int )err);
		} else {
			PRINTF("   Error %d\r\n", (int )err);
		}
	}

END:
	if (bad_format) {
		PUTS("Invalid format; use '[[20]YY.MM.DD] [hh:mm[:ss]]'\r\n", 0);
	}
	return 0;
}
#endif

#if uHAL_USE_RTC || uHAL_USE_UPTIME
// Format: 'delay <N>'
static int terminalcmd_delay_S(const char *line_in) {
	utime_t n;

	n = atoi(NEXT_TOK(line_in, ' '));

#if ! uHAL_USE_SMALL_MESSAGES
	PRINTF("Delaying %u seconds...", (uint )n);
#endif
	n += NOW();
	//delay_ms(n * 1000);
	while (NOW() < n) {
		delay_ms(50);
	}
#if ! uHAL_USE_SMALL_MESSAGES
	PRINTF("done.\r\n");
#endif

	return 0;
}
#endif

#if uHAL_USE_FDISK
static int terminalcmd_fdisk(const char *line_in, txsize_t size) {
	static FMEM_STORAGE const char confirm_string[] = "ERASE MY CARD!";
	const char *tc;
	uint8_t len;

	tc = FROM_FSTR(confirm_string);
	PRINTF("Format SD card? Type '%s' to confirm:\r\n", tc);
	terminal_gets(line_in, size);

	len = sizeof(confirm_string)-1;
	for (uiter_t i = 0; i < len; ++i) {
		if (line_in[i] != tc[i]) {
			PUTS("Aborting format", 0);
			return 0;
		}
	}

	format_SD();

	return;
}
#endif // uHAL_USE_FDISK


#endif // uHAL_USE_TERMINAL
