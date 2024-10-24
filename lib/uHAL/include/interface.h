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
/// @brief Interface between the frontend and backend code.
/// @attention
///    Backend-specific interface components and overrides are defined in
///    platform.h in the individual backend directories
///
/// @attention
///    Some platforms may not implement the full interface because I haven't
///    gotten around to it yet.
///
#ifndef _uHAL_PLATFORM_INTERFACE_H
#define _uHAL_PLATFORM_INTERFACE_H

/*
// Can't do this because the AVR headers only check if __DOXYGEN__ is defined
// and break the build when it's 0.
#ifndef __DOXYGEN__
# define __DOXYGEN__ 0
#endif
*/
#ifndef __HAVE_DOXYGEN__
# define __HAVE_DOXYGEN__ 0
#endif

//
// Include the configuration files before anything else so that they can override
// any other library settings if needed
#if ! __HAVE_DOXYGEN__
# ifndef uHAL_CONFIG
#  define uHAL_CONFIG config_uHAL.h
# endif
# define uHAL_STRINGIZE(_x_) #_x_
# define uHAL_XTRINGIZE(_x_) uHAL_STRINGIZE(_x_)
# define uHAL_INCLUDE_PLATFORM_HEADER(_dir_) uHAL_STRINGIZE(../src/platform/_dir_/platform.h)

# include uHAL_XTRINGIZE(uHAL_CONFIG)
# include uHAL_XTRINGIZE(uHAL_PLATFORM_CONFIG)
# include "uHAL_config_fixer.h"

# include uHAL_INCLUDE_PLATFORM_HEADER(uHAL_PLATFORM)
# undef uHAL_STRINGIZE
# undef uHAL_XTRINGIZE
# undef uHAL_INCLUDE_PLATFORM_HEADER
#endif

#include "ulib/include/bits.h"
//#include "ulib/include/debug.h"
#include "ulib/include/types.h"
#include "ulib/include/util.h"

///
/// Error status return values.
typedef enum {
	ERR_OK = 0, ///< No error

	ERR_BADARG,   ///< Function passed a bad argument.
	ERR_INIT,     ///< Structure or peripheral not initialized.
	ERR_IO,       ///< Input/output error.
	ERR_NODEV,    ///< No such device.
	ERR_NOMEM,    ///< Not enough memory.
	ERR_NOTSUP,   ///< Operation not supported.
	ERR_IMPOSSIBLE, ///< Operation not possible.
	ERR_RETRY,    ///< Device or resource temporarily unavailable.
	ERR_PERM,     ///< Operation not permitted.
	ERR_TIMEOUT,  ///< Operation timed out.
	ERR_INTERRUPT, ///< Operation interrupted.

	ERR_UNKNOWN = 127 ///< Unknown error.
} err_t;

///
/// The type used to track byte counts during data transmissions.
typedef uint_fast16_t txsize_t;


#include "interface/gpio.h"
#include "interface/serial.h"
#include "interface/system.h"
#include "interface/time.h"

#if uHAL_USE_ADC || __HAVE_DOXYGEN__
# include "interface/adc.h"
#endif
#if uHAL_USE_I2C || __HAVE_DOXYGEN__
# include "interface/i2c.h"
#endif
#if uHAL_USE_PWM || __HAVE_DOXYGEN__
# include "interface/pwm.h"
#endif
#if uHAL_USE_SPI || __HAVE_DOXYGEN__
# include "interface/spi.h"
#endif
#if uHAL_USE_UART || __HAVE_DOXYGEN__
# include "interface/uart.h"
#endif

#include PLATFORM_INTERFACE_H

#endif // _uHAL_PLATFORM_INTERFACE_H
