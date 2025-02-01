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
// defaults.c
// Default overrideable functions
// NOTES:
//

#include "common.h"

#include "ulib/include/util.h"


uHAL_flags_t uHAL_status = 0;


#if uHAL_USE_UART_COMM
uart_port_t uHAL_uart_comm_port;
#endif

//
// Hooks
//
// Hibernation
__attribute__((weak))
void pre_hibernate_hook(utime_t *s, sleep_mode_t *sleep_mode, uHAL_flags_t flags) {
	UNUSED(s);
	UNUSED(sleep_mode);
	UNUSED(flags);
	return;
}
void pre_hibernate_hook_caller(utime_t *s, sleep_mode_t *sleep_mode, uHAL_flags_t flags) {
# if uHAL_USE_UART_COMM && uHAL_ANNOUNCE_HIBERNATE
	const char *type, *mode;

	type = BIT_IS_SET(flags, uHAL_CFG_ALLOW_INTERRUPTS) ? "Interruptable" : "Uninteruptable";

	switch (*sleep_mode) {
	case HIBERNATE_LIGHT:
		mode = "light";
		break;
	case HIBERNATE_MAX:
		mode = "max";
		break;
	case HIBERNATE_DEEP:
		mode = "deep";
		break;
	default:
		mode = "unknown";
		break;
	}
	if (s != NULL) {
		const uint ss = *s;
		LOGGER("%s %s sleep mode for %us", type, mode, ss);
	} else {
		LOGGER("%s %s sleep mode for unknown time", type, mode);
	}
# endif
	return pre_hibernate_hook(s, sleep_mode, flags);
}
__attribute__((weak))
void post_hibernate_hook(utime_t s, sleep_mode_t sleep_mode, uHAL_flags_t flags) {
	UNUSED(s);
	UNUSED(sleep_mode);
	UNUSED(flags);
	return;
}
void post_hibernate_hook_caller(utime_t s, sleep_mode_t sleep_mode, uHAL_flags_t flags) {
	return post_hibernate_hook(s, sleep_mode, flags);
}
//
// System reset
__attribute__((weak))
void pre_reset_hook(void) {
	return;
}
//
// UART interrupt
__attribute__((weak))
void uart_rx_irq_hook(uart_port_t *p) {
	UNUSED(p);

	return;
}
//
// Error state periodic hook
__attribute__((weak))
void error_state_hook(void) {
	return;
}

//
// Error state handling
//
// Crude fallback handler (no serial)
__attribute__((weak))
void error_state_crude(void) {
	while (1) {
		error_state_hook();
		dumb_delay_ms(300);
	}
	return;
}
//
// Normal error state handler
#if uHAL_USE_UART_COMM
__attribute__((weak))
void error_state(const char *file_path, uint32_t lineno, const char *func_name, const char *msg) {
	const char *basename;
	int_fast16_t msg_timeout;

	msg_timeout = -1;
	//basename = cstring_basename(file_path, '/');
	basename = file_path;
	while (1) {
		if ((msg_timeout <= 0) && uHAL_CHECK_STATUS(uHAL_FLAG_SERIAL_IS_UP)) {
			if (msg_timeout == -1) {
				// Another message at 30s should give enough time to get the serial console up if it's not already...
				msg_timeout = 30 * 2;
			} else {
				// ...and after that try every five minutes to limit spam
				// 5 * 60 seconds * 2 loops per second
				msg_timeout = (5 * 60) * 2;
			}
			// F() and F1() were likely both used for arguments and using any of
			// the other F() macros would cause another buffer to be used, so
			// using a plain string for the format saves RAM in this particular
			// case
			PRINTF_NOF("ERR %s:%u in %s(): %s\r\n", basename, (uint )lineno, func_name, msg);
		}

		--msg_timeout;
		error_state_hook();
		dumb_delay_ms(500);
	}
	return;
}
#else // !uHAL_USE_UART_COMM
__attribute__((weak))
void error_state(const char *file_path, uint32_t lineno, const char *func_name, const char *msg) {
	UNUSED(file_path);
	UNUSED(lineno);
	UNUSED(func_name);
	UNUSED(msg);
	error_state_crude();
	return;
}
#endif // !uHAL_USE_UART_COMM
//
// Failed assertion handler
__attribute__((weak))
void ulib_assert_failed(const char *file_path, uint32_t lineno, const char *func_name, const char *expr) {
	error_state(file_path, lineno, func_name, expr);

	return;
}
//
// Panic handler
__attribute__((weak))
void ulib_panic_abort(const char *file_path, uint32_t lineno, const char *func_name, const char *expr) {
	error_state(file_path, lineno, func_name, expr);

	return;
}
