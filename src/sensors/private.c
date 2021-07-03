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
// private.c
// Module private definitions
// NOTES:
//

#ifdef __cplusplus
 extern "C" {
#endif

//#define DEBUG 1
//#define NDEBUG 1

#include "sensors.h"
#include "private.h"

#include "ulib/time.h"


#if USE_ADC_SENSORS
uint16_t sensor_read_ADC(uiter_t si) {
	uint16_t value;
	pin_t pin;

	pin = SENSORS[si].pin;
	gpio_set_mode(pin, GPIO_MODE_AIN, GPIO_FLOAT);

	if (BIT_IS_SET(SENSORS[si].cflags, SENS_FLAG_AC)) {
		value = adc_read_ac_amplitude(pin, HZ_TO_MS(PWM_MAX_FREQUENCY)+1);
	} else {
		value = adc_read_pin(pin);
	}

	gpio_set_mode(pin, GPIO_MODE_HiZ, GPIO_FLOAT);

	return value;
}
#endif // USE_ADC_SENSORS

#ifdef __cplusplus
 }
#endif
