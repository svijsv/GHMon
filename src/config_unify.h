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
// config_unify.h
// Determine which subsystems to enable or disable based on user configuration
// NOTES:
//

#ifdef __cplusplus
 extern "C" {
#endif
#ifndef _CONFIG_UNIFY_H
#define _CONFIG_UNIFY_H


#if USE_ADC_SENSORS || CALIBRATE_VREF
# define USE_ADC 1
#endif

#if USE_FDISK && !USE_TERMINAL
# warning "Setting USE_TERMINAL because USE_FDISK was set"
# undef USE_TERMINAL
# define USE_TERMINAL 1
#endif

#if USE_TERMINAL && !USE_SERIAL
# warning "Setting USE_SERIAL because USE_TERMINAL was set"
# undef USE_SERIAL
# define USE_SERIAL 1
#endif

#if USE_LOGGING || USE_FDISK
# define USE_SD  1
#endif

#if USE_SD || USE_SPI_SENSORS
# define USE_SPI 1
#endif

#if USE_I2C_SENSORS
# define USE_I2C 1
#endif

#if USE_CONTROLLERS && ! CONTROLLER_COUNT
# warning "Disabling USE_CONTROLLERS because CONTROLLER_COUNT is 0"
# undef USE_CONTROLLERS
# define USE_CONTROLLERS 0
#endif

#ifndef USE_SMALL_SENSORS
# define USE_SMALL_SENSORS USE_SMALL_BUILD
#endif
#ifndef USE_SMALL_CONTROLLERS
# define USE_SMALL_CONTROLLERS USE_SMALL_BUILD
#endif
#ifndef USE_SMALL_CODE
# define USE_SMALL_CODE USE_SMALL_BUILD
#endif

//
// Calculate IO buffer sizes
//
// Serial output
#if USE_SERIAL
# if USE_SMALL_CODE < 1
#  define SERIAL_BUFFER_SIZE 64
# else
#  define SERIAL_BUFFER_SIZE 0
# endif
#else
# define SERIAL_BUFFER_SIZE 0
#endif
//
// Serial input
#if USE_TERMINAL
# if USE_SMALL_CODE < 1
#  define TERMINAL_BUFFER_SIZE 64
# else
// This is just big enough to set the time
#  define TERMINAL_BUFFER_SIZE 32
# endif
#else
# define TERMINAL_BUFFER_SIZE 0
#endif
//
// SD output
#if USE_LOGGING
// Certain platforms (like AVR) will require additional memory for strings,
// so for those just forgo the buffer entirely; there's no good way to predict
// what's needed.
# if USE_AVR
#  define SD_PRINT_BUFFER_SIZE 0

# elif USE_SMALL_CODE < 1
// This is a very crude method of determining the needed buffer size based
// on expected typical setups - 17 characters for uptime, 4 for MCU Vcc, 5
// each per sensor, 10 each per controller, the tabs between them, and the
// newline
#  define SD_PRINT_BUF_PER_LINE (17 + 4 + (5*SENSOR_COUNT) + (10*CONTROLLER_COUNT) + (2 + CONTROLLER_COUNT + SENSOR_COUNT) + 1)
// This is an even cruder method of determining how much space we need to
// reserve for everything else - other IO, the log file buffer, the FatFS
// internal buffers, the sensor and controller arrays, and general usage
// TERMINAL_BUFFER_SIZE is allocated on the stack when entering the terminal
// but still needs to be taken into account because writing to the SD card
// can happen from within the terminal
#  if USE_CONTROLLERS
#   define SD_PRINT_BUF_RESERVE_CTRL (CONTROLLER_COUNT * 16)
#  else
#   define SD_PRINT_BUF_RESERVE_CTRL 0
#  endif
#  define SD_PRINT_BUF_RESERVE (SERIAL_BUFFER_SIZE + TERMINAL_BUFFER_SIZE + (16 * LOGFILE_BUFFER_COUNT) + 1024 + (SENSOR_COUNT * 20) + SD_PRINT_BUF_RESERVE_CTRL + 256)
#  if (SD_PRINT_BUF_PER_LINE * (LOGFILE_BUFFER_COUNT+1)) < (RAM_PRESENT - SD_PRINT_BUF_RESERVE)
#   define SD_PRINT_BUFFER_SIZE (SD_PRINT_BUF_PER_LINE * (LOGFILE_BUFFER_COUNT+1))
#  else
// If this is <0 log.c will treat it as 0
#   define SD_PRINT_BUFFER_SIZE (RAM_PRESENT - SD_PRINT_BUF_RESERVE)
#  endif
# else // ! USE_SMALL_CODE < 1
#  define SD_PRINT_BUFFER_SIZE 0
# endif
#endif // USE_LOGGING

#endif // _CONFIG_UNIFY_H
#ifdef __cplusplus
 }
#endif
