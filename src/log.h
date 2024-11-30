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
// log.h
// Manage log files
// NOTES:
//

#ifndef _LOG_H
#define _LOG_H

#include "common.h"

#if USE_LOGGING

//
// Initialize the logging subsystem
void log_init(void);
//
// Log the current status, writing the buffer to storage if full
void log_status(void);
//
// Write the log buffer to storage
void write_log_to_storage(void);
//
// Print the log header using pf()
void print_log_header(void (*pf)(const char *format, ...));
//
// Print the buffered log using pf()
// This includes previously-written entries
void print_log(void (*pf)(const char *format, ...));

#else // !USE_LOGGING
# define log_init()   ((void )0U)
# define log_status() ((void )0U)
# define write_log_to_storage() ((void )0U)
# define print_log_header(...) ((void )0U)
# define print_log(...) ((void )0U)
#endif // USE_LOGGING

#endif // _LOG_H
