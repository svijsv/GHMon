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
// The FatFS library has an internal 512-byte buffer used to cache sector
// writes, the only time our buffer matters is if the total size of a write
// is multiple sectors (which it may well be, especially with larger log line
// buffers)
#if USE_LOGGING && ! SD_PRINT_BUFFER_SIZE
# undef SD_PRINT_BUFFER_SIZE
# if USE_SMALL_CODE < 1
// This is a crude method of determining the needed buffer size based on
// maximum possible usage - 17 characters for uptime, 9 for warnings, 4 for
// MCU Vcc, 5 per sensor, 5+3 per controller, the tabs between fields, any
// '!'s, and the newline
// This doesn't take the size of the header into account, so at least the
// first write will be split in two
#  define SD_PRINT_BUF_PER_LINE (17 + 9 + 4 + (5*SENSOR_COUNT) + (8*CONTROLLER_COUNT) + (2 + (CONTROLLER_COUNT*2) + SENSOR_COUNT) + (CONTROLLER_COUNT + SENSOR_COUNT) + 1)
#  define SD_PRINT_BUF_TOTAL (SD_PRINT_BUF_PER_LINE * (LOGFILE_BUFFER_COUNT+1))
// This is an even cruder method of determining how much space we need to
// reserve for everything else - other IO, the log file buffer, the FatFS
// internal buffers, the sensor and controller arrays, and general usage
// TERMINAL_BUFFER_SIZE is allocated on the stack when entering the terminal
// but still needs to be taken into account because writing to the SD card
// can happen from within the terminal
#  if USE_CONTROLLERS
// This includes both G_controllers[] and the memory used by the controller
// fields in log_buffer_t
#   define SD_PRINT_BUF_RESERVE_CTRL ((CONTROLLER_COUNT * 16) + (4 * LOGFILE_BUFFER_COUNT))
#  else
#   define SD_PRINT_BUF_RESERVE_CTRL 0
#  endif
// This includes both G_sensors[] and the memory used by the sensor fields
// in log_buffer_t
#  define SD_PRINT_BUF_RESERVE_SENS   ((SENSOR_COUNT * 12) + (4 * LOGFILE_BUFFER_COUNT))
#  define SD_PRINT_BUF_RESERVE_IO     (SERIAL_BUFFER_SIZE + TERMINAL_BUFFER_SIZE)
#  define SD_PRINT_BUF_RESERVE_LOG    (8 * LOGFILE_BUFFER_COUNT)
#  define SD_PRINT_BUF_RESERVE_FATFS   1024
// ATMegas have separate name spaces for flash and RAM and the work-around
// requires a little RAM itself
#  if _HAVE_FLASH_NAMESPACE
#   define SD_PRINT_BUF_RESERVE_GENERAL (256 + (_FLASH_TMP_SIZE * 3))
#  else
#   define SD_PRINT_BUF_RESERVE_GENERAL  256
#  endif
#  define SD_PRINT_BUF_RESERVE (SD_PRINT_BUF_RESERVE_CTRL + SD_PRINT_BUF_RESERVE_SENS + SD_PRINT_BUF_RESERVE_IO + SD_PRINT_BUF_RESERVE_LOG + SD_PRINT_BUF_RESERVE_FATFS + SD_PRINT_BUF_RESERVE_GENERAL)
#  if SD_PRINT_BUF_TOTAL < (RAM_PRESENT - SD_PRINT_BUF_RESERVE)
#   define SD_PRINT_BUFFER_SIZE SD_PRINT_BUF_TOTAL
// No point in a local buffer if it's got to be smaller than the library's
#  elif (RAM_PRESENT - SD_PRINT_BUF_RESERVE) > 512
#   define SD_PRINT_BUFFER_SIZE (RAM_PRESENT - SD_PRINT_BUF_RESERVE)
#  else
// Use something non-zero just to prevent constant function calls
#   define SD_PRINT_BUFFER_SIZE 32
#  endif

# else // ! USE_SMALL_CODE < 1
#  define SD_PRINT_BUFFER_SIZE 0
# endif
#endif // USE_LOGGING


#endif // _CONFIG_UNIFY_H
#ifdef __cplusplus
 }
#endif
