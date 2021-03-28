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
// log.h
// Manage log files
// NOTES:
//

#ifdef __cplusplus
 extern "C" {
#endif
#ifndef _LOG_H
#define _LOG_H

/*
* Includes
*/
#include "config.h"
#include "common.h"

// Need to check this *after* the headers are included
#if USE_LOGGING


/*
* Static values
*/
// The SD card will need a period between when it's gone inactive and when the
// power is removed to minimize chances of corrupted data; see
// https:// github.com/greiman/SdFat/issues/21
// by way of
// https:// thecavepearlproject.org/2017/05/21/switching-off-sd-cards-for-low-power-data-logging/
// Delay is in milliseconds.
#define SD_POWEROFF_DELAY_MS 1000

// Use a print buffer this size for SD card writes
// This is on top of any buffering provided by the FatFS library.
// Set to 0 to disable.
#if USE_SMALL_CODE < 1
  // This is a very crude method of determining the needed buffer size based
  // on expected typical setups - 17 characters for uptime, 2 for MCU temp,
  // 4 for MCU Vcc, 5 each per sensor, 8 each per controller, the tabs
  // between them, the newline, 2 so we don't undercount, and an assumption
  // that the rest of the program only needs 2KB
# define PRINT_BUF_PER_LINE (17 + 2 + 4 + (5*SENSOR_COUNT) + (8*CONTROLLER_COUNT) + (3 + CONTROLLER_COUNT + SENSOR_COUNT) + 1 + 2)
# if (PRINT_BUF_PER_LINE * (LOGFILE_BUFFER_COUNT+1)) < (RAM_PRESENT - (2048 + (16 * LOGFILE_BUFFER_COUNT)))
#  define PRINT_BUFFER_SIZE (PRINT_BUF_PER_LINE * (LOGFILE_BUFFER_COUNT+1))
# else
#  define PRINT_BUFFER_SIZE (RAM_PRESENT - (2048 + (16 * LOGFILE_BUFFER_COUNT)))
# endif
#else
# define PRINT_BUFFER_SIZE 0
#endif


/*
* Types
*/


/*
* Variable declarations (defined in log.c)
*/


/*
* Function prototypes (defined in log.c)
*/
// Initialize the logging subsystem
void log_init(void);
// If force_write is true, sync anything buffered to disk without adding
// a new reading.
void log_status(bool force_write);

/*
* Macros
*/


#else // !USE_LOGGING
#define log_status(args) ((void )0U)
#define log_init()       ((void )0U)

#endif // USE_LOGGING


#endif // _LOG_H
#ifdef __cplusplus
 }
#endif
