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
// binary.c
// Manage binary (pin on/off) sensors
// NOTES:
//

#ifdef __cplusplus
 extern "C" {
#endif

//#define DEBUG 1
//#define NDEBUG 1

//#include "binary.h"
#include "sensors.h"
#include "private.h"
#include "power.h"

#if USE_BINARY_SENSORS


uint16_t sensor_read_binary(uiter_t si) {
	uint8_t value;
	pin_t pin;

	pin = SENSORS[si].pin;
	power_on_input(pin);
	value = gpio_get_state(pin);
	power_off_input(pin);

	if (BIT_IS_SET(SENSORS[si].cflags, SENS_FLAG_INVERT)) {
		// This won't work right if the value is somehow not high or low (e.g.
		// if GPIO_FLOAT was returned) but that shouldn't even be possible in
		// practice
		value = !value;
	}
	G_sensors[si].status = value;

	return 0;
}

#endif // USE_BINARY_SENSORS
#ifdef __cplusplus
 }
#endif
