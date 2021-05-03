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
// ADC_ohm.h
// Manage ADC-based resistance sensors
// NOTES:
//

#ifdef __cplusplus
 extern "C" {
#endif
#ifndef _SENSORS_ADC_OHM_H
#define _SENSORS_ADC_OHM_H
#if USE_OHM_SENSORS

//
// Enable the appropriate peripheral to read this sensor
#if ! USE_ADC_SENSORS
# undef  USE_ADC_SENSORS
# define USE_ADC_SENSORS 1
#endif
//
// Resistance readings can easily go higher than what an int16_t can hold
#if USE_SMALL_SENSORS < 1 && _STATUS_BITS < 32
# undef _STATUS_BITS
# define _STATUS_BITS 32
#endif

//
// Macros for the lists in sensors.h
#define _SENS_ADC_OHM    SENS_ADC_OHM,
#define SENS_ADC_OHM_CFG sensor_opt_ohm_t ohm;
#define SENS_ADC_OHM_DISPATCH { .init = sensor_init_adc_ohm, .read = sensor_read_ADC, .update = sensor_update_adc_ohm },
//
// Dispatch function declarations
void sensor_init_adc_ohm(uiter_t si);
void sensor_update_adc_ohm(uiter_t si, uint16_t adc);

//
// Sensor type-specific settings for SENS_*_CFG
typedef struct {
	// The value of the other resistor in the voltage divider
	uint32_t series_R_ohms;
} sensor_opt_ohm_t;



#else  // !USE_OHM_SENSORS
#define _SENS_ADC_OHM
#define SENS_ADC_OHM_DISPATCH
#define SENS_ADC_OHM_CFG

#endif // USE_OHM_SENSORS
#endif // _SENSORS_ADC_OHM_H
#ifdef __cplusplus
 }
#endif
