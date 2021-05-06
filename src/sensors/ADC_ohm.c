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
// ADC_ohm.c
// Manage ADC-based resistance sensors
// NOTES:
//

#ifdef __cplusplus
 extern "C" {
#endif

//#define DEBUG 1
//#define NDEBUG 1

//#include "ADC_ohms.h"
#include "sensors.h"
#include "private.h"

#if USE_OHM_SENSORS


void sensor_init_adc_ohm(uiter_t si) {
	_FLASH const sensor_opt_ohm_t *dev_cfg;

	dev_cfg = &SENSORS[si].devcfg.ohm;
	// This wouldn't actually cause any problems, but it indicates the
	// structure was never set
	if (dev_cfg->series_R_ohms == 0) {
		SETUP_ERR(si, "Series resistance of 0 not allowed");
	}
	if ((dev_cfg->slopeX10 == 0) && ((dev_cfg->ref_ohms != 0) || (dev_cfg->ref_value != 0))) {
		SETUP_ERR(si, "Slope can't be 0 when ref_ohms or ref_value are set");
	}

	return;
}
void sensor_update_adc_ohm(uiter_t si, uint16_t adc) {
	imath_t adjust;
	imath_t r, sR, slopeX10, ref_value, ref_ohms;
	sensor_t *s;
	_FLASH const sensor_static_t *cfg;
	_FLASH const sensor_opt_ohm_t *dev_cfg;

	assert(adc <= ADC_MAX);

	s = &G_sensors[si];
	cfg = &SENSORS[si];
	dev_cfg = &cfg->devcfg.ohm;

#if USE_SMALL_SENSORS < 2
	adjust = cfg->adjust;
#else
	adjust = 0;
#endif
	sR = dev_cfg->series_R_ohms;
	slopeX10 = dev_cfg->slopeX10;
	ref_value = dev_cfg->ref_value;
	ref_ohms = dev_cfg->ref_ohms;

	if (BIT_IS_SET(cfg->cflags, SENS_FLAG_INVERT)) {
		adc = ADC_MAX - adc;
	}
	// adc == ADC_MAX would cause a divide-by-0 in the R calculation
	if (adc == ADC_MAX) {
		s->status = (slopeX10 >= 0) ? SENSOR_HIGH : SENSOR_LOW;
	} else {
		r = ADC_TO_R(adc, sR);

		if (slopeX10 == 0) {
			r = SCALE_INT(r + adjust);
			r = (r < STATUS_MIN) ? SENSOR_LOW : (r > STATUS_MAX) ? SENSOR_HIGH : r;
			s->status = r;
		} else {
			imath_t status;

			// y = mx+b
			// status = ((1/slope)*(input-ref_input)) + ref_value
			// slope is in tenths of an ohm, so multiply R by 10 to compensate
			r = (r - ref_ohms) * 10;
			ref_value += adjust;
			status = (SCALE_INT(r) / slopeX10) + SCALE_INT(ref_value);
			status = (status < STATUS_MIN) ? SENSOR_LOW : (status > STATUS_MAX) ? SENSOR_HIGH : status;
			s->status = status;
		}
	}

	return;
}

#endif // USE_OHM_SENSORS
#ifdef __cplusplus
 }
#endif
