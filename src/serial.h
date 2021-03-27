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
// serial.h
// Manage serial communication
// NOTES:
//    Most of the public functions defined in serial.c are prototyped in
//    platform/interface.h
//

#ifdef __cplusplus
 extern "C" {
#endif
#ifndef _SERIAL_H
#define _SERIAL_H

/*
* Includes
*/
#include "config.h"
#include "common.h"

#include "ulib/cstrings.h"

#if USE_SERIAL

/*
* Static values
*/
// Size of the output buffer
// Set to 0 to disable.
#if USE_SMALL_CODE < 1
# define SERIAL_BUFFER_SIZE 64
#else
# define SERIAL_BUFFER_SIZE 0
#endif


/*
* Types
*/


/*
* Variable declarations (defined in serial.c)
*/
// True if serial output has be initialized
extern bool G_serial_is_up;


/*
* Function prototypes (defined in serial.c)
*/
// Initialize the serial communication subsystem
void serial_init(void);

// Print information about the running system to the serial line
void print_system_info(void);


/*
* Macros
*/


#endif // USE_SERIAL

#endif // _SERIAL_H
#ifdef __cplusplus
 }
#endif
