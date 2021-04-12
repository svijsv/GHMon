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
// power.c
// Manage power to peripherals
// NOTES:
//   Pull the power pins low (rather than Hi-Z) while disabled to prevent
//   spurious power-ups
//

#ifdef __cplusplus
 extern "C" {
#endif

//#define DEBUG 1
/*
* Includes
*/
#include "power.h"
#include "sensors.h"

#include "fatfs/diskio.h"


/*
* Static values
*/
// Wait this many ms for power to come up when it's turned on
#define POWER_UP_DELAY 10


/*
* Types
*/


/*
* Variables
*/
#if USE_SPI
// The number of times SPI has been turned on without later being turned off
static uint8_t SPI_callers = 0;
#endif
#if USE_I2C
// The number of times I2C has been turned on without later being turned off
static uint8_t I2C_callers = 0;
#endif
#if USE_SD
// Whether or not the SD card has been initialized yet
static bool SD_initialized = false;
#endif


/*
* Local function prototypes
*/


/*
* Interrupt handlers
*/


/*
* Functions
*/
void power_on_sensors(void) {
#if SENSOR_POWER_PIN
	power_on_output(SENSOR_POWER_PIN);
	sleep(POWER_UP_DELAY);
#endif

	return;
}
void power_off_sensors(void) {
#if SENSOR_POWER_PIN
	power_off_output(SENSOR_POWER_PIN);
#endif

	return;
}

#if USE_SD
// Actual power on/off of the SD card is handled in power_[on|off]_SPI()
// because the card can't be powered off while the SPI pins are on
void power_on_SD(void) {
	power_on_SPI();

	return;
}
void power_off_SD(void) {
	power_off_SPI();

	return;
}
#endif // USE_SD

#if USE_SPI
// SD card powering needs to be handled along with SPI powering or else current
// leakage through powered SPI pins may cause problems
void power_on_SPI(void) {
	if (SPI_callers == 0) {
		// Per a sandisk datasheet by way of
		// https:// thecavepearlproject.org/2015/11/05/a-diy-arduino-data-logger-build-instructions-part-4-power-optimization/
		// we should apply power to an SD card before any pins are set HIGH;
		// this is true for pretty much all other devices too though, as
		// applying power to an IO pin without also powering Vcc may cause
		// damage.
		//   1. Turn on the power
		//   2. Set the SD CS pin high to get the SD card to configure itself
		//      in SPI mode immediately; other devices follow
		//   3. Turn on the SPI subsystem (which controls the other pins)
		//   4. Initialize the SD card (if present) to keep it from interfering
		//      with anything else on the bus
		// TODO: The function call overhead may take too much time here
#if SPI_POWER_PIN
		power_on_output(SPI_POWER_PIN);
#if USE_SD
		gpio_set_mode(SPIx_CS_SD_PIN, GPIO_MODE_PP, GPIO_HIGH);
#endif // USE_SD
#if USE_SPI_SENSORS
		for (uiter_t i = 0; i < SENSOR_COUNT; ++i) {
			if (BIT_IS_SET(G_sensors[i].iflags, SENS_FLAG_SPI)) {
				// The BMP280 requires the CS pin to be pulled down briefly to
				// put it in SPI mode; hopefully any other sensors added in the
				// future won't care
				gpio_set_mode(SENSORS[i].pin, GPIO_MODE_PP, GPIO_LOW);
				gpio_set_state(SENSORS[i].pin, GPIO_HIGH);
			}
		}
#endif // USE_SPI_SENSORS
#endif // SPI_POWER_PIN

		spi_on();
		// Delay to let power come up
		sleep(POWER_UP_DELAY);
	}
#if USE_SD
	// The SD card needs to be initialized or it may interfere with
	// anything else on the SPI bus
	// We're assuming here that the card won't have been removed and then
	// reinserted while the SPI bus is on; the only good way to handle cases
	// where it might be involves detecting insertion with a pin interrupt
	// and initializing the card there
	if (!SD_initialized) {
		SD_initialized = (disk_initialize(0) == 0);
	}
#endif
	++SPI_callers;

	return;
}
void power_off_SPI(void) {
	// This function may be called to initialize the power pins, in which case
	// it should go through the motions of turning everything off
	if (SPI_callers > 0) {
		--SPI_callers;
	}

	if (SPI_callers == 0) {
		spi_off();

		// The idea here is similar to turning power on:
		//   1. Turn off SPI subsystem
		//   2. Turn off CS pins so that leaking current doesn't damage anything
		//   3. Turn off SPI device power
#if USE_SD
		// Mark the card as 'unitialized' even when power isn't removed because
		// the card itself may be removed at some point and we won't be alerted
		SD_initialized = false;
#endif
#if SPI_POWER_PIN
#if USE_SD
		gpio_set_mode(SPIx_CS_SD_PIN, GPIO_MODE_HiZ, GPIO_LOW);
#endif // USE_SD
#if USE_SPI_SENSORS
		for (uiter_t i = 0; i < SENSOR_COUNT; ++i) {
			if (BIT_IS_SET(G_sensors[i].iflags, SENS_FLAG_SPI)) {
				gpio_set_mode(SENSORS[i].pin, GPIO_MODE_HiZ, GPIO_LOW);
			}
		}
#endif // USE_SPI_SENSORS
		power_off_output(SPI_POWER_PIN);

#else // !SPI_POWER_PIN
#if USE_SD
		gpio_set_mode(SPIx_CS_SD_PIN, GPIO_MODE_PP, GPIO_HIGH);
#endif // USE_SD
#if USE_SPI_SENSORS
		for (uiter_t i = 0; i < SENSOR_COUNT; ++i) {
			if (BIT_IS_SET(G_sensors[i].iflags, SENS_FLAG_SPI)) {
				gpio_set_mode(SENSORS[i].pin, GPIO_MODE_PP, GPIO_HIGH);
			}
		}
#endif // USE_SPI_SENSORS
#endif // SPI_POWER_PIN
	}

	return;
}
#endif // USE_SPI

#if USE_I2C
void power_on_I2C(void) {
	if (I2C_callers == 0) {
		// The I2C lines are all open-drain with pullups so there *shouldn't*
		// be an issue with the power-up order unless there's a device connected
		// that's switched on GND
#if I2C_POWER_PIN
		power_on_output(I2C_POWER_PIN);
#endif // I2C_POWER_PIN

		i2c_on();
		// Delay to let power come up
		sleep(POWER_UP_DELAY);
	}
	++I2C_callers;

	return;
}
void power_off_I2C(void) {
	// This function may be called to initialize the power pins, in which case
	// it should go through the motions of turning everything off
	if (I2C_callers > 0) {
		--I2C_callers;
	}

	if (I2C_callers == 0) {
		i2c_off();

#if I2C_POWER_PIN
		power_off_output(I2C_POWER_PIN);
#endif
	}

	return;
}
#endif // USE_I2C

void power_on_output(pin_t pin) {
	assert(pin != 0);

	switch (GPIO_GET_BIAS(pin)) {
	case BIAS_HIGH:
		gpio_set_mode(pin, GPIO_MODE_PP, GPIO_LOW);
		break;
	default:
		gpio_set_mode(pin, GPIO_MODE_PP, GPIO_HIGH);
		break;
	}

	return;
}
void power_off_output(pin_t pin) {
	assert(pin != 0);

	switch (GPIO_GET_BIAS(pin)) {
	case BIAS_HIGH:
		gpio_set_mode(pin, GPIO_MODE_PP, GPIO_HIGH);
		break;
	case BIAS_LOW:
		gpio_set_mode(pin, GPIO_MODE_PP, GPIO_LOW);
		break;
	default:
		gpio_set_mode(pin, GPIO_MODE_HiZ, GPIO_FLOAT);
		break;
	}

	return;
}

void power_on_input(pin_t pin) {
	assert(pin != 0);

	switch (GPIO_GET_BIAS(pin)) {
	case BIAS_HIGH:
		gpio_set_mode(pin, GPIO_MODE_IN, GPIO_HIGH);
		break;
	case BIAS_LOW:
		gpio_set_mode(pin, GPIO_MODE_IN, GPIO_LOW);
		break;
	default:
		gpio_set_mode(pin, GPIO_MODE_IN, GPIO_FLOAT);
		break;
	}

	return;
}
void power_off_input(pin_t pin) {
	assert(pin != 0);

	gpio_set_mode(pin, GPIO_MODE_HiZ, GPIO_FLOAT);

	return;
}


#ifdef __cplusplus
 }
#endif
