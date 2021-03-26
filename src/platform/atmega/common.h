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
// Platform-specific common header
// NOTES:
//

#ifdef __cplusplus
 extern "C" {
#endif
#ifndef _PLATFORM_ATMEGA_COMMON_H
#define _PLATFORM_ATMEGA_COMMON_H

/*
* Includes
*/
#include "platform.h"
#include "platform/interface.h"

#include "ulib/assert.h"
#include "ulib/types.h"
#include "ulib/bits.h"
#include "ulib/time.h"
#include "ulib/util.h"


/*
* Static values
*/


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
// An LED flash for use when debugging
#define DFLASH(t) do { gpio_toggle_state(LED_PIN); dumb_delay(t); gpio_toggle_state(LED_PIN); dumb_delay(t/2); } while (0);


#endif // _PLATFORM_ATMEGA_COMMON_H
#ifdef __cplusplus
 }
#endif
