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

#include "actuators.h"
#include "sensors.h"
#include "controllers.h"

#include "ulib/include/cstrings.h"
#include "ulib/include/fmem.h"
#include "ulib/include/halloc.h"
#include "ulib/include/printf.h"

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

#include GHMON_INCLUDE_CONFIG_HEADER(log/logfile.h)

typedef enum {
	TIME_FORMAT_AUTO = 0,
	TIME_FORMAT_SECONDS,
	TIME_FORMAT_DURATION,
	TIME_FORMAT_DATE
} time_format_t;

#if LOG_LINES_PER_FILE == 1
  // Why not? There was a note about it causing an infinite recursion but I
  // can't find where that would happen.
# error "LOG_LINES_PER_FILE can't be '1'"
#endif

#ifndef USE_LOG_FILE_NAME
# define USE_LOG_FILE_NAME 0
#endif

#if USE_LOG_FILE_NAME
 static char logfile_name[] = LOG_FILE_NAME_PATTERN;
#else
 static char logfile_name[] = "";
//# define logfile_name ((char *)NULL)
#endif

static const char invalid_value[] = LOG_INVALID_VALUE;
static const char no_value[] = LOG_NO_VALUE;
static const char line_end[] = LOG_LINE_END;

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
#if LOG_ACTUATORS_BY_DEFAULT
# define DO_ACTUATOR(_i_) (!BIT_IS_SET(ACTUATORS[_i_].cfg_flags, ACTUATOR_CFG_FLAG_NOLOG))
#else
# define DO_ACTUATOR(_i_) (BIT_IS_SET(ACTUATORS[_i_].cfg_flags, ACTUATOR_CFG_FLAG_LOG))
#endif

// These only need to save information which changes and is actually recorded
#if USE_SENSORS
typedef struct {
	sensor_reading_t *reading;
#if USE_SENSOR_STATUS
	SENSOR_STATUS_T status;
#endif
	uint8_t status_flags;
} sensor_log_buffer_t;
static const sensor_reading_t default_reading_value = {
	SENSOR_BAD_VALUE,
	0
};
#endif

typedef struct {
# if USE_CONTROLLER_STATUS
	CONTROLLER_STATUS_T status;
# endif
	uint8_t status_flags;
} controller_log_buffer_t;

typedef struct {
# if USE_ACTUATOR_STATUS_CHANGE_TIME
	utime_t status_change_time;
# endif
# if USE_ACTUATOR_ON_TIME_COUNT
	utime_t on_time_seconds;
# endif
# if USE_ACTUATOR_STATUS_CHANGE_COUNT
	uint_t status_change_count;
# endif
	ACTUATOR_STATUS_T status;
	uint8_t status_flags;
} actuator_log_buffer_t;

typedef struct {
	utime_t  system_time;

#if USE_SENSORS
	sensor_log_buffer_t *sensors;
#endif
#if USE_CONTROLLERS
	controller_log_buffer_t *controllers;
#endif
#if USE_ACTUATORS
	actuator_log_buffer_t *actuators;
#endif

	uint8_t ghmon_warnings;
} log_line_buffer_t;

#if USE_SENSORS
 static SENSOR_INDEX_T sensor_count;
 static uint_fast16_t sensor_reading_total_count;
 static uint8_t *sensor_reading_count;
#endif
#if USE_CONTROLLERS
 static CONTROLLER_INDEX_T controller_count;
#endif
#if USE_ACTUATORS
 static ACTUATOR_INDEX_T actuator_count;
#endif

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

static err_t _write_log_to_storage(uint_fast8_t flags);
static err_t write_log_line_to_storage(log_line_buffer_t *line, const char *extra, uint_fast8_t flags);
static void print_log_line(void (*pf)(const char *format, ...), log_line_buffer_t *line, const char *extra);
static bool buffer_is_full(void);
static void buffer_status_line(void);
static void lprintf_putc(uint_fast8_t c);
static void lprintf(const char *format, ...)
	__attribute__ ((format(printf, 1, 2)));
static const char* format_print_time(char *timestr, utime_t uptime, time_format_t format);
static const char* format_warnings(uint8_t warnings);
static err_t open_log_storage(void);
static err_t open_log_file(void);
static void close_log_storage(void);
static err_t rotate_log_file(void);
static void reset_print_buffer(void);
static void write_log_header(void);

void log_init(void) {
	SENSOR_INDEX_T sn = 0;
	CONTROLLER_INDEX_T cn = 0;
	ACTUATOR_INDEX_T an = 0;

#if USE_SENSORS
	for (SENSOR_INDEX_T i = 0; i < SENSOR_COUNT; ++i) {
		if (DO_SENSOR(i)) {
			++sn;
		}
	}
	sensor_count = sn;
#endif

#if USE_CONTROLLERS
	for (CONTROLLER_INDEX_T i = 0; i < CONTROLLER_COUNT; ++i) {
		if (DO_CONTROLLER(i)) {
			++cn;
		}
	}
	controller_count = cn;
#endif

#if USE_ACTUATORS
	for (ACTUATOR_INDEX_T i = 0; i < ACTUATOR_COUNT; ++i) {
		if (DO_ACTUATOR(i)) {
			++an;
		}
	}
	actuator_count = an;
#endif

#if LOG_LINE_BUFFER_COUNT > 0
	for (log_line_buffer_size_t i = 0; i < LOG_LINE_BUFFER_COUNT; ++i) {

#if USE_SENSORS
		if (sn > 0) {
			log_buffer.lines[i].sensors = halloc(sn * sizeof(log_buffer.lines[0].sensors[0]));
			sensor_reading_count = halloc(sn * sizeof(sensor_reading_count[0]));

			for (SENSOR_INDEX_T ii = 0, si = 0; ii < SENSOR_COUNT; ++ii) {
				if (!DO_SENSOR(ii)) {
					continue;
				}
				uint_fast8_t cnt = (SENSORS[ii].value_count > 0) ? SENSORS[ii].value_count : 1;

				sensor_reading_total_count += cnt;
				sensor_reading_count[si] = cnt;
				log_buffer.lines[i].sensors[si].reading = halloc(cnt * sizeof(log_buffer.lines[0].sensors[0].reading[0]));
				++si;
			}
		}
#endif
# if USE_CONTROLLERS
		if (cn > 0) {
			log_buffer.lines[i].controllers = halloc(cn * sizeof(log_buffer.lines[0].controllers[0]));
		}
# endif
# if USE_ACTUATORS
		if (an > 0) {
			log_buffer.lines[i].actuators = halloc(an * sizeof(log_buffer.lines[0].actuators[0]));
		}
# endif // USE_ACTUATORS
	}
#endif // LOG_LINE_BUFFER_COUNT > 0

	init_output_device();

	LOGGER("Initialized logging for %u sensors, %u controllers, and %u actuators", (uint )sn, (uint )cn, (uint )an);

	return;
}

static void log_status_line(log_line_buffer_t *line) {
	assert(line != NULL);

	check_common_sensor_warnings();
	check_common_controller_warnings();
	check_common_actuator_warnings();

	line->ghmon_warnings = ghmon_warnings;
	line->system_time = NOW();
#if USE_SENSORS
	for (SENSOR_INDEX_T i = 0, si = 0; i < SENSOR_COUNT; ++i) {
		if (!DO_SENSOR(i)) {
			continue;
		}
		if (LOG_UPDATES_SENSORS) {
			read_sensor(&SENSORS[i], &sensors[i], false, 0);
		}

		uint_fast8_t cnt = sensor_reading_count[si];
		//
		// Don't use memcpy() here, it would cause problems if either the struct
		// used for the buffer or the one in sensor_status_t are ever changed and
		// I doubt it saves many (if any) cycles in this case.
		if (sensors[i].reading != NULL) {
			for (uint_fast8_t vi = 0; vi < cnt; ++vi) {
				line->sensors[si].reading[vi] = sensors[i].reading[vi];
			}
		} else {
			for (uint_fast8_t vi = 0; vi < cnt; ++vi) {
				line->sensors[si].reading[vi] = default_reading_value;
			}
		}

# if USE_SENSOR_STATUS
		line->sensors[si].status = sensors[i].status;
# endif
		line->sensors[si].status_flags = sensors[i].status_flags;
		++si;
	}
#endif

#if USE_CONTROLLERS
	for (CONTROLLER_INDEX_T i = 0, si = 0; i < CONTROLLER_COUNT; ++i) {
		if (!DO_CONTROLLER(i)) {
			continue;
		}
# if USE_CONTROLLER_STATUS
		line->controllers[si].status = controllers[i].status;
# endif
		line->controllers[si].status_flags = controllers[i].status_flags;
		++si;
	}
#endif

#if USE_ACTUATORS
	for (ACTUATOR_INDEX_T i = 0, si = 0; i < ACTUATOR_COUNT; ++i) {
		if (!DO_ACTUATOR(i)) {
			continue;
		}
		line->actuators[si].status = actuators[i].status;
		line->actuators[si].status_flags = actuators[i].status_flags;
# if USE_ACTUATOR_STATUS_CHANGE_TIME
		line->actuators[si].status_change_time = actuators[i].status_change_time;
# endif
# if USE_ACTUATOR_ON_TIME_COUNT
		line->actuators[si].on_time_seconds = actuators[i].on_time_seconds;
# endif
# if USE_ACTUATOR_STATUS_CHANGE_COUNT
		line->actuators[si].status_change_count = actuators[i].status_change_count;
# endif
		++si;
	}
#endif

	return;
}

void log_status(void) {
	bool buffer_line = true;

	if (buffer_is_full()) {
		if (skip_log_writes()) {
			SET_BIT(ghmon_warnings, WARN_LOG_SKIPPED);
		} else if (open_log_storage() == ERR_OK) {
			if (_write_log_to_storage(0) == ERR_OK) {
				buffer_line = false;
			} else {
				close_log_storage();
			}
		}
	}

	if (!buffer_line) {
		log_line_buffer_t current_status = { 0 };
#if USE_SENSORS
		sensor_log_buffer_t sensors_m[sensor_count];
		sensor_reading_t readings_m[sensor_reading_total_count];
		for (SENSOR_INDEX_T si = 0, ri = 0; si < sensor_count; ++si) {
			sensors_m[si].reading = &readings_m[ri];
			ri += sensor_reading_count[si];
		}
		current_status.sensors = sensors_m;
#endif
#if USE_CONTROLLERS
		controller_log_buffer_t controllers_m[controller_count];
		current_status.controllers = controllers_m;
#endif
#if USE_ACTUATORS
		actuator_log_buffer_t actuators_m[actuator_count];
		current_status.actuators = actuators_m;
#endif

		log_status_line(&current_status);
		buffer_line_extra(LOG_LINE_BUFFER_COUNT);
		write_log_line_to_storage(&current_status, print_line_extra(LOG_LINE_BUFFER_COUNT), 0);
		close_log_storage();
	} else {
		buffer_status_line();
	}
	return;
}

static err_t _write_log_to_storage(uint_fast8_t flags) {
#if LOG_LINE_BUFFER_COUNT > 0
	err_t res;
	log_line_buffer_size_t head, size;

	size = (BIT_IS_SET(flags, LOG_WRITE_REWRITE_ALL)) ? LOG_LINE_BUFFER_COUNT : log_buffer.size;

	if (size > log_buffer.tail) {
		head = LOG_LINE_BUFFER_COUNT - (size - log_buffer.tail);
	} else {
		head = log_buffer.tail - size;
	}
	while (size > 0) {
		if ((res = write_log_line_to_storage(&log_buffer.lines[head], print_line_extra(head), flags)) != ERR_OK) {
			goto END;
		}

		++head;
		if (head == LOG_LINE_BUFFER_COUNT) {
			head = 0;
		}
		--size;
	}

END:
	if (!BIT_IS_SET(flags, LOG_WRITE_PRESERVE_STATE) && (size < log_buffer.size)) {
		log_buffer.size = size;
	}
	return res;

#endif // LOG_LINE_BUFFER_COUNT > 0
	return ERR_OK;
}
void print_log(void (*pf)(const char *format, ...)) {
#if LOG_LINE_BUFFER_COUNT > 0
	log_line_buffer_size_t head, size;

	// Replay the whole buffer, even if it's been written out and even if
	// there was never an entry
	size = LOG_LINE_BUFFER_COUNT;
	head = log_buffer.tail;

	for (; size > 0; --size) {
		print_log_line(pf, &log_buffer.lines[head], print_line_extra(head));

		++head;
		if (head == LOG_LINE_BUFFER_COUNT) {
			head = 0;
		}
	}
#endif // LOG_LINE_BUFFER_COUNT > 0

	return;
}

static void print_log_line(void (*pf)(const char *format, ...), log_line_buffer_t *line, const char *extra) {
	// 20 is enough to hold '2021.02.15 12:00:00' with a trailing NUL
	char timestr[20];

	pf("%s\t%s", format_print_time(timestr, line->system_time, LOG_TIME_FORMAT), format_warnings(line->ghmon_warnings));

#if USE_SENSORS
	for (SENSOR_INDEX_T i = 0, si = 0; i < SENSOR_COUNT; ++i) {
		if (!DO_SENSOR(i)) {
			continue;
		}

		const char *es;
		es = BIT_IS_SET(line->sensors[si].status_flags, SENSOR_STATUS_FLAG_ERROR) ? "\t!" : "\t";

		if (!BIT_IS_SET(line->sensors[si].status_flags, SENSOR_STATUS_FLAG_INITIALIZED)) {
			pf("%s%s", es, invalid_value);
			for (uint_fast8_t ri = 0; ri < sensor_reading_count[si]; ++ri) {
				pf("\t%s", invalid_value);
			}
		} else {
# if USE_SENSOR_STATUS
			pf("%s%d", es, (int )line->sensors[si].status);
# else
			pf("%s%s", es, no_value);
# endif
			for (uint_fast8_t ri = 0; ri < sensor_reading_count[si]; ++ri) {
				const char *tn;

				if (LOG_PRINT_SENSOR_TYPE && (tn = sensor_type_to_name(line->sensors[si].reading[ri].type)) != NULL) {
					pf("\t%d %s", (int )line->sensors[si].reading[ri].value, tn);
				} else {
					pf("\t%d", (int )line->sensors[si].reading[ri].value);
				}
			}
		}
		++si;
	}
#endif

#if USE_CONTROLLERS
	for (CONTROLLER_INDEX_T i = 0, si = 0; i < CONTROLLER_COUNT; ++i) {
		if (!DO_CONTROLLER(i)) {
			continue;
		}

		const char *es;
		es = BIT_IS_SET(line->controllers[si].status_flags, CONTROLLER_STATUS_FLAG_ERROR) ? "\t!" : "\t";

		if (!BIT_IS_SET(line->controllers[si].status_flags, CONTROLLER_STATUS_FLAG_INITIALIZED)) {
			pf("%s%s", es, invalid_value);
		} else {
# if USE_CONTROLLER_STATUS
			pf("%s%d", es, (int )line->controllers[si].status);
# else
			pf("%s%s", es, no_value);
# endif
		}
		++si;
	}
#endif

#if USE_ACTUATORS
	for (ACTUATOR_INDEX_T i = 0, si = 0; i < ACTUATOR_COUNT; ++i) {
		if (!DO_ACTUATOR(i)) {
			continue;
		}

		if (BIT_IS_SET(line->actuators[si].status_flags, ACTUATOR_STATUS_FLAG_ERROR)) {
			pf("\t!");
		} else {
			pf("\t");
		}
		if (!BIT_IS_SET(line->actuators[si].status_flags, ACTUATOR_STATUS_FLAG_INITIALIZED)) {
			pf("%s", invalid_value);
			if (USE_ACTUATOR_STATUS_CHANGE_TIME) {
				pf("\t%s", invalid_value);
			}
			if (USE_ACTUATOR_ON_TIME_COUNT) {
				pf("\t%s", invalid_value);
			}
			if (USE_ACTUATOR_STATUS_CHANGE_COUNT) {
				pf("\t%s", invalid_value);
			}
		} else {
			pf("%d", (int )line->actuators[si].status);
# if USE_ACTUATOR_STATUS_CHANGE_TIME
			pf("\t%s", format_print_time(timestr, line->actuators[si].status_change_time, LOG_TIME_FORMAT));
# endif
# if USE_ACTUATOR_ON_TIME_COUNT
			pf("\t%s", format_print_time(timestr, line->actuators[si].on_time_seconds, TIME_FORMAT_DURATION));
# endif
# if USE_ACTUATOR_STATUS_CHANGE_COUNT
			pf("\t%u", (unsigned )line->actuators[si].status_change_count);
# endif
		}
		++si;
	}
#endif

	if (extra != NULL) {
		pf("\t%s%s", extra, line_end);
	} else {
		pf("%s", line_end);
	}

	return;
}
static err_t write_log_line_to_storage(log_line_buffer_t *line, const char *extra, uint_fast8_t flags) {
	if (BIT_IS_SET(flags, LOG_WRITE_PRESERVE_STATE)) {
		print_log_line(lprintf, line, extra);
		return ERR_OK;
	}

	if (LOG_LINES_PER_FILE > 0 && lines_logged_this_file == LOG_LINES_PER_FILE) {
		err_t res;

		if ((res = rotate_log_file()) != ERR_OK) {
			return res;
		}
	}

	print_log_line(lprintf, line, extra);
	++lines_logged_this_file;

	return ERR_OK;
}
void write_log_to_storage(uint_fast8_t flags) {
	if (open_log_storage() != ERR_OK) {
		return;
	}
	_write_log_to_storage(flags);
	close_log_storage();
	return;
}

static const char* format_print_time(char *timestr, utime_t uptime, time_format_t format) {
	const char *ret = timestr;

	assert(timestr != NULL);

	if (format == TIME_FORMAT_AUTO) {
		// Assume that if the year hasn't been set, this is an uptime not a date
		// Also assume that it's at least 3 years since YEAR_0 and that this won't
		// run continuously for more than 3 years
		if (uptime < (SECONDS_PER_YEAR * 3)) {
			format = TIME_FORMAT_DURATION;
		} else {
			format = TIME_FORMAT_DATE;
		}
	}

	if (format == TIME_FORMAT_SECONDS) {
		cstring_from_uint(timestr, SIZEOF_ARRAY(timestr), uptime, 10);

	} else if (format == TIME_FORMAT_DURATION) {
		// FIXME: Buffer overflow with days > 999999 (~2740 years)
		ret = print_duration(timestr, SIZEOF_ARRAY(timestr), uptime);

	} else {
		datetime_t dt;

		seconds_to_datetime(uptime, &dt);
		ret = print_datetime(timestr, SIZEOF_ARRAY(timestr), &dt);
	}

	return ret;
}

static const char* format_warnings(uint8_t warnings) {
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

#if LOG_LINE_BUFFER_COUNT > 0
static bool buffer_is_full(void) {
	return (log_buffer.size == LOG_LINE_BUFFER_COUNT);
}
static void buffer_status_line(void) {
	uint lineno;

	assert(log_buffer.tail <  LOG_LINE_BUFFER_COUNT);
	assert(log_buffer.size <= LOG_LINE_BUFFER_COUNT);

	// This check makes sure we don't print e.g. '5 of 4' if there was a problem
	// writing the log out and we're overwriting old entries.
	lineno = log_buffer.size+1;
	if (lineno > LOG_LINE_BUFFER_COUNT) {
		lineno = LOG_LINE_BUFFER_COUNT;
	}
	LOGGER("Buffering log line %u of %u", (uint )lineno, (uint )LOG_LINE_BUFFER_COUNT);

	log_status_line(&log_buffer.lines[log_buffer.tail]);
	buffer_line_extra(log_buffer.tail);

#if DEBUG && uHAL_USE_UART_COMM
	print_log_line(serial_printf, &log_buffer.lines[log_buffer.tail], print_line_extra(log_buffer.tail));
#endif

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
	ulib_vprintf(lprintf_putc, format, arp);
	va_end(arp);

	return;
}

#if LOG_PRINT_BUFFER_SIZE > 0
static void lprintf_putc(uint_fast8_t c) {
	assert(print_buffer.size < LOG_PRINT_BUFFER_SIZE);

	print_buffer.buffer[print_buffer.size] = c;
	++print_buffer.size;

	if (print_buffer.size == LOG_PRINT_BUFFER_SIZE) {
		if (write_buffer_to_storage(print_buffer.buffer, print_buffer.size) != ERR_OK) {
			SET_BIT(ghmon_warnings, WARN_LOG_ERROR);
		}
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
	if (write_byte_to_storage(c) != ERR_OK) {
		SET_BIT(ghmon_warnings, WARN_LOG_ERROR);
	}
	return;
}
static void reset_print_buffer(void) {
	return;
}
#endif // LOG_PRINT_BUFFER_SIZE > 0

static err_t open_log_storage(void) {
	err_t res;

	LOGGER("Writing log data");
	CLEAR_BIT(ghmon_warnings, WARN_LOG_ERROR);
	CLEAR_BIT(ghmon_warnings, WARN_LOG_SKIPPED);
	reset_print_buffer();

	if ((res = open_output_device()) == ERR_OK && (res = open_log_file()) != ERR_OK) {
		close_output_device();
	}
	if (res != ERR_OK) {
		SET_BIT(ghmon_warnings, WARN_LOG_ERROR);
	}

	return res;
}
static err_t open_log_file(void) {
	if (!have_log_header) {
		return rotate_log_file();
	}

	return open_output_file(logfile_name);
}

static err_t flush_print_buffer(void) {
	err_t res = ERR_OK;

#if LOG_PRINT_BUFFER_SIZE > 0
	if (print_buffer.size > 0) {
		res = write_buffer_to_storage(print_buffer.buffer, print_buffer.size);
		reset_print_buffer();
	}
#endif

	return res;
}
static void close_log_storage(void) {
	flush_print_buffer();
	close_output_device();

	return;
}

static err_t find_log_file_name(void) {
#if USE_LOG_FILE_NAME && LOG_LINES_PER_FILE > 0
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
			switch (output_file_is_available(logfile_name)) {
			case ERR_OK:
				return ERR_OK;
			case ERR_RETRY:
				return ERR_RETRY;
			default:
				break;
			}
		}
		logfile_name[mod_p+1] = '0';
	}
	// TODO: Start using letter suffixes when numbers run out
	logfile_name[mod_p] = 'A';
	logfile_name[mod_p+1] = '0';

#endif
	return ERR_OK;
}
static err_t rotate_log_file(void) {
	err_t res;

	have_log_header = false;
	if ((res = flush_print_buffer()) != ERR_OK) {
		goto END;
	}
	if ((res = close_output_file()) != ERR_OK) {
		goto END;
	}
	//
	// Don't return an error when we couldn't find the name, that just means the
	// device is temporarily unvavailable and we'll try again later.
	//if ((res = find_log_file_name()) != ERR_OK) {
	if ((find_log_file_name()) != ERR_OK) {
		goto END;
	}

	lines_logged_this_file = 0;
	if (USE_LOG_FILE_NAME) {
		LOGGER("Using log file %s", logfile_name);
	}
	if ((res = open_output_file(logfile_name)) != ERR_OK) {
		goto END;
	}

	write_log_header();
	have_log_header = true;

END:
	return res;
}

void print_log_header(void (*pf)(const char *format, ...)) {
	pf(F("# Time\tWarnings"));

#if USE_SENSORS
	for (SENSOR_INDEX_T i = 0, si = 0; i < SENSOR_COUNT; ++i) {
		if (!DO_SENSOR(i)) {
			continue;
		}
#if USE_SENSOR_NAME
		const char *name = FROM_FSTR(SENSORS[i].name);
#else
		char name[] = "sens_XX";
		name[5] = '0' + i/10;
		name[6] = '0' + i%10;
#endif

		pf("\t%s_status", name);
		for (uint8_t ri = 0; ri < sensor_reading_count[si]; ++ri) {
			pf("\t%s_reading_%u", name, (uint )ri);
		}
		++si;
	}
#endif

#if USE_CONTROLLERS
	for (CONTROLLER_INDEX_T i = 0; i < CONTROLLER_COUNT; ++i) {
		if (!DO_CONTROLLER(i)) {
			continue;
		}
# if USE_CONTROLLER_NAME
		const char *name = FROM_FSTR(CONTROLLERS[i].name);
# else
		char name[] = "ctrl_XX";
		name[5] = '0' + i/10;
		name[6] = '0' + i%10;
# endif
		pf("\t%s_status", name);
	}
#endif

#if USE_ACTUATORS
	for (ACTUATOR_INDEX_T i = 0; i < ACTUATOR_COUNT; ++i) {
		if (!DO_ACTUATOR(i)) {
			continue;
		}
# if USE_ACTUATOR_NAME
		const char *name = FROM_FSTR(ACTUATORS[i].name);
# else
		char name[] = "actr_XX";
		name[5] = '0' + i/10;
		name[6] = '0' + i%10;
# endif
		pf("\t%s_status", name);
# if USE_ACTUATOR_STATUS_CHANGE_TIME
		pf("\t%s_last_status_change", name);
# endif
# if USE_ACTUATOR_ON_TIME_COUNT
		pf("\t%s_on_time", name);
# endif
# if USE_ACTUATOR_STATUS_CHANGE_COUNT
		pf("\t%s_status_change_count", name);
# endif
	}
#endif

	const char *extra = print_header_extra();
	if (extra != NULL) {
		pf("\t%s", extra);
	}

	pf("%s", line_end);
	// This needs to be split to fit in the buffer for F()
	pf(F("# Warnings: B=battery low, V=Vcc low, S=sensor warning, C=controller warning, "));
	pf(F("A=Actuator warning, l=log write skipped, L=log write error%s"), line_end);

	return;
}
static void write_log_header(void) {
	LOGGER("Writing log header");
	print_log_header(lprintf);
	return;
}

#endif
