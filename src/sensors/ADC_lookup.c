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

#if USE_LOOKUP_SENSORS


void sensor_init_adc_lookup(uiter_t si) {
	uint8_t cflags;
	_FLASH const sensor_opt_lookup_t *dev_cfg;

	dev_cfg = &SENSORS[si].devcfg.lookup;
	cflags = LOOKUP_TABLES[dev_cfg->lutno].cflags;

	// This wouldn't actually cause any internal problems, but it indicates
	// the structure was never set
	if (BIT_IS_SET(cflags, SENS_FLAG_OHMS) && (dev_cfg->calibration == 0)) {
		SETUP_ERR(si, "Series resistance of 0 not allowed");
	}
	// Make sure we have exactly one of SENS_FLAG_{OHMS,VOLTS} set
	if (!BIT_IS_SET(cflags, SENS_FLAG_OHMS|SENS_FLAG_VOLTS) || BITS_ARE_SET(cflags, SENS_FLAG_OHMS|SENS_FLAG_VOLTS)) {
		SETUP_ERR(si, "Need to set one of SENS_FLAG_OHMS or SENS_FLAG_VOLTS");
	}

	return;
}
void sensor_update_adc_lookup(uiter_t si, uint16_t adc) {
	imath_t status, adjust, input;
	imath_t min, max, size, scale;
	sensor_t *s;
	_FLASH const sensor_static_t *cfg;
	_FLASH const sensor_opt_lookup_t *dev_cfg;
	_FLASH const LUT_T *lut;
	_FLASH const sensor_LUT_t *table_cfg;

	assert(adc <= ADC_MAX);

	s = &G_sensors[si];
	cfg = &SENSORS[si];
	dev_cfg = &cfg->devcfg.lookup;
	table_cfg = &LOOKUP_TABLES[dev_cfg->lutno];

#if USE_SMALL_SENSORS < 2
	adjust = cfg->adjust;
#else
	adjust = 0;
#endif
	lut    = table_cfg->table;
	min    = table_cfg->min;
	max    = table_cfg->max;
	scale  = table_cfg->table_multiplier;
	size   = LUT_SIZE;
	if (scale == 0) {
		scale = 1;
	}
	if (BIT_IS_SET(cfg->cflags, SENS_FLAG_INVERT)) {
		adc = ADC_MAX - adc;
	}

	if (BIT_IS_SET(table_cfg->cflags, SENS_FLAG_VOLTS)) {
		imath_t Vref;

		Vref = table_cfg->Vref;
		if (Vref == 0) {
			Vref = G_vcc_voltage;
		}
		// Convert voltage values to ADC equivalents
		// adc/ADC_MAX = v/Vref
		// adc = (v*ADC_MAX)/Vref
		min = (min * ADC_MAX)/Vref;
		max = (max * ADC_MAX)/Vref;
		input = adc;
	} else {
		// adc == ADC_MAX would cause a divide-by-0 in the R calculation
		if (adc == ADC_MAX) {
			s->status = (lut[0] < lut[size-1]) ? SENSOR_LOW : SENSOR_HIGH;
			return;
		}
		input = ADC_TO_R(adc, dev_cfg->calibration);
	}

	if (input < min) {
		s->status = (lut[0] < lut[size-1]) ? SENSOR_LOW : SENSOR_HIGH;
		return;
	} else if (input >= max) {
		s->status = (lut[0] < lut[size-1]) ? SENSOR_HIGH : SENSOR_LOW;
		return;
	} else {
		imath_t I1, I2, T1, T2, i;
		imath_t slope;

		// TODO: Support slopes < 0
		// Reduce calculation errors by using slope * 128 (2^7)
		slope  = ((max - min) << 7) / (size-1);
		i = ((input - min) << 7) / slope;
		I1 = min + ((i * slope) >> 7);
		I2 = min + (((i+1) * slope) >> 7);
		T1 = lut[i];
		T2 = lut[i+1];
		// (t-T1)/(T2-T1) = (i-I1)/(I2-I1)
		// (t-T1) = ((i-I1)/(I2-I1))*(T2-T1)
		// t = (((i-I1)/(I2-I1))*(T2-T1)) + T1
		// t = (((i-I1)*(T2-T1))/(I2-I1)) + T1
		status = (((input-I1)*(T2-T1))/(I2-I1)) + T1;
	}
	status += (adjust * scale);
	status = SCALE_INT(status) / scale;
	s->status = status;

	return;
}

#endif // USE_LOOKUP_SENSORS
#ifdef __cplusplus
 }
#endif
