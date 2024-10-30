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
// controllers.h
// Manage external device control
// NOTES:
//
#ifndef _CONTROLLERS_H
#define _CONTROLLERS_H

#include "common.h"


//
// Status flags for controller_status_t structs
typedef enum {
	CONTROLLER_STATUS_FLAG_INITIALIZED = 0x01U, // Controller successfully initialized
	CONTROLLER_STATUS_FLAG_ERROR       = 0x02U, // Controller in error state; set when run() or init() return error codes
} controller_status_flag_t;
//
// Status of a controller
typedef struct {
#if USE_CONTROLLER_DATA
	//
	// Data for controller-internal use
	void *data;
#endif
#if USE_CONTROLLER_SCHEDULE
	//
	// Time of next check
	utime_t next_run_time;
#endif
#if USE_CONTROLLER_STATUS
	CONTROLLER_STATUS_T status;
#endif
	//
	// Status flags
	uint8_t status_flags;
} controller_status_t;

//
// Configuration flags for controller_cfg_t structs
typedef enum {
	CONTROLLER_CFG_FLAG_IGNORE_FORCED_RUN = 0x01U, // Ignore forced controller runs
	CONTROLLER_CFG_FLAG_USE_TIME_OF_DAY   = 0x02U, // Schedule is time-of-day not period
} controller_cfg_flags_t;
//
// Static configuration of a controller
typedef struct controller_cfg_t {
#if USE_CONTROLLER_INIT
	//
	// An optional function used to initialize the controller
	// If this returns anything other than ERR_OK, the controller is marked as
	// uninitialied and considered to be in a state of error.
	// Ignored if NULL.
	err_t (*init)(CONTROLLER_CFG_STORAGE struct controller_cfg_t *cfg, controller_status_t *status);
#endif
	//
	// The function used to run the controller
	// If this returns anything other than ERR_OK, the controller is considered
	// to be in a state of error.
	// Must not be NULL.
	err_t (*run)(CONTROLLER_CFG_STORAGE struct controller_cfg_t *cfg, controller_status_t *status);
#if USE_CONTROLLER_NEXTTIME
	//
	// An optional function used to calculate the next time the controller
	// should run
	// The time returned is the system time in seconds
	// If NULL or if this function returns 0, a time will be calculated based
	// on the scheduled run time and configuration settings
	utime_t (*next_run_time)(CONTROLLER_CFG_STORAGE struct controller_cfg_t *cfg, controller_status_t *status);
#endif
#if USE_CONTROLLER_NAME
	//
	// Name of the controller
	// The size of name[] includes a trailing NUL byte.
	char name[DEVICE_NAME_LEN+1];
#endif
#if USE_CONTROLLER_SCHEDULE
	//
	// When to run the controller
	// By default this represents the number of minutes between checks, but
	// if the CONTROLLER_CFG_FLAG_USE_TIME_OF_DAY flag is set it represents the
	// number of minutes after midnight to check.
	// If 0 and CONTROLLER_CFG_FLAG_USE_TIME_OF_DAY is unset, the controller will
	// only be run when manually requested
	uint16_t schedule_minutes;
#endif
	//
	// Configuration flags
	uint8_t cfg_flags;
} controller_cfg_t;


err_t init_controller(CONTROLLER_CFG_STORAGE controller_cfg_t *cfg, controller_status_t *status);
err_t run_controller(CONTROLLER_CFG_STORAGE controller_cfg_t *cfg, controller_status_t *status);
err_t calculate_controller_alarm(CONTROLLER_CFG_STORAGE controller_cfg_t *cfg, controller_status_t *status);

void init_common_controllers(void);
void run_common_controllers(bool manual, bool force);
void calculate_common_controller_alarms(bool force);
utime_t find_next_common_controller_alarm(void);
void check_common_controller_warnings(void);

controller_status_t* get_controller_status_by_index(CONTROLLER_INDEX_T i);

extern CONTROLLER_CFG_STORAGE controller_cfg_t CONTROLLERS[];
extern const CONTROLLER_INDEX_T CONTROLLER_COUNT;

#endif // _CONTROLLERS_H
