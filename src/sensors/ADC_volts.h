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
#define SENS_ADC_VOLT_DISPATCH { .init = NULL, .read = sensor_read_ADC, .update = sensor_update_adc_volt },
//
// Dispatch function declarations
void sensor_update_adc_volt(uiter_t si, uint16_t adc);



#else  // !USE_VOLT_SENSORS
#define _SENS_ADC_VOLT
#define SENS_ADC_VOLT_DISPATCH

#endif // USE_VOLT_SENSORS
#endif // _SENSORS_ADC_VOLT_H
#ifdef __cplusplus
 }
#endif
