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
// ADC_lookupR.c
// Manage ADC-based lookup-table resistance sensors
// NOTES:
//

#ifdef __cplusplus
 extern "C" {
#endif

//#define DEBUG 1
//#define NDEBUG 1

//#include "ADC_lookupR.h"
#include "sensors.h"
#include "private.h"

#if USE_LOOKUP_R_SENSORS


void sensor_init_adc_lookupR(uiter_t si) {
	_FLASH const sensor_opt_lookupR_t *dev_cfg;

	dev_cfg = &SENSORS[si].devcfg.lookup_R;
	// This wouldn't actually cause any internal problems, but it indicates
	// the structure was never set
	if (dev_cfg->series_R_ohms == 0) {
		SETUP_ERR(si, "Series resistance of 0 not allowed");
	}

	return;
}
void sensor_update_adc_lookupR(uiter_t si, uint16_t adc) {
	imath_t status, adjust;
	imath_t r, sR;
	imath_t min, max, size, scale;
	sensor_t *s;
	_FLASH const sensor_static_t *cfg;
	_FLASH const sensor_opt_lookupR_t *dev_cfg;
	_FLASH const LUT_T *lut;
	_FLASH const sensor_LUT_t *table_cfg;

	assert(adc <= ADC_MAX);

	s = &G_sensors[si];
	cfg = &SENSORS[si];
	dev_cfg = &cfg->devcfg.lookup_R;
	table_cfg = &LOOKUP_TABLES[dev_cfg->lutno];

#if USE_SMALL_SENSORS < 2
	adjust = cfg->adjust;
#else
	adjust = 0;
#endif
	sR     = dev_cfg->series_R_ohms;
	lut    = table_cfg->table;
	min    = table_cfg->min;
	max    = table_cfg->max;
	scale  = table_cfg->scale;
	size   = LUT_SIZE;
	if (scale == 0) {
		scale = 1;
	}

	if (BIT_IS_SET(cfg->cflags, SENS_FLAG_INVERT)) {
		adc = ADC_MAX - adc;
	}
	// adc == ADC_MAX would cause a divide-by-0 in the R calculation
	if (adc == ADC_MAX) {
		s->status = (lut[0] < lut[size-1]) ? SENSOR_LOW : SENSOR_HIGH;
		return;
	}

	r = ADC_TO_R(adc, sR);
	// TODO: Support slopes < 0
	if (r < min) {
		s->status = (lut[0] < lut[size-1]) ? SENSOR_LOW : SENSOR_HIGH;
		return;
	} else if (r >= max) {
		s->status = (lut[0] < lut[size-1]) ? SENSOR_HIGH : SENSOR_LOW;
		return;
	} else {
		imath_t R1, R2, T1, T2, i;
		imath_t slope;

		// Reduce calculation errors by using slope * 128 (2^7)
		slope  = ((max - min) << 7) / (size-1);
		i = ((r - min) << 7) / slope;
		R1 = min + ((i * slope) >> 7);
		R2 = min + (((i+1) * slope) >> 7);
		T1 = lut[i];
		T2 = lut[i+1];
		// (t-T1)/(T2-T1) = (r-R1)/(R2-R1)
		// (t-T1) = ((r-R1)/(R2-R1))*(T2-T1)
		// t = (((r-R1)/(R2-R1))*(T2-T1)) + T1
		// t = (((r-R1)*(T2-T1))/(R2-R1)) + T1
		status = (((r-R1)*(T2-T1))/(R2-R1)) + T1;
	}

	s->status = (SCALE_INT(status + (adjust * scale))) / scale;

	return;
}

#endif // USE_LOOKUP_R_SENSORS
#ifdef __cplusplus
 }
#endif
