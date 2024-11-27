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
// log.c
// Manage log files
// NOTES:
//
#include "common.h"

#if USE_LOGGING

#include "log.h"

#include "sensors.h"
#include "controllers.h"

#include "ulib/include/cstrings.h"
#include "ulib/include/fmem.h"
#include "ulib/include/halloc.h"
#include "ulib/include/printf.h"

#if LOG_LINES_PER_FILE == 1
  // Why not? There was a note about it causing an infinite recursion but I
  // can't find where that would happen.
# error "LOG_LINES_PER_FILE can't be '1'"
#endif

#define NEED_LOG_FILE_NAME (WRITE_LOG_TO_SD)

#if LOG_PRINT_BUFFER_SIZE > 0xFFFFFFFF
 typedef uint64_t print_buffer_size_t;
#elif LOG_PRINT_BUFFER_SIZE > 0xFFFF
 typedef uint32_t print_buffer_size_t;
#elif LOG_PRINT_BUFFER_SIZE > 0xFF
 typedef uint16_t print_buffer_size_t;
#else
 typedef uint8_t print_buffer_size_t;
#endif

#if LOG_LINE_BUFFER_COUNT > 0xFFFFFFFF
 typedef uint64_t log_line_buffer_size_t;
#elif LOG_LINE_BUFFER_COUNT > 0xFFFF
 typedef uint32_t log_line_buffer_size_t;
#elif LOG_LINE_BUFFER_COUNT > 0xFF
 typedef uint16_t log_line_buffer_size_t;
#else
 typedef uint8_t log_line_buffer_size_t;
#endif

#if NEED_LOG_FILE_NAME
 static char logfile_name[] = LOGFILE_NAME_PATTERN;
#endif

static const char *invalid_value = "(invalid)";
static const char *no_status = "(unavailable)";
static const char *line_end = "\r\n";

static bool have_log_header = false;
static uint32_t lines_logged_this_file = 0;

#if LOG_SENSORS_BY_DEFAULT
# define DO_SENSOR(_i_) (!BIT_IS_SET(SENSORS[_i_].cfg_flags, SENSOR_CFG_FLAG_NOLOG))
#else
# define DO_SENSOR(_i_) (BIT_IS_SET(SENSORS[_i_].cfg_flags, SENSOR_CFG_FLAG_LOG))
#endif
#if LOG_CONTROLLERS_BY_DEFAULT
# define DO_CONTROLLER(_i_) (!BIT_IS_SET(CONTROLLERS[_i_].cfg_flags, CONTROLLER_CFG_FLAG_NOLOG))
#else
# define DO_CONTROLLER(_i_) (BIT_IS_SET(CONTROLLERS[_i_].cfg_flags, CONTROLLER_CFG_FLAG_LOG))
#endif

// This only need to save information which changes and is actually recorded
typedef struct {
	utime_t  system_time;

#if USE_CONTROLLER_STATUS
	CONTROLLER_STATUS_T *controller_status;
#endif
	SENSOR_READING_T *sensor_reading;

	uint8_t *controller_status_flags;
	uint8_t *sensor_status_flags;

	uint8_t ghmon_warnings;
} log_line_buffer_t;
static SENSOR_INDEX_T sensor_count;
static CONTROLLER_INDEX_T controller_count;

#if LOG_LINE_BUFFER_COUNT > 0
// Use a ring buffer so that if there's a problem preventing us from writing
// out the log it's the oldest data that gets overwritten.
static struct {
	log_line_buffer_t lines[LOG_LINE_BUFFER_COUNT];
	log_line_buffer_size_t tail;
	log_line_buffer_size_t size;
} log_buffer = { 0 };
#endif

#if LOG_PRINT_BUFFER_SIZE > 0
static struct {
	print_buffer_size_t size;
	uint8_t buffer[LOG_PRINT_BUFFER_SIZE];
} print_buffer = { 0 };
#endif

extern err_t log_pre_write_hook(void);
extern err_t log_post_write_hook(void);

static void _write_log_to_storage(void);
static err_t write_log_line_to_storage(log_line_buffer_t *line);
static void print_log_line(void (*pf)(const char *format, ...), log_line_buffer_t *line);
static bool buffer_is_full(void);
static void buffer_status_line(void);
static void lprintf_putc(uint_fast8_t c);
static void lprintf(const char *format, ...)
	__attribute__ ((format(printf, 1, 2)));
static char* format_print_time(utime_t uptime);
static char* format_warnings(uint8_t warnings);
static err_t open_log_storage(void);
static err_t open_log_file(void);
static void close_log_storage(void);
static err_t rotate_log_file(void);
static void reset_print_buffer(void);
static void write_log_header(void);

#include "log_uart.h"
#include "log_sd.h"

void log_init(void) {
	SENSOR_INDEX_T sn = 0;
	CONTROLLER_INDEX_T cn = 0;

	for (SENSOR_INDEX_T i = 0; i < SENSOR_COUNT; ++i) {
		if (DO_SENSOR(i)) {
			++sn;
		}
	}
	sensor_count = sn;

	for (CONTROLLER_INDEX_T i = 0; i < CONTROLLER_COUNT; ++i) {
		if (DO_CONTROLLER(i)) {
			++cn;
		}
	}
	controller_count = cn;

#if LOG_LINE_BUFFER_COUNT > 0
	for (log_line_buffer_size_t i = 0; i < LOG_LINE_BUFFER_COUNT; ++i) {
		if (sn > 0) {
			log_buffer.lines[i].sensor_reading = halloc(sn * sizeof(*log_buffer.lines[0].sensor_reading));
			log_buffer.lines[i].sensor_status_flags = halloc(sn * sizeof(*log_buffer.lines[0].sensor_status_flags));
		}

		if (cn > 0) {
			log_buffer.lines[i].controller_status = halloc(cn * sizeof(*log_buffer.lines[0].controller_status));
			log_buffer.lines[i].controller_status_flags = halloc(cn * sizeof(*log_buffer.lines[0].controller_status_flags));
		}
	}
#endif

#if WRITE_LOG_TO_SD
	log_init_SD();
#endif
#if WRITE_LOG_TO_UART
	log_init_UART();
#endif

	LOGGER("Initialized logging for %u sensors and %u controllers", (uint )sn, (uint )cn);

	return;
}

static void log_status_line(log_line_buffer_t *line) {
	assert(line != NULL);

	check_common_sensor_warnings();
	check_common_controller_warnings();

	line->ghmon_warnings = ghmon_warnings;
	line->system_time = NOW();
	for (SENSOR_INDEX_T i = 0, si = 0; i < SENSOR_COUNT; ++i) {
		if (!DO_SENSOR(i)) {
			continue;
		}
		line->sensor_reading[si] = read_sensor_by_index(i, false, 0);
		line->sensor_status_flags[si] = sensors[i].status_flags;
		++si;
	}
	for (CONTROLLER_INDEX_T i = 0, si = 0; i < CONTROLLER_COUNT; ++i) {
		if (!DO_CONTROLLER(i)) {
			continue;
		}
#if USE_CONTROLLER_STATUS
		line->controller_status[si] = controllers[i].status;
#endif
		line->controller_status_flags[si] = controllers[i].status_flags;
		++si;
	}

	return;
}

void log_status(void) {
	if (buffer_is_full()) {
		log_line_buffer_t current_status;
		SENSOR_READING_T sensor_reading[sensor_count];
#if USE_CONTROLLER_STATUS
		CONTROLLER_STATUS_T controller_status[controller_count];
#endif
		uint8_t sensor_status_flags[sensor_count];
		uint8_t controller_status_flags[controller_count];

		current_status.sensor_reading = sensor_reading;
		current_status.sensor_status_flags = sensor_status_flags;
#if USE_CONTROLLER_STATUS
		current_status.controller_status = controller_status;
#endif
		current_status.controller_status_flags = controller_status_flags;

		if (open_log_storage() != ERR_OK) {
			buffer_status_line();
			return;
		}

		log_status_line(&current_status);
		_write_log_to_storage();
		write_log_line_to_storage(&current_status);
		close_log_storage();
	} else {
		buffer_status_line();
	}
	return;
}
static void _write_log_to_storage(void) {
#if LOG_LINE_BUFFER_COUNT > 0
	log_line_buffer_size_t head;

	if (log_buffer.size > log_buffer.tail) {
		head = LOG_LINE_BUFFER_COUNT - (log_buffer.size - log_buffer.tail);
	} else {
		head = log_buffer.tail - log_buffer.size;
	}
	while (log_buffer.size > 0) {
		if (write_log_line_to_storage(&log_buffer.lines[head]) != ERR_OK) {
			return;
		}

		++head;
		if (head == LOG_LINE_BUFFER_COUNT) {
			head = 0;
		}
		--log_buffer.size;
	}

#endif // LOG_LINE_BUFFER_COUNT > 0
	return;
}
static void print_log_line(void (*pf)(const char *format, ...), log_line_buffer_t *line) {
	pf("%s\t%s", format_print_time(line->system_time), format_warnings(line->ghmon_warnings));

	for (SENSOR_INDEX_T i = 0, si = 0; i < SENSOR_COUNT; ++i) {
		if (!DO_SENSOR(i)) {
			continue;
		}

		if (BIT_IS_SET(line->sensor_status_flags[si], SENSOR_STATUS_FLAG_ERROR)) {
			pf("\t!");
		} else {
			pf("\t");
		}
		if (!BIT_IS_SET(line->sensor_status_flags[si], SENSOR_STATUS_FLAG_INITIALIZED)) {
			pf("%s", invalid_value);
		} else {
			pf("%d", (int )line->sensor_reading[si]);
		}
		++si;
	}

	for (CONTROLLER_INDEX_T i = 0, si = 0; i < CONTROLLER_COUNT; ++i) {
		if (!DO_CONTROLLER(i)) {
			continue;
		}

		if (BIT_IS_SET(line->controller_status_flags[si], CONTROLLER_STATUS_FLAG_ERROR)) {
			pf("\t!");
		} else {
			pf("\t");
		}
		if (!BIT_IS_SET(line->controller_status_flags[si], CONTROLLER_STATUS_FLAG_INITIALIZED)) {
			pf("%s", invalid_value);
		} else {
#if USE_CONTROLLER_STATUS
			pf("%d", (int )line->controller_status[si]);
#else
			pf("%s", no_status);
#endif
		}
		++si;
	}
	pf("%s", line_end);

	return;
}
static err_t write_log_line_to_storage(log_line_buffer_t *line) {
	if (LOG_LINES_PER_FILE > 0 && lines_logged_this_file == LOG_LINES_PER_FILE) {
		err_t res;

		if ((res = rotate_log_file()) != ERR_OK) {
			return res;
		}
	}

	print_log_line(lprintf, line);
	++lines_logged_this_file;

	return ERR_OK;
}
void write_log_to_storage(void) {
	if (open_log_storage() != ERR_OK) {
		return;
	}
	_write_log_to_storage();
	close_log_storage();
	return;
}

static char* format_print_time(utime_t uptime) {
	// 17 is enough to hold '2021.02.15 12:00' with a trailing NUL
	static char timestr[17];

	// Assume that if the year hasn't been set, this is an uptime not a date
	// Also assume that it's at least 3 years since YEAR_0 and that this won't
	// run continuously for more than 3 years
	if (uptime < (SECONDS_PER_YEAR * 3)) {
		uint offset;
		const uint len = SIZEOF_ARRAY(timestr);

		// Days
		offset = cstring_from_uint(timestr, len, (uptime / SECONDS_PER_DAY), 10);
		timestr[offset] = 'd'; ++offset;

		// Hours
		uptime %= SECONDS_PER_DAY;
		offset += cstring_from_uint(&timestr[offset], len - offset, (uptime / SECONDS_PER_HOUR), 10);
		timestr[offset] = 'h'; ++offset;

		// Minutes
		offset += cstring_from_uint(&timestr[offset], len - offset, ((uptime % SECONDS_PER_HOUR) / SECONDS_PER_MINUTE), 10);
		timestr[offset] = 'm';
		timestr[offset+1] = 0;
	} else {
		uiter_t i;
		uint8_t year, month, day, hour, minute, second;
		uint16_t ayear;

		seconds_to_date(uptime, &year, &month, &day);
		seconds_to_time(uptime, &hour, &minute, &second);

		i = 0;
		ayear = year + TIME_YEAR_0;
		timestr[i++] = '0' + ((ayear / 1000));
		timestr[i++] = '0' + ((ayear % 1000) / 100);
		timestr[i++] = '0' + ((ayear % 100 ) / 10);
		timestr[i++] = '0' + ((ayear % 10  ));
		timestr[i++] = '.';
		timestr[i++] = '0' + ((month / 10 ));
		timestr[i++] = '0' + ((month % 10 ));
		timestr[i++] = '.';
		timestr[i++] = '0' + ((day / 10 ));
		timestr[i++] = '0' + ((day % 10 ));
		timestr[i++] = '_';
		timestr[i++] = '0' + ((hour / 10 ));
		timestr[i++] = '0' + ((hour % 10 ));
		timestr[i++] = ':';
		timestr[i++] = '0' + ((minute / 10 ));
		timestr[i++] = '0' + ((minute % 10 ));
		// Dont do seconds
		// timestr[i++] = ':';
		// timestr[i++] = '0' + ((second / 10 ));
		// timestr[i++] = '0' + ((second % 10 ));

		timestr[i] = 0;
	}

	return timestr;
}
static char* format_warnings(uint8_t warnings) {
	// 10 is enough to hold eight warning bits + a leading '!' + a trailing NUL
	static char wstr[10];
	// The order of the symbols needs to correspond to the warning bits set in
	// common.h
	static FMEM_STORAGE const char symbols[] = GHMON_WARNING_FLAGS;

	if (warnings == 0) {
		wstr[0] = 'O';
		wstr[1] = 'K';
		wstr[2] = 0;
	} else {
		uiter_t i = 1;

		wstr[0] = '!';
		for (uiter_t j = 0; j < SIZEOF_ARRAY(symbols); j++) {
			if (BIT_IS_SET(warnings, 1U << j)) {
				wstr[i++] = symbols[j];
			}
		}
		wstr[i] = 0;
	}

	return wstr;
}

#if LOG_LINE_BUFFER_COUNT != 0
static bool buffer_is_full(void) {
	return (log_buffer.size == LOG_LINE_BUFFER_COUNT);
}
static void buffer_status_line(void) {
	uint lineno;

	assert(log_buffer.tail <  LOG_LINE_BUFFER_COUNT);
	assert(log_buffer.size <= LOG_LINE_BUFFER_COUNT);

	lineno = log_buffer.size+1;
	if (lineno > LOG_LINE_BUFFER_COUNT) {
		lineno = LOG_LINE_BUFFER_COUNT;
	}
	LOGGER("Buffering log line %u of %u", (uint )lineno, (uint )LOG_LINE_BUFFER_COUNT);

	log_status_line(&log_buffer.lines[log_buffer.tail]);
	if (DEBUG) {
		print_log_line(serial_printf, &log_buffer.lines[log_buffer.tail]);
	}

	++log_buffer.tail;
	if (log_buffer.tail == LOG_LINE_BUFFER_COUNT) {
		log_buffer.tail = 0;
	}
	if (log_buffer.size != LOG_LINE_BUFFER_COUNT) {
		++log_buffer.size;
	}

	return;
}

#else // !LOG_LINE_BUFFER_COUNT != 0
static bool buffer_is_full(void) {
	return true;
}
static void buffer_status_line(void) {
	return;
}
#endif

__attribute__ ((format(printf, 1, 2)))
static void lprintf(const char *format, ...) {
	va_list arp;

	va_start(arp, format);
	printf_va(lprintf_putc, format, arp);
	va_end(arp);

	return;
}

#if LOG_PRINT_BUFFER_SIZE > 0
static void lprintf_putc(uint_fast8_t c) {
	assert(print_buffer.size < LOG_PRINT_BUFFER_SIZE);

	print_buffer.buffer[print_buffer.size] = c;
	++print_buffer.size;

	if (print_buffer.size == LOG_PRINT_BUFFER_SIZE) {
#if WRITE_LOG_TO_SD
		write_buffer_to_SD();
#endif
#if WRITE_LOG_TO_UART
		write_buffer_to_UART();
#endif

		print_buffer.size = 0;
	}

	return;
}
static void reset_print_buffer(void) {
	print_buffer.size = 0;
	return;
}
#else // LOG_PRINT_BUFFER_SIZE > 0
static void lprintf_putc(uint_fast8_t c) {
#if WRITE_LOG_TO_SD
	write_char_to_SD(c);
#endif
#if WRITE_LOG_TO_UART
		write_char_to_UART();
#endif
	return;
}
static void reset_print_buffer(void) {
	return;
}
#endif // LOG_PRINT_BUFFER_SIZE > 0

static err_t open_log_storage(void) {
	err_t res = ERR_OK;
	bool abort_logging = false;

	LOGGER("Writing log data");
	CLEAR_BIT(ghmon_warnings, WARN_LOG_ERROR);
	CLEAR_BIT(ghmon_warnings, WARN_LOG_SKIPPED);
	reset_print_buffer();

	if ((res = log_pre_write_hook()) != ERR_OK) {
		LOGGER("Log pre-write hook failed: error %d", (int )res);
		SET_BIT(ghmon_warnings, WARN_LOG_ERROR);
		return res;
	}

	if (WRITE_LOG_TO_UART && !abort_logging) {
		if ((res = open_UART()) != ERR_OK) {
			LOGGER("Failed to open log UART: error %d", (int )res);
			SET_BIT(ghmon_warnings, WARN_LOG_ERROR);
			if (!LOG_WITH_MISSING_UART) {
				abort_logging = true;
			}
		}
	}

	if (WRITE_LOG_TO_SD && !abort_logging) {
		if ((res = open_SD()) != ERR_OK) {
			LOGGER("Failed to open log SD: error %d", (int )res);
			SET_BIT(ghmon_warnings, WARN_LOG_ERROR);
			if (!LOG_WITH_MISSING_SD) {
				abort_logging = true;
				close_UART();
			}
		}
	}

	if (!abort_logging) {
		if ((res = open_log_file()) != ERR_OK) {
			abort_logging = true;
			close_UART();
			close_SD();
		}
	}

	return (abort_logging) ? res : ERR_OK;
}
static err_t open_log_file(void) {
	if (!have_log_header) {
		return rotate_log_file();
	}

	err_t res, err = ERR_OK;
	if ((res = open_SD_file()) != ERR_OK && !LOG_WITH_MISSING_SD) {
		err = res;
	}

	return err;
}

static err_t flush_print_buffer(void) {
	err_t err = ERR_OK;

#if LOG_PRINT_BUFFER_SIZE > 0
	err_t res;

	if (print_buffer.size > 0) {
		if ((res = write_buffer_to_UART()) != ERR_OK) {
			err = res;
		}
		if ((res = write_buffer_to_SD()) != ERR_OK) {
			err = res;
		}
		reset_print_buffer();
	}
#endif

	return err;
}
static void close_log_storage(void) {
	flush_print_buffer();
	close_SD();
	close_UART();
	log_post_write_hook();

	return;
}

static void find_log_file_name(void) {
#if NEED_LOG_FILE_NAME && LOG_LINES_PER_FILE > 0
	// This gets us two positions before the file extension
	const uint_fast8_t mod_p = SIZEOF_ARRAY(logfile_name) - 7;

	if (logfile_name[mod_p] == 'X') {
		logfile_name[mod_p] = '0';
		logfile_name[mod_p+1] = '0';
	}

	// 'A' > '9' so the loop is skipped completely if we've already exhausted the
	// options
	// TODO: If the most recent existing log file has fewer than LOG_LINES_PER_FILE lines,
	// reopen that and set the current line number.
	for (; logfile_name[mod_p] <= '9'; ++logfile_name[mod_p]) {
		for (; logfile_name[mod_p+1] <= '9'; ++logfile_name[mod_p+1]) {
			bool valid = true;

			if (WRITE_LOG_TO_SD && SD_file_exists(logfile_name)) {
				valid = false;
			}
			if (valid) {
				return;
			}
		}
		logfile_name[mod_p] = '0';
	}
	// TODO: Start using letter suffixes when numbers run out
	logfile_name[mod_p] = 'A';
	logfile_name[mod_p+1] = '0';

	LOGGER("Using log file %s", logfile_name);
#endif
	return;
}
static err_t rotate_log_file(void) {
	err_t res, err = ERR_OK;

	have_log_header = false;
	if ((res = flush_print_buffer()) != ERR_OK) {
		return res;
	}
	find_log_file_name();

	if (WRITE_LOG_TO_SD && err == ERR_OK) {
		close_SD_file();
		if ((res = open_SD_file()) != ERR_OK) {
			err = res;
		}
	}

	if (err == ERR_OK) {
		lines_logged_this_file = 0;
		have_log_header = true;
		write_log_header();
	}

	return err;
}

void print_log_header(void (*pf)(const char *format, ...)) {
	pf(F("# Time\tWarnings"));

	for (SENSOR_INDEX_T i = 0; i < SENSOR_COUNT; ++i) {
		if (!DO_SENSOR(i)) {
			continue;
		}
#if USE_SENSOR_NAME
		const char *name = FROM_FSTR(SENSORS[i].name);
#else
		char name[] = "sens_XX";
		name[5] = i/10;
		name[6] = i%10;
#endif
		pf("\t%s_reading", name);
	}

	for (CONTROLLER_INDEX_T i = 0; i < CONTROLLER_COUNT; ++i) {
		if (!DO_CONTROLLER(i)) {
			continue;
		}
#if USE_CONTROLLER_NAME
		const char *name = FROM_FSTR(CONTROLLERS[i].name);
#else
		char name[] = "ctrl_XX";
		name[5] = i/10;
		name[6] = i%10;
#endif
		pf("\t%s_status", name);
	}
	pf("%s", line_end);

	// This needs to be split to fit in the buffer for F()
	pf(F("# Warnings: B=battery low, V=Vcc low, S=sensor warning, C=controller warning, "));
	pf(F("l=log write skipped, L=log write error%s"), line_end);

	return;
}
static void write_log_header(void) {
	LOGGER("Writing log header");
	print_log_header(lprintf);
	return;
}

#endif
