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
// ADC_volts.h
// Manage ADC-based voltage sensors
// NOTES:
//

#ifdef __cplusplus
 extern "C" {
#endif
#ifndef _SENSORS_ADC_VOLT_H
#define _SENSORS_ADC_VOLT_H
#if USE_VOLT_SENSORS

//
// Enable the appropriate peripheral to read this sensor
#if ! USE_ADC_SENSORS
# undef  USE_ADC_SENSORS
# define USE_ADC_SENSORS 1
#endif

//
// Macros for the lists in sensors.h
#define _SENS_ADC_VOLT   SENS_ADC_VOLT,
#define SENS_ADC_VOLT_CFG sensor_opt_volt_t volt;
#define SENS_ADC_VOLT_DISPATCH { .init = sensor_init_adc_volt, .read = sensor_read_ADC, .update = sensor_update_adc_volt },
//
// Dispatch function declarations
void sensor_init_adc_volt(uiter_t si);
void sensor_update_adc_volt(uiter_t si, uint16_t adc);

//
// Sensor type-specific settings for SENS_*_CFG
typedef struct {
	// The voltage in mV of the system the reference value was taken in; used
	// to calibrate steps against the ADC voltage reference when the sensor
	// is operating off the same power supply and the output is Vcc-dependent.
	// Ignored if 0.
	int16_t sys_mV;
	// The voltage in millivolts at the reference value
	// Leave unset or set to 0 for direct voltage measurement
	int16_t ref_mV;
	// The reference value
	// Leave unset or set to 0 for direct voltage measurement
	int16_t ref_value;
	// The voltage change in 1/10 millivolts when the value increases by 1
	// Leave unset or set to 0 for direct voltage measurement
	int16_t slopeX10;
} sensor_opt_volt_t;


#else  // !USE_VOLT_SENSORS
#define _SENS_ADC_VOLT
#define SENS_ADC_VOLT_CFG
#define SENS_ADC_VOLT_DISPATCH

#endif // USE_VOLT_SENSORS
#endif // _SENSORS_ADC_VOLT_H
#ifdef __cplusplus
 }
#endif
