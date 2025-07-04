// SPDX-License-Identifier: GPL-3.0-only
/***********************************************************************
*                                                                      *
*                                                                      *
* Copyright 2022, 2023 svijsv                                          *
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
* along with this program. If not, see <http:// www.gnu.org/licenses/>.*
*                                                                      *
*                                                                      *
***********************************************************************/
// msg.h
// Communicate with the user
// NOTES:
//    This is very much *not* thread-safe.
//
//    Unless otherwise noted, functions which return an error code return
//    -errno where 'errno' is the errno number corresponding to the problem
//    encountered.
//
// BUGS:
//    When not using malloc(), the prefix and format strings can't be set to
//    an empty string because that's taken to mean leave the value alone.
//
//
#ifndef _ULIB_MSG_H
#define _ULIB_MSG_H

#include "src/configify.h"
#if ULIB_ENABLE_MSG

#include "types.h"

#include <time.h>

/*
 * Named verbosity levels
 */
#define MSG_VERB_ERROR   -2
#define MSG_VERB_WARNING -1
#define MSG_VERB_NORMAL   0
#define MSG_VERB_INFO     1
#define MSG_VERB_EXTRA    2
#define MSG_VERB_TMI      3

/*
 * Flags passed to msg_config() to change msg.c behavior
 */
// Set forced mode for questions; this only changes the default answer,
// MSG_FLAG_INTERACT is still respected
#define MSG_FLAG_FORCE         0x01U
// Set interactive mode for questions
#define MSG_FLAG_INTERACT      0x02U
// Allow modules to print error messages
#define MSG_FLAG_LIBERRORS     0x04U
// Print the time before log messages
#define MSG_FLAG_LOG_PRINTTIME 0x08U
// Use direct I/O when writing to log files (used only with MSG_USE_UNIX_IO)
#define MSG_FLAG_LOG_DIRECT    0x10U
// Always print messages in msg_ask() even when not interactive
#define MSG_FLAG_ALWAYS_PRINT_QUESTIONS 0x20U

/*
 * Structure passed to msg_config() to configure the module.
 */
typedef struct {
#if MSG_USE_MALLOC
	// Prefix used before warning messages; defaults to "WARNING: ". Set to
	// NULL to keep current value.
	char *warn_prefix;
	// Prefix used before error messages; defaults to "ERROR: ". Set to NULL
	// to keep current value.
	char *error_prefix;
	// Prefix used before debug messages; defaults to "DEBUG: ". Set to NULL
	// to keep current value.
	char *debug_prefix;
	// Name of the program printed before messages; defaults to none. Set to
	// NULL to keep current value or empty string to disable.
	char *program_name;
	// Name of the log file; defaults to "LOG". Set to NULL to keep current value.
	// The configured value is overridden if msg_open_log() is called.
	// Only used for messages.
	char *log_name;
#else
	// Same as above but set to an empty string to keep current value.
	char warn_prefix[MSG_STR_BYTES];
	char error_prefix[MSG_STR_BYTES];
	char debug_prefix[MSG_STR_BYTES];
	char program_name[MSG_STR_BYTES];
	char log_name[MSG_STR_BYTES];
#endif

	// Function used to print the current time to a string for logging purposes.
	// Must return the pointer to the start of the formatted time, which doesn't
	// necessarily need to be in 'buf'.
	// Set to NULL to keep the current value. Disabled by default.
	const char* (*print_log_time)(char *buf, size_t size);

#if MSG_USE_UNIX_IO
	// FD used for stdin; defaults to STDIN_FILENO. Set to -1 to keep current value
	// and any other negative number to disable stdin.
	int stdin_fd;
	// FD used for stdout; defaults to STDOUT_FILENO. Set to -1 to keep current value
	// and any other negative number to disable stdout.
	int stdout_fd;
	// FD used for stderr; defaults to STDERR_FILENO. Set to -1 to keep current value
	// and any other negative number to disable stderr.
	int stderr_fd;
	// FD used for writing to the log; defaults to -1 (disabled). Set to -1 to keep
	// current value and any other negative number to disable log output.
	// msg_open_log() and msg_close_log() will close this fd if called.
	int log_fd;
#else
	// Function used for stdin. Set to NULL to keep current value. Disabled by default.
	ssize_t (*stdin_read)(uint8_t *buf, size_t count);
	// Function used for stdout. Set to NULL to keep current value. Disabled by default.
	ssize_t (*stdout_write)(const uint8_t *buf, size_t count);
	// Function used for stderr. Set to NULL to keep current value. Disabled by default.
	ssize_t (*stderr_write)(const uint8_t *buf, size_t count);
	// Function used to write to the log. Set to NULL to keep current value. Disabled by default.
	ssize_t (*log_write)(const uint8_t *buf, size_t count);
#endif

	// Verbosity level for printing messages; default is 0.
	int_fast8_t verbosity;
	// Behavior-modifying flags; defaults to 0.
	uint_fast8_t flags;
} msg_config_t;

/*
 * msg_config()
 * Set up the message system.
 *
 * This doesn't need to be called unless changing from the defaults, which
 * are defined in msg.c.
 *
 * Can be called repeatedly without issues.
 *
 * The return value is always 0.
 */
int msg_config(msg_config_t *cfg);
/*
 * msg_set_flags()
 * Change the active configuration flags.
 *
 * Returns the old flags.
 */
uint_fast8_t msg_set_flags(uint_fast8_t flags);
/*
 * msg_set_verbosity()
 * Change the active verbosity level.
 *
 * Returns the old level.
 */
int_fast8_t msg_set_verbosity(int_fast8_t verbosity);

/*
 * msg_get_config()
 * Get a copy of the current configuration.

 * If successful, returns 0 and cfg is overwritten with a copy of the current
 * configuration. Otherwise returns an error code.
 *
 * If MSG_USE_MALLOC is set:
 *    Do not free any pointers in cfg
 *    If msg_config() is called, the pointers may become invalid
 *
 * If MSG_USE_UNIS_IO is set:
 *    Do not close any file descriptors in cfg
 *    If msg_config() is called, the file descriptors may become invalid
 */
int msg_get_config(msg_config_t *cfg);

#if MSG_USE_UNIX_IO
/*
 * msg_open_log()
 * Open a log file.
 *
 * If a log file is already opened, it's closed after opening the new one.
 *
 * The log is always opened in append mode.
 *
 * If opening the new file fails, the current log output is left unchanged.
 *
 * If path is NULL or empty or opening the new file fails, return an error.
 */
int msg_open_log(const char* path);
/*
 * msg_close_log()
 * Close the log file.
 *
 * If no log file is currently open, return successfully.
 */
int msg_close_log(void);
#endif

/*
 * msg_ask()
 * Ask a yes/no question.
 *
 * Keeps asking until either 'y' or 'n' are the first letter of the response,
 * or until a blank response is given and the default used.
 *
 * If MSG_FORCE is set, the default answer is ans_force. Otherwise it's ans_default.
 * If MSG_INTERACT isn't set, retuns immediately with the default answer.
 */
bool msg_ask(bool ans_default, bool ans_forced, const char *restrict fmt, ...)
	__attribute__ ((format(printf, 3, 4)));

/*
 * msg_log()
 * Print a message to the log.
 *
 * If there is no log opened, nothing is done.
 */
void msg_log(const char *restrict fmt, ...)
	__attribute__ ((format(printf, 1, 2)));

/*
 * msg_error()
 * Print an error message to stderr, followed by a newline.
 *
 * Messages are prefixed with the error prefix string set in msg_config().
 */
void msg_error(const char *restrict fmt, ...)
	__attribute__ ((format(printf, 1, 2)));
/*
 * msg_errno()
 * Print an error message to stderr.
 *
 * Same as msg_error(), but after printing the message the result of strerror(errnum)
 * is printed in the format ': %s.\n'
 */
void msg_errno(int errnum, const char *restrict fmt, ...)
	__attribute__ ((format(printf, 2, 3)));
/*
 * msg_liberrno()
 * Print an error message to stderr.
 *
 * Same as msg_errno(), but output is disabled unless MSG_LIBERRORS is set.
 */
void msg_liberrno(int errnum, const char *restrict fmt, ...)
	__attribute__ ((format(printf, 2, 3)));
/*
 * msg_warn()
 * Print a warning message to stderr, followed by a newline.
 *
 * Same as msg_error() but prefixed with the warning prefix.
 */
void msg_warn(const char *restrict fmt, ...)
	__attribute__ ((format(printf, 1, 2)));
/*
 * msg_warnno()
 * Print a warning message to stderr, followed by a newline.
 *
 * Same as msg_errno() but prefixed with the warning prefix.
 */
void msg_warnno(int errnum, const char *restrict fmt, ...)
	__attribute__ ((format(printf, 2, 3)));
/*
 * msg_debug()
 * Print a string to stderr if the macro DEBUG is non-zero.
 */
void msg_debug(const char *restrict fmt, ...)
	__attribute__ ((format(printf, 1, 2)));
/*
 * msg_print()
 * Print a message to stdout.
 *
 * Nothing is printed unless priority <= msg_verbosity.
 */
void msg_print(int8_t priority, const char *restrict fmt, ...)
	__attribute__ ((format(printf, 2, 3)));
// This may be more efficient, but not majorly so (and I'd have to export msg_verbosity):
//#define msg_print(priority, ...) (if (priority <= msg_verbosity) _msg_print( ## __VA_ARGS__);)
/*
 * msg_puts()
 * Print a string to stdout
 *
 * This differs from puts() in that no trailing newline is printed and the
 * return value is -errno on error.
 */
int msg_puts(const char *s);
/*
 * msg_gets()
 * Retrieve a string from stdin
 *
 * This behaves like fgets() except for the return value, that any trailing
 * newline is *not* stored in the string, and that reading stops if a NUL
 * byte is recieved.
 *
 * The return value is the number of non-NUL bytes placed in the string on
 * success or -errno on error.
 */
int msg_gets(char *s, int size);

#endif // _ULIB_MSG_H
#endif // ULIB_ENABLE_MSG
