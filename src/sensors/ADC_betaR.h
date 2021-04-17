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
// ADC_betaR.h
// Manage ADC-based logarithmic resistance sensors
// NOTES:
//

#ifdef __cplusplus
 extern "C" {
#endif
#ifndef _SENSORS_ADC_BETAR_H
#define _SENSORS_ADC_BETAR_H
#if USE_BETA_R_SENSORS

//
// Enable the appropriate peripheral to read this sensor
#if ! USE_ADC_SENSORS
# undef  USE_ADC_SENSORS
# define USE_ADC_SENSORS 1
#endif

//
// Macros for the lists in sensors.h
#define _SENS_ADC_BETA_R    SENS_ADC_BETA_R,
#define SENS_ADC_BETA_R_CFG sensor_opt_betaR_t beta_R;
#define SENS_ADC_BETA_R_DISPATCH { .init = sensor_init_adc_betaR, .read = sensor_read_ADC, .update = sensor_update_adc_betaR },
//
// Dispatch function declarations
void sensor_init_adc_betaR(uiter_t si);
void sensor_update_adc_betaR(uiter_t si, uint16_t adc);

//
// Sensor type-specific settings for SENS_*_CFG
typedef struct {
	// The value of the other resistor in the voltage divider
	uint32_t series_R_ohms;
	// The resistance in ohms at the reference value
	uint32_t ref_ohms;
	// The reference value
	int32_t ref_value;
	// The beta coefficient
	int32_t beta;
} sensor_opt_betaR_t;



#else  // !USE_BETA_R_SENSORS
#define _SENS_ADC_BETA_R
#define SENS_ADC_BETA_R_DISPATCH
#define SENS_ADC_BETA_R_CFG

#endif // USE_BETA_R_SENSORS
#endif // _SENSORS_ADC_BETAR_H
#ifdef __cplusplus
 }
#endif
