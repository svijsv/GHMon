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
static _FLASH const char l_invalid_msg[] = "Invalid sensor %u configuration";
static _FLASH const char e_invalid_msg[] = "Invalid sensor configuration";
#define SETUP_ERR(i) \
	LOGGER_NOF(FROM_FSTR(l_invalid_msg), (uint )(i)); \
	ERROR_STATE_NOF(FROM_FSTR(e_invalid_msg))

static _FLASH const char unknown_msg[] = "Unknown sensor type '%u' at " __FILE__ ":%u";
#define UNKNOWN_MSG(i) LOGGER_NOF(FROM_FSTR(unknown_msg), (uint )(i), (uint )__LINE__);


/*
* Types
*/
// The type used for integer math
typedef adcm_t imath_t;

/*
* Variables
*/
int16_t G_vcc_voltage = REGULATED_VOLTAGE;

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
#if USE_DHT11_SENSOR
static void read_dht11_sensor(pin_t pin);
#endif
#if USE_BMx280_SPI_SENSOR
static void read_bmx280_spi_sensor(sensor_t *s);
#endif
#if USE_BMx280_I2C_SENSOR
static void read_bmx280_i2c_sensor(sensor_t *s);
#endif


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
	_FLASH const sensor_devcfg_t *dev_cfg;
	_FLASH const sensor_static_t *cfg;

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

#if USE_BINARY_SENSOR
		case SENS_BINARY:
			// Nothing to do here
			break;
#endif

#if USE_DHT11_SENSOR
		case SENS_DHT11_HUM:
		case SENS_DHT11_TEMP:
			// Nothing to do here
			break;
#endif

#if USE_BMx280_SPI_SENSOR
		case SENS_BMx280_SPI_PRESSURE:
		case SENS_BMx280_SPI_TEMP:
		case SENS_BMx280_SPI_HUM:
			SET_BIT(s->iflags, SENS_FLAG_SPI);
#if ! SPI_POWER_PIN
			// The CS pin needs to be pulled low once to put the device into
			// SPI mode
			gpio_set_mode(cfg->pin, GPIO_MODE_PP, GPIO_LOW);
			gpio_set_state(cfg->pin, GPIO_HIGH);
#endif
			break;
#endif

#if USE_BMx280_I2C_SENSOR
		case SENS_BMx280_I2C_PRESSURE:
		case SENS_BMx280_I2C_TEMP:
		case SENS_BMx280_I2C_HUM:
			SET_BIT(s->iflags, SENS_FLAG_I2C);
			// The pin is actually the I2C address, which is 0x77 or 0x76 for
			// both the BMP280 and BME280
			if ((cfg->pin != 0x77) && (cfg->pin != 0x76)) {
				SETUP_ERR(i);
			}
			break;
#endif

		case SENS_NONE:
			SETUP_ERR(i);
			break;

		default:
			UNKNOWN_MSG(cfg->type);
			SETUP_ERR(i);
			break;
		}
	}

	power_off_sensors();

#if CALIBRATE_VREF == 1
	adc_on();
	G_vcc_voltage = adc_read_vref_mV();
	adc_off();
#endif

	return;
}

void check_sensors() {
	imath_t value[SENSOR_COUNT];

	if (!RTC_TIMES_UP(cooldown)) {
		LOGGER("Not updating sensor status; cooldown in effect");
		return;
	}

	LOGGER("Updating sensor status");

	for (uiter_t i = 0; i < SENSOR_COUNT; ++i) {
		CLEAR_BIT(G_sensors[i].iflags, SENS_FLAG_DONE);
	}
	power_on_sensors();

#if USE_DHT11_SENSOR
	// Per the DHT11 data sheet, don't send any instructions for at least 1
	// second after power-on
	// Only sleeping for 1s doesn't seem to reliably work
	sleep(1500);
#endif

#if CALIBRATE_VREF >= 2
	G_vcc_voltage = adc_read_vref_mV();
#endif

	for (uiter_t i = 0; i < SENSOR_COUNT; ++i) {
		value[i] = read_sensor(&G_sensors[i]);
	}
	power_off_sensors();
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
	_FLASH const sensor_static_t *cfg;

	cfg = &SENSORS[GET_SENSOR_I(s)];

	switch (cfg->type) {
#if USE_BINARY_SENSOR
	case SENS_BINARY:
		power_on_input(cfg->pin);
		value = gpio_get_state(cfg->pin);
		power_off_input(cfg->pin);
		break;
#endif // USE_BINARY_SENSOR

#if USE_DHT11_SENSOR
	case SENS_DHT11_HUM:
	case SENS_DHT11_TEMP:
		if (!BIT_IS_SET(s->iflags, SENS_FLAG_DONE)) {
			// The mode of the input pin needs to be changed in order to trigger
			// the read in read_dht11_sensor(), so don't bother setting it here
			//power_on_input(cfg->pin);
			read_dht11_sensor(cfg->pin);
			//power_off_input(cfg->pin);
		}
		// The sensor values are set in read_dht11_sensor()
		value = 0;
		break;
#endif // USE_DHT11_SENSOR

#if USE_BMx280_SPI_SENSOR
	case SENS_BMx280_SPI_PRESSURE:
	case SENS_BMx280_SPI_TEMP:
	case SENS_BMx280_SPI_HUM:
		if (!BIT_IS_SET(s->iflags, SENS_FLAG_DONE)) {
			read_bmx280_spi_sensor(s);
		}
		// The sensor values are set in read_bmx280_sensor()
		value = 0;
		break;
#endif // USE_BMx280_SPI_SENSOR

#if USE_BMx280_I2C_SENSOR
	case SENS_BMx280_I2C_PRESSURE:
	case SENS_BMx280_I2C_TEMP:
	case SENS_BMx280_I2C_HUM:
		if (!BIT_IS_SET(s->iflags, SENS_FLAG_DONE)) {
			read_bmx280_i2c_sensor(s);
		}
		// The sensor values are set in read_bmx280_sensor()
		value = 0;
		break;
#endif // USE_BMx280_I2C_SENSOR

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
	_FLASH const sensor_static_t *cfg;

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

#if USE_DHT11_SENSOR
	case SENS_DHT11_HUM:
	case SENS_DHT11_TEMP:
		// Nothing to do here; the statuses were set when they were read
		break;
#endif

#if USE_BMx280_SPI_SENSOR
	case SENS_BMx280_SPI_PRESSURE:
	case SENS_BMx280_SPI_TEMP:
	case SENS_BMx280_SPI_HUM:
		// Nothing to do here; the statuses were set when they were read
		break;
#endif

#if USE_BMx280_I2C_SENSOR
	case SENS_BMx280_I2C_PRESSURE:
	case SENS_BMx280_I2C_TEMP:
	case SENS_BMx280_I2C_HUM:
		// Nothing to do here; the statuses were set when they were read
		break;
#endif

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
	_FLASH const sensor_static_t *cfg;

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
	_FLASH const sensor_static_t *cfg;

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
	_FLASH const sensor_opt_ohm_t *dev_cfg;
	_FLASH const sensor_static_t *cfg;

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
	_FLASH const sensor_opt_linear_t *dev_cfg;
	_FLASH const sensor_static_t *cfg;

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
	_FLASH const sensor_opt_linear_t *dev_cfg;
	_FLASH const sensor_static_t *cfg;

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
	_FLASH const sensor_opt_log_beta_t *dev_cfg;
	_FLASH const sensor_static_t *cfg;

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
	_FLASH const LUT_T *lut;
	_FLASH const sensor_opt_lut_t *dev_cfg;
	_FLASH const sensor_LUT_t *table_cfg;
	_FLASH const sensor_static_t *cfg;

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
	_FLASH const LUT_T *lut;
	_FLASH const sensor_opt_lut_t *dev_cfg;
	_FLASH const sensor_LUT_t *table_cfg;
	_FLASH const sensor_static_t *cfg;

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
	_FLASH const sensor_static_t *cfg;

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

#if USE_DHT11_SENSOR
static void read_dht11_sensor(pin_t pin) {
	utime_t timeout;
	uint8_t us_count;
	uint8_t reading[5] = { 0 };
	gpio_quick_t qpin;

	// Pull the data pin low at least 18ms to signal the sensor to start
	gpio_set_mode(pin, GPIO_MODE_PP, GPIO_LOW);
	sleep(20);

	uscounter_on();
	gpio_quickread_prepare(&qpin, pin);
	timeout = SET_TIMEOUT(1000);
	power_on_input(pin);

	// Wait for sensor to pull data low, 20-40us
	do {
		if (TIMES_UP(timeout)) {
			goto END;
		}
	} while (GPIO_QUICK_READ(qpin) == GPIO_HIGH);
	// Wait for data to go high again, at least 80us
	do {
		if (TIMES_UP(timeout)) {
			goto END;
		}
	} while (GPIO_QUICK_READ(qpin) == GPIO_LOW);
	// Wait for data to go low, indicating transmission will begin
	do {
		if (TIMES_UP(timeout)) {
			goto END;
		}
	} while (GPIO_QUICK_READ(qpin) == GPIO_HIGH);

	// Read the 5 bytes: humidity x 2, temperature x2, checksum
	for (uiter_t i = 0; i != 5; ++i) {
		for (uiter_t j = 8; j != 0; --j) {
			// Wait for data to go high, 50us
			do {
				if (TIMES_UP(timeout)) {
					goto END;
				}
			} while (GPIO_QUICK_READ(qpin) == GPIO_LOW);

			// Wait for data to go low, 26-70us
			USCOUNTER_START();
			do {
				if (TIMES_UP(timeout)) {
					goto END;
				}
			} while (GPIO_QUICK_READ(qpin) == GPIO_HIGH);
			USCOUNTER_STOP(us_count);

			// A high period of 26-28us is 0 and 70us is 1
			reading[i] = (reading[i] << 1) | (us_count > 35);
		}
	}

	/*
	// Wait for data to go high, 50us
	do {
		if (TIMES_UP(timeout)) {
			goto END;
		}
	} while (GPIO_QUICK_READ(qpin) == GPIO_LOW);
	*/

END:
	power_off_input(pin);
	uscounter_off();

#if DEBUG
	uint16_t cksum = reading[0] + reading[1] + reading[2] + reading[3];
	if ((cksum & 0xFF) != reading[4]) {
		LOGGER("DHT11 sensor on pin 0x%02X: invalid checksum: have %u, expected %u", (uint )pin, (uint )(cksum & 0xFF), (uint )reading[4]);
	}
	if ((reading[0] == 0) && (reading[1] == 0) && (reading[2] == 0) && (reading[3] == 0) && (reading[4] == 0)) {
		LOGGER("DHT11 sensor on pin 0x%02X: all readings were 0", (uint )pin);
	}
#endif

	for (uiter_t i = 0; i < SENSOR_COUNT; ++i) {
		if (PINID(SENSORS[i].pin) == PINID(pin)) {
			imath_t adjust, tmp;
			_FLASH const sensor_static_t *cfg;

			cfg = &SENSORS[i];
#if USE_SMALL_SENSORS < 2
			// Adjust the adjustment to compensate for the shift used to deal
			// with the fractional part of the response
			adjust = (uint16_t )cfg->adjust << 8;
#else
			adjust = 0;
#endif

			switch (cfg->type) {
			case SENS_DHT11_HUM:
				SET_BIT(G_sensors[i].iflags, SENS_FLAG_DONE);
				// To preserve the fraction part for scaling, shift it all left
				// then right - effectively multiply by 2^8 then divide by same
				tmp = ((uint16_t )reading[0] << 8) | reading[1];
				G_sensors[i].status = (SCALE_INT(tmp + adjust)) >> 8;
				break;
			case SENS_DHT11_TEMP:
				SET_BIT(G_sensors[i].iflags, SENS_FLAG_DONE);
				tmp = ((uint16_t )reading[2] << 8) | reading[3];
				G_sensors[i].status = (SCALE_INT(tmp + adjust)) >> 8;
				break;
			// There may be accidental matches with non-pin 'pins' like the
			// BMP280's I2C address
			/*
			default:
				UNKNOWN_MSG(cfg->type);
				break;
			*/
			}
		}
	}

	return;
}
#endif // USE_DHT11_SENSOR

#if USE_BMx280_SPI_SENSOR || USE_BMx280_SPI_SENSOR
typedef struct {
	int32_t temp, press, hum;
} bmx280_status_t;
typedef struct {
	uint8_t adc[8];
	uint8_t cal[33];
} bmx280_raw_t;

//
// Calculate temperature, pressure, and humidity from the raw measurements
// Adapted from the code given in the data sheet and in the Bosh Sensortec
// github repos
//
// The github code uses division instead of right shifts (I assume because
// someone had a problem with the sign bit at some point) but I'm sticking
// with the shifts for now because the resulting binary is smaller.
//
// Again, sorry for the names, not my doing
static void calculate_bmx280_sensor(bmx280_status_t *status, bmx280_raw_t *raw, bool do_humidity) {
	int32_t temp = 0;
	uint32_t press = 0, hum = 0;
	uint32_t t_adc, p_adc, h_adc;
	// Don't blame me for the names, I copied them out of the datasheet
	// And why are T1 and P1 unsigned? they're cast to signed the only places
	// they're used.
	uint16_t dig_T1, dig_P1;
	int16_t dig_T2, dig_T3, dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9, dig_H2, dig_H4, dig_H5;
	uint8_t dig_H1, dig_H3;
	int8_t dig_H6;
	int32_t var1, var2, var3, var4, var5, t_fine = 0, tmp;

	assert(raw != NULL);
	assert(status != NULL);

	//
	// Calibration data; 33ish bytes
	// The MSB is the higher byte for calibration data
	//
	// Temperature calibration; 3 16-bit words
	READ_SPLIT_U16(dig_T1, raw->cal[1],  raw->cal[0]);
	READ_SPLIT_S16(dig_T2, raw->cal[3],  raw->cal[2]);
	READ_SPLIT_S16(dig_T3, raw->cal[5],  raw->cal[4]);
	//
	// Pressure calibration; 9 16-bit words
	READ_SPLIT_U16(dig_P1, raw->cal[7],  raw->cal[6]);
	READ_SPLIT_S16(dig_P2, raw->cal[9],  raw->cal[8]);
	READ_SPLIT_S16(dig_P3, raw->cal[11], raw->cal[10]);
	READ_SPLIT_S16(dig_P4, raw->cal[13], raw->cal[12]);
	READ_SPLIT_S16(dig_P5, raw->cal[15], raw->cal[14]);
	READ_SPLIT_S16(dig_P6, raw->cal[17], raw->cal[16]);
	READ_SPLIT_S16(dig_P7, raw->cal[19], raw->cal[18]);
	READ_SPLIT_S16(dig_P8, raw->cal[21], raw->cal[20]);
	READ_SPLIT_S16(dig_P9, raw->cal[23], raw->cal[22]);
	//
	// Humidity calibration; 8 bytes
	// cal[24] is unused
	dig_H1 = (int8_t )(raw->cal[25]);
	READ_SPLIT_S16(dig_H2, raw->cal[27], raw->cal[26]);
	dig_H3 = raw->cal[28];
	dig_H4 = (((int16_t )(raw->cal[29])) << 4) | ((int16_t )(raw->cal[30] & 0x0F));
	dig_H5 = (((int16_t )(raw->cal[31])) << 4) | (((int16_t )(raw->cal[30] & 0xF0)) >> 4);
	dig_H6 = (int8_t )(raw->cal[32]);

	//
	// Sensor measurement data; 8 bytes
	// The MSB is the lower byte for measurement data
	// Pressure measurement
	p_adc = ((uint32_t )raw->adc[0] << 12) | ((uint32_t )raw->adc[1] << 4) | ((uint32_t )raw->adc[2] >> 4);
	// Temperature measurement
	t_adc = ((uint32_t )raw->adc[3] << 12) | ((uint32_t )raw->adc[4] << 4) | ((uint32_t )raw->adc[5] >> 4);
	// Humidity measurement
	h_adc = ((uint32_t )raw->adc[6] << 8) | ((uint32_t )raw->adc[7]);

	// The readings for pressure, temperature, and humidity are 0x80000,
	// 0x80000, and 0x8000 respectively when they're skipped and 0xFFFFF,
	// 0xFFFFF, and 0xFFFF when the sensor is absent; it's easier to track this
	// stuff by just setting them to 0 though
	if ((t_adc == 0x80000) || (t_adc == 0xFFFFF)) {
		t_adc = 0;
	}
	if ((p_adc == 0x80000) || (p_adc == 0xFFFFF)) {
		p_adc = 0;
	}
	if ((h_adc == 0x8000) || (h_adc == 0xFFFF)) {
		h_adc = 0;
	}

	//
	// Temperature; result is DegC * 100
	if (t_adc != 0) {
		var1 = ((((t_adc >> 3) - ((int32_t )dig_T1 << 1))) * ((int32_t )dig_T2)) >> 11;
		tmp = (t_adc >> 4) - (int32_t )dig_T1;
		var2 = (((tmp * tmp) >> 12) * (int32_t )dig_T3) >> 14;
		t_fine = var1 + var2;
		temp = ((t_fine * 5) + 128) >> 8;
	}
	//
	// Pressure; result is Pa (hPa * 100, or millibars * 100)
	if (p_adc != 0) {
		var1 = (t_fine >> 1) - (int32_t )64000;
		tmp = var1 >> 2;
		var2 = (tmp * tmp) * (int32_t )dig_P6;
		var2 = var2 + ((var1 * ((int32_t )dig_P5)) << 1);
		var2 = (var2 >> 2) + (((int32_t )dig_P4) << 16);
		var1 = (((dig_P3 * ((tmp * tmp) >> 13)) >> 3) + ((((int32_t )dig_P2) * var1) >> 1)) >> 18;
		var1 = ((((32768 + var1)) * ((int32_t )dig_P1)) >> 15);
		if (var1 != 0) {
			press = (((uint32_t )(((int32_t )1048576) - p_adc) - (var2 >> 12))) * 3125;
			if (press < 0x80000000) {
				press = (press << 1) / ((uint32_t )var1);
			} else {
				press = (press / (uint32_t )var1) * 2;
			}
			var1 = (((int32_t )dig_P9) * ((int32_t )(((press >> 3) * (press >> 3)) >> 13))) >> 12;
			var2 = (((int32_t )(press >> 2)) * ((int32_t )dig_P8)) >> 13;
			press = (uint32_t )((int32_t )press + ((var1 + var2 + dig_P7) >> 4));
		} else {
			press = 0;
		}
	}
	//
	// Humidity; %RH in 22.10 fixed-point format (or %RH * 1024)
	if (do_humidity && (h_adc != 0)) {
		var1 = t_fine - ((int32_t )76800);
		var2 = (int32_t )(h_adc << 14);
		var3 = (int32_t )(((int32_t )dig_H4) << 20);
		var4 = ((int32_t )dig_H5) * var1;
		var5 = (((var2 - var3) - var4) + (int32_t )16384) >> 15;
		var2 = (var1 * ((int32_t )dig_H6)) >> 10;
		var3 = (var1 * ((int32_t )dig_H3)) >> 11;
		var4 = ((var2 * (var3 + (int32_t )32768)) >> 10) + (int32_t )2097152;
		var2 = ((var4 * ((int32_t )dig_H2)) + 8192) >> 14;
		var3 = var5 * var2;
		tmp = (var3 >> 15);
		var4 = (tmp * tmp) / 128;
		var5 = var3 - ((var4 * ((int32_t )dig_H1)) >> 4);
		var5 = (var5 < 0 ? 0 : var5);
		var5 = (var5 > 419430400 ? 419430400 : var5);
		hum = (uint32_t )(var5 >> 12);
	}

	status->temp = temp;
	status->press = press;
	status->hum = hum;

	return;
}
#endif // USE_BMx280_SPI_SENSOR || USE_BMx280_SPI_SENSOR

#if USE_BMx280_SPI_SENSOR
static void read_bmx280_spi_sensor(sensor_t *s) {
	pin_t pin;
	uint8_t byte, cmd[2];
	bmx280_raw_t raw = { { 0 }, { 0 } };
	bmx280_status_t status = { 0 };
	bool do_humidity = false;
	utime_t timeout;
	_FLASH const sensor_static_t *cfg;

	cfg = &SENSORS[GET_SENSOR_I(s)];
	pin = cfg->pin;

	gpio_set_mode(pin, GPIO_MODE_PP, GPIO_HIGH);

	// Check if this is a BMP280 (pressure and temperature) or a BME280 (that
	// plus humidity)
	gpio_set_state(pin, GPIO_LOW);
	spi_exchange_byte((0xD0 | 0x80), &byte, 100);
	spi_exchange_byte(0xFF, &byte, 100);
	gpio_set_state(pin, GPIO_HIGH);
	if (byte == 0x60) {
		do_humidity = true;
	}

	if (do_humidity) {
		// Set ctrl_hum to oversample humidity 1x
		// This doesn't take effect until ctrl_meas has been set
		gpio_set_state(pin, GPIO_LOW);
		cmd[0] = (0xF2 & 0x7F);
		cmd[1] = (0b001);
		spi_transmit_block(cmd, 2, 100);
		gpio_set_state(pin, GPIO_HIGH);
	}
	//
	// Set ctrl_meas register to oversample temperature and pressure 1x and
	// begin measuring
	// The most significant byte of the register address is used to indicate
	// read/write mode: 0 is write, 1 is read
	gpio_set_state(pin, GPIO_LOW);
	cmd[0] = (0xF4 & 0x7F);
	cmd[1] = (0b00100101);
	spi_transmit_block(cmd, 2, 100);
	gpio_set_state(pin, GPIO_HIGH);

	//
	// Wait for measurement to finish
	// The datasheet gives 5.5-6.4ms measurement time for 1x oversampling
	timeout = SET_TIMEOUT(100);
	do {
		delay(10);
		gpio_set_state(pin, GPIO_LOW);

		spi_exchange_byte((0xF3 | 0x80), &byte, 100);
		spi_exchange_byte(0xFF, &byte, 100);

		gpio_set_state(pin, GPIO_HIGH);
	} while ((byte != 0) && (!TIMES_UP(timeout)));

	//
	// Read device calibration data part 1; 26 bytes
	gpio_set_state(pin, GPIO_LOW);
	spi_exchange_byte((0x88 | 0x80), &byte, 100);
	spi_receive_block(raw.cal, 26, 0xFF, 500);
	gpio_set_state(pin, GPIO_HIGH);
	//
	// Read device calibration data part 2; 16 bytes (but only 7 are used?)
	if (do_humidity) {
		gpio_set_state(pin, GPIO_LOW);
		spi_exchange_byte((0xE1 | 0x80), &byte, 100);
		spi_receive_block(&raw.cal[26], 7, 0xFF, 500);
		gpio_set_state(pin, GPIO_HIGH);
	}
	//
	// Read sensor measurement data; 8 bytes
	// The humidity register is read even when it's not supported so that the
	// registers can be read in a single go; it shouldn't hurt anything.
	gpio_set_state(pin, GPIO_LOW);
	spi_exchange_byte((0xF7 | 0x80), &byte, 100);
	spi_receive_block(raw.adc, 8, 0xFF, 500);
	gpio_set_state(pin, GPIO_HIGH);

	power_off_output(pin);

	calculate_bmx280_sensor(&status, &raw, do_humidity);
#if DEBUG
	if ((status.temp | status.press | status.hum) == 0) {
		LOGGER("BMx280_SPI sensor on pin 0x%02X: all readings were invalid", (uint )pin);
	}
#endif

	for (uiter_t i = 0; i < SENSOR_COUNT; ++i) {
		if (PINID(SENSORS[i].pin) == PINID(pin)) {
			imath_t adjust;

			cfg = &SENSORS[i];
#if USE_SMALL_SENSORS < 2
			adjust = cfg->adjust;
#else
			adjust = 0;
#endif
			switch (cfg->type) {
			case SENS_BMx280_SPI_PRESSURE:
				SET_BIT(G_sensors[i].iflags, SENS_FLAG_DONE);
				// Adjust the adjustment to compensate for the fractional part of
				// the measurement
				adjust *= 100;
				G_sensors[i].status = (SCALE_INT(status.press + adjust)) / 100;
				break;
			case SENS_BMx280_SPI_TEMP:
				SET_BIT(G_sensors[i].iflags, SENS_FLAG_DONE);
				adjust *= 100;
				G_sensors[i].status = (SCALE_INT(status.temp + adjust)) / 100;
				break;
			case SENS_BMx280_SPI_HUM:
				SET_BIT(G_sensors[i].iflags, SENS_FLAG_DONE);
				adjust <<= 10;
				G_sensors[i].status = (SCALE_INT(status.hum + adjust)) >> 10;
				break;
			// There may be accidental matches with other 'pins' like the
			// I2C address used by the sister function
			/*
			default:
				UNKNOWN_MSG(cfg->type);
				break;
			*/
			}
		}
	}

	return;
}
#endif // USE_BMx280_SPI_SENSOR

#if USE_BMx280_I2C_SENSOR
static void read_bmx280_i2c_sensor(sensor_t *s) {
	uint8_t addr, cmd[2];
	bmx280_raw_t raw = { { 0 }, { 0 } };
	bmx280_status_t status = { 0 };
	bool do_humidity = false;
	utime_t timeout;
	_FLASH const sensor_static_t *cfg;

	cfg = &SENSORS[GET_SENSOR_I(s)];
	addr = cfg->pin;

	// Check if this is a BMP280 (pressure and temperature) or a BME280 (that
	// plus humidity)
	cmd[0] = 0xD0;
	i2c_transmit_block(addr, cmd, 1, 100);
	i2c_receive_block(addr, cmd, 1, 100);
	if (cmd[0] == 0x60) {
		do_humidity = true;
	}

	if (do_humidity) {
		// Set ctrl_hum to oversample humidity 1x
		// This doesn't take effect until ctrl_meas has been set
		cmd[0] = 0xF2;
		cmd[1] = (0b001);
		i2c_transmit_block(addr, cmd, 2, 100);
	}
	//
	// Set ctrl_meas register to oversample temperature and pressure 1x and
	// begin measuring
	cmd[0] = 0xF4;
	cmd[1] = (0b00100101);
	i2c_transmit_block(addr, cmd, 2, 100);

	//
	// Wait for measurement to finish
	// The datasheet gives 5.5-6.4ms measurement time for 1x oversampling
	timeout = SET_TIMEOUT(100);
	do {
		delay(10);

		cmd[0] = 0xF3;
		i2c_transmit_block(addr, cmd, 1, 100);
		i2c_receive_block(addr, cmd, 1, 100);
	} while ((cmd[0] != 0) && (!TIMES_UP(timeout)));

	//
	// Read device calibration data part 1; 26 bytes
	cmd[0] = 0x88;
	i2c_transmit_block(addr, cmd, 1, 100);
	i2c_receive_block(addr, raw.cal, 26, 500);
	//
	// Read device calibration data part 2; 16 bytes (but only 7 are used?)
	if (do_humidity) {
		cmd[0] = 0xE1;
		i2c_transmit_block(addr, cmd, 1, 100);
		i2c_receive_block(addr, &raw.cal[26], 7, 500);
	}
	//
	// Read sensor measurement data; 8 bytes
	// The humidity register is read even when it's not supported so that the
	// registers can be read in a single go; it shouldn't hurt anything.
	cmd[0] = 0xF7;
	i2c_transmit_block(addr, cmd, 1, 100);
	i2c_receive_block(addr, raw.adc, 8, 500);

	calculate_bmx280_sensor(&status, &raw, do_humidity);
#if DEBUG
	if ((status.temp | status.press | status.hum) == 0) {
		LOGGER("BMx280_I2C sensor at addres 0x%02X: all readings were invalid", (uint )addr);
	}
#endif

	for (uiter_t i = 0; i < SENSOR_COUNT; ++i) {
		// The 'pin' is actually the I2C address so don't use PINID()
		if (SENSORS[i].pin == addr) {
			imath_t adjust;

			cfg = &SENSORS[i];
#if USE_SMALL_SENSORS < 2
			adjust = cfg->adjust;
#else
			adjust = 0;
#endif
			switch (cfg->type) {
			case SENS_BMx280_I2C_PRESSURE:
				SET_BIT(G_sensors[i].iflags, SENS_FLAG_DONE);
				// Adjust the adjustment to compensate for the fractional part of
				// the measurement
				adjust *= 100;
				G_sensors[i].status = (SCALE_INT(status.press + adjust)) / 100;
				break;
			case SENS_BMx280_I2C_TEMP:
				SET_BIT(G_sensors[i].iflags, SENS_FLAG_DONE);
				adjust *= 100;
				G_sensors[i].status = (SCALE_INT(status.temp + adjust)) / 100;
				break;
			case SENS_BMx280_I2C_HUM:
				SET_BIT(G_sensors[i].iflags, SENS_FLAG_DONE);
				adjust <<= 10;
				G_sensors[i].status = (SCALE_INT(status.hum + adjust)) >> 10;
				break;
			// There may be accidental matches with actual pins
			/*
			default:
				UNKNOWN_MSG(cfg->type);
				break;
			*/
			}
		}
	}

	return;
}
#endif // USE_BMx280_I2C_SENSOR

void check_sensor_warnings(void) {
	CLEAR_BIT(G_warnings, (WARN_BATTERY_LOW|WARN_VCC_LOW|WARN_SENSOR));

#if CALIBRATE_VREF >= 2
	if (G_vcc_voltage < REGULATED_VOLTAGE_LOW) {
		SET_BIT(G_warnings, WARN_VCC_LOW);
	}
#endif

#if USE_SMALL_SENSORS < 2
	sensor_t *s;
	_FLASH const sensor_static_t *cfg;

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


#ifdef __cplusplus
 }
#endif
