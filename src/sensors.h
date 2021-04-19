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
// sensors.h
// Manage sensors
// NOTES:
//     This file is closely tied to sensors/config.h even though it doesn't
//     directly include it
//

#ifdef __cplusplus
 extern "C" {
#endif
#ifndef _SENSORS_H
#define _SENSORS_H

/*
* Includes
*/
#include "config.h"
#include "common.h"
// This is included indirectly from config.h
//#include "sensors/config.h"

/*
* Static values
*/
// If a bulk sensor request is made less than this many seconds after the last
// one finished, don't do it. Mostly to stop controllers from powering the
// sensors on and off repeatedly.
#define SENS_COOLDOWN 30

//
// Configuration flags for sensor_t structs
//
// Choose between resistance and voltage sensing for sensor backends that
// support both
#define SENS_FLAG_VOLTS 0x01
#define SENS_FLAG_OHMS  0x02
// Invert the relationship between the pin voltage reading and the value
// Use this if the sensor is on the high side of the voltage divider.
#define SENS_FLAG_INVERT  0x04
//
// This flag depends on structure members that may not exist
#if USE_SMALL_SENSORS < 2
// If set, this sensor falling below warn_below indicates a low battery
# define SENS_FLAG_BATTERY 0x10
#endif

//
// Internal flags for sensor_t structs
//
// Set internally if the sensor is connected to the I2C bus
#define SENS_FLAG_I2C       0x08
// Set internally if the sensor is connected to the SPI bus
#define SENS_FLAG_SPI       0x10
// Set internally if the sensor has already been read in a given cycle
// Used for multi-sensor inputs like the dht11
#define SENS_FLAG_DONE      0x20
// Set internally if the sensor is being monitored (that is, either warn_below
// or warn_above is non-zero and not SENS_THRESHOLD_IGNORE)
#define SENS_FLAG_MONITORED 0x40
// Set internally if there was a warning issued for the sensor
#define SENS_FLAG_WARNING   0x80

// The value range a status_t can hold
#define STATUS_MIN (-32768)
#define STATUS_MAX 32767

// Ignore a sensor warning threshold if set to this value
#define SENS_THRESHOLD_IGNORE STATUS_MIN


/*
* Types
*/
typedef int16_t status_t;

//
// Description of the user-configured portion of a sensor struct
typedef struct {
	// Union used for configuring the underlying device parameters
	sensor_devcfg_t devcfg;

	//
	// The name is used only for logging
	// The size of name[] includes a trailing NUL byte.
	char name[DEVICE_NAME_LEN+1];

#if USE_SMALL_SENSORS < 2
	// Adjust the value by this amount for e.g. conversion between Celsius and
	// Kelvin
	// This value is added to the status after all other calculations are
	// performed and before the multiplier is applied.
	status_t adjust;

	// Issue a warning when the sensor's status is outside these bounds
	// If above > below warn when outside a window
	// If above < below warn when inside a window
	// If above == SENS_THRESHOLD_IGNORE only warn when below 'below'
	// If below == SENS_THRESHOLD_IGNORE only warn when above 'above'
	// These are ignored if both fields are 0 (unset) or SENS_THRESHOLD_IGNORE
	status_t warn_above;
	status_t warn_below;

	// Scale values to this %
	// Used for (for example) voltage dividers on sources greater than the
	// ADC reference voltage. May be useful for soft calibration too.
	// Set to 0 to disable.
	int16_t scale;
#endif // USE_SMALL_SENSORS < 2

	// Control flags
	uint8_t cflags;

	// MCU pin used to check the sensor
	pin_t pin;

	// The type of device
	// this can be any value of sensor_type_t; it's specified as uint8_t to
	// keep the size down.
	uint8_t type;
} sensor_static_t;
//
// Description of the internal portion of a sensor struct
typedef struct {
#if USE_SMALL_SENSORS < 1
#if USE_BETA_R_SENSORS
	// Cache the terms (B/T0) and log(R0) used in sensor interpretation
	// calculations in order to cut out a division and a log()
	FIXEDP_ITYPE B_div_T0;
	FIXEDP_ITYPE log_R0;
#endif // USE_BETA_R_SENSORS
#endif // USE_SMALL_SENSORS < 1

	// Converted sensor reading
	status_t status;

	// Internal flags
	uint8_t iflags;
	// Index in G_sensors[]
	// At time of last test, storing 'i' in the struct used less space on flash
	// than pointer arithmetic for controllers but more for sensors; both used
	// the same amount of RAM
	// uint8_t i;
} sensor_t;


/*
* Variable declarations (defined in sensors.c)
*/
// Last-measured VCC voltage in mV
extern int16_t G_vcc_voltage;

// Array of sensor structs representing available input
extern sensor_t G_sensors[SENSOR_COUNT];

// Array of sensor configuration structs representing available input
// Defined in config/sensors.c
extern _FLASH const sensor_static_t SENSORS[SENSOR_COUNT];

/*
* Function prototypes (defined in sensors.c)
*/
// Initialize sensor subsystem
void sensors_init(void);

// Update sensor readings
void check_sensors(void);
// Invalidate current sensor statuses, forcing a re-read the next time they're
// updated regardless of the cooldown period
void invalidate_sensors(void);

// Check for sensor warnings
void check_sensor_warnings(void);


/*
* Macros
*/
// VDIV-to-scale calculation: (100) * (Rv + Rg) / Rg
// Rv is resistance from the supply to the test point, Rg is test point to
// ground.
#define CALC_VDIV_SCALE(Rv, Rg) ((((uint64_t )100) * ((uint64_t )Rv + (uint64_t )Rg)) / (uint64_t )Rg)
// Calculate a multiplier
#define MULTIPLY_BY(mul) ((mul) * (100))
// Calculate a divider
#define DIVIDE_BY(div) ((100) / (div))

// Determine the index in G_sensors of a sensor_t instance
#define GET_SENSOR_I(s) ((uint )(s - G_sensors))
//#define GET_SENSOR_I(s) ((s)->i)

#endif // _SENSORS_H
#ifdef __cplusplus
 }
#endif
