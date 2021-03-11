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
// Invert the relationship between the pin voltage reading and the value
// Use this if the sensor is on the high side of the voltage divider.
#define SENS_FLAG_INVERT  0x02
//
// This flag depends on structure members that may not exist
#if USE_SMALL_SENSORS < 2
// If set, this sensor falling below warn_below indicates a low battery
# define SENS_FLAG_BATTERY 0x10
#endif

//
// Internal flags for sensor_t structs
//
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

typedef enum {
	SENS_NONE = 0,

#if USE_VOLT_SENSOR
	SENS_VOLT,
#endif

#if USE_OHM_SENSOR
	SENS_OHM,
#endif

#if USE_LINEAR_V_SENSOR
	SENS_LINEAR_V,
#endif

#if USE_LINEAR_R_SENSOR
	SENS_LINEAR_R,
#endif

#if USE_LOOKUP_R_SENSOR
	SENS_LOOKUP_R,
#endif

#if USE_LOOKUP_V_SENSOR
	SENS_LOOKUP_V,
#endif

#if USE_LOG_BETA_SENSOR
	SENS_LOG_BETA,
#endif

#if USE_BINARY_SENSOR
	SENS_BINARY,
#endif
} sensor_type_t;

//
// Settings for direct resistance sensors
typedef struct {
	// The value of the other resistor in the voltage divider
	uint32_t series_R;
} sensor_opt_ohm_t;
//
// Settings for linear sensors
typedef struct {
	// For resistance sensors, the resistance in ohms of the other resistor in
	// the voltage divider.
	// For voltage sensors, the reference voltage in mV of the system the
	// reference value was taken in; used to calibrate steps against the ADC
	// voltage reference when the sensor is operating off the same power supply
	// and the output is Vcc-dependent.
	// Ignored by voltage sensors if 0.
	uint32_t calibration;
	// The voltage in mV or resistance in ohms at the reference value
	int32_t ref_value;
	// The reference value
	status_t ref;
	// The voltage increase in 1/100 mV at each step or resistance increase in
	// 1/10 ohms at each step
	int16_t slope;
} sensor_opt_linear_t;
//
// Settings for sensors with lookup tables
typedef struct {
	// For resistance tables, the value of the other resistor in the sensor's
	// voltage divider
	// For voltage tables, set the table max to this % of it's listed value,
	// effectively scaling the voltage steps by the same
	// Ignored for voltage tables if 0.
	uint32_t calibration;
	// The index of the lookup table in LOOKUP_TABLES[]
	uint8_t lutno;
} sensor_opt_lut_t;
//
// Settings for non-linear sensors with beta coefficents
typedef struct {
	// The value of the other resistor in the voltage divider
	uint32_t series_R;
	// The resistance in ohms at the reference value
	uint32_t ref_R;
	// The reference value
	int32_t ref;
	// The beta coefficient
	int32_t beta;
} sensor_opt_log_beta_t;
//
// Configuration of the underlying device
// Depending on configuration options, this could be 0, 4, 5, 12, or 16 bytes.
typedef union {
#if USE_OHM_SENSOR
	sensor_opt_ohm_t ohm;
#endif

#if USE_LINEAR_V_SENSOR || USE_LINEAR_R_SENSOR
	sensor_opt_linear_t linear;
#endif

#if USE_LOOKUP_R_SENSOR || USE_LOOKUP_V_SENSOR
	sensor_opt_lut_t lut;
#endif

#if USE_LOG_BETA_SENSOR
	sensor_opt_log_beta_t log_beta;
#endif
} sensor_devcfg_t;

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
#if USE_LOG_BETA_SENSOR
	// Cache the terms (B/T0) and log(R0) used in sensor interpretation
	// calculations in order to cut out a division and a log()
	FIXEDP_ITYPE B_div_T0;
	FIXEDP_ITYPE log_R0;
#endif // USE_LOG_BETA_SENSOR
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

//
// Lookup table for determining sensor status
typedef struct {
	// The voltage in mV or resistance in ohms represented by the first value
	// in the table
	uint32_t min;
	// The voltage in mV or resistance in ohms represented by the last value
	// in the table
	uint32_t max;
	// For voltage tables, the reference voltage in mV of the system the values
	// were calculated for; used to calibrate steps against the ADC voltage
	// reference when the sensor is operating off the same power supply and the
	// output is Vcc-dependent.
	// Ignored for resistance tables.
	// Ignored if 0.
	uint16_t Vref;
	// Table values are scaled by this
	uint16_t scale;

	LUT_T table[LUT_SIZE];
} sensor_LUT_t;


/*
* Variable declarations (defined in sensors.c)
*/
// Last-measured VCC voltage in mV
extern int16_t G_vcc_voltage;

// Last-measure MCU temperature in degrees C * 10
extern int16_t G_mcu_temp;

// Array of sensor structs representing available input
extern sensor_t G_sensors[SENSOR_COUNT];

// Array of sensor configuration structs representing available input
// Defined in config.c
extern const sensor_static_t SENSORS[SENSOR_COUNT];
// Array of lookup tables used for associated sensors
// Defined in config.c
extern const sensor_LUT_t LOOKUP_TABLES[];

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

#define GET_SENSOR_I(s) ((uint )(s - G_sensors))
//#define GET_SENSOR_I(s) ((s)->i)

#endif // _SENSORS_H
#ifdef __cplusplus
 }
#endif
