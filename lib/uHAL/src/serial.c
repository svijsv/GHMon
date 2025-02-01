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
// serial.c
// Manage serial communication
// NOTES:
//

#include "common.h"

#include "ulib/include/time.h"
#include "ulib/include/cstrings.h"
#include "ulib/include/printf.h"

// ff.h is only needed for the FatFS revision number
#if uHAL_USE_FATFS
# include FATFS_FF_H_PATH
#endif

#include <stdarg.h>
// Needed for strlen()
#include <string.h>


#if uHAL_USE_UART_COMM

DEBUG_CPP_MACRO(UART_COMM_BUFFER_BYTES)
DEBUG_CPP_MACRO(LOGGER_HISTORY_BUFFER_BYTES)

#if LOGGER_HISTORY_BUFFER_BYTES > 0xFFFFFFFFU
# error "LOGGER_HISTORY_BUFFER_BYTES must be <= 0xFFFFFFFF"
#elif LOGGER_HISTORY_BUFFER_BYTES > 0xFFFFU
typedef uint_fast32_t replay_size_t;
#elif LOGGER_HISTORY_BUFFER_BYTES > 0xFFU
typedef uint_fast16_t replay_size_t;
#elif LOGGER_HISTORY_BUFFER_BYTES > 0
typedef uint_fast8_t replay_size_t;
#endif


#if LOGGER_HISTORY_BUFFER_BYTES > 0
typedef struct {
	char output[LOGGER_HISTORY_BUFFER_BYTES];
	// 'tail' is the index of the next byte to be written.
	replay_size_t tail;
	// 'size' is the number of bytes that contain valid data.
	replay_size_t size;
} logger_replay_buffer_t;

logger_replay_buffer_t logger_replay_buffer = { { 0 }, 0, 0 };
#endif


#if UART_COMM_BUFFER_BYTES > 0
static uint8_t printf_buffer[UART_COMM_BUFFER_BYTES];
static uint8_t printf_buffer_size = 0;
#endif

void _print_platform_info(void (*printf_putc)(uint_fast8_t c));

static void flush_printf_buffer(void);
static void serial_putc(uint_fast8_t c);


err_t serial_init(void) {
#if ! uHAL_SKIP_INIT_CHECKS
	if (!uart_is_on(NULL)) {
		return ERR_INIT;
	}
#endif

	uHAL_SET_STATUS(uHAL_FLAG_SERIAL_IS_UP);

	print_system_info();
#if ! uHAL_USE_SMALL_MESSAGES
# if uHAL_USE_TERMINAL
	PUTS("Press any key to enter the console\r\n", 0);
	PUTS("If that doesn't work, you may need to press a button first\r\n", 0);
	PUTS("If *that* doesn't work, you may be out of luck\r\n", 0);
# endif
#endif

	return ERR_OK;
}

static void flush_printf_buffer(void) {
#if UART_COMM_BUFFER_BYTES > 0
	if (printf_buffer_size > 0) {
		uart_transmit_block(NULL, printf_buffer, printf_buffer_size, UART_COMM_TIMEOUT_MS);
		printf_buffer_size = 0;
	}
#endif

	return;
}
#if UART_COMM_BUFFER_BYTES > 0
static void serial_putc(uint_fast8_t c) {
	uHAL_assert(printf_buffer_size < UART_COMM_BUFFER_BYTES);

	printf_buffer[printf_buffer_size] = c;
	++printf_buffer_size;
	if (printf_buffer_size == UART_COMM_BUFFER_BYTES) {
		flush_printf_buffer();
	}

	return;
}
#else // !UART_COMM_BUFFER_BYTES > 0
static void serial_putc(uint_fast8_t c) {
	uint8_t c8 = c;
	uart_transmit_block(NULL, &c8, 1, UART_COMM_TIMEOUT_MS);

	return;
}
#endif // UART_COMM_BUFFER_BYTES > 0
void serial_print(const char *msg, txsize_t len) {
	uHAL_assert(msg != NULL);

#if ! uHAL_SKIP_INIT_CHECKS
#endif
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (msg == NULL) {
		return;
	}
#endif

	if (!uHAL_CHECK_STATUS(uHAL_FLAG_SERIAL_IS_UP)) {
		return;
	}

	if (len == 0) {
		len = strlen(msg);
	}
	if (len == 0) {
		return;
	}

	flush_printf_buffer();
	uart_transmit_block(NULL, (uint8_t *)msg, len, UART_COMM_TIMEOUT_MS);

	return;
}
void serial_printf(const char *fmt, ...) {
	va_list arp;

	uHAL_assert(fmt != NULL);
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (fmt == NULL) {
		return;
	}
#endif

	if (!uHAL_CHECK_STATUS(uHAL_FLAG_SERIAL_IS_UP)) {
		return;
	}

	va_start(arp, fmt);
	printf_va(serial_putc, fmt, arp);
	flush_printf_buffer();
	va_end(arp);

	return;
}

#if LOGGER_HISTORY_BUFFER_BYTES > 0
static void logger_putc(uint_fast8_t c) {
	uHAL_assert(logger_replay_buffer.size <= LOGGER_HISTORY_BUFFER_BYTES);

	serial_putc(c);

	logger_replay_buffer.output[logger_replay_buffer.tail] = c;
	logger_replay_buffer.tail = (logger_replay_buffer.tail + 1) % LOGGER_HISTORY_BUFFER_BYTES;
	if (logger_replay_buffer.size != LOGGER_HISTORY_BUFFER_BYTES) {
		++logger_replay_buffer.size;
	}

	return;
}
void logger_replay(void) {
	txsize_t len;
	uint8_t *msg;

#if ! uHAL_SKIP_INIT_CHECKS
#endif
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
#endif

	if (!uHAL_CHECK_STATUS(uHAL_FLAG_SERIAL_IS_UP)) {
		return;
	}

	flush_printf_buffer();
	// If the size is > the tail we've wrapped around, so output the second
	// (older) half first then do the first (newer) half
	len = logger_replay_buffer.size - logger_replay_buffer.tail;
	if (len > 0) {
		msg = (uint8_t *)&logger_replay_buffer.output[logger_replay_buffer.tail];
		uart_transmit_block(NULL, msg, len, UART_COMM_TIMEOUT_MS);
	}
	if (logger_replay_buffer.tail > 0) {
		len = logger_replay_buffer.tail;
		msg = (uint8_t *)logger_replay_buffer.output;
		uart_transmit_block(NULL, msg, len, UART_COMM_TIMEOUT_MS);
	}

	return;
}
#else // !LOGGER_HISTORY_BUFFER_BYTES > 0
static void logger_putc(uint_fast8_t c) {
	serial_putc(c);

	return;
}
void logger_replay(void) {
	return;
}
#endif // LOGGER_HISTORY_BUFFER_BYTES > 0
void logger(const char *fmt, ...) {
	va_list arp;

	uHAL_assert(fmt != NULL);
#if ! uHAL_SKIP_INIT_CHECKS
#endif
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (fmt == NULL) {
		return;
	}
#endif

	if (!uHAL_CHECK_STATUS(uHAL_FLAG_SERIAL_IS_UP)) {
		return;
	}

	va_start(arp, fmt);
#if uHAL_USE_RTC
	// Prefix message with system up time
	printf_vv(logger_putc, "%04lu:  ", (long unsigned int )NOW());
#endif
	printf_va(logger_putc, fmt, arp);
	// Append message with a newline
	logger_putc('\r'); logger_putc('\n');
	flush_printf_buffer();
	va_end(arp);

	return;
}

void print_system_info(void) {
	if (!uHAL_CHECK_STATUS(uHAL_FLAG_SERIAL_IS_UP)) {
		return;
	}

#if defined(PROGNAME)
# if defined(PROGVERS)
	PRINTF("%s version %s\r\n", PROGNAME, PROGVERS);
# else
	PRINTF("%s\r\n", PROGNAME);
# endif
#endif

#if ! uHAL_USE_SMALL_MESSAGES
	PRINTF("Build Date: %s\r\nPlatformIO: %u\r\n", BUILD_DATE, (uint )PLATFORMIO);

# if uHAL_USE_RTC
	datetime_t dt;
	get_RTC_datetime(&dt);

	PRINTF("Current system time is %04u.%02u.%02u %02u:%02u:%02u\r\n",
		(uint )dt.year,
		(uint )dt.month,
		(uint )dt.day,
		(uint )dt.hour,
		(uint )dt.minute,
		(uint )dt.second
	);
# endif

#if uHAL_USE_UPTIME
	char buf[16];
	PRINTF("Current system uptime is %s\r\n", print_uptime(get_uptime(), buf, SIZEOF_ARRAY(buf)));
#endif

# if uHAL_USE_FATFS
	PRINTF("Using FatFS revision %u\r\n", (uint )FFCONF_DEF);
# endif

	_print_platform_info(serial_putc);
#endif

	flush_printf_buffer();

	return;
}
void print_platform_info(void) {
	_print_platform_info(serial_putc);
	return;
}


#endif // uHAL_USE_UART_COMM
