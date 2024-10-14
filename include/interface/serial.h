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
/// @file
/// @brief User Serial Communication Interface
/// @attention
///    This file should only be included by interface.h.
///

#if uHAL_USE_UART_COMM || __HAVE_DOXYGEN__
///
/// Initialize serial line output.
///
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t serial_init(void);

///
/// Print information about the running system to the serial line.
void print_system_info(void);

///
/// Print platform-specific information about the running system to the serial line.
void print_platform_info(void);

///
/// Print a formatted debug message
///
/// @param fmt A @c printf()-style format string.
/// @param ... The values for the format string.
void logger(const char *fmt, ...)
	__attribute__ ((format(printf, 1, 2)));

///
/// Replay past logger() output.
///
/// Requires that @c LOGGER_HISTORY_BUFFER_BYTES be > 0
void logger_replay(void);

///
/// Print a formatted string.
///
/// @param fmt A @c printf()-style format string.
/// @param ... The values for the format string.
void serial_printf(const char *fmt, ...)
	__attribute__ ((format(printf, 1, 2)));

///
/// Print an un-formatted string.
///
/// @param msg The string to print.
/// @param len The length of the string. Calculated if 0.
void serial_print(const char *msg, txsize_t len);

# if uHAL_USE_TERMINAL || __HAVE_DOXYGEN__
///
/// The structure used to store information about a terminal command.
typedef struct {
	/// The handler to pass the command line to.
	int (*cmd_handler)(const char *line_in);
	/// The command string.
	char cmd[TERMINAL_CMD_NAME_LEN];
	/// The number of characters to match.
	uint8_t length;
} terminal_cmd_t;

///
/// An (optional) array describing extra terminal commands.
///
/// @attention
/// The last element must be { NULL, 0, 0 }.
extern FMEM_STORAGE const terminal_cmd_t terminal_extra_cmds[];

///
/// A string serving as the help message for the extra commands.
extern FMEM_STORAGE const char terminal_extra_help[];

///
/// Enter the interactive terminal.
void terminal(void);
# endif //uHAL_USE_TERMINAL

#else // uHAL_USE_UART_COMM
//# define serial_init()       (ERR_NOTSUP)
# define serial_init()       ((void )0)
# define print_system_info() ((void )0)
# define logger(...)         ((void )0)
# define serial_printf(...)  ((void )0)
# define serial_print(...)   ((void )0)
#endif

///
/// @name Convenience Macros.
///
/// Macros for common arguments to the print functions.
///
/// Variants of macros with *_NOF() don't use the F()/F1() macros to store
/// strings in program memory on devices with separate memory namespaces.
///
/// @{
//
#if DEBUG || __HAVE_DOXYGEN__
# ifndef LOGGER
///
/// Convenience macro for @c logger().
///
/// @attention
/// This macro uses the @c F1() macro on systems with separate namespaces.
///
/// @param fmt A @c printf()-style format string.
/// @param ... The values for the format string.
#  define LOGGER(fmt, ...)      logger(F1(fmt), ## __VA_ARGS__)
# endif
# ifndef LOGGER_NOF
///
/// Convenience macro for @c logger().
///
/// @param fmt A @c printf()-style format string.
/// @param ... The values for the format string.
#  define LOGGER_NOF(fmt, ...)  logger(fmt, ## __VA_ARGS__)
# endif

#else // DEBUG
# ifndef LOGGER
#  define LOGGER(...)      ((void )0U)
# endif
# ifndef LOGGER_NOF
#  define LOGGER_NOF(...)  ((void )0U)
# endif
#endif

///
/// Convenience macro for @c serial_printf().
///
/// @attention
/// This macro uses the @c F1() macro on systems with separate namespaces.
///
/// @param fmt A @c printf()-style format string.
/// @param ... The values for the format string.
#define PRINTF(fmt, ...)     serial_printf(F1(fmt), ## __VA_ARGS__)
///
/// Convenience macro for @c serial_printf().
///
/// @param fmt A @c printf()-style format string.
/// @param ... The values for the format string.
#define PRINTF_NOF(fmt, ...) serial_printf(fmt, ## __VA_ARGS__)

///
/// Convenience macro for @c serial_print().
///
/// @attention
/// This macro uses the @c F1() macro on systems with separate namespaces.
///
/// @param msg The string to print.
/// @param len The length of the string. Calculated if 0.
#define PUTS(msg, len)       serial_print(F1(msg), len)
///
/// Convenience macro for @c serial_print().
///
/// @param msg The string to print.
/// @param len The length of the string. Calculated if 0.
#define PUTS_NOF(msg, len)   serial_print(msg, len)
/// @}
