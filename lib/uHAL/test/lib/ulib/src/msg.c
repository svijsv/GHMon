// SPDX-License-Identifier: GPL-3.0-only
/***********************************************************************
*                                                                      *
*                                                                      *
* Copyright 2022, 2023, 2025 svijsv                                    *
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
// msg.c
// Communicate with the user
// NOTES:
//
//
#include "msg.h"
#if ULIB_ENABLE_MSG

#include "bits.h"
#include "cstrings.h"
#include "debug.h"

#include <errno.h>
#include <string.h>

#if MSG_USE_MALLOC
# include <stdlib.h>
#endif

#if MSG_USE_INTERNAL_PRINTF
# include "printf.h"
#else
# include <stdio.h>
#endif

#if MSG_USE_UNIX_IO
# include <fcntl.h>
# include <unistd.h>

# if !defined(O_DIRECT)
#  define O_DIRECT 0
# endif
# if !defined(O_CLOEXEC)
#  define O_CLOEXEC 0
# endif
#endif

#define CONFIG_FLAG_FORCED_SET(_f_) ((MSG_FORCED_CONFIG_FLAGS) & (_f_))
#define CONFIG_FLAG_FORCED_UNSET(_f_) ((MSG_FORBIDDEN_CONFIG_FLAGS) & (_f_))
#define CONFIG_FLAG_IS_SET(_f_) ((!CONFIG_FLAG_FORCED_UNSET(_f_)) && (CONFIG_FLAG_FORCED_SET(_f_) || (config.flags & (_f_))))

#define DEFAULT_ERROR_PREFIX "ERROR: "
#define DEFAULT_WARN_PREFIX  "WARNING: "
#define DEFAULT_DEBUG_PREFIX  "DEBUG: "
#define DEFAULT_PROGRAM_NAME ""
#define DEFAULT_LOG_NAME "LOG"

static const char newline[] = MSG_NEWLINE_STRING;
static const uint_fast8_t newline_len = SIZEOF_ARRAY(newline) - 1;

#if MSG_USE_MALLOC
static char default_error_prefix[] = DEFAULT_ERROR_PREFIX;
static char default_warn_prefix[] = DEFAULT_WARN_PREFIX;
static char default_debug_prefix[] = DEFAULT_DEBUG_PREFIX;
static char default_program_name[] = DEFAULT_PROGRAM_NAME;
static char default_log_name[] = DEFAULT_LOG_NAME;
#endif

static msg_config_t config = {
#if MSG_USE_MALLOC
	.error_prefix = default_error_prefix,
	.warn_prefix = default_warn_prefix,
	.debug_prefix = default_debug_prefix,
	.program_name = default_program_name,
	.log_name = default_log_name,
#else
	.error_prefix = DEFAULT_ERROR_PREFIX,
	.warn_prefix = DEFAULT_WARN_PREFIX,
	.debug_prefix = DEFAULT_DEBUG_PREFIX,
	.program_name = DEFAULT_PROGRAM_NAME,
	.log_name = DEFAULT_LOG_NAME,
#endif

	.print_log_time = NULL,

#if MSG_USE_UNIX_IO
	.stdin_fd = STDIN_FILENO,
	.stdout_fd = STDOUT_FILENO,
	.stderr_fd = STDERR_FILENO,
	.log_fd = -1,
#else
	.stdin_read = NULL,
	.stdout_write = NULL,
	.stderr_write = NULL,
	.log_write = NULL,
#endif
};

#if MSG_USE_UNIX_IO
static ssize_t stdin_read(void *buf, size_t count) {
	return (config.stdin_fd >= 0) ? read(config.stdin_fd, buf, count) : 0;
}
# define WRITE_ALLOWED(_nm_) (config. _nm_  ##  _fd >= 0)
# define WRITE(_nm_, _buf_, _cnt_) (write(config. _nm_ ##  _fd, (_buf_), (_cnt_)))

#else // ! MSG_USE_UNIX_IO
static ssize_t stdin_read(void *buf, size_t count) {
	return (config.stdin_read != NULL) ? config.stdin_read(buf, count) : 0;
}
# define WRITE_ALLOWED(_nm_) (config. _nm_ ##  _write != NULL)
# define WRITE(_nm_, _buf_, _cnt_) (config. _nm_ ##  _write((_buf_), (_cnt_)))
#endif // MSG_USE_UNIX_IO

static ssize_t stdout_write(const void *buf, size_t count) {
	if (WRITE_ALLOWED(stdout)) {
		return WRITE(stdout, buf, count);
	}
	return (ssize_t )count;
}
static ssize_t stderr_write(const void *buf, size_t count) {
	if (WRITE_ALLOWED(stderr)) {
		return WRITE(stderr, buf, count);
	}
	return (ssize_t )count;
}
static ssize_t log_write(const void *buf, size_t count) {
	if (WRITE_ALLOWED(log)) {
		return WRITE(log, buf, count);
	}
	return (ssize_t )count;
}

#if MSG_USE_INTERNAL_PRINTF
static void stdout_putc(uint8_t c) {
	WRITE(stdout, &c, 1);
}
static void stderr_putc(uint8_t c) {
	WRITE(stderr, &c, 1);
}
static void log_putc(uint8_t c) {
	WRITE(log, &c, 1);
}

static void _vprintf(bool OK, void (*pputc)(uint8_t c), const char *restrict format, va_list ap) {
	if (OK) {
		ulib_vprintf(pputc, format, ap);
	}
	return;
}
# define stdout_vprintf(...) _vprintf(WRITE_ALLOWED(stdout), stdout_putc, __VA_ARGS__)
# define stderr_vprintf(...) _vprintf(WRITE_ALLOWED(stderr), stderr_putc, __VA_ARGS__)
# define log_vprintf(...) _vprintf(WRITE_ALLOWED(log), log_putc, __VA_ARGS__)

static void _printf(bool OK, void (*pputc)(uint8_t c), const char *restrict format, ...) {
	if (OK) {
		va_list ap;

		va_start(ap, format);
		ulib_vprintf(pputc, format, ap);
		va_end(ap);
	}
	return;
}
# define stdout_printf(...) _printf(WRITE_ALLOWED(stdout), stdout_putc, __VA_ARGS__)
# define stderr_printf(...) _printf(WRITE_ALLOWED(stderr), stderr_putc, __VA_ARGS__)
# define log_printf(...) _printf(WRITE_ALLOWED(log), log_putc, __VA_ARGS__)

#else // ! MSG_USE_INTERNAL_PRINTF
// This requires UNIX IO because there's no standard printf version that would
// accept the write() functions
# if ! MSG_USE_UNIX_IO
#  error "MSG_USE_UNIX_IO must be set if MSG_USE_INTERNAL_PRINTF is not set"
# endif

static void _vprintf(int fd, const char *restrict format, va_list ap) {
	if (fd >= 0) {
		vdprintf(fd, format, ap);
	}
	return;
}
# define stdout_vprintf(...) _vprintf(config.stdout_fd, ## __VA_ARGS__)
# define stderr_vprintf(...) _vprintf(config.stderr_fd, ## __VA_ARGS__)
# define log_vprintf(...) _vprintf(config.log_fd, ## __VA_ARGS__)

static void _printf(int fd, const char *restrict format, ...) {
	if (fd >= 0) {
		va_list ap;

		va_start(ap, format);
		vdprintf(fd, format, ap);
		va_end(ap);
	}
	return;
}
# define stdout_printf(...) _printf(config.stdout_fd, ## __VA_ARGS__)
# define stderr_printf(...) _printf(config.stderr_fd, ## __VA_ARGS__)
# define log_printf(...) _printf(config.log_fd, ## __VA_ARGS__)
#endif // MSG_USE_INTERNAL_PRINTF

int msg_puts(const char *s) {
	size_t len;
	ssize_t writ = 0;

	ulib_assert(s != NULL);
#if DO_MSG_SAFETY_CHECKS
	if (s == NULL) {
		return -EINVAL;
	}
#endif

	len = strlen(s);
	do {
		if (writ > 0) {
			len -= (size_t )writ;
		}
		writ = stdout_write(s, len);
	} while ((writ < (ssize_t )len) && ((writ != -1) || (errno == EINTR)));
	if (writ == -1) {
		return -errno;
	}

	return 0;
}
int msg_gets(char *restrict s, int size) {
	char b;
	ssize_t r;
	int have;

	ulib_assert(s != NULL);
	ulib_assert(size > 0);
#if DO_MSG_SAFETY_CHECKS
	if (s == NULL) {
		return -EINVAL;
	}
	if (size <= 0) {
		return -EINVAL;
	}
#endif

	size -= 1;
	for (have = 0; have < size;) {
		r = stdin_read(&b, 1);
		if (r == -1) {
			if (errno != EINTR) {
				return -errno;
			}
		} else if ((r == 0) || (b == '\n') || (b == 0)) {
			break;
		} else if (b == '\r') {
			// Nothing to  do here.
		} else if (b == '\b') {
			if (have > 0) {
				--have;
			}
		} else {
			s[have] = b;
			++have;
		}
	}
	s[have] = 0;

	return have;
}

#if MSG_USE_MALLOC
static char* set_config_string(char *now, const char *new, const char *def) {
	if (new == NULL || new == now) {
		/*
		if (now == NULL) {
			now = strdup(def);
		}
		*/
		return now;
	}
	if (now != NULL && now != def) {
		free(now);
	}
	return strdup(new);
}
#else
// FIXME: Can't set any of these to an empty string...
static void set_config_string(char now[MSG_STR_BYTES], char new[MSG_STR_BYTES]) {
	if (new[0] == 0) {
		return;
	}
	strncpy(now, new, MSG_STR_BYTES);
	now[MSG_STR_BYTES-1] = 0;

	return;
}
#endif

#if MSG_USE_UNIX_IO
static bool is_closeable_fd(int fd) {
	if (fd < 0) {
		return false;
	}
	switch (fd) {
	case STDIN_FILENO:
	case STDOUT_FILENO:
	case STDERR_FILENO:
		return false;
	}

	return true;
}
static int set_config_fd(int old_fd, int new_fd) {
	if (new_fd != -1 && new_fd != old_fd) {
		if (MSG_CLOSE_FDS_ON_CONFIG && is_closeable_fd(old_fd)) {
			close(old_fd);
		}
		return new_fd;
	}
	return old_fd;
}
#endif

int msg_config(msg_config_t *new_config) {
	ulib_assert(new_config != NULL);

#if DO_MSG_SAFETY_CHECKS
	if (new_config == NULL) {
		return 0;
	}
#endif

	config.verbosity = new_config->verbosity;
	config.flags = new_config->flags;

	// FIXME: Can't disable once set.
	if (new_config->print_log_time != NULL) {
		config.print_log_time = new_config->print_log_time;
	}

#if MSG_USE_UNIX_IO
	config.stdin_fd = set_config_fd(config.stdin_fd, new_config->stdin_fd);
	config.stdout_fd = set_config_fd(config.stdout_fd, new_config->stdout_fd);
	config.stderr_fd = set_config_fd(config.stderr_fd, new_config->stderr_fd);
	config.log_fd = set_config_fd(config.log_fd, new_config->log_fd);

#else // MSG_USE_UNIX_IO
	// FIXME: Can't disable once a value has been set.
	if (new_config->stdin_read != NULL) {
		config.stdin_read = new_config->stdin_read;
	}
	if (new_config->stdout_write != NULL) {
		config.stdout_write = new_config->stdout_write;
	}
	if (new_config->stderr_write != NULL) {
		config.stderr_write = new_config->stderr_write;
	}
	if (new_config->log_write != NULL) {
		config.log_write = new_config->log_write;
	}

#endif // MSG_USE_UNIX_IO

#if MSG_USE_MALLOC
	config.warn_prefix = set_config_string(config.warn_prefix, new_config->warn_prefix, default_warn_prefix);
	config.debug_prefix = set_config_string(config.debug_prefix, new_config->debug_prefix, default_debug_prefix);
	config.error_prefix = set_config_string(config.error_prefix, new_config->error_prefix, default_error_prefix);
	config.program_name = set_config_string(config.program_name, new_config->program_name, default_program_name);
	config.log_name = set_config_string(config.log_name, new_config->log_name, default_log_name);
#else
	set_config_string(config.warn_prefix, new_config->warn_prefix);
	set_config_string(config.debug_prefix, new_config->debug_prefix);
	set_config_string(config.error_prefix, new_config->error_prefix);
	set_config_string(config.program_name, new_config->program_name);
	set_config_string(config.log_name, new_config->log_name);
#endif

	return 0;
}
uint_fast8_t msg_set_flags(uint_fast8_t flags) {
	uint_fast8_t old = config.flags;
	config.flags = flags;

	return old;
}
int_fast8_t msg_set_verbosity(int_fast8_t verbosity) {
	int_fast8_t old = config.verbosity;
	config.verbosity = verbosity;

	return old;
}

int msg_get_config(msg_config_t *cfg) {
#if DO_MSG_SAFETY_CHECKS
	if (cfg == NULL) {
		return -EINVAL;
	}
#endif

	memcpy(cfg, &config, sizeof(config));
	return 0;
}

#if MSG_USE_UNIX_IO
int msg_open_log(const char* path) {
	int o_flags = O_WRONLY|O_APPEND|O_CREAT|O_CLOEXEC;
	int new_fd;

#if DO_MSG_SAFETY_CHECKS
	if (path == NULL || path[0] == 0) {
		return -EINVAL;
	}
#endif

	if (CONFIG_FLAG_IS_SET(MSG_FLAG_LOG_DIRECT)) {
		SET_BIT(o_flags, O_DIRECT);
	}

	do {
		new_fd = open(path, o_flags, 0755);
	} while ((new_fd == -1) && (errno == EINTR));

	if (new_fd == -1) {
		int ret = -errno;

		msg_liberrno(errno, "%s: failed to open log file", path);
		return ret;
	}
	if (is_closeable_fd(config.log_fd)) {
		if (close(config.log_fd) == -1) {
			msg_liberrno(errno, "%s: close() error", config.log_name);
		}
	}
	config.log_fd = new_fd;

#if MSG_USE_MALLOC
	if (config.log_name != NULL && config.log_name != default_log_name) {
		free(config.log_name);
	}
	config.log_name = strdup(cstring_basename(path));
#else
	strncpy(config.log_name, cstring_basename(path), sizeof(config.log_name));
#endif

	return 0;
}
int msg_close_log(void) {
	int ret = 0;

	if (is_closeable_fd(config.log_fd)) {
		if (close(config.log_fd) == -1) {
			ret = -errno;
			msg_liberrno(errno, "%s: close() error", config.log_name);
		}
	}
	config.log_fd = -1;

	return ret;
}
#endif // MSG_USE_UNIX_IO

bool msg_ask(bool ans_default, bool ans_forced, const char *restrict fmt, ...) {
	va_list args;
	char answer[8] = { 0 };
	const char *def_str;
	bool ans;

	ulib_assert(fmt != NULL);

	if (CONFIG_FLAG_IS_SET(MSG_FLAG_FORCE)) {
		ans = ans_forced;
	} else {
		ans = ans_default;
	}

#if DO_MSG_SAFETY_CHECKS
	if (fmt == NULL) {
		return ans;
	}
#endif

	if (!CONFIG_FLAG_IS_SET(MSG_FLAG_INTERACT) && !CONFIG_FLAG_IS_SET(MSG_FLAG_ALWAYS_PRINT_QUESTIONS)) {
		return ans;
	}

	va_start(args, fmt);
	stdout_vprintf(fmt, args);
	va_end(args);

	while (true) {
		if (ans) {
			msg_puts(" (YES/no): ");
			def_str = "[YES]" MSG_NEWLINE_STRING;
		} else {
			msg_puts(" (yes/NO): ");
			def_str = "[NO]" MSG_NEWLINE_STRING;
		}
		if (!CONFIG_FLAG_IS_SET(MSG_FLAG_INTERACT)) {
			msg_puts(def_str);
			return ans;
		}

		msg_gets(answer, 8);
		switch (answer[0]) {
			case 'y':
			case 'Y':
				return true;
				break;
			case 'n':
			case 'N':
				return false;
				break;
			case 0:
			case '\n':
				//msg_puts(def_str);
				return ans;
				break;
		}
	}

	return ans;
}

void msg_log(const char *restrict fmt, ...) {
	ulib_assert(fmt != NULL);
#if DO_MSG_SAFETY_CHECKS
	if (fmt == NULL) {
		return;
	}
#endif

	if (WRITE_ALLOWED(log)) {
		va_list args;

		if (CONFIG_FLAG_IS_SET(MSG_FLAG_LOG_PRINTTIME) && config.print_log_time != NULL) {
			char time_buf[MSG_STR_BYTES];
			log_printf("[%s] ", config.print_log_time(time_buf, SIZEOF_ARRAY(time_buf)));
		}

		va_start(args, fmt);
		log_vprintf(fmt, args);
		va_end(args);

		log_write(newline, newline_len);
	}

	return;
}

void msg_error(const char *restrict fmt, ...) {
	va_list args;

	ulib_assert(fmt != NULL);
#if DO_MSG_SAFETY_CHECKS
	if (fmt == NULL) {
		return;
	}
#endif

	if (config.program_name[0] != 0) {
		stderr_printf("%s: ", config.program_name);
	}
	if (config.error_prefix[0] != 0) {
		stderr_printf("%s", config.error_prefix);
	}

	va_start(args, fmt);
	stderr_vprintf(fmt, args);
	va_end(args);

	stderr_write(newline, newline_len);

	return;
}

static void _msg_errno(int errnum, const char *restrict fmt, va_list args) {
	ulib_assert(fmt != NULL);
#if DO_MSG_SAFETY_CHECKS
	if (fmt == NULL) {
		return;
	}
#endif
	if (errnum < 0) {
		errnum = -errnum;
	}

	if (config.program_name[0] != 0) {
		stderr_printf("%s: ", config.program_name);
	}
	if (config.error_prefix[0] != 0) {
		stderr_printf("%s", config.error_prefix);
	}

	stderr_vprintf(fmt, args);
	stderr_printf(": %s." MSG_NEWLINE_STRING, strerror(errnum));

	return;
}
void msg_errno(int errnum, const char *restrict fmt, ...) {
	va_list args;

	va_start(args, fmt);
	_msg_errno(errnum, fmt, args);
	va_end(args);

	return;
}
void msg_liberrno(int errnum, const char *restrict fmt, ...) {
	if (CONFIG_FLAG_IS_SET(MSG_FLAG_LIBERRORS)) {
		va_list args;

		va_start(args, fmt);
		_msg_errno(errnum, fmt, args);
		va_end(args);
	}

	return;
}

void msg_warnno(int errnum, const char *restrict fmt, ...) {
	va_list args;

	ulib_assert(fmt != NULL);
#if DO_MSG_SAFETY_CHECKS
	if (fmt == NULL) {
		return;
	}
#endif
	if (errnum < 0) {
		errnum = -errnum;
	}

	if (config.program_name[0] != 0) {
		stderr_printf("%s: ", config.program_name);
	}
	if (config.warn_prefix[0] != 0) {
		stderr_printf("%s", config.warn_prefix);
	}

	va_start(args, fmt);
	stderr_vprintf(fmt, args);
	va_end(args);
	stderr_printf(": %s.%s", strerror(errnum), newline);

	return;
}
void msg_warn(const char *restrict fmt, ...) {
	va_list args;

	ulib_assert(fmt != NULL);
#if DO_MSG_SAFETY_CHECKS
	if (fmt == NULL) {
		return;
	}
#endif

	if (config.program_name[0] != 0) {
		stderr_printf("%s: ", config.program_name);
	}
	if (config.warn_prefix[0] != 0) {
		stderr_printf("%s", config.warn_prefix);
	}

	va_start(args, fmt);
	stderr_vprintf(fmt, args);
	va_end(args);

	stderr_write(newline, newline_len);

	return;
}

void msg_print(int8_t priority, const char *restrict fmt, ...) {
	ulib_assert(fmt != NULL);
#if DO_MSG_SAFETY_CHECKS
	if (fmt == NULL) {
		return;
	}
#endif

	if (priority <= config.verbosity) {
		va_list args;

		if (config.program_name[0] != 0) {
			stdout_printf("%s: ", config.program_name);
		}

		va_start(args, fmt);
		stdout_vprintf(fmt, args);
		va_end(args);

		stdout_write(newline, newline_len);
	}

	return;
}
void msg_debug(const char *restrict fmt, ...) {
#if DEBUG
	va_list args;

	ulib_assert(fmt != NULL);
# if DO_MSG_SAFETY_CHECKS
	if (fmt == NULL) {
		return;
	}
# endif

	if (config.program_name[0] != 0) {
		stderr_printf("%s: ", config.program_name);
	}
	if (config.debug_prefix[0] != 0) {
		stderr_printf("%s", config.debug_prefix);
	}

	va_start(args, fmt);
	stderr_vprintf(fmt, args);
	va_end(args);

	stderr_write(newline, newline_len);

#else // !DEBUG
	UNUSED(fmt);
#endif

	return;
}


#else
	// ISO C forbids empty translation units, this makes it happy.
	typedef int make_iso_compilers_happy;
#endif // ULIB_ENABLE_MSG
