// SPDX-License-Identifier: GPL-3.0-only
/***********************************************************************
*                                                                       *
*                                                                       *
* Copyright (C) 2021 svijsv
* This program is free software: you can redistribute it and/or modify  *
* it under the terms of the GNU General Public License as published by  *
* the Free Software Foundation, version 3.                              *
*                                                                       *
* This program is distributed in the hope that it will be useful, but   *
* WITHOUT ANY WARRANTY; without even the implied warranty of            *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU      *
* General Public License for more details.                              *
*                                                                       *
* You should have received a copy of the GNU General Public License     *
* along with this program. If not, see <https://www.gnu.org/licenses/>. *
*                                                                       *
*                                                                       *
***********************************************************************/
// private.h
// Manage ADC-based linear resistance sensors
// NOTES:
//    Everything that needs to be added to sensors.h when a new sensor type is
//    added should be in this file to avoid a circular dependency between
//    sensors.h and config_unify.h
//
//    Steps for a new sensor type:
//       * Copy a sensors/*.[ch] pair and modify to suit the new sensor
//          * Make sure any needed USE_{ADC,SPI,I2C}_SENSORS macro is set for
//            the underlying peripheral
//       * #include the header below
//       * Add it to sensor_type_t
//       * Add it to SENSOR_DISPATCHES
//       * Add it to sensor_devcfg_t if required
//       * Add USE_x_SENSORS to config/templates/config.h
//

#ifdef __cplusplus
 extern "C" {
#endif
#ifndef _SENSORS_CONFIG_H
#define _SENSORS_CONFIG_H

#include "ulib/fmem.h"
#include "ulib/types.h"

//
// The struct used to handle the sensor functions
//
// Reading is a separate step from updating only to minimize the time the
// power needs to be on; at last check it added ~600B for 16 sensors
//
// Any field may be unused, in which case it should be set to NULL
typedef struct {
	// Initialize sensor instances
	void (*init)(uiter_t si);
	// Read data from a sensor
	uint16_t (*read)(uiter_t si);
	// Use the data from read() to update the sensor_t struct
	void (*update)(uiter_t si, uint16_t adc);
} sensor_dispatch_t;
// This general-purpose sensor_dispatch_t function is defined in private.c
uint16_t sensor_read_ADC(uiter_t si);
//
// Sensor header inclusions
#include "sensors/ADC_linear.h"
#include "sensors/ADC_betaR.h"
#include "sensors/ADC_lookupR.h"
#include "sensors/ADC_lookupV.h"
#include "sensors/ADC_ohm.h"
#include "sensors/ADC_volts.h"
#include "sensors/binary.h"
#include "sensors/BMx280.h"
#include "sensors/DHT11.h"
//
// Sensor type names
//
// Devices with multiple internal sensors need one type per sensor
//
// There's no way to (even hackily) auto-increment macros which is what I
// would need to avoid having to add each sensor to an enum manually, so
// instead the enum members are defined in their respective headers as their
// own names when used and as empty when skipped and then listed here
typedef enum {
	SENS_NONE = 0,

	_SENS_BINARY

	_SENS_ADC_VOLT
	_SENS_ADC_OHM
	_SENS_ADC_LINEAR
	_SENS_ADC_LOOKUP_R
	_SENS_ADC_LOOKUP_V
	_SENS_ADC_BETA_R

	_SENS_DHT11_HUMIDITY
	_SENS_DHT11_TEMPERATURE

	_SENS_BMx280_SPI_TEMPERATURE
	_SENS_BMx280_SPI_PRESSURE
	_SENS_BMx280_SPI_HUMIDITY

	_SENS_BMx280_I2C_TEMPERATURE
	_SENS_BMx280_I2C_PRESSURE
	_SENS_BMx280_I2C_HUMIDITY

	// This must be the last element, it's used to get the number of types
	SENSOR_TYPE_COUNT
} sensor_type_t;
//
// The functions used to initialize , read, and update the status of each
// sensor type are listed here in the same order as they appear in the
// sensor_type_t definition
extern _FLASH const sensor_dispatch_t sensor_dispatch[SENSOR_TYPE_COUNT];
#define SENSOR_DISPATCHES \
	SENS_BINARY_DISPATCH \
 \
	SENS_ADC_VOLT_DISPATCH \
	SENS_ADC_OHM_DISPATCH \
	SENS_ADC_LINEAR_DISPATCH \
	SENS_ADC_LOOKUP_R_DISPATCH \
	SENS_ADC_LOOKUP_V_DISPATCH \
	SENS_ADC_BETA_R_DISPATCH \
 \
	SENS_DHT11_HUMIDITY_DISPATCH \
	SENS_DHT11_TEMPERATURE_DISPATCH \
 \
	SENS_BMx280_SPI_TEMPERATURE_DISPATCH \
	SENS_BMx280_SPI_PRESSURE_DISPATCH \
	SENS_BMx280_SPI_HUMIDITY_DISPATCH \
 \
	SENS_BMx280_I2C_TEMPERATURE_DISPATCH \
	SENS_BMx280_I2C_PRESSURE_DISPATCH \
	SENS_BMx280_I2C_HUMIDITY_DISPATCH \

//
// Sensor type-specific configuration is contained in this union if required
// These don't need to be in the same order as sensor_type_t and multiple
// sensor types can use the same set
typedef union {
	SENS_ADC_LINEAR_CFG
	SENS_ADC_LOOKUP_R_CFG
	SENS_ADC_LOOKUP_V_CFG
	SENS_ADC_OHM_CFG
	SENS_ADC_BETA_R_CFG
} sensor_devcfg_t;

#endif // _SENSORS_CONFIG_H
#ifdef __cplusplus
 }
#endif
