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
// serial.c
// Manage serial communication
// NOTES:
//

#ifdef __cplusplus
 extern "C" {
#endif

//#define DEBUG 1
/*
* Includes
*/
#include "serial.h"

#include "ulib/time.h"
#include "ulib/cstrings.h"
// fatfs/ff.h is only needed for the FatFS revision number
#include "fatfs/ff.h"


#if USE_SERIAL

#if DEBUG
# pragma message "SERIAL_BUFFER_SIZE: " XTRINGIZE(SERIAL_BUFFER_SIZE)
# pragma message "LOGGER_REPLAY_BUFFER_SIZE: " XTRINGIZE(LOGGER_REPLAY_BUFFER_SIZE)
#endif

/*
* Static values
*/
// Timeout in ms for serial communications; this is the timeout for a whole
// message
#define SERIAL_TIMEOUT_MS 10000 // 10s


/*
* Types
*/
#if LOGGER_REPLAY_BUFFER_SIZE > 0
typedef struct {
	char output[LOGGER_REPLAY_BUFFER_SIZE];
	// 'tail' is the index of the next byte to be written.
	uint16_t tail;
	// 'size' is the number of bytes that contain valid data.
	uint16_t size;
} logger_replay_buffer_t;
#endif


/*
* Variables
*/
bool G_serial_is_up = false;

#if SERIAL_BUFFER_SIZE > 0
static uint8_t printf_buffer[SERIAL_BUFFER_SIZE];
static uint8_t printf_buffer_size = 0;
#endif // SERIAL_BUFFER_SIZE > 0

#if LOGGER_REPLAY_BUFFER_SIZE > 0
logger_replay_buffer_t logger_replay_buffer = { { 0 }, 0, 0 };
#endif
/*
* Local function prototypes
*/
static void flush_printf_buffer(void);
static void serial_putc(int c);


/*
* Interrupt handlers
*/


/*
* Functions
*/
void serial_init(void) {
	// TODO: Handle uart_init() failure
	print_system_info();
#if USE_TERMINAL
	PUTS("Press any key to enter the console\r\n", 0);
	PUTS("If that doesn't work, press the user button first\r\n", 0);
#endif

	G_serial_is_up = true;

	return;
}

static void flush_printf_buffer(void) {
#if SERIAL_BUFFER_SIZE > 0
	if (printf_buffer_size > 0) {
		uart_transmit_block(printf_buffer, printf_buffer_size, SERIAL_TIMEOUT_MS);
		printf_buffer_size = 0;
	}
#endif // SERIAL_BUFFER_SIZE > 0

	return;
}
#if SERIAL_BUFFER_SIZE > 0
static void serial_putc(int c) {
	assert(printf_buffer_size < SERIAL_BUFFER_SIZE);

	printf_buffer[printf_buffer_size] = c;
	++printf_buffer_size;
	if (printf_buffer_size == SERIAL_BUFFER_SIZE) {
		flush_printf_buffer();
	}

	return;
}
#else // !SERIAL_BUFFER_SIZE > 0
static void serial_putc(int c) {
	uint8_t c8 = c;
	uart_transmit_block(&c8, 1, SERIAL_TIMEOUT_MS);

	return;
}
#endif // SERIAL_BUFFER_SIZE > 0
void serial_print(const char *msg, txsize_t len) {
	assert(msg != NULL);

	if (len == 0) {
		len = cstring_len(msg);
	}
	if (len == 0) {
		return;
	}

	flush_printf_buffer();
	uart_transmit_block((uint8_t *)msg, len, SERIAL_TIMEOUT_MS);

	return;
}
void serial_printf(const char *fmt, ...) {
	va_list arp;

	assert(fmt != NULL);

	va_start(arp, fmt);
	vaprintf(serial_putc, fmt, arp);
	flush_printf_buffer();
	va_end(arp);

	return;
}

#if LOGGER_REPLAY_BUFFER_SIZE > 0
static void logger_putc(int c) {
	serial_putc(c);

	logger_replay_buffer.output[logger_replay_buffer.tail] = c;
	logger_replay_buffer.tail = (logger_replay_buffer.tail + 1) % LOGGER_REPLAY_BUFFER_SIZE;
	if (logger_replay_buffer.size != LOGGER_REPLAY_BUFFER_SIZE) {
		++logger_replay_buffer.size;
	}

	return;
}
void logger_replay(void) {
	txsize_t len;
	uint8_t *msg;

	flush_printf_buffer();
	len = logger_replay_buffer.size-logger_replay_buffer.tail;
	if (len > 0) {
		msg = (uint8_t *)&logger_replay_buffer.output[logger_replay_buffer.tail];
		uart_transmit_block(msg, len, SERIAL_TIMEOUT_MS);
	}
	if (logger_replay_buffer.tail > 0) {
		len = logger_replay_buffer.tail;
		msg = (uint8_t *)logger_replay_buffer.output;
		uart_transmit_block(msg, len, SERIAL_TIMEOUT_MS);
	}

	return;
}
#else // !LOGGER_REPLAY_BUFFER_SIZE > 0
static void logger_putc(int c) {
	serial_putc(c);

	return;
}
void logger_replay(void) {
	return;
}
#endif // LOGGER_REPLAY_BUFFER_SIZE > 0
void logger(const char *fmt, ...) {
	va_list arp;

	assert(fmt != NULL);

	if (!G_serial_is_up) {
		return;
	}

	va_start(arp, fmt);
	// Prefix message with system up time
	vvprintf(logger_putc, "%04lu:  ", (long unsigned int )NOW());
	vaprintf(logger_putc, fmt, arp);
	// Append message with a newline
	logger_putc('\r'); logger_putc('\n');
	flush_printf_buffer();
	va_end(arp);

	return;
}

void print_system_info(void) {
	PRINTF("%s version %s\r\n", PROGNAME, PROGVERS);

#if USE_SMALL_CODE < 2
	uint8_t year, month, day, hour, minute, second;
	utime_t seconds;

	seconds = get_RTC_seconds();
	seconds_to_date(seconds, &year, &month, &day);
	seconds_to_time(seconds, &hour, &minute, &second);

	PRINTF("Build Date: %s PlatformIO: %u\r\n", BUILD_DATE, (uint )PLATFORMIO);
	PRINTF("Current system time is %04u.%02u.%02u %02u:%02u:%02u\r\n",
		(uint )(YEAR_0 + year),
		(uint )month,
		(uint )day,
		(uint )hour,
		(uint )minute,
		(uint )second
	);
#if USE_SD
	PRINTF("Using FatFS revision %u\r\n", (uint )FFCONF_DEF);
#endif // USE_SD

	print_platform_info(serial_putc);
	flush_printf_buffer();
#endif

	return;
}

#endif // USE_SERIAL

#ifdef __cplusplus
 }
#endif
