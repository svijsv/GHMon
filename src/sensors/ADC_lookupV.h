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
// ADC_lookupV.h
// Manage ADC-based lookup-table resistance sensors
// NOTES:
//

#ifdef __cplusplus
 extern "C" {
#endif
#ifndef _SENSORS_ADC_LOOKUPV_H
#define _SENSORS_ADC_LOOKUPV_H
#if USE_LOOKUP_V_SENSORS

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
#define _SENS_ADC_LOOKUP_V    SENS_ADC_LOOKUP_V,
#define SENS_ADC_LOOKUP_V_CFG sensor_opt_lookupV_t lookup_V;
#define SENS_ADC_LOOKUP_V_DISPATCH { .init = NULL, .read = sensor_read_ADC, .update = sensor_update_adc_lookupV },
//
// Dispatch function declarations
void sensor_update_adc_lookupV(uiter_t si, uint16_t adc);

//
// Sensor type-specific settings for SENS_*_CFG
typedef struct {
	// Set the table max and min to this % of their listed values, effectively
	// scaling the voltage steps by the same
	// Ignored if 0.
	uint16_t scale;
	// The index of the lookup table in LOOKUP_TABLES[]
	uint8_t lutno;
} sensor_opt_lookupV_t;



#else  // !USE_LOOKUP_V_SENSORS
#define _SENS_ADC_LOOKUP_V
#define SENS_ADC_LOOKUP_V_DISPATCH
#define SENS_ADC_LOOKUP_V_CFG

#endif // USE_LOOKUP_V_SENSORS
#endif // _SENSORS_ADC_LOOKUPV_H
#ifdef __cplusplus
 }
#endif
