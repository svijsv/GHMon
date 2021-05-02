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
// Module private declarations
// NOTES:
//

#ifdef __cplusplus
 extern "C" {
#endif
#ifndef _SENSORS_PRIVATE_H
#define _SENSORS_PRIVATE_H

#include "../sensors.h"

#include "ulib/math.h"

//
// The type used for integer math
typedef adcm_t imath_t;

//
// Print an error message and enter an error state
// Store multi-use strings in const arrays so they aren't duplicated
extern _FLASH const char sens_invalid_msg_l[];
extern _FLASH const char sens_invalid_msg_e[];
#define SETUP_ERR(i, msg) \
	LOGGER_NOF(FROM_FSTR(sens_invalid_msg_l), (uint )(i), F1(msg)); \
	ERROR_STATE_NOF(FROM_FSTR(sens_invalid_msg_e))
//
// Calculate a voltage from an ADC reading
// Vo/Vs = adc/ADC_MAX
// Vo = (adc/ADC_MAX)*Vs
// Vo = (Vs*adc)/ADC_MAX
#define ADC_TO_V(adc) (((imath_t )G_vcc_voltage * (imath_t )(adc)) / ADC_MAX)
//
// Calculate a resistance from an ADC reading
// The ADC values are proportional to the voltages so:
// R2 = (Vo*R1)/(Vs-Vo)
// Rt = (adc*Rs)/(max-adc)
#define ADC_TO_R(adc, R1) (((imath_t )(adc) * (imath_t )(R1)) / (ADC_MAX - (imath_t )(adc)))
//
// Apply the sensor multiplier to a value
// It's assumed the sensor's SENSOR[] member has been assigned to a pointer
// named 'cfg'.
#if USE_SMALL_SENSORS < 2
# define SCALE_INT(x) ((cfg->scale_percent == 0) ? (imath_t )(x) : \
	((imath_t )(x) * (imath_t )cfg->scale_percent) / (imath_t )100)
# define SCALE_FLOAT(x) ((cfg->scale_percent == 0) ? (float )(x) : \
	((float )(x) * (float )cfg->scale_percent) / 100.0f)
# define SCALE_FIXED(x) ((cfg->scale_percent == 0) ? (FIXEDP_ITYPE )(x) : \
	FIXEDP_DIVI(((FIXEDP_MTYPE )(x) * (FIXEDP_MTYPE )cfg->scale_percent), (FIXEDP_MTYPE )100))
#else
# define SCALE_INT(x) (x)
# define SCALE_FLOAT(x) (x)
# define SCALE_FIXED(x) (x)
#endif


#endif // _SENSORS_PRIVATE_H
#ifdef __cplusplus
 }
#endif
