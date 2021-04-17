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
#ifndef _SENSORS_ADC_LOOKUPR_H
#define _SENSORS_ADC_LOOKUPR_H
#if USE_LOOKUP_R_SENSORS

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
// Macros for the lists in sensors.h
#define _SENS_ADC_LOOKUP_R    SENS_ADC_LOOKUP_R,
#define SENS_ADC_LOOKUP_R_CFG sensor_opt_lookupR_t lookup_R;
#define SENS_ADC_LOOKUP_R_DISPATCH { .init = sensor_init_adc_lookupR, .read = sensor_read_ADC, .update = sensor_update_adc_lookupR },
//
// Dispatch function declarations
void sensor_init_adc_lookupR(uiter_t si);
void sensor_update_adc_lookupR(uiter_t si, uint16_t adc);

//
// Sensor type-specific settings for SENS_*_CFG
typedef struct {
	// The value of the other resistor in the sensor's voltage divider
	uint32_t series_R_ohms;
	// The index of the lookup table in LOOKUP_TABLES[]
	uint8_t lutno;
} sensor_opt_lookupR_t;



#else  // !USE_LOOKUP_R_SENSORS
#define _SENS_ADC_LOOKUP_R
#define SENS_ADC_LOOKUP_R_DISPATCH
#define SENS_ADC_LOOKUP_R_CFG

#endif // USE_LOOKUP_R_SENSORS
#endif // _SENSORS_ADC_LOOKUPR_H
#ifdef __cplusplus
 }
#endif
