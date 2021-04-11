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
// terminal.h
// An interactive serial terminal
// NOTES:
//

#ifdef __cplusplus
 extern "C" {
#endif
#ifndef _TERMINAL_H
#define _TERMINAL_H

/*
* Includes
*/
#include "config.h"
#include "common.h"

#if USE_TERMINAL

/*
* Static values
*/
// Timeout in seconds for the command terminal (that is, when it gives up
// listening)
#define TERMINAL_TIMEOUT_S 3600 // 1Hr

// Size of the input buffer
#if USE_SMALL_CODE < 1
# define TERMINAL_BUFFER_SIZE 64
#else
# define TERMINAL_BUFFER_SIZE 32 // This is just big enough to set the time
#endif

/*
* Types
*/


/*
* Variable declarations (defined in terminal.c)
*/


/*
* Function prototypes (defined in terminal.c)
*/
// Enter the interactive terminal
void terminal(void);


/*
* Macros
*/


#else // !USE_TERMINAL
# define terminal() ((void )0U)
#endif // USE_TERMINAL

#endif // _TERMINAL_H
#ifdef __cplusplus
 }
#endif
