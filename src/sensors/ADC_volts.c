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
// ADC_volts.c
// Manage ADC-based voltage sensors
// NOTES:
//

#ifdef __cplusplus
 extern "C" {
#endif

//#define DEBUG 1
//#define NDEBUG 1

//#include "ADC_volts.h"
#include "sensors.h"
#include "private.h"

#if USE_VOLT_SENSORS

void sensor_init_adc_volt(uiter_t si) {
	_FLASH const sensor_opt_volt_t *dev_cfg;

	dev_cfg = &SENSORS[si].devcfg.volt;
	if ((dev_cfg->slopeX10 == 0) && ((dev_cfg->ref_mV != 0) || (dev_cfg->ref_value != 0))) {
		SETUP_ERR(si, "Slope can't be 0 when ref_mV or ref_value are set");
	}
	return;
}
void sensor_update_adc_volt(uiter_t si, uint16_t adc) {
	imath_t adjust;
	imath_t v, sys_mV, slopeX10, ref_value, ref_mV;
	sensor_t *s;
	_FLASH const sensor_static_t *cfg;
	_FLASH const sensor_opt_volt_t *dev_cfg;

	assert(adc <= ADC_MAX);

	s = &G_sensors[si];
	cfg = &SENSORS[si];
	dev_cfg = &cfg->devcfg.volt;

#if USE_SMALL_SENSORS < 2
	adjust = cfg->adjust;
#else
	adjust = 0;
#endif
	sys_mV = dev_cfg->sys_mV;
	slopeX10 = dev_cfg->slopeX10;
	ref_value = dev_cfg->ref_value;
	ref_mV = dev_cfg->ref_mV;

	if (BIT_IS_SET(cfg->cflags, SENS_FLAG_INVERT)) {
		adc = ADC_MAX - adc;
	}

	if (slopeX10 == 0) {
		v = ADC_TO_V(adc) + adjust;
		s->status = SCALE_INT(v);
	} else {
		imath_t status;

		if (sys_mV != 0) {
			// v1/V1 = v2/V2
			// v1 = (v2*V1)/V2
			// Round by adding 1/2 denominator
			slopeX10 = (((slopeX10 * G_vcc_voltage) + (sys_mV / 2)) / sys_mV);
			ref_mV = (((ref_mV * G_vcc_voltage) + (sys_mV / 2)) / sys_mV);
		}
		v = ADC_TO_V(adc);

		// y = mx+b
		// status = ((1/slope)*(input-ref_input)) + ref_value
		// slope is in tenths of a mV, so multiply input by 10 to compensate
		v = (v - ref_mV) * 10;
		ref_value += adjust;
		status = (SCALE_INT(v) / slopeX10) + SCALE_INT(ref_value);
		status = (status < STATUS_MIN) ? SENSOR_LOW : (status > STATUS_MAX) ? SENSOR_HIGH : status;
		s->status = status;
	}

	return;
}

#endif // USE_VOLT_SENSORS
#ifdef __cplusplus
 }
#endif
