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
// actuators.h
// Manage actuators
// NOTES:
//
#ifndef _ACTUATORS_H
#define _ACTUATORS_H

#include "common.h"

//
// Status flags for actuator_status_t structs
typedef enum {
	ACTUATOR_STATUS_FLAG_INITIALIZED = 0x01U, // Actuator successfully initialized
	ACTUATOR_STATUS_FLAG_ERROR       = 0x02U, // Actuator in error state; set when set() returns an error code
} actuator_status_flag_t;
//
// Status of an actuator
typedef struct {
#if USE_ACTUATOR_DATA
	//
	// Data for actuator-internal use
	void *data;
#endif
#if USE_ACTUATOR_STATUS_CHANGE_TIME
	//
	// The number time of the last status change
	utime_t status_change_time;
#endif
#if USE_ACTUATOR_ON_TIME_COUNT
	//
	// The total time the actuator has been engaged as determined by actuator_cfg_t.is_on()
	// If it's currently on, this excludes the current on-time
	utime_t on_time_minutes;
#endif
#if USE_ACTUATOR_STATUS_CHANGE_COUNT
	//
	// The number of times the status has changed
	uint_t status_change_count;
#endif
	ACTUATOR_STATUS_T status;
	//
	// Status flags
	uint8_t status_flags;
} actuator_status_t;

//
// Configuration flags for actuator_cfg_t structs
typedef enum {
	ACTUATOR_CFG_FLAG_LOG   = 0x40U, // Log this actuator
	ACTUATOR_CFG_FLAG_NOLOG = 0x80U, // Don't log this actuator
} actuator_cfg_flag_t;
//
// Static configuration of an actuator
typedef struct actuator_cfg_t {
#if USE_ACTUATOR_CFG_DATA
	//
	// An optional data field to be used as needed by the actuator definitions
	ACTUATOR_CFG_DATA_T data;
#endif
#if USE_ACTUATOR_INIT
	//
	// An optional function used to initialize the actuator
	// If this returns anything other than ERR_OK, the actuator is marked as
	// uninitialied and considered to be in a state of error. Initialization may
	// be re-attempted at a later time, in which case the status struct is not
	// zeroed out.
	// Ignored if NULL.
	err_t (*init)(ACTUATOR_CFG_STORAGE struct actuator_cfg_t *cfg, actuator_status_t *status);
#endif
#if USE_ACTUATOR_IS_ON
	//
	// An optional function used to check if an actuator is engaged
	// Ignored if NULL.
	bool (*is_on)(ACTUATOR_CFG_STORAGE struct actuator_cfg_t *cfg, actuator_status_t *status);
#endif
	//
	// The function used to read the actuator
	// This returns an array of readings from the actuator, all but the last of which
	// has the flag .more set. This array must remain valid until the next time
	// read() is called if a cooldown period is specified.
	// If this returns NULL, the actuator is considered to be in a state of error.
	// Must not be NULL.
	err_t (*set)(ACTUATOR_CFG_STORAGE struct actuator_cfg_t *cfg, actuator_status_t *status, ACTUATOR_STATUS_T value);
#if USE_ACTUATOR_CFG_PIN
	//
	// An optional data field to be used as needed by the actuator definitions
	gpio_pin_t pin;
#endif
#if USE_ACTUATOR_NAME
	//
	// Name of the controller
	// The size of name[] includes a trailing NUL byte.
	char name[DEVICE_NAME_LEN+1];
#endif
	//
	// Configuration flags
	uint8_t cfg_flags;
} actuator_cfg_t;

err_t init_actuator(ACTUATOR_CFG_STORAGE actuator_cfg_t *cfg, actuator_status_t *status);
err_t set_actuator(ACTUATOR_CFG_STORAGE actuator_cfg_t *cfg, actuator_status_t *status, ACTUATOR_STATUS_T value);
err_t set_actuator_by_name(const char *name, ACTUATOR_STATUS_T value);
err_t set_actuator_by_index(ACTUATOR_INDEX_T i, ACTUATOR_STATUS_T value);

// Initialization of the common actuators can be skipped if there's nothing in the
// actuator initializers that needs to be run on startup
void init_common_actuators(void);
ACTUATOR_INDEX_T find_actuator_index_by_name(const char *name);
void check_common_actuator_warnings(void);

extern ACTUATOR_CFG_STORAGE actuator_cfg_t ACTUATORS[];
extern actuator_status_t actuators[];
extern const ACTUATOR_INDEX_T ACTUATOR_COUNT;

#endif // _ACTUATORS_H
