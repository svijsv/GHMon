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
// ADC_lookupR.h
// Manage ADC-based lookup-table resistance sensors
// NOTES:
//

#ifdef __cplusplus
 extern "C" {
#endif
#ifndef _SENSORS_ADC_LOOKUP_H
#define _SENSORS_ADC_LOOKUP_H
#if USE_LOOKUP_SENSORS

//
// Enable the appropriate peripheral to read this sensor
#if ! USE_ADC_SENSORS
# undef  USE_ADC_SENSORS
# define USE_ADC_SENSORS 1
#endif
#if ! USE_LOOKUP_TABLES
# undef  USE_LOOKUP_TABLES
# define USE_LOOKUP_TABLES 1
#endif
//
// Resistance readings can easily go higher than what an int16_t can hold
#if USE_SMALL_SENSORS < 1 && _STATUS_BITS < 32
# undef _STATUS_BITS
# define _STATUS_BITS 32
#endif

//
// Macros for the lists in sensors.h
#define _SENS_ADC_LOOKUP    SENS_ADC_LOOKUP,
#define SENS_ADC_LOOKUP_CFG sensor_opt_lookup_t lookup;
#define SENS_ADC_LOOKUP_DISPATCH { .init = sensor_init_adc_lookup, .read = sensor_read_ADC, .update = sensor_update_adc_lookup },
//
// Dispatch function declarations
void sensor_init_adc_lookup(uiter_t si);
void sensor_update_adc_lookup(uiter_t si, uint16_t adc);

//
// Sensor type-specific settings for SENS_*_CFG
typedef struct {
	// For voltage sensors, ignored
	// For resistance sensors, the value of the other resistor in the sensor's
	// voltage divider
	uint32_t calibration;
	// The index of the lookup table in LOOKUP_TABLES[]
	uint8_t lutno;
} sensor_opt_lookup_t;

//
// Lookup table for determining sensor status
typedef struct {
	// The voltage in mV or resistance in ohms corresponding to the first value
	// in the table
	uint32_t min;
	// The voltage in mV or resistance in ohms corresponding to the last value
	// in the table
	uint32_t max;
	// For voltage tables, the reference voltage in mV of the system the values
	// were calculated for; used to calibrate steps against the ADC voltage
	// reference when the sensor is operating off the same power supply and the
	// output is Vcc-dependent.
	// Ignored if 0.
	//
	// For resistance tables, ignored
	uint16_t Vref;
	// Interpret table values as being multiplied by this
	uint16_t table_multiplier;
	// Set to SENS_FLAG_OHMS or SENS_FLAG_VOLTS for resistance and voltage
	// tables respectively
	uint8_t cflags;

	LUT_T table[LUT_SIZE];
} sensor_LUT_t;

// Array of lookup tables used for associated sensors
// Defined in config/tables.c
extern _FLASH const sensor_LUT_t LOOKUP_TABLES[];


#else  // !USE_LOOKUP_SENSORS
#define _SENS_ADC_LOOKUP
#define SENS_ADC_LOOKUP_DISPATCH
#define SENS_ADC_LOOKUP_CFG

#endif // USE_LOOKUP_SENSORS
#endif // _SENSORS_ADC_LOOKUP_H
#ifdef __cplusplus
 }
#endif
