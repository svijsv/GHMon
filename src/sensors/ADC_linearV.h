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
// ADC_linearV.h
// Manage ADC-based linear voltage sensors
// NOTES:
//

#ifdef __cplusplus
 extern "C" {
#endif
#ifndef _SENSORS_ADC_LINEARV_H
#define _SENSORS_ADC_LINEARV_H
#if USE_LINEAR_V_SENSORS

//
// Enable the appropriate peripheral to read this sensor
#if ! USE_ADC_SENSORS
# undef  USE_ADC_SENSORS
# define USE_ADC_SENSORS 1
#endif

//
// Macros for the lists in sensors.h
#define _SENS_ADC_LINEAR_V    SENS_ADC_LINEAR_V,
#define SENS_ADC_LINEAR_V_CFG sensor_opt_linearV_t linear_V;
#define SENS_ADC_LINEAR_V_DISPATCH { .init = sensor_init_adc_linearV, .read = sensor_read_ADC, .update = sensor_update_adc_linearV },
//
// Dispatch function declarations
void sensor_init_adc_linearV(uiter_t si);
void sensor_update_adc_linearV(uiter_t si, uint16_t adc);

//
// Sensor type-specific settings for SENS_*_CFG
typedef struct {
	// The reference voltage in mV of the system the reference value was taken
	// in; used to calibrate steps against the ADC voltage reference when the
	// sensor is operating off the same power supply and the output is
	// Vcc-dependent.
	// Ignored if 0.
	uint16_t ref_Vcc_mV;
	// The voltage in mV at the reference value
	int32_t ref_mV;
	// The reference value
	int16_t ref_value;
	// The voltage increase in 1/100 mV at each step
	int16_t slope_mVx100;
} sensor_opt_linearV_t;



#else  // !USE_LINEAR_V_SENSORS
#define _SENS_ADC_LINEAR_V
#define SENS_ADC_LINEAR_V_CFG
#define SENS_ADC_LINEAR_V_DISPATCH

#endif // USE_LINEAR_V_SENSORS
#endif // _SENSORS_ADC_LINEARV_H
#ifdef __cplusplus
 }
#endif
