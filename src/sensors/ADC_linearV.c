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
// ADC_linearV.c
// Manage ADC-based linear voltage sensors
// NOTES:
//

#ifdef __cplusplus
 extern "C" {
#endif

//#define DEBUG 1
//#define NDEBUG 1

//#include "ADC_linearV.h"
#include "sensors.h"
#include "private.h"

#if USE_LINEAR_V_SENSORS


void sensor_init_adc_linearV(uiter_t si) {
	_FLASH const sensor_opt_linearV_t *dev_cfg;

	dev_cfg = &SENSORS[si].devcfg.linear_V;
	// This would cause a divide-by-0 error if allowed
	if (dev_cfg->slope_mVx100 == 0) {
		SETUP_ERR(si, "Slope of 0 not allowed");
	}

	return;
}
void sensor_update_adc_linearV(uiter_t si, uint16_t adc) {
	imath_t adjust;
	imath_t ref, refV, slope;
	imath_t maxV;
	sensor_t *s;
	_FLASH const sensor_opt_linearV_t *dev_cfg;
	_FLASH const sensor_static_t *cfg;

	assert(adc <= ADC_MAX);

	s = &G_sensors[si];
	cfg = &SENSORS[si];
	dev_cfg = &cfg->devcfg.linear_V;

#if USE_SMALL_SENSORS < 2
	adjust = cfg->adjust;
#else
	adjust = 0;
#endif
	ref   = dev_cfg->ref_value;
	refV  = dev_cfg->ref_mV;
	maxV  = dev_cfg->ref_Vcc_mV;
	slope = dev_cfg->slope_mVx100;
	// There's no need to actually figure out the ratio of the original
	// reference voltage to ours, we can just convert from voltage steps to
	// adc steps
	if (maxV == 0) {
		maxV = G_vcc_voltage;
	}

	if (BIT_IS_SET(cfg->cflags, SENS_FLAG_INVERT)) {
		adc = ADC_MAX - adc;
	}

	// Convert voltage slope to ADC slope
	// ADCstep/ADC_MAX = Vstep/Vmax
	// ADCstep = (Vstep/Vmax)*ADC_MAX
	// ADCstep = (Vstep*ADC_MAX)/Vmax
	slope = ((slope * ADC_MAX) / maxV);

	// Convert voltage reference to ADC reference
	// ADCref/ADC_MAX = Vref/Vmax
	// ADCref = (Vref/Vmax)*ADC_MAX
	// ADCref = (Vref*ADC_MAX)/Vmax
	refV = (refV * ADC_MAX) / maxV;

	// y = mx+b
	// status = ((1/slope)*(V-refV)) + ref
	// slope is in hundredths of a mV, so multiply voltage by 100 to compensate
	s->status = (SCALE_INT(((imath_t )adc - refV) * 100) / slope) + SCALE_INT(ref + adjust);

	return;
}

#endif // USE_LINEAR_V_SENSORS
#ifdef __cplusplus
 }
#endif
