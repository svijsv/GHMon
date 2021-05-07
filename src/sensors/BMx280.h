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
// BMx280.h
// Manage BME280 and BMP280 sensors
// NOTES:
//   These sensors have two different interfaces, SPI and I2C, which are
//   managed separately
//
//   Temperature is reported in degrees celsius, humidity in %, and air
//   pressure in millibars

#ifdef __cplusplus
 extern "C" {
#endif
#ifndef _SENSORS_BMX280_H
#define _SENSORS_BMX280_H

//
// SPI interface
//
#if USE_BMx280_SPI_SENSORS
//
// Enable the appropriate peripheral to read this sensor
#if ! USE_SPI_SENSORS
# undef  USE_SPI_SENSORS
# define USE_SPI_SENSORS 1
#endif

//
// Macros for the lists in sensors.h
#define _SENS_BMx280_SPI_HUMIDITY    SENS_BMx280_SPI_HUMIDITY,
#define _SENS_BMx280_SPI_PRESSURE    SENS_BMx280_SPI_PRESSURE,
#define _SENS_BMx280_SPI_TEMPERATURE SENS_BMx280_SPI_TEMPERATURE,
//
#define SENS_BMx280_SPI_DISPATCH { .init = sensor_init_bmx280_spi, .read = sensor_read_bmx280_spi, .update = NULL },
#define SENS_BMx280_SPI_HUMIDITY_DISPATCH    SENS_BMx280_SPI_DISPATCH
#define SENS_BMx280_SPI_TEMPERATURE_DISPATCH SENS_BMx280_SPI_DISPATCH
#define SENS_BMx280_SPI_PRESSURE_DISPATCH    SENS_BMx280_SPI_DISPATCH
//
// Dispatch function declarations
void sensor_init_bmx280_spi(uiter_t si);
uint16_t sensor_read_bmx280_spi(uiter_t si);


#else  // !USE_BMx280_SPI_SENSORS
#define _SENS_BMx280_SPI_HUMIDITY
#define _SENS_BMx280_SPI_PRESSURE
#define _SENS_BMx280_SPI_TEMPERATURE
#define SENS_BMx280_SPI_HUMIDITY_DISPATCH
#define SENS_BMx280_SPI_TEMPERATURE_DISPATCH
#define SENS_BMx280_SPI_PRESSURE_DISPATCH
#endif // USE_BMx280_SPI_SENSORS


//
// I2C interface
//
#if USE_BMx280_I2C_SENSORS
//
// Enable the appropriate peripheral to read this sensor
#if ! USE_I2C_SENSORS
# undef  USE_I2C_SENSORS
# define USE_I2C_SENSORS 1
#endif

//
// Macros for the lists in sensors.h
#define _SENS_BMx280_I2C_HUMIDITY    SENS_BMx280_I2C_HUMIDITY,
#define _SENS_BMx280_I2C_PRESSURE    SENS_BMx280_I2C_PRESSURE,
#define _SENS_BMx280_I2C_TEMPERATURE SENS_BMx280_I2C_TEMPERATURE,
//
#define SENS_BMx280_I2C_DISPATCH { .init = sensor_init_bmx280_i2c, .read = sensor_read_bmx280_i2c, .update = NULL },
#define SENS_BMx280_I2C_HUMIDITY_DISPATCH    SENS_BMx280_I2C_DISPATCH
#define SENS_BMx280_I2C_TEMPERATURE_DISPATCH SENS_BMx280_I2C_DISPATCH
#define SENS_BMx280_I2C_PRESSURE_DISPATCH    SENS_BMx280_I2C_DISPATCH
//
// Dispatch function declarations
void sensor_init_bmx280_i2c(uiter_t si);
uint16_t sensor_read_bmx280_i2c(uiter_t si);


#else  // !USE_BMx280_I2C_SENSORS
#define _SENS_BMx280_I2C_HUMIDITY
#define _SENS_BMx280_I2C_PRESSURE
#define _SENS_BMx280_I2C_TEMPERATURE
#define SENS_BMx280_I2C_HUMIDITY_DISPATCH
#define SENS_BMx280_I2C_TEMPERATURE_DISPATCH
#define SENS_BMx280_I2C_PRESSURE_DISPATCH
#endif // USE_BMx280_I2C_SENSORS

#endif // _SENSORS_BMX280_H
#ifdef __cplusplus
 }
#endif
