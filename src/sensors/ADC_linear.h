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
// ADC_linear.h
// Manage ADC-based linear sensors
// NOTES:
//

#ifdef __cplusplus
 extern "C" {
#endif
#ifndef _SENSORS_ADC_LINEAR_H
#define _SENSORS_ADC_LINEAR_H
#if USE_LINEAR_SENSORS

//
// Enable the appropriate peripheral to read this sensor
#if ! USE_ADC_SENSORS
# undef  USE_ADC_SENSORS
# define USE_ADC_SENSORS 1
#endif

//
// Macros for the lists in sensors.h
#define _SENS_ADC_LINEAR    SENS_ADC_LINEAR,
#define SENS_ADC_LINEAR_DISPATCH { .init = sensor_init_adc_linear, .read = sensor_read_ADC, .update = sensor_update_adc_linear },
#define SENS_ADC_LINEAR_CFG sensor_opt_linear_t linear;
#define SENS_ADC_LINEAR_CACHE sensor_cache_linear_t linear;
//
// Dispatch function declarations
void sensor_init_adc_linear(uiter_t si);
void sensor_update_adc_linear(uiter_t si, uint16_t adc);

//
// Sensor type-specific settings for SENS_*_CFG
typedef struct {
	// For voltage sensors, the reference voltage in mV of the system the
	// reference value was taken in; used to calibrate steps against the ADC
	// voltage reference when the sensor is operating off the same power
	// supply and the output is Vcc-dependent.
	// Ignored if 0.
	//
	// For resistance sensors, the series resistor used between Vcc and the
	// test pin in the voltage divider
	int32_t calibration;
	// For voltage sensors, the voltage in mV at the reference value
	// For resistance sensors, the resistance in ohms at the reference value
	int32_t ref_input;
	// The reference value
	int16_t ref_value;
	// For voltage sensors, the voltage increase in 1/10 mV at each step
	// For resistance sensors, the resistance increase in 1/10 ohms at each
	// step
	int16_t slopeX10;
} sensor_opt_linear_t;

typedef struct {
	// The slopeX10 and ref_input adjusted for the calibration value with
	// voltage sensors
	int16_t slopeX10_adj;
	int32_t ref_input_adj;
} sensor_cache_linear_t;


#else  // !USE_LINEAR_SENSORS
#define _SENS_ADC_LINEAR
#define SENS_ADC_LINEAR_CFG
#define SENS_ADC_LINEAR_CACHE
#define SENS_ADC_LINEAR_DISPATCH

#endif // USE_LINEAR_SENSORS
#endif // _SENSORS_ADC_LINEAR_H
#ifdef __cplusplus
 }
#endif
