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
// DHT11.h
// Manage DHT11 sensors
// NOTES:
//

#ifdef __cplusplus
 extern "C" {
#endif
#ifndef _SENSORS_DHT11_H
#define _SENSORS_DHT11_H
#if USE_DHT11_SENSORS

// The datasheet says to wait 1 second before issuing commands, but I had
// problems with anything less than 1.5
#if SENSOR_POWER_UP_DELAY_MS < 1500
# undef  SENSOR_POWER_UP_DELAY_MS
# define SENSOR_POWER_UP_DELAY_MS 1500
#endif
//
// Macros for the lists in sensors.h
#define _SENS_DHT11_HUMIDITY    SENS_DHT11_HUMIDITY,
#define _SENS_DHT11_TEMPERATURE SENS_DHT11_TEMPERATURE,
//
#define SENS_DHT11_DISPATCH { .init = NULL, .read = sensor_read_dht11, .update = NULL },
#define SENS_DHT11_HUMIDITY_DISPATCH    SENS_DHT11_DISPATCH
#define SENS_DHT11_TEMPERATURE_DISPATCH SENS_DHT11_DISPATCH
//
// Dispatch function declarations
uint16_t sensor_read_dht11(uiter_t si);



#else  // !USE_DHT11_SENSORS
#define _SENS_DHT11_HUMIDITY
#define _SENS_DHT11_TEMPERATURE
#define SENS_DHT11_HUMIDITY_DISPATCH
#define SENS_DHT11_TEMPERATURE_DISPATCH

#endif // USE_DHT11_SENSORS
#endif // _SENSORS_DHT11_H
#ifdef __cplusplus
 }
#endif
