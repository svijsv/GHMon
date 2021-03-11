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


/*
* Local function prototypes
*/
static void power_on_SPI(void);
static void power_off_SPI(void);

/*
* Interrupt handlers
*/


/*
* Functions
*/
void power_on_sensors(void) {
	power_on_output(SENSOR_POWER_PIN);
	// Delay to let power come up
	sleep(POWER_UP_DELAY);

	return;
}
void power_off_sensors(void) {
	power_off_output(SENSOR_POWER_PIN);

	return;
}

#if USE_SD
// Actual power on/off of the SD card is handled in power_[on|off]_SPI()
// because the card can't be powered off while the SPI pins are on
// TODO: Handle the case where the SD card shares a power pin with another
// peripheral
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
static void power_on_SPI(void) {
	if (SPI_callers == 0) {
		// Per a sandisk datasheet by way of
		// https:// thecavepearlproject.org/2015/11/05/a-diy-arduino-data-logger-build-instructions-part-4-power-optimization/
		// we should apply power to the SD card before any pins are set HIGH. It's
		// unclear however if this is meant to apply for SPI.
		// The ideal way to go about this would be to simultaneously turn on power
		// to the SD card and set all the pins connected to it to the correct modes
		// to prevent parasitic voltage either causing the card to draw excess
		// power or damaging it; we can't do that so instead:
		//   1. Turn on the power; there's an internal capacitor and (probably)
		//      another external one which will hopefully give us a brief window
		//   2. Set the CS pin high to get the SD card to configure itself in SPI
		//      mode immediately
		//   3. Turn on the SPI subsystem (which controls the other pins)
		// TODO: The function call overhead may be too much time here
#if USE_SD
#if SD_POWER_PIN
		power_on_output(SD_POWER_PIN);
#endif // SD_POWER_PIN
		gpio_set_mode(SPIx_CS_SD_PIN, GPIO_MODE_PP, GPIO_HIGH);
#endif // USE_SD

		spi_on();
		// Delay to let power come up
		sleep(POWER_UP_DELAY);
	}
	++SPI_callers;

	return;
}
static void power_off_SPI(void) {
	if (SPI_callers == 0) {
		return;
	}
	--SPI_callers;

	if (SPI_callers == 0) {
		spi_off();

		// The idea here is similar to turning power on:
		//   1. Turn off SPI subsystem
		//   2. Turn off SD card power, let the caps give us a buffer
		//   3. Turn off CS pin last so that the card never tries to wake up
#if USE_SD
#if SD_POWER_PIN
			power_off_output(SD_POWER_PIN);
			gpio_set_mode(SPIx_CS_SD_PIN, GPIO_MODE_HiZ, GPIO_LOW);
#else // !SD_POWER_PIN
			gpio_set_mode(SPIx_CS_SD_PIN, GPIO_MODE_PP, GPIO_HIGH);
#endif // SD_POWER_PIN
#endif // USE_SD
	}

	return;
}
#else // !USE_SPI
static void power_on_SPI(void) {
	return;
}
static void power_off_SPI(void) {
	return;
}
#endif // USE_SPI

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
