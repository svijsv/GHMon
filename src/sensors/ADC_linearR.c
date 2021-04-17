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
// ADC_linearR.c
// Manage ADC-based linear resistance sensors
// NOTES:
//

#ifdef __cplusplus
 extern "C" {
#endif

//#define DEBUG 1
//#define NDEBUG 1

//#include "ADC_linearR.h"
#include "sensors.h"
#include "private.h"

#if USE_LINEAR_R_SENSORS


void sensor_init_adc_linearR(uiter_t si) {
	_FLASH const sensor_opt_linearR_t *dev_cfg;

	dev_cfg = &SENSORS[si].devcfg.linear_R;
	// This would cause a divide-by-0 error if allowed
	if (dev_cfg->slope_ohmsX10 == 0) {
		SETUP_ERR(si, "Slope of 0 not allowed");
	}
	// This wouldn't actually cause any problems, but it indicates the
	// structure was never set
	if (dev_cfg->series_R_ohms == 0) {
		SETUP_ERR(si, "Series resistance of 0 not allowed");
	}

	return;
}
void sensor_update_adc_linearR(uiter_t si, uint16_t adc) {
	imath_t adjust;
	imath_t r, slope, ref, refR, sR;
	sensor_t *s;
	_FLASH const sensor_opt_linearR_t *dev_cfg;
	_FLASH const sensor_static_t *cfg;

	assert(adc <= ADC_MAX);

	s = &G_sensors[si];
	cfg = &SENSORS[si];
	dev_cfg = &cfg->devcfg.linear_R;

#if USE_SMALL_SENSORS < 2
	adjust = cfg->adjust;
#else
	adjust = 0;
#endif
	slope = dev_cfg->slope_ohmsX10;
	refR  = dev_cfg->ref_ohms;
	ref   = dev_cfg->ref_value;
	sR    = dev_cfg->series_R_ohms;

	if (BIT_IS_SET(cfg->cflags, SENS_FLAG_INVERT)) {
		adc = ADC_MAX - adc;
	}
	// adc == ADC_MAX would cause a divide-by-0 in the R calculation
	if (adc >= ADC_MAX) {
		s->status = (slope > 0) ? SENSOR_HIGH : SENSOR_LOW;
		return;
	}

	r = ADC_TO_R(adc, sR);
	// y = mx+b
	// value = ((1/slope)*(R-refR)) + ref
	// slope is 1/10 ohms so multiply numerator by 10 to compensate
	s->status = (SCALE_INT((r - refR) * 10) / slope) + SCALE_INT(ref + adjust);

	return;
}

#endif // USE_LINEAR_R_SENSORS
#ifdef __cplusplus
 }
#endif
