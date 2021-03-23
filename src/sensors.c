// SPDX-License-Identifier: GPL-3.0-only
/***********************************************************************
*                                                                      *
*                                                                      *
* Copyright 2021 svijsv                                                *
* This program is free software: you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation, version 3.                             *
*                                                                      *
* This program is distributed in the hope that it will be useful, but  *
* WITHOUT ANY WARRANTY; without even the implied warranty of           *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU    *
* General Public License for more details.                             *
*                                                                      *
* You should have received a copy of the GNU General Public License    *
* along with this program.  If not, see <http:// www.gnu.org/licenses/>.*
*                                                                      *
*                                                                      *
***********************************************************************/
// sensors.c
// Manage sensors
// NOTES:
//   https:// www.daycounter.com/Calculators/Steinhart-Hart-Thermistor-Calculator.phtml
//   https:// www.allaboutcircuits.com/industry-articles/how-to-obtain-the-temperature-value-from-a-thermistor-measurement/
//   https:// www.electroniclinic.com/what-is-a-thermistor-thermistor-types-thermistor-circuits/#Thermistor_Overview
//   https:// www.digikey.com/en/articles/how-to-accurately-sense-temperature-using-thermistors
//

#ifdef __cplusplus
 extern "C" {
#endif

//#define DEBUG 1
/*
* Includes
*/
#include "sensors.h"
#include "power.h"
#include "serial.h"

#include "ulib/time.h"
#include "ulib/math.h"


#if SENSOR_COUNT > 255 || SENSOR_COUNT < 1
# error "SENSOR_COUNT not between 1 and 255"
#endif

/*
* Static values
*/
// Store multi-use strings in const arrays so they aren't duplicated
static const char l_invalid_msg[] = "Invalid sensor %u configuration";
static const char e_invalid_msg[] = "Invalid sensor configuration";
#define SETUP_ERR(i) \
	LOGGER(l_invalid_msg, (uint )(i)); \
	ERROR_STATE(e_invalid_msg)

static const char unknown_msg[] = "Unknown sensor type '%u' in %s.";
#define UNKNOWN_MSG(i) LOGGER(unknown_msg, (uint )(i), __func__);


/*
* Types
*/
// The type used for integer math
typedef adcm_t imath_t;


/*
* Variables
*/
int16_t G_vcc_voltage = REGULATED_VOLTAGE;
int16_t G_mcu_temp    = 0;

utime_t cooldown = 0;

sensor_t G_sensors[SENSOR_COUNT];

/*
* Local function prototypes
*/
static imath_t read_sensor(sensor_t *s);
static void update_sensor(sensor_t *s, imath_t value);
static void update_sensor_warning(sensor_t *s);
#if USE_VOLT_SENSOR
static void calculate_millivolts(sensor_t *s, imath_t adc);
#endif
#if USE_OHM_SENSOR
static void calculate_ohms(sensor_t *s, imath_t adc);
#endif
#if USE_LOG_BETA_SENSOR
static void calculate_log_beta(sensor_t *s, imath_t adc);
#endif
#if USE_LINEAR_V_SENSOR
static void calculate_linearV(sensor_t *s, imath_t adc);
#endif
#if USE_LINEAR_R_SENSOR
static void calculate_linearR(sensor_t *s, imath_t adc);
#endif
#if USE_LOOKUP_R_SENSOR
static void calculate_lookupR(sensor_t *s, imath_t adc);
#endif
#if USE_LOOKUP_V_SENSOR
static void calculate_lookupV(sensor_t *s, imath_t adc);
#endif
#if USE_BINARY_SENSOR
static void calculate_binary(sensor_t *s, imath_t value);
#endif
static void power_on(void);
static void power_off(void);


/*
* Interrupt handlers
*/


/*
* Functions
*/
// Calculate a voltage from an ADC reading
// Vo/Vs = adc/ADC_MAX
// Vo = (adc/ADC_MAX)*Vs
// Vo = (Vs*adc)/ADC_MAX
#define ADC_TO_VOLTAGE(adc) (((imath_t )G_vcc_voltage * (imath_t )(adc)) / ADC_MAX)

// Calculate a resistance from an ADC reading
// The sensor is the second resistor in the divider:
// R2 = (Vo*R1)/(Vs-Vo)
// The ADC values are proportional to the voltages so:
// Rt = (adc*Rs)/(max-adc)
#define ADC_TO_R(adc, R1) (((imath_t )(adc) * (imath_t )(R1)) / (ADC_MAX - (imath_t )(adc)))

#if USE_SMALL_SENSORS < 2
// Apply the sensor multiplier to a value
# define SCALE_INT(x) ((cfg->scale == 0) ? (imath_t )(x) : \
	((imath_t )(x) * (imath_t )cfg->scale) / (imath_t )100)
# define SCALE_FLOAT(x) ((cfg->scale == 0) ? (float )(x) : \
	((float )(x) * (float )cfg->scale) / 100.0f)
# define SCALE_FIXED(x) ((cfg->scale == 0) ? (FIXEDP_ITYPE )(x) : \
	FIXEDP_DIVI(((FIXEDP_MTYPE )(x) * (FIXEDP_MTYPE )cfg->scale), (FIXEDP_MTYPE )100))

#else // !USE_SMALL_SENSORS < 2
# define SCALE_INT(x) (x)
# define SCALE_FLOAT(x) (x)
# define SCALE_FIXED(x) (x)
#endif // USE_SMALL_SENSORS < 2

void sensors_init(void) {
	sensor_t *s;
	const sensor_devcfg_t *dev_cfg;
	const sensor_static_t *cfg;

	power_off();

	// Check for any problems with the sensor configuration
	for (uiter_t i = 0; i < SENSOR_COUNT; ++i) {
		s = &G_sensors[i];
		cfg = &SENSORS[i];
		dev_cfg = &cfg->devcfg;

		//s->i = i;

		if (cfg->name[0] == 0) {
			LOGGER("Unset name in SENSORS[%u]; is SENSOR_COUNT correct?", (uint )i);
			ERROR_STATE("Unset name in SENSORS[]; is SENSOR_COUNT correct?");
		}

#if USE_SMALL_SENSORS < 2
		switch (cfg->warn_above) {
		case 0:
			// If both warn_above and warn_below were 0, they almost certainly
			// weren't set
			if (cfg->warn_below != 0) {
				SET_BIT(s->iflags, SENS_FLAG_MONITORED);
			}
			break;
		case SENS_THRESHOLD_IGNORE:
			if (cfg->warn_below != SENS_THRESHOLD_IGNORE) {
				SET_BIT(s->iflags, SENS_FLAG_MONITORED);
			}
			break;
		default:
			SET_BIT(s->iflags, SENS_FLAG_MONITORED);
			break;
		}
#endif // USE_SMALL_SENSORS < 2

		switch (cfg->type) {
#if USE_VOLT_SENSOR
		case SENS_VOLT:
			// Nothing to check here
			break;
#endif // USE_VOLT_SENSOR

#if USE_OHM_SENSOR
		case SENS_OHM:
			// This wouldn't actually cause any problems, but it indicates the
			// structure was never set
			if (dev_cfg->ohm.series_R == 0) {
				SETUP_ERR(i);
			}
			break;
#endif // USE_OHM_SENSOR

#if USE_LOG_BETA_SENSOR
		case SENS_LOG_BETA:
			// This would cause a divide-by-0 error if allowed
			if (dev_cfg->log_beta.beta == 0) {
				SETUP_ERR(i);
			}
#if USE_SMALL_SENSORS < 1
			s->B_div_T0 = FIXEDP_DIV(FIXEDP_FROM(dev_cfg->log_beta.beta), FIXEDP_FROM(dev_cfg->log_beta.ref));
			s->log_R0   = log_fixedp(FIXEDP_FROM(dev_cfg->log_beta.ref_R));
#endif // USE_SMALL_SENSORS < 1
			break;
#endif // USE_LOG_BETA_SENSOR

#if USE_LINEAR_V_SENSOR
		case SENS_LINEAR_V:
			// This would cause a divide-by-0 error if allowed
			if (dev_cfg->linear.slope == 0) {
				SETUP_ERR(i);
			}
			break;
#endif // USE_LINEAR_V_SENSOR

#if USE_LINEAR_R_SENSOR
		case SENS_LINEAR_R:
			// This would cause a divide-by-0 error if allowed
			if (dev_cfg->linear.slope == 0) {
				SETUP_ERR(i);
			}
			// This wouldn't actually cause any problems, but it indicates the
			// structure was never set
			if (dev_cfg->linear.calibration == 0) {
				SETUP_ERR(i);
			}
			break;
#endif // USE_LINEAR_R_SENSOR

#if USE_LOOKUP_R_SENSOR
		case SENS_LOOKUP_R:
			// This wouldn't actually cause any problems, but it indicates the
			// structure was never set
			if (dev_cfg->lut.calibration == 0) {
				SETUP_ERR(i);
			}
			break;
#endif // USE_LOOKUP_R_SENSOR

#if USE_LOOKUP_R_SENSOR
		case SENS_LOOKUP_V:
			// Nothing to do here
			break;
#endif // USE_LOOKUP_R_SENSOR

		case SENS_NONE:
			SETUP_ERR(i);
			break;

		default:
			UNKNOWN_MSG(cfg->type);
			SETUP_ERR(i);
			break;
		}
	}

	return;
}

void check_sensors() {
	imath_t value[SENSOR_COUNT];

	if (!RTC_TIMES_UP(cooldown)) {
		LOGGER("Not updating sensor status; cooldown in effect");
		return;
	}

	LOGGER("Updating sensor status");

	power_on();

#if CHECK_VREF
	adc_read_internals(&G_vcc_voltage, &G_mcu_temp);
#endif // CHECK_VREF

	for (uiter_t i = 0; i < SENSOR_COUNT; ++i) {
		value[i] = read_sensor(&G_sensors[i]);
	}
	power_off();
	cooldown = SET_RTC_TIMEOUT(SENS_COOLDOWN);

	for (uiter_t i = 0; i < SENSOR_COUNT; ++i) {
		update_sensor(&G_sensors[i], value[i]);
	}

	return;
}
void invalidate_sensors(void) {
	cooldown = 0;

	return;
}

static imath_t read_sensor(sensor_t *s) {
	imath_t value;
	const sensor_static_t *cfg;

	cfg = &SENSORS[GET_SENSOR_I(s)];

	switch (cfg->type) {
#if USE_BINARY_SENSOR
	case SENS_BINARY:
		power_on_input(cfg->pin);
		value = gpio_get_state(cfg->pin);
		power_off_input(cfg->pin);
		break;
#endif // USE_BINARY_SENSOR

#if USE_ADC
	default:
		gpio_set_mode(cfg->pin, GPIO_MODE_AIN, GPIO_FLOAT);
		value = adc_read_pin(cfg->pin);
		gpio_set_mode(cfg->pin, GPIO_MODE_HiZ, GPIO_FLOAT);
		break;

#else // !USE_ADC
	default:
		UNKNOWN_MSG(cfg->type);
		value = -1;
		break;
#endif // USE_ADC
	}

	return value;
}

static void update_sensor(sensor_t *s, imath_t value) {
	const sensor_static_t *cfg;

	cfg = &SENSORS[GET_SENSOR_I(s)];

	switch (cfg->type) {
#if USE_VOLT_SENSOR
	case SENS_VOLT:
		calculate_millivolts(s, value);
		break;
#endif // USE_VOLT_SENSOR

#if USE_OHM_SENSOR
	case SENS_OHM:
		calculate_ohms(s, value);
		break;
#endif // USE_OHM_SENSOR

#if USE_LOG_BETA_SENSOR
	case SENS_LOG_BETA:
		calculate_log_beta(s, value);
		break;
#endif // USE_LOG_BETA_SENSOR

#if USE_LINEAR_V_SENSOR
	case SENS_LINEAR_V:
		calculate_linearV(s, value);
		break;
#endif // USE_LINEAR_V_SENSOR

#if USE_LINEAR_R_SENSOR
	case SENS_LINEAR_R:
		calculate_linearR(s, value);
		break;
#endif // USE_LINEAR_R_SENSOR

#if USE_LOOKUP_R_SENSOR
	case SENS_LOOKUP_R:
		calculate_lookupR(s, value);
		break;
#endif // USE_LOOKUP_R_SENSOR

#if USE_LOOKUP_V_SENSOR
	case SENS_LOOKUP_V:
		calculate_lookupV(s, value);
		break;
#endif // USE_LOOKUP_V_SENSOR

#if USE_BINARY_SENSOR
	case SENS_BINARY:
		calculate_binary(s, value);
		break;
#endif // USE_BINARY_SENSOR

	default:
		UNKNOWN_MSG(cfg->type);
		break;
	}

	update_sensor_warning(s);

	return;
}
static void update_sensor_warning(sensor_t *s) {
#if USE_SMALL_SENSORS < 2
	bool high, low, inside;
	const sensor_static_t *cfg;

	cfg = &SENSORS[GET_SENSOR_I(s)];

	if (!BIT_IS_SET(s->iflags, SENS_FLAG_MONITORED)) {
		return;
	}

	high = (cfg->warn_above == SENS_THRESHOLD_IGNORE) ? false : (s->status > cfg->warn_above);
	low  = (cfg->warn_below == SENS_THRESHOLD_IGNORE) ? false : (s->status < cfg->warn_below);
	inside = ((cfg->warn_above != SENS_THRESHOLD_IGNORE) && (cfg->warn_below != SENS_THRESHOLD_IGNORE) && (cfg->warn_above < cfg->warn_below));

	CLEAR_BIT(s->iflags, SENS_FLAG_WARNING);
	if (inside) {
		if (high && low) {
			SET_BIT(s->iflags, SENS_FLAG_WARNING);
		}
	} else if (high || low) {
		SET_BIT(s->iflags, SENS_FLAG_WARNING);
	}
#endif // USE_SMALL_SENSORS < 2

	return;
}

#if USE_VOLT_SENSOR
static void calculate_millivolts(sensor_t *s, imath_t adc) {
	status_t adjust;
	const sensor_static_t *cfg;

	assert(s != NULL);
	assert(adc <= ADC_MAX);

	cfg = &SENSORS[GET_SENSOR_I(s)];

#if USE_SMALL_SENSORS < 2
	adjust = cfg->adjust;
#else
	adjust = 0;
#endif

	if (BIT_IS_SET(cfg->cflags, SENS_FLAG_INVERT)) {
		adc = ADC_MAX - adc;
	}

	s->status = SCALE_INT(ADC_TO_VOLTAGE(adc) + adjust);

	return;
}
#endif // USE_VOLT_SENSOR

#if USE_OHM_SENSOR
static void calculate_ohms(sensor_t *s, imath_t adc) {
	imath_t adjust;
	imath_t r, sR;
	const sensor_opt_ohm_t *dev_cfg;
	const sensor_static_t *cfg;

	assert(s != NULL);
	assert(adc <= ADC_MAX);

	cfg = &SENSORS[GET_SENSOR_I(s)];
	dev_cfg = &cfg->devcfg.ohm;

#if USE_SMALL_SENSORS < 2
	adjust = cfg->adjust;
#else
	adjust = 0;
#endif
	sR = dev_cfg->series_R;

	if (BIT_IS_SET(cfg->cflags, SENS_FLAG_INVERT)) {
		adc = ADC_MAX - adc;
	}
	// adc == ADC_MAX would cause a divide-by-0 in the R calculation
	if (adc == ADC_MAX) {
		// Resistances might plausibly be much higher than SENSOR_HIGH, so use
		// STATUS_MAX instead
		s->status = STATUS_MAX;
		return;
	}
	r = ADC_TO_R(adc, sR);

	s->status = SCALE_INT(r + adjust);

	return;
}
#endif // USE_OHM_SENSOR

#if USE_LINEAR_V_SENSOR
static void calculate_linearV(sensor_t *s, imath_t adc) {
	imath_t adjust;
	imath_t ref, refV, slope;
	imath_t maxV;
	const sensor_opt_linear_t *dev_cfg;
	const sensor_static_t *cfg;

	assert(s != NULL);
	assert(adc <= ADC_MAX);

	cfg = &SENSORS[GET_SENSOR_I(s)];
	dev_cfg = &cfg->devcfg.linear;

#if USE_SMALL_SENSORS < 2
	adjust = cfg->adjust;
#else
	adjust = 0;
#endif
	ref   = dev_cfg->ref;
	refV  = dev_cfg->ref_value;
	maxV  = dev_cfg->calibration;
	slope = dev_cfg->slope;
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
#endif // USE_LINEAR_V_SENSOR

#if USE_LINEAR_R_SENSOR
static void calculate_linearR(sensor_t *s, imath_t adc) {
	imath_t adjust;
	imath_t r, slope, ref, refR, sR;
	const sensor_opt_linear_t *dev_cfg;
	const sensor_static_t *cfg;

	assert(s != NULL);
	assert(adc <= ADC_MAX);

	cfg = &SENSORS[GET_SENSOR_I(s)];
	dev_cfg = &cfg->devcfg.linear;

#if USE_SMALL_SENSORS < 2
	adjust = cfg->adjust;
#else
	adjust = 0;
#endif
	slope = dev_cfg->slope;
	ref   = dev_cfg->ref;
	refR  = dev_cfg->ref_value;
	sR    = dev_cfg->calibration;

	if (BIT_IS_SET(cfg->cflags, SENS_FLAG_INVERT)) {
		adc = ADC_MAX - adc;
	}
	// adc == ADC_MAX would cause a divide-by-0 in the R calculation
	if (adc == ADC_MAX) {
		s->status = (slope > 0) ? SENSOR_HIGH : SENSOR_LOW;
		return;
	}

	r = ADC_TO_R(adc, sR);
	// y = mx+b
	// value = ((1/slope)*(R-refR)) + ref
	// slope is 1/10 ohms so multiply numerator by 10 compensate
	s->status = (SCALE_INT((r - refR) * 10) / slope) + SCALE_INT(ref + adjust);

	return;
}
#endif // USE_LINEAR_R_SENSOR

#if USE_LOG_BETA_SENSOR
static void calculate_log_beta(sensor_t *s, imath_t adc) {
	FIXEDP_ITYPE r, beta, tmp, adjust, BdivT0, logR0;
	imath_t sR;
	const sensor_opt_log_beta_t *dev_cfg;
	const sensor_static_t *cfg;

	assert(s != NULL);
	assert(adc <= ADC_MAX);

	cfg = &SENSORS[GET_SENSOR_I(s)];
	dev_cfg = &cfg->devcfg.log_beta;

#if USE_SMALL_SENSORS < 1
	BdivT0 = s->B_div_T0;
	logR0 = s->log_R0;
#else
	BdivT0 = FIXEDP_DIV(FIXEDP_FROM(dev_cfg->beta), FIXEDP_FROM(dev_cfg->ref));
	logR0   = log_fixedp(FIXEDP_FROM(dev_cfg->ref_R));
#endif

#if USE_SMALL_SENSORS < 2
	adjust = FIXEDP_FROM(cfg->adjust);
#else
	adjust = 0;
#endif
	beta = FIXEDP_FROM(dev_cfg->beta);
	// ref  = FIXEDP_FROM(dev_cfg->ref);
	// refR = FIXEDP_FROM(dev_cfg->ref_R);
	sR   = dev_cfg->series_R;

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
	// Do this in steps to simplify things:
	//
	// (B/T0) + log(R1 / R0)
	// Due to library constraints:
	// (B/T0) + (log(R) - log(R0))
	tmp = BdivT0 + (log_fixedp(r) - logR0);
	//
	// B / ^
	if (tmp != 0) {
		tmp = FIXEDP_DIV(beta, tmp);
	} else {
		s->status = (beta > 0) ? SENSOR_HIGH : SENSOR_LOW;
		return;
	}

	s->status = FIXEDP_AWAY(SCALE_FIXED(tmp + adjust));

	return;
}
#endif // USE_LOG_BETA_SENSOR

#if USE_LOOKUP_R_SENSOR
static void calculate_lookupR(sensor_t *s, imath_t adc) {
	imath_t status, adjust;
	imath_t r, sR;
	imath_t min, max, size, scale;
	const LUT_T *lut;
	const sensor_opt_lut_t *dev_cfg;
	const sensor_LUT_t *table_cfg;
	const sensor_static_t *cfg;

	assert(s != NULL);
	assert(adc <= ADC_MAX);

	cfg = &SENSORS[GET_SENSOR_I(s)];
	dev_cfg = &cfg->devcfg.lut;
	table_cfg = &LOOKUP_TABLES[dev_cfg->lutno];

#if USE_SMALL_SENSORS < 2
	adjust = cfg->adjust;
#else
	adjust = 0;
#endif
	sR     = dev_cfg->calibration;
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
#endif // USE_LOOKUP_R_SENSOR

#if USE_LOOKUP_V_SENSOR
static void calculate_lookupV(sensor_t *s, imath_t adc) {
	imath_t status, adjust, calibration;
	imath_t min, max, Vref, size, scale;
	const LUT_T *lut;
	const sensor_opt_lut_t *dev_cfg;
	const sensor_LUT_t *table_cfg;
	const sensor_static_t *cfg;

	assert(s != NULL);
	assert(adc <= ADC_MAX);

	cfg = &SENSORS[GET_SENSOR_I(s)];
	dev_cfg = &cfg->devcfg.lut;
	table_cfg = &LOOKUP_TABLES[dev_cfg->lutno];

#if USE_SMALL_SENSORS < 2
	adjust = cfg->adjust;
#else
	adjust = 0;
#endif
	calibration = dev_cfg->calibration;
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
#endif // USE_LOOKUP_V_SENSOR

#if USE_BINARY_SENSOR
static void calculate_binary(sensor_t *s, imath_t value) {
	const sensor_static_t *cfg;

	cfg = &SENSORS[GET_SENSOR_I(s)];

	if (!BIT_IS_SET(cfg->cflags, SENS_FLAG_INVERT)) {
		switch (value) {
		case GPIO_HIGH:
			s->status = 1;
			break;
		case GPIO_LOW:
			s->status = 0;
			break;
		default:
			s->status = -1;
			break;
		}
	} else {
		switch (value) {
		case GPIO_HIGH:
			s->status = 0;
			break;
		case GPIO_LOW:
			s->status = 1;
			break;
		default:
			s->status = -1;
			break;
		}
	}

	return;
}
#endif // USE_BINARY_SENSOR

void check_sensor_warnings(void) {
	CLEAR_BIT(G_warnings, (WARN_BATTERY_LOW|WARN_VCC_LOW|WARN_SENSOR));

	if (G_vcc_voltage < REGULATED_VOLTAGE_LOW) {
		SET_BIT(G_warnings, WARN_VCC_LOW);
	}

#if USE_SMALL_SENSORS < 2
	sensor_t *s;
	const sensor_static_t *cfg;

	for (uiter_t i = 0; i < SENSOR_COUNT; ++i) {
		s = &G_sensors[i];
		cfg = &SENSORS[i];

		// Batteries are checked even if they're not flagged for monitoring
		// otherwise
		if (BIT_IS_SET(cfg->cflags, SENS_FLAG_BATTERY)) {
			if (s->status < cfg->warn_below) {
				SET_BIT(G_warnings, WARN_BATTERY_LOW);
				// If there's already a sensor warning there's no reason to check
				// the rest of the sensors
				if (BIT_IS_SET(G_warnings, WARN_SENSOR)) {
					break;
				}
			}
		}

		if (BIT_IS_SET(s->iflags, SENS_FLAG_WARNING)) {
			SET_BIT(G_warnings, WARN_SENSOR);
			// If there's already a battery warning there's no reason to check the
			// rest of the sensors
			if (BIT_IS_SET(G_warnings, WARN_BATTERY_LOW)) {
				break;
			}
		}
	}
#endif // USE_SMALL_SENSORS < 2

	return;
}

static void power_on(void) {
	power_on_sensors();
	adc_on();

	return;
}
static void power_off(void) {
	power_off_sensors();
	adc_off();

	return;
}


#ifdef __cplusplus
 }
#endif
