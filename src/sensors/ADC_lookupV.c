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
// ADC_lookupV.c
// Manage ADC-based lookup-table resistance sensors
// NOTES:
//

#ifdef __cplusplus
 extern "C" {
#endif

//#define DEBUG 1
//#define NDEBUG 1

//#include "ADC_lookupV.h"
#include "sensors.h"
#include "private.h"

#if USE_LOOKUP_V_SENSORS


void sensor_update_adc_lookupV(uiter_t si, uint16_t adc) {
	imath_t status, adjust, calibration;
	imath_t min, max, Vref, size, scale;
	sensor_t *s;
	_FLASH const sensor_static_t *cfg;
	_FLASH const sensor_opt_lookupV_t *dev_cfg;
	_FLASH const LUT_T *lut;
	_FLASH const sensor_LUT_t *table_cfg;

	assert(adc <= ADC_MAX);

	s = &G_sensors[si];
	cfg = &SENSORS[si];
	dev_cfg = &cfg->devcfg.lookup_V;
	table_cfg = &LOOKUP_TABLES[dev_cfg->lutno];

#if USE_SMALL_SENSORS < 2
	adjust = cfg->adjust;
#else
	adjust = 0;
#endif
	calibration = dev_cfg->scale;
	lut    = table_cfg->table;
	min    = table_cfg->min;
	max    = table_cfg->max;
	Vref   = table_cfg->Vref;
	scale  = table_cfg->scale;
	size   = LUT_SIZE;
	if (scale == 0) {
		scale = 1;
	}

	if (Vref == 0) {
		Vref = G_vcc_voltage;
	}
	if (calibration != 0) {
		max = (max * calibration) / 100;
		min = (min * calibration) / 100;
	}
	if (BIT_IS_SET(cfg->cflags, SENS_FLAG_INVERT)) {
		adc = ADC_MAX - adc;
	}

	// Convert voltage values to ADC equivalents
	// adc/ADC_MAX = v/Vref
	// adc = (v*ADC_MAX)/Vref
	min = (min * ADC_MAX)/Vref;
	max = (max * ADC_MAX)/Vref;

	// TODO: Support slopes < 0
	// if (max > min) {
	if (adc < min) {
		s->status = (lut[0] < lut[size-1]) ? SENSOR_LOW : SENSOR_HIGH;
		return;
	} else if (adc >= max) {
		s->status = (lut[0] < lut[size-1]) ? SENSOR_HIGH : SENSOR_LOW;
		return;
	} else {
		imath_t V1, V2, T1, T2, i;
		imath_t slope;

		// Reduce calculation errors by using slope * 128 (2^7)
		slope  = ((max - min) << 7) / (size-1);
		i = ((adc - min) << 7) / slope;
		V1 = min + ((i * slope) >> 7);
		V2 = min + (((i+1) * slope) >> 7);
		T1 = lut[i];
		T2 = lut[i+1];
		// (t-T1)/(T2-T1) = (v-V1)/(V2-V1)
		// (t-T1) = ((v-V1)/(V2-V1))*(T2-T1)
		// t = (((v-V1)/(V2-V1))*(T2-T1)) + T1
		// t = (((v-V1)*(T2-T1))/(V2-V1)) + T1
		status = (((adc-V1)*(T2-T1))/(V2-V1)) + T1;
	}

	s->status = (SCALE_INT(status + (adjust * scale))) / scale;

	return;
}

#endif // USE_LOOKUP_V_SENSORS
#ifdef __cplusplus
 }
#endif
