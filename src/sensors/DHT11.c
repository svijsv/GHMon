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
// DHT11.c
// Manage DHT11 sensors
// NOTES:
//

#ifdef __cplusplus
 extern "C" {
#endif

//#define DEBUG 1
//#define NDEBUG 1

//#include "dht11.h"
#include "sensors.h"
#include "private.h"
#include "power.h"

#include "ulib/time.h"

#if USE_DHT11_SENSORS


uint16_t sensor_read_dht11(uiter_t si) {
	utime_t timeout;
	uint8_t us_count;
	uint8_t reading[5] = { 0 };
	pin_t pin;
	gpio_quick_t qpin;

	pin = SENSORS[si].pin;

	// Pull the data pin low at least 18ms to signal the sensor to start
	gpio_set_mode(pin, GPIO_MODE_PP, GPIO_LOW);
	sleep_ms(20);

	gpio_quickread_prepare(&qpin, pin);
	uscounter_on();
	power_on_input(pin);
	timeout = SET_TIMEOUT(1000);

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
			reading[i] = (reading[i] << 1) | ((us_count > 35) ? 1 : 0);
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
	uint16_t cksum = (uint16_t )reading[0] + (uint16_t )reading[1] + (uint16_t )reading[2] + (uint16_t )reading[3];
	if ((cksum & 0xFF) != reading[4]) {
		LOGGER("DHT11 sensor on pin 0x%02X: invalid checksum: have %u, expected %u", (uint )pin, (uint )(cksum & 0xFF), (uint )reading[4]);
	}
	if ((reading[0] == 0) && (reading[1] == 0) && (reading[2] == 0) && (reading[3] == 0) && (reading[4] == 0)) {
		LOGGER("DHT11 sensor on pin 0x%02X: all readings were 0", (uint )pin);
	}
#endif

	for (uiter_t i = si; i < SENSOR_COUNT; ++i) {
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
			case SENS_DHT11_HUMIDITY:
				SET_BIT(G_sensors[i].iflags, SENS_FLAG_DONE);
				// To preserve the fraction part for scaling, shift it all left
				// then right - effectively multiply by 2^8 then divide by same
				tmp = ((uint16_t )reading[0] << 8) | reading[1];
				G_sensors[i].status = (SCALE_INT(tmp + adjust)) >> 8;
				break;
			case SENS_DHT11_TEMPERATURE:
				SET_BIT(G_sensors[i].iflags, SENS_FLAG_DONE);
				tmp = ((uint16_t )reading[2] << 8) | reading[3];
				G_sensors[i].status = (SCALE_INT(tmp + adjust)) >> 8;
				break;
			default:
				// There may be accidental matches with non-pin 'pins' like the
				// BMP280's I2C address
				//UNKNOWN_MSG(cfg->type);
				LOGGER("Sensor %u pin matches sensor %u pin", (uint )i, (uint )si);
				break;
			}
		}
	}

	return 0;
}


#endif // USE_DHT11_SENSORS
#ifdef __cplusplus
 }
#endif
