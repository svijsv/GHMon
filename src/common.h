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
// common.h
// Header common to all components in the frontend
// NOTES:
//
#ifndef _COMMON_H
#define _COMMON_H

// The main configuration is pulled in (indirectly) by interface.h.
#include "uHAL/include/interface.h"

#include "ulib/include/debug.h"
#include "ulib/include/types.h"
#include "ulib/include/bits.h"
#include "ulib/include/util.h"
#include "ulib/include/fmem.h"


//
// Warning flags
typedef enum {
	WARN_BATTERY_LOW = 0x01U,
	WARN_VCC_LOW     = 0x02U,
	WARN_SENSOR      = 0x04U,
	WARN_CONTROLLER  = 0x08U,
	WARN_LOG_SKIPPED = 0x10U,
	WARN_LOG_ERROR   = 0x20U,
} ghmon_warning_flags_t;
//
// Character representations of the above flags
// If these change, the log header needs to be changed to match.
#define GHMON_WARNING_FLAGS "BVSClL"
//
// Used to track warnings in effect
extern uint_fast8_t ghmon_warnings;

//
// Control the status LED
void led_on(void);
void led_off(void);
void led_toggle(void);
void led_flash(uint8_t count, uint16_t ms);
void issue_warning(void);


#endif // _COMMON_H
