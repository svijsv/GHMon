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

#include "sensors/private.h"

#include "ulib/time.h"
#include "ulib/math.h"


#if SENSOR_COUNT > 255 || SENSOR_COUNT < 1
# error "SENSOR_COUNT not between 1 and 255"
#endif


/*
* Static values
*/
// Store multi-use strings in const arrays so they aren't duplicated
_FLASH const char sens_invalid_msg_l[] = "Invalid sensor %u configuration: %s";
_FLASH const char sens_invalid_msg_e[] = "Invalid sensor configuration";


/*
* Types
*/


/*
* Variables
*/
int16_t G_vcc_voltage = REGULATED_VOLTAGE_mV;
utime_t cooldown = 0;
sensor_t G_sensors[SENSOR_COUNT];

_FLASH const sensor_dispatch_t sensor_dispatch[SENSOR_TYPE_COUNT] = {
	{ NULL, NULL, NULL }, // SENS_NONE
	SENSOR_DISPATCHES
};

/*
* Local function prototypes
*/
static void update_sensor_warning(uiter_t si);


/*
* Interrupt handlers
*/


/*
* Functions
*/
void sensors_init(void) {
	_FLASH const sensor_static_t *cfg;

	// If Vref is going to be calibrated here, do it before the sensors are
	// initialized so they have access to the calibrated value
#if CALIBRATE_VREF == 1
	adc_on();
	G_vcc_voltage = adc_read_vref_mV();
	adc_off();
#endif

	// Check for any problems with the sensor configuration
	for (uiter_t i = 0; i < SENSOR_COUNT; ++i) {
		cfg = &SENSORS[i];

		//s->i = i;

		if (cfg->name[0] == 0) {
			SETUP_ERR(i, "No name set; is SENSOR_COUNT correct?");
		}

#if USE_SMALL_SENSORS < 2
		sensor_t *s = &G_sensors[i];

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

		if (cfg->type == SENS_NONE) {
			SETUP_ERR(i, "No sensor type specified");
		} else if (cfg->type >= SENSOR_TYPE_COUNT) {
			SETUP_ERR(i, "Unknown sensor type");
		}

		if (sensor_dispatch[cfg->type].init != NULL) {
			sensor_dispatch[cfg->type].init(i);
		}
	}

	return;
}

void check_sensors() {
	uint16_t value[SENSOR_COUNT];
	sensor_t *s;
	uint8_t type;

	if (!RTC_TIMES_UP(cooldown)) {
		LOGGER("Not updating sensor status; cooldown in effect");
		return;
	}

	LOGGER("Updating sensor status");

	for (uiter_t i = 0; i < SENSOR_COUNT; ++i) {
		CLEAR_BIT(G_sensors[i].iflags, SENS_FLAG_DONE);
	}

	//
	// To minimize the time any given subsystem is powered on, the sensors are
	// checked in groups
	//
	// First handle the generic sensors
	power_on_sensors();
#if (CALIBRATE_VREF >= 2) || USE_ADC_SENSORS
	adc_on();
#endif
#if CALIBRATE_VREF >= 2
	G_vcc_voltage = adc_read_vref_mV();
#endif

	for (uiter_t i = 0; i < SENSOR_COUNT; ++i) {
		s = &G_sensors[i];
		type = SENSORS[i].type;

		if (!BIT_IS_SET(s->iflags, SENS_FLAG_I2C) && !BIT_IS_SET(s->iflags, SENS_FLAG_SPI) && !BIT_IS_SET(s->iflags, SENS_FLAG_DONE)) {
			value[i] = (sensor_dispatch[type].read != NULL) ? sensor_dispatch[type].read(i) : 0;
		}
	}
#if (CALIBRATE_VREF >= 2) || USE_ADC_SENSORS
	adc_off();
#endif
	power_off_sensors();

	//
	// Then SPI sensors
#if USE_SPI_SENSORS
	power_on_SPI();
	for (uiter_t i = 0; i < SENSOR_COUNT; ++i) {
		s = &G_sensors[i];
		type = SENSORS[i].type;

		if (BIT_IS_SET(s->iflags, SENS_FLAG_SPI) && !BIT_IS_SET(s->iflags, SENS_FLAG_DONE)) {
			value[i] = (sensor_dispatch[type].read != NULL) ? sensor_dispatch[type].read(i) : 0;
		}
	}
	power_off_SPI();
#endif // USE_SPI_SENSORS

	//
	// Then I2C sensors
#if USE_I2C_SENSORS
	power_on_I2C();
	for (uiter_t i = 0; i < SENSOR_COUNT; ++i) {
		s = &G_sensors[i];
		type = SENSORS[i].type;

		if (BIT_IS_SET(s->iflags, SENS_FLAG_I2C) && !BIT_IS_SET(s->iflags, SENS_FLAG_DONE)) {
			value[i] = (sensor_dispatch[type].read != NULL) ? sensor_dispatch[type].read(i) : 0;
		}
	}
	power_off_I2C();
#endif // USE_I2C_SENSORS

	for (uiter_t i = 0; i < SENSOR_COUNT; ++i) {
		//s = &G_sensors[i];
		type = SENSORS[i].type;

		if (sensor_dispatch[type].update != NULL) {
			sensor_dispatch[type].update(i, value[i]);
		}
		update_sensor_warning(i);
	}
	cooldown = SET_RTC_TIMEOUT(SENS_COOLDOWN);

	return;
}
void invalidate_sensors(void) {
	cooldown = 0;

	return;
}
static void update_sensor_warning(uiter_t si) {
#if USE_SMALL_SENSORS < 2
	bool high, low, inside;
	sensor_t *s;
	_FLASH const sensor_static_t *cfg;

	s = &G_sensors[si];
	cfg = &SENSORS[si];

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

#else
	UNUSED(si);
#endif // USE_SMALL_SENSORS < 2

	return;
}

void check_sensor_warnings(void) {
	CLEAR_BIT(G_warnings, (WARN_BATTERY_LOW|WARN_VCC_LOW|WARN_SENSOR));

#if CALIBRATE_VREF >= 2
	if (G_vcc_voltage < REGULATED_VOLTAGE_LOW_mV) {
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
