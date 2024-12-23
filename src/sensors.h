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
// sensors.h
// Manage sensors
// NOTES:
//
#ifndef _SENSORS_H
#define _SENSORS_H

#include "common.h"
#if USE_SENSORS

//
// The value of a sensor reading
typedef struct {
	SENSOR_READING_T value;
	//
	// The type is for the configuration files sensor_defs.h and controller_defs.h
	// to coordinate, we don't care about the actual value unless it's 0, in which
	// case the first reading in the array is always matched
	uint8_t type;
} sensor_reading_t;

//
// Status flags for sensor_status_t structs
typedef enum {
	SENSOR_STATUS_FLAG_INITIALIZED = 0x01U, // Sensor successfully initialized
	SENSOR_STATUS_FLAG_ERROR       = 0x02U, // Sensor in error state
} sensor_status_flag_t;
//
// Status of a sensor
typedef struct {
#if USE_SENSOR_DATA
	//
	// Data for sensor-internal use
	void *data;
#endif
	//
	// The most recent sensor readings
	sensor_reading_t* reading;
#if USE_SENSOR_COOLDOWN
	//
	// Time of last successful reading
	utime_t previous_reading_time;
#endif
#if USE_SENSOR_STATUS
	//
	// The status of the sensor
	// This is set and maintained by the sensor and only used externally for
	// logging
	SENSOR_STATUS_T status;
#endif
	//
	// Status flags
	uint8_t status_flags;
} sensor_status_t;

//
// Configuration flags for sensor_cfg_t structs
typedef enum {
	SENSOR_CFG_FLAG_LOG   = 0x40U, // Log this sensor
	SENSOR_CFG_FLAG_NOLOG = 0x80U, // Don't log this sensor
} sensor_cfg_flag_t;
//
// Static configuration of a sensor
typedef struct sensor_cfg_t {
#if USE_SENSOR_CFG_DATA
	//
	// An optional data field to be used as needed by the sensor definitions
	SENSOR_CFG_DATA_T data;
#endif
#if USE_SENSOR_INIT
	//
	// An optional function used to initialize the sensor
	// If this returns anything other than ERR_OK, the sensor is marked as
	// uninitialied and considered to be in a state of error. Initialization may
	// be re-attempted at a later time, in which case the status struct is not
	// zeroed out.
	// Ignored if NULL.
	err_t (*init)(SENSOR_CFG_STORAGE struct sensor_cfg_t *cfg, sensor_status_t *status);
#endif
	//
	// The function used to read the sensor
	// This returns an array of readings from the sensor, the size of which must
	// match the value_count field.
	// This array must remain valid until the next time read() is called.
	// If this returns NULL, the sensor is considered to be in a state of error.
	// Must not be NULL.
	sensor_reading_t* (*read)(SENSOR_CFG_STORAGE struct sensor_cfg_t *cfg, sensor_status_t *status);
#if USE_SENSOR_COOLDOWN
	//
	// How long to let the sensor cool down between readings
	// If an attempt to read is made before this many seconds have elapsed since
	// the previous successful reading, the value of the previous reading is returned
	// If 0, use SENSOR_COOLDOWN_SECONDS.
	uint16_t cooldown_seconds;
#endif
#if USE_SENSOR_CFG_PIN
	//
	// An optional data field to be used as needed by the sensor definitions
	gpio_pin_t pin;
#endif
#if USE_SENSOR_NAME
	//
	// Name of the controller
	// The size of name[] includes a trailing NUL byte.
	char name[DEVICE_NAME_LEN+1];
#endif
	//
	// The number of values in the sensor_reading_t array returned by .read()
	// If 0, assume there's actually 1.
	uint8_t value_count;
	//
	// Configuration flags
	uint8_t cfg_flags;
} sensor_cfg_t;

//
// Initialize a sensor
err_t init_sensor(SENSOR_CFG_STORAGE sensor_cfg_t *cfg, sensor_status_t *status);
//
// Read a sensor
SENSOR_READING_T read_sensor(SENSOR_CFG_STORAGE sensor_cfg_t *cfg, sensor_status_t *status, bool force_update, uint_fast8_t type);
//
// Read a sensor identified by it's name
SENSOR_READING_T read_sensor_by_name(const char *name, bool force_update, uint_fast8_t type);
//
// Read a sensor identified by it's index in SENSORS[]
SENSOR_READING_T read_sensor_by_index(SENSOR_INDEX_T i, bool force_update, uint_fast8_t type);

//
// Initialization of the common sensors can be skipped if there's nothing in the
// sensor initializers that needs to be run on startup
void init_common_sensors(void);
SENSOR_READING_T find_sensor_value_by_type(SENSOR_CFG_STORAGE sensor_cfg_t *cfg, sensor_status_t *status, uint_fast8_t type);
SENSOR_INDEX_T find_sensor_index_by_name(const char *name);
void check_common_sensor_warnings(void);

extern SENSOR_CFG_STORAGE sensor_cfg_t SENSORS[];
extern sensor_status_t sensors[];
extern const SENSOR_INDEX_T SENSOR_COUNT;

#else // USE_SENSORS
INLINE void init_common_sensors(void) {
	return;
}
INLINE void check_common_sensor_warnings(void) {
	return;
}
static const SENSOR_INDEX_T SENSOR_COUNT = 0;
#endif // USE_SENSORS
#endif // _SENSORS_H
