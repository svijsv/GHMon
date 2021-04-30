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
// log.c
// Manage log files
// NOTES:
//

#ifdef __cplusplus
 extern "C" {
#endif

//#define DEBUG 1
/*
* Includes
*/
#include "log.h"
#include "fatfs/ff.h"

#include "sensors.h"
#include "controllers.h"
#include "power.h"
#include "serial.h"

#include "ulib/cstrings.h"


// Need to check this *after* the headers are included
#if USE_LOGGING

#if LINES_PER_FILE == 1
# error "LINES_PER_FILE can't be '1'; it would create an infinite recursion."
#endif

#if DEBUG
# pragma message "SD_PRINT_BUFFER_SIZE: " XTRINGIZE(SD_PRINT_BUFFER_SIZE)
#endif


/*
* Static values
*/
// Use variables to prevent duplication
static const char *prefix_std = "\t";
static const char *prefix_warn = "\t!";

// Store multi-use strings in const arrays so they aren't duplicated
static _FLASH const char open_err_msg[] = "f_open(): fatfs error %u";
#define OPEN_ERR_MSG(err) LOGGER_NOF(FROM_FSTR(open_err_msg),  (uint )(err))

static _FLASH const char write_err_msg[] = "f_write(): fatfs error %u";
#define WRITE_ERR_MSG(err) LOGGER_NOF(FROM_FSTR(write_err_msg), (uint )(err))

static _FLASH const char close_err_msg[] = "f_close(): fatfs error %u";
#define CLOSE_ERR_MSG(err) LOGGER_NOF(FROM_FSTR(close_err_msg), (uint )(err))

static _FLASH const char unmount_err_msg[] = "f_unmount(): fatfs error %u";
#define UNMOUNT_ERR_MSG(err) LOGGER_NOF(FROM_FSTR(unmount_err_msg),  (uint )(err))

/*
* Types
*/
// This only needs to save information which changes and is actually recorded
typedef struct {
	utime_t  uptime;
	int16_t  vcc_voltage;
#if USE_CONTROLLERS
#if USE_SMALL_CONTROLLERS < 1
	uint16_t run_time[CONTROLLER_COUNT];
	uint8_t  run_count[CONTROLLER_COUNT];
	uint8_t  controller_iflags[CONTROLLER_COUNT];
#endif // USE_SMALL_CONTROLLERS < 1
#endif // USE_CONTROLLERS
	status_t status[SENSOR_COUNT];
	uint8_t  sensor_iflags[SENSOR_COUNT];
	uint8_t  warnings;
} log_buffer_t;

#if SD_PRINT_BUFFER_SIZE > 0
// We can't use a normal string_t to buffer output because those truncate
// the line if we go over; instead we need to roll our own
// The buffer does not need to be NUL-terminated.
typedef struct {
	uint16_t size;
	uint8_t buffer[SD_PRINT_BUFFER_SIZE];
} print_buffer_t;
#endif // SD_PRINT_BUFFER_SIZE > 0


/*
* Variables
*/
//uint32_t lines_logged = 0;
uint32_t lines_logged_this_file = 0;
uint8_t  write_errors;

// True if the header has already been printed to the current file
static bool have_log_header = false;
static char logfile_name[] = LOGFILE_NAME_PATTERN;

// The print buffer, fs handle, and file handle could all be allocated on the
// stack in log_status() if saving RAM for other functions was important, but
// I think reserving it so that needed space isn't allocated by the user to
// other areas like sensors/controllers/tables is more important. I also can't
// think of anywhere else that uses a lot of temporary RAM except the input
// buffer for the terminal.
#if SD_PRINT_BUFFER_SIZE > 0
static print_buffer_t print_buffer;
#endif
static FATFS fs;
static FIL fh;


#if LOGFILE_BUFFER_COUNT != 0
// Use a ring buffer so that if there's a problem preventing us from writing
// out the log it's the oldest data that gets overwritten.
static struct {
	log_buffer_t lines[LOGFILE_BUFFER_COUNT];
	uint16_t tail;
	uint16_t size;
} log_buffer = { 0 };
#endif // LOGFILE_BUFFER_COUNT != 0


/*
* Local function prototypes
*/
static bool buffer_is_full(void);
static void buffer_line(utime_t now);
static bool file_exists(const char *path);
static void new_name(void);
static void lprintf_putc(int c);
static void lprintf(const char *format, ...)
	__attribute__ ((format(printf, 1, 2)));
static FRESULT close_file(void);
static FRESULT next_line(void);
static FRESULT print_header(void);
static void power_on(void);
static void power_off(void);
static char* format_uptime(utime_t uptime);
static char* format_warnings(uint8_t warnings);


/*
* Interrupt handlers
*/


/*
* Functions
*/
void log_init(void) {
	power_off();

	return;
}

void log_status(bool force_write) {
	uint32_t now;
	FRESULT  err;
	const char *prefix;
	bool is_opened = false;

	// Determine the time at the same time the status is checked so that they
	// match
	now = NOW();
	check_sensors();
	check_sensor_warnings();
	check_controller_warnings();

	if (!force_write && !buffer_is_full()) {
		buffer_line(now);
		return;
	}

	if (!force_write && BIT_IS_SET(G_warnings, (WARN_BATTERY_LOW|WARN_VCC_LOW))) {
		LOGGER("Skipping log sync; low voltage");
		SET_BIT(G_warnings, WARN_SD_SKIPPED);
		buffer_line(now);
		return;
	}
	CLEAR_BIT(G_warnings, WARN_SD_SKIPPED);

	//
	// From this point on, don't return - go to END instead.
	//
	power_on();
	write_errors = 0;

	if ((err = f_mount(&fs, "", 1)) != FR_OK) {
		LOGGER("Skipping log sync: fatfs error %u", (uint )err);
		SET_BIT(G_warnings, WARN_SD_SKIPPED);
		have_log_header = false;
		goto END;
	}

	// Use this as a simple check for when to create a new log file.
#if LINES_PER_FILE > 0
	if (!have_log_header) {
		new_name();
	}

#else // !LINES_PER_FILE > 0
	if (!have_log_header) {
		if (file_exists(logfile_name)) {
			have_log_header = true;
		}
	}
#endif // LINES_PER_FILE > 0

	LOGGER("Logging to %s", logfile_name);
	if ((err = f_open(&fh, logfile_name, FA_WRITE|FA_OPEN_APPEND)) != FR_OK) {
		OPEN_ERR_MSG(err);
		SET_BIT(G_warnings, WARN_SD_FAILED);
		goto END;
	}
	is_opened = true;

	if ((!have_log_header) && (print_header() != FR_OK)) {
		// print_header() will print the error message
		SET_BIT(G_warnings, WARN_SD_FAILED);
		goto END;
	}

	LOGGER("Writing log data");

#if LOGFILE_BUFFER_COUNT != 0
	uint16_t      head;
	log_buffer_t *line;

	if (log_buffer.size > log_buffer.tail) {
		head = LOGFILE_BUFFER_COUNT - (log_buffer.size - log_buffer.tail);
	} else {
		head = log_buffer.tail - log_buffer.size;
	}
	while (log_buffer.size > 0) {
		line = &log_buffer.lines[head];

		lprintf("%s\t%s\t%u", format_uptime(line->uptime), format_warnings(line->warnings), (uint )line->vcc_voltage);
		for (uiter_t i = 0; i < SENSOR_COUNT; ++i) {
			if (BIT_IS_SET(line->sensor_iflags[i], SENS_FLAG_WARNING)) {
				prefix = prefix_warn;
			} else {
				prefix = prefix_std;
			}
			lprintf("%s%d", prefix, (int )line->status[i]);
		}
#if USE_CONTROLLERS
#if USE_SMALL_CONTROLLERS < 1
		for (uiter_t i = 0; i < CONTROLLER_COUNT; ++i) {
			if (BIT_IS_SET(line->controller_iflags[i], CTRL_FLAG_WARNING)) {
				prefix = prefix_warn;
			} else {
				prefix = prefix_std;
			}
			lprintf("%s%u\t%u", prefix, (uint )line->run_count[i], (uint )line->run_time[i]);
		}
#endif // USE_SMALL_CONTROLLERS < 1
#endif // USE_CONTROLLERS
		lprintf_putc('\n');

		if ((err = next_line()) != FR_OK) {
			SET_BIT(G_warnings, WARN_SD_FAILED);
			goto END;
		}

		++head;
		head %= LOGFILE_BUFFER_COUNT;
		--log_buffer.size;
	}
	// The tail tracks the next write position which remains the same until
	// it's empty even if a write fails
	log_buffer.tail = 0;
#endif // LOGFILE_BUFFER_COUNT != 0

	// We only need to add the latest measurements if this is a scheduled call;
	// otherwise the duration between lines won't be even.
	if (!force_write) {
		lprintf("%s\t%s\t%u", format_uptime(now), format_warnings(G_warnings), (uint )G_vcc_voltage);
		for (uiter_t i = 0; i < SENSOR_COUNT; ++i) {
			if (BIT_IS_SET(G_sensors[i].iflags, SENS_FLAG_WARNING)) {
				prefix = prefix_warn;
			} else {
				prefix = prefix_std;
			}
			lprintf("%s%d", prefix, (int )G_sensors[i].status);
		}
#if USE_CONTROLLERS
#if USE_SMALL_CONTROLLERS < 1
		for (uiter_t i = 0; i < CONTROLLER_COUNT; ++i) {
			if (BIT_IS_SET(G_controllers[i].iflags, CTRL_FLAG_WARNING)) {
				prefix = prefix_warn;
			} else {
				prefix = prefix_std;
			}
			lprintf("%s%u\t%u", prefix, (uint )G_controllers[i].run_count, (uint )G_controllers[i].run_time);
		}
#endif // USE_SMALL_CONTROLLERS < 1
#endif // USE_CONTROLLERS
		lprintf_putc('\n');

		// Unset this before writing this line but after recording errors so
		// that it's set the next time we're called if there's a problem
		CLEAR_BIT(G_warnings, WARN_SD_FAILED);

		if ((err = next_line()) != FR_OK) {
			SET_BIT(G_warnings, WARN_SD_FAILED);
			goto END;
		}
	}

END:
	if (write_errors != 0) {
		SET_BIT(G_warnings, WARN_SD_FAILED);
	}
	if (is_opened && ((err = close_file()) != FR_OK)) {
		// close_file() handles the error message
		SET_BIT(G_warnings, WARN_SD_FAILED);
	}

	if ((fs.fs_type != 0) && ((err = f_unmount("")) != FR_OK)) {
		UNMOUNT_ERR_MSG(err);
		SET_BIT(G_warnings, WARN_SD_FAILED);
	}

	// TODO: Wait for the card to be not-busy
#if SPI_POWER_PIN
	if (!BIT_IS_SET(G_warnings, WARN_SD_SKIPPED)) {
		sleep(SD_POWEROFF_DELAY_MS);
	}
#endif
	power_off();

	// The warnings will have been erased on successful write; if they weren't
	// we know to keep rotating the buffered data
	if (BIT_IS_SET(G_warnings, WARN_SD_FAILED|WARN_SD_SKIPPED)) {
		if (!force_write) {
			buffer_line(now);
		} else {
			issue_warning();
		}
	} else if (force_write) {
		led_flash(1, DELAY_SHORT);
	}

	return;
}
static char* format_uptime(utime_t uptime) {
	// 17 is enough to hold '2021.02.15 12:00' with a trailing NUL
	static char timestr[17];

	// Assume that if the year hasn't been set, this is an uptime not a date
	// Also assume that it's at least 3 years since YEAR_0 and that this won't
	// run continuously for more than 3 years
	if (uptime < (YEARS * 3)) {
		char *c;

		c = cstring_from_int(timestr, (uptime / HOURS), 2, '0');
		*c = ':'; ++c;
		cstring_from_int(c, ((uptime % HOURS) / MINUTES), 2, '0');
	} else {
		uiter_t i;
		uint8_t year, month, day, hour, minute, second;
		uint16_t ayear;

		seconds_to_date(uptime, &year, &month, &day);
		seconds_to_time(uptime, &hour, &minute, &second);

		i = 0;
		ayear = year + YEAR_0;
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
		timestr[i++] = ' ';
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
	static _FLASH const char symbols[] = "BVSClL";

	if (warnings == 0) {
		wstr[0] = 'O';
		wstr[1] = 'K';
		wstr[2] = 0;
	} else {
		uiter_t i = 1;

		wstr[0] = '!';
		for (uiter_t j = 0; j < sizeof(symbols); j++) {
			if (BIT_IS_SET(warnings, 1 << j)) {
				wstr[i++] = symbols[j];
			}
		}
		wstr[i] = 0;
	}

	return wstr;
}

#if LOGFILE_BUFFER_COUNT != 0
static bool buffer_is_full(void) {
	return (log_buffer.size == LOGFILE_BUFFER_COUNT);
}
static void buffer_line(utime_t now) {
	log_buffer_t *line;

	assert(log_buffer.tail <  LOGFILE_BUFFER_COUNT);
	assert(log_buffer.size <= LOGFILE_BUFFER_COUNT);

	LOGGER("Buffering log line");

	line = &log_buffer.lines[log_buffer.tail];
	line->uptime      = now;
	line->warnings    = G_warnings;
	line->vcc_voltage = G_vcc_voltage;
	for (uiter_t i = 0; i < SENSOR_COUNT; ++i) {
		line->status[i] = G_sensors[i].status;
		line->sensor_iflags[i] = G_sensors[i].iflags;
	}
#if USE_CONTROLLERS
#if USE_SMALL_CONTROLLERS < 1
	for (uiter_t i = 0; i < CONTROLLER_COUNT; ++i) {
		line->run_time[i] = G_controllers[i].run_time;
		line->run_count[i] = G_controllers[i].run_count;
		line->controller_iflags[i] = G_controllers[i].iflags;
	}
#endif // USE_SMALL_CONTROLLERS < 1
#endif // USE_CONTROLLERS

#if DEBUG
	PRINTF("%s\t%s\t%u", format_uptime(line->uptime), format_warnings(line->warnings), (uint )line->vcc_voltage);
	for (uiter_t i = 0; i < SENSOR_COUNT; ++i) {
		PRINTF("\t%d", (int )line->status[i]);
	}
#if USE_CONTROLLERS
#if USE_SMALL_CONTROLLERS < 1
	for (uiter_t i = 0; i < CONTROLLER_COUNT; ++i) {
		PRINTF("\t%u\t%u", (uint )line->run_count[i], (uint )line->run_time[i]);
	}
#endif // USE_SMALL_CONTROLLERS < 1
#endif // USE_CONTROLLERS
	PUTS("\r\n", 2);
#endif // DEBUG

	++log_buffer.tail;
	log_buffer.tail %= LOGFILE_BUFFER_COUNT;
	if (log_buffer.size != LOGFILE_BUFFER_COUNT) {
		++log_buffer.size;
	}

	return;
}
#else // !LOGFILE_BUFFER_COUNT != 0
static bool buffer_is_full(void) {
	return true;
}
static void buffer_line(utime_t now) {
	return;
}
#endif // LOGFILE_BUFFER_COUNT != 0

static bool file_exists(const char *path) {
	FILINFO st;

	return (f_stat(path, &st) == FR_OK);
}

// TODO: Start using letter suffixes when numbers run out
#if LINES_PER_FILE > 0
static void new_name(void) {
	uiter_t i;

	for (i = 0; i <= 99; ++i) {
		logfile_name[6] = '0' + (i/10);
		logfile_name[7] = '0' + (i%10);
		if (!file_exists(logfile_name)) {
			return;
		}
	}
	// Default to XX if we've created too many files already.
	if (i > 99) {
		logfile_name[6] = 'X';
		logfile_name[7] = 'X';
	}

	return;
}
#endif // LINES_PER_FILE > 0

#if SD_PRINT_BUFFER_SIZE > 0
static void lprintf_putc(int c) {
	UINT bh;
	FRESULT err;

	assert(print_buffer.size < SD_PRINT_BUFFER_SIZE);

	print_buffer.buffer[print_buffer.size] = c;
	++print_buffer.size;

	if (print_buffer.size == SD_PRINT_BUFFER_SIZE) {
		print_buffer.size = 0;

		bh = 0;
		LOGGER("Writing %u bytes to SD from %u byte buffer", (uint )SD_PRINT_BUFFER_SIZE, (uint )SD_PRINT_BUFFER_SIZE);
		if ((err = f_write(&fh, print_buffer.buffer, SD_PRINT_BUFFER_SIZE, &bh)) != FR_OK) {
			// Not much else we can do about it here
			++write_errors;
			WRITE_ERR_MSG(err);
			return;
		}
#if DEBUG
		// This will cut off the last byte, which may not be a newline here
		print_buffer.buffer[print_buffer.size-1] = 0;
		LOGGER("%s?", print_buffer.buffer);
#endif
	}

	return;
}
#else // !SD_PRINT_BUFFER_SIZE > 0
static void lprintf_putc(int c) {
	UINT bh, cb;
	FRESULT err;

	cb = (uint8_t )c;
	if ((err = f_write(&fh, &cb, 1, &bh)) != FR_OK) {
		// Not much else we can do about it here
		++write_errors;
		WRITE_ERR_MSG(err);
		return;
	}

	return;
}
#endif // SD_PRINT_BUFFER_SIZE > 0

__attribute__ ((format(printf, 1, 2)))
static void lprintf(const char *format, ...) {
	va_list arp;

	va_start(arp, format);
	vaprintf(lprintf_putc, format, arp);
	va_end(arp);

	return;
}

static FRESULT close_file(void) {
	FRESULT err, res;

	res = FR_OK;

#if SD_PRINT_BUFFER_SIZE > 0
	UINT bh;

	assert(print_buffer.size < SD_PRINT_BUFFER_SIZE);

	if (print_buffer.size != 0) {
		LOGGER("Writing %u bytes to SD from %u byte buffer", (uint )print_buffer.size, (uint )SD_PRINT_BUFFER_SIZE);
		if ((err = f_write(&fh, print_buffer.buffer, print_buffer.size, &bh)) != FR_OK) {
			++write_errors;
			WRITE_ERR_MSG(err);
			res = err;
		}
#if DEBUG
		// This will cut off the last byte, but that should just be a newline
		print_buffer.buffer[print_buffer.size-1] = 0;
		LOGGER("%s\r\n", print_buffer.buffer);
#endif
		print_buffer.size = 0;
	}
#endif // SD_PRINT_BUFFER_SIZE > 0

	if ((err = f_close(&fh)) != FR_OK) {
		CLOSE_ERR_MSG(err);
		res = err;
	}

	return res;
}
static FRESULT next_line(void) {
	FRESULT err, res;

	res = FR_OK;

	//++lines_logged;
	++lines_logged_this_file;

#if LINES_PER_FILE > 0
	if (lines_logged_this_file > LINES_PER_FILE) {
		// close_file() handles printing the error
		if ((err = close_file()) != FR_OK) {
			res = err;
		}

		new_name();
		lines_logged_this_file = 0;

		if ((err = f_open(&fh, logfile_name, FA_WRITE|FA_OPEN_APPEND)) != FR_OK) {
			OPEN_ERR_MSG(err);
			res = err;
		}
		if ((err = print_header()) != FR_OK) {
			res = err;
		}
	}
#endif // LINES_PER_FILE > 0

	return res;
}
static FRESULT print_header(void) {
	FRESULT err;

	LOGGER("Writing log header");
	lines_logged_this_file = 0;

	lprintf(F("# uptime\twarnings\tVcc_mV"));
	for (uiter_t i = 0; i < SENSOR_COUNT; ++i) {
		if (BIT_IS_SET(G_sensors[i].iflags, SENS_FLAG_MONITORED)) {
			lprintf("\t[!]%s", FROM_FSTR(SENSORS[i].name));
		} else {
			lprintf("\t%s", FROM_FSTR(SENSORS[i].name));
		}
	}
#if USE_CONTROLLERS
#if USE_SMALL_CONTROLLERS < 1
	for (uiter_t i = 0; i < CONTROLLER_COUNT; ++i) {
		const char *n = FROM_FSTR(CONTROLLERS[i].name);
		lprintf(F("\t[!]%s_count\t%s_time"), n, n);
	}
#endif // USE_SMALL_CONTROLLERS < 1
#endif // USE_CONTROLLERS
	lprintf_putc('\n');

	// This needs to be split to fit in the buffer for F()
	lprintf(F("# Warnings: B=battery low, V=Vcc low, S=sensor warning, C=controller warning, "));
	lprintf(F("L=log error, l=log sync skipped\n"));

	if ((err = next_line()) == FR_OK) {
		have_log_header = true;
	}

	return err;
}

static void power_on(void) {
	power_on_SPI();

	return;
}
static void power_off(void) {
	power_off_SPI();

	return;
}

#endif // USE_LOGGING


#ifdef __cplusplus
 }
#endif
