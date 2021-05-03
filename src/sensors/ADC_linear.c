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
// ADC_linear.c
// Manage ADC-based linear sensors
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

#if USE_LINEAR_SENSORS

// In theory checking for this in the header would allow the struct to be
// skipped altogether but the configuration isn't parsed until after the
// header is
#if USE_SMALL_SENSORS < 1 && CALIBRATE_VREF <= 1
# define CACHE_LINEAR_SENSORS 1
#endif


void sensor_init_adc_linear(uiter_t si) {
	_FLASH const sensor_static_t *cfg;
	_FLASH const sensor_opt_linear_t *dev_cfg;

	cfg = &SENSORS[si];
	dev_cfg = &cfg->devcfg.linear;

	// This would cause a divide-by-0 error if allowed
	if (dev_cfg->slopeX10 == 0) {
		SETUP_ERR(si, "Slope of 0 not allowed");
	}
	// This wouldn't actually cause any problems, but it indicates the
	// structure was never set
	if (BIT_IS_SET(cfg->cflags, SENS_FLAG_OHMS) && (dev_cfg->calibration == 0)) {
		SETUP_ERR(si, "Series resistance of 0 not allowed");
	}
	// Make sure we have exactly one of SENS_FLAG_{OHMS,VOLTS} set
	if (!BIT_IS_SET(cfg->cflags, SENS_FLAG_OHMS|SENS_FLAG_VOLTS) || BITS_ARE_SET(cfg->cflags, SENS_FLAG_OHMS|SENS_FLAG_VOLTS)) {
		SETUP_ERR(si, "Need to set one of SENS_FLAG_OHMS or SENS_FLAG_VOLTS");
	}

#if CACHE_LINEAR_SENSORS
	if (BIT_IS_SET(cfg->cflags, SENS_FLAG_VOLTS)) {
		sensor_cache_linear_t *cache;

		cache = &G_sensors[si].dev_cache.linear;
		if ((dev_cfg->calibration != 0) && (dev_cfg->calibration != G_vcc_voltage)) {
			imath_t maxV;

			maxV = dev_cfg->calibration;
			// Round by adding 1/2 denominator
			cache->slopeX10_adj = ((((imath_t )(dev_cfg->slopeX10) * G_vcc_voltage) + (maxV / 2)) / maxV);
			cache->ref_input_adj = ((((imath_t )(dev_cfg->ref_input) * G_vcc_voltage) + (maxV / 2)) / maxV);
		} else {
			cache->slopeX10_adj = dev_cfg->slopeX10;
			cache->ref_input_adj = dev_cfg->ref_input;
		}
	}
#endif

	return;
}
void sensor_update_adc_linear(uiter_t si, uint16_t adc) {
	imath_t adjust, input;
	imath_t ref_value, ref_input, slopeX10;
	sensor_t *s;
	_FLASH const sensor_opt_linear_t *dev_cfg;
	_FLASH const sensor_static_t *cfg;

	assert(adc <= ADC_MAX);

	s = &G_sensors[si];
	cfg = &SENSORS[si];
	dev_cfg = &cfg->devcfg.linear;

#if USE_SMALL_SENSORS < 2
	adjust = cfg->adjust;
#else
	adjust = 0;
#endif
	ref_value = dev_cfg->ref_value;
	ref_input = dev_cfg->ref_input;
	slopeX10  = dev_cfg->slopeX10;
	if (BIT_IS_SET(cfg->cflags, SENS_FLAG_INVERT)) {
		adc = ADC_MAX - adc;
	}

	if (BIT_IS_SET(cfg->cflags, SENS_FLAG_VOLTS)) {
#if CACHE_LINEAR_SENSORS
		slopeX10 = s->dev_cache.linear.slopeX10_adj;
		ref_input = s->dev_cache.linear.ref_input_adj;
#else
		if ((dev_cfg->calibration != 0) && (dev_cfg->calibration != G_vcc_voltage)) {
			imath_t maxV;

			maxV = dev_cfg->calibration;
			// v1/V1 = v2/V2
			// v1 = (v2*V1)/V2
			// Round by adding 1/2 denominator
			slopeX10 = (((slopeX10 * G_vcc_voltage) + (maxV / 2)) / maxV);
			ref_input = (((ref_input * G_vcc_voltage) + (maxV / 2)) / maxV);
		}
#endif
		input = ADC_TO_V(adc);

	} else {
		// adc == ADC_MAX would cause a divide-by-0 in the R calculation
		if (adc >= ADC_MAX) {
			s->status = (slopeX10 > 0) ? SENSOR_HIGH : SENSOR_LOW;
			return;
		}
		input = ADC_TO_R(adc, dev_cfg->calibration);
	}
	// y = mx+b
	// status = ((1/slope)*(input-ref_input)) + ref_value
	// slope is in tenths of a unit, so multiply input by 10 to compensate
	input = (input - ref_input) * 10;
	ref_value += adjust;
	input = (SCALE_INT(input) / slopeX10) + SCALE_INT(ref_value);
#if USE_SMALL_CODE < 2 && STATUS_BITS < 32
	input = (input < STATUS_MIN) ? SENSOR_LOW : (input > STATUS_MAX) ? SENSOR_HIGH : input;
#endif
	s->status = input;

	return;
}

#endif // USE_LINEAR_SENSORS
#ifdef __cplusplus
 }
#endif
