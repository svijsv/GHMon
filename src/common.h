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
// common.h
// Header common to all components in the frontend
// NOTES:
//

#ifdef __cplusplus
 extern "C" {
#endif
#ifndef _COMMON_H
#define _COMMON_H

/*
* Includes
*/
#include "ulib/assert.h"
#include "ulib/types.h"
#include "ulib/bits.h"
#include "ulib/util.h"
#include "ulib/fmem.h"

#include "platform/interface.h"
#include "platform.h"
#include "config.h"


/*
* Static values
*/
//
// Warning flags
// If the order changes or an addition is made, modify format_warnings() and
// print_header() in log.c to match.
#define WARN_BATTERY_LOW 0x01
#define WARN_VCC_LOW     0x02
#define WARN_SENSOR      0x04
#define WARN_CONTROLLER  0x08
#define WARN_CONTROLLER_SKIPPED 0x10
#define WARN_SD_SKIPPED  0x20
#define WARN_SD_FAILED   0x40


/*
* Types
*/


/*
* Variable declarations (defined in main.c)
*/
// Warnings in effect
extern volatile uint8_t G_warnings;


/*
* Function prototypes (defined in main.c)
*/


/*
* Macros
*/


#endif // _COMMON_H
#ifdef __cplusplus
 }
#endif
