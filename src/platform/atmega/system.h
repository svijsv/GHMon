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
// system.h
// General platform initialization
// NOTES:
//   Prototypes for some of the related functions are in interface.h
//

#ifdef __cplusplus
 extern "C" {
#endif
#ifndef _PLATFORM_ATMEGA_SYSTEM_H
#define _PLATFORM_ATMEGA_SYSTEM_H

/*
* Includes
*/
#include "common.h"


/*
* Static values
*/
// Enter light sleep for this many seconds before entering deep sleep to give
// time to enter the command terminal because UART interrupts are disabled
// during stop mode
#define LIGHT_SLEEP_PERIOD (5) // 5 seconds
// If a deep sleep would last fewer than this many seconds, light sleep a bit
// longer instead
#define MIN_DEEP_SLEEP_PERIOD 2


/*
* Types
*/


/*
* Variable declarations
*/


/*
* Function prototypes
*/


/*
* Macros
*/
// Enable/disable interrupts while preserving original state
#define ENABLE_INTERRUPTS(sreg)  do { sreg = SREG; sei(); } while (0);
#define DISABLE_INTERRUPTS(sreg) do { sreg = SREG; cli(); } while (0);
#define RESTORE_INTERRUPTS(sreg) do { SREG = sreg; } while (0);


#endif // _PLATFORM_ATMEGA_SYSTEM_H
#ifdef __cplusplus
 }
#endif
