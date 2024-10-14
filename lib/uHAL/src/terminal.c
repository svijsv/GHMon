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

// Needed for atoi()
#include <stdlib.h>

//#define atoi(_n_) strtol((_n_), NULL, 10)

#if uHAL_USE_TERMINAL

#if TIME_YEAR_0 < 2000
# error "TIME_YEAR_0 must be >= 2000"
#endif

DEBUG_CPP_MACRO(TERMINAL_BUFFER_BYTES)


#if uHAL_USE_SMALL_CODE < 2
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
	{ terminalcmd_set_time,    "set_time",    8 },
	{ terminalcmd_delay_S,     "delay",       5 },
	{ terminalcmd_show_info,   "info",        4 },
	{ terminalcmd_show_help,   "help",        4 },
	{ terminalcmd_exit,        "exit",        4 },
#if uHAL_USE_FDISK
	{ terminalcmd_fdisk,       "format",      6 },
#endif
	{ NULL, {0}, 0 }
};

static FMEM_STORAGE const char default_help[] =
#if uHAL_USE_SMALL_CODE < 2
"Accepted commands:\r\n"
"   set_time YY.MM.DD hh:mm:ss - Set system time, clock is 24-hour\r\n"
"   info                       - Print system information\r\n"
"   delay <seconds>\r\n"
# if uHAL_USE_FDISK
"   fdisk                      - Format the SD card\r\n"
# endif // uHAL_USE_FDISK
"   help                       - Display this help\r\n"
"   exit                       - Exit the command terminal\r\n"

#else // uHAL_USE_SMALL_CODE
"Commands: set_time info delay fdisk help exit\r\n"
#endif // uHAL_USE_SMALL_CODE
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
#if uHAL_USE_SMALL_CODE < 2
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

// Format: 'set_time YY.MM.DD hh:mm:ss'
static int terminalcmd_set_time(const char *line_in) {
	int year, month, day, hour, minute, second;
	bool format_ok;
	err_t err;
	const char *nt;

	// Year
	// Two-digit years won't be an issue until the distant year 2000.
	nt = NEXT_TOK(line_in, ' ');
	year = atoi(nt);
	// Month
	nt = NEXT_TOK(nt, '.');
	month = atoi(nt);
	// Day
	nt = NEXT_TOK(nt, '.');
	day = atoi(nt);

	// Hour
	nt = NEXT_TOK(nt, ' ');
	hour = atoi(nt);
	// Minute
	nt = NEXT_TOK(nt, ':');
	minute = atoi(nt);
	// Second
	nt = NEXT_TOK(nt, ':');
	second = atoi(nt);

	for (uiter_t i = 0; nt[i] != 0; ++i) {
		switch (nt[i]) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			break;
		default:
			PRINTF("Unexpected token(s): %s'\r\n", &nt[i]);
			goto END;
			break;
		}
	}

	format_ok = true;
	if (!IS_IN_RANGE(hour, 0, 23)) {
		format_ok = false;
	}
	if (!IS_IN_RANGE(minute, 0, 59)) {
		format_ok = false;
	}
	if (!IS_IN_RANGE(second, 0, 59)) {
		format_ok = false;
	}
	if ((!IS_IN_RANGE(year, 0, 99)) && (year < 2000)) {
		format_ok = false;
	}
	if (!IS_IN_RANGE(month, 1, 12)) {
		format_ok = false;
	}
	if (!IS_IN_RANGE(day, 1, 31)) {
		format_ok = false;
	}

	if (!format_ok) {
		PUTS("Invalid format; use 'YY.MM.DD hh:mm:ss'\r\n", 0);
		goto END;
	}

	if (year > 2000) {
		year -= 2000;
	}
#if uHAL_USE_SMALL_CODE < 2
	PRINTF("Setting time to 20%02u.%02u.%02u %02u:%02u:%02u\r\n",
		(uint )year, (uint )month, (uint )day, (uint )hour, (uint )minute, (uint )second);
#endif

	year -= (TIME_YEAR_0 - 2000);
	if (year < 0) {
		year = 0;
	}
	if (((err = set_date(year, month, day)) != ERR_OK) || ((err = set_time(hour, minute, second)) != ERR_OK)) {
#if uHAL_USE_SMALL_CODE < 2
		PRINTF("   Error %d while setting time\r\n", (int )err);
#else
		PRINTF("   Error %d\r\n", (int )err);
#endif
		goto END;
	}

END:
	return 0;
}

// Format: 'delay <N>'
static int terminalcmd_delay_S(const char *line_in) {
	utime_t n;

	n = atoi(NEXT_TOK(line_in, ' '));

#if uHAL_USE_SMALL_CODE < 2
	PRINTF("Delaying %u seconds...", (uint )n);
#endif
	n += NOW();
	//delay_ms(n * 1000);
	while (NOW() < n) {
		delay_ms(50);
	}
#if uHAL_USE_SMALL_CODE < 2
	PRINTF("done.\r\n");
#endif

	return 0;
}
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
