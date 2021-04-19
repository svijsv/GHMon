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
		imath_t maxV;

		maxV = dev_cfg->calibration;
		// There's no need to actually figure out the ratio of the original
		// reference voltage to ours, we can just convert from voltage steps
		// to adc steps
		input = adc;
		if (maxV == 0) {
			maxV = G_vcc_voltage;
		}
		// Convert voltage slope to ADC slope
		// ADCstep/ADC_MAX = Vstep/Vmax
		// ADCstep = (Vstep/Vmax)*ADC_MAX
		// ADCstep = (Vstep*ADC_MAX)/Vmax
		slopeX10 = ((slopeX10 * ADC_MAX) / maxV);
		// Convert voltage reference to ADC reference
		// ADCref/ADC_MAX = Vref/Vmax
		// ADCref = (Vref/Vmax)*ADC_MAX
		// ADCref = (Vref*ADC_MAX)/Vmax
		ref_input = (ref_input * ADC_MAX) / maxV;

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
	s->status = (SCALE_INT(input) / slopeX10) + SCALE_INT(ref_value);

	return;
}

#endif // USE_LINEAR_SENSORS
#ifdef __cplusplus
 }
#endif
