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
// ADC_betaR.c
// Manage ADC-based betaRage sensors
// NOTES:
//

#ifdef __cplusplus
 extern "C" {
#endif

//#define DEBUG 1
//#define NDEBUG 1

//#include "ADC_betaR.h"
#include "sensors.h"
#include "private.h"

#if USE_BETA_R_SENSORS

#if USE_SMALL_SENSORS < 1
# define CACHE_BETA_R_SENSORS 1
#endif

void sensor_init_adc_betaR(uiter_t si) {
	_FLASH const sensor_opt_betaR_t *dev_cfg;

	dev_cfg = &SENSORS[si].devcfg.beta_R;
	// This would cause a divide-by-0 error if allowed
	if (dev_cfg->beta == 0) {
		SETUP_ERR(si, "Beta constant of 0 not allowed");
	}

#if CACHE_BETA_R_SENSORS
	sensor_cache_betaR_t *cache;

	cache = &G_sensors[si].dev_cache.betaR;
	cache->B_div_T0 = FIXEDP_DIV(FIXEDP_FROM(dev_cfg->beta), FIXEDP_FROM(dev_cfg->ref_value));
	cache->log_R0   = log_fixedp(FIXEDP_FROM(dev_cfg->ref_ohms));
#endif

	return;
}
void sensor_update_adc_betaR(uiter_t si, uint16_t adc) {
	FIXEDP_ITYPE r, beta, tmp, adjust, BdivT0, logR0;
	imath_t sR;
	sensor_t *s;
	_FLASH const sensor_static_t *cfg;
	_FLASH const sensor_opt_betaR_t *dev_cfg;

	assert(adc <= ADC_MAX);

	s = &G_sensors[si];
	cfg = &SENSORS[si];
	dev_cfg = &cfg->devcfg.beta_R;

#if CACHE_BETA_R_SENSORS
	sensor_cache_betaR_t *cache;

	cache = &G_sensors[si].dev_cache.betaR;
	BdivT0 = cache->B_div_T0;
	logR0 = cache->log_R0;
#else
	BdivT0 = FIXEDP_DIV(FIXEDP_FROM(dev_cfg->beta), FIXEDP_FROM(dev_cfg->ref_value));
	logR0   = log_fixedp(FIXEDP_FROM(dev_cfg->ref_ohms));
#endif

#if USE_SMALL_SENSORS < 2
	adjust = FIXEDP_FROM(cfg->adjust);
#else
	adjust = 0;
#endif
	beta = FIXEDP_FROM(dev_cfg->beta);
	sR   = dev_cfg->series_R_ohms;

	if (BIT_IS_SET(cfg->cflags, SENS_FLAG_INVERT)) {
		adc = ADC_MAX - adc;
	}
	// adc == ADC_MAX would cause a divide-by-0 in the R calculation
	if (adc == ADC_MAX) {
		s->status = (beta > 0) ? SENSOR_LOW : SENSOR_HIGH;
		return;
	}

	r = FIXEDP_FROM(ADC_TO_R(adc, sR));
	if (r == 0) {
		s->status = (beta > 0) ? SENSOR_HIGH : SENSOR_LOW;
		return;
	}
	// Using 1/T = 1/T0 + 1/B * log(R/R0)
	// T = 1/(1/T0 + (log(R/R0)/B))
	//
	// With a little work, this becomes:
	// T = B /((B/T0) + log(R1 / R0))
	//
	// ...thereby reducing the number of divisions by 1 AND reducing the
	// required fixed-point precision from >=18 to >=4, meaning we can fit our
	// numbers into a 32-bit integers instead of having to use the slower
	// software 64-bit divisions. To further reduce runtime, (B/T0) and log(R0)
	// can be cached at initialization.
	//
	// (B/T0) + log(R1 / R0)
	// Due to library constraints:
	// (B/T0) + (log(R) - log(R0))
	tmp = BdivT0 + (log_fixedp(r) - logR0);
	//
	// B / ^
	if (tmp != 0) {
		tmp = FIXEDP_DIV(beta, tmp);
		s->status = FIXEDP_AWAY(SCALE_FIXED(tmp + adjust));
	} else {
		s->status = (beta > 0) ? SENSOR_HIGH : SENSOR_LOW;
	}

	return;
}

#endif // USE_BETA_R_SENSORS
#ifdef __cplusplus
 }
#endif
