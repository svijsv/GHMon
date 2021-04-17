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
// binary.h
// Manage binary (pin on/off) sensors
// NOTES:
//

#ifdef __cplusplus
 extern "C" {
#endif
#ifndef _SENSORS_BINARY_H
#define _SENSORS_BINARY_H
#if USE_BINARY_SENSORS

//
// Macros for the lists in sensors.h
#define _SENS_BINARY SENS_BINARY,
#define SENS_BINARY_DISPATCH { .init = NULL, .read = sensor_read_binary, .update = NULL },
//
// Dispatch function declarations
uint16_t sensor_read_binary(uiter_t si);



#else  // !USE_BINARY_SENSORS
#define _SENS_BINARY
#define SENS_BINARY_DISPATCH

#endif // USE_BINARY_SENSORS
#endif // _SENSORS_BINARY_H
#ifdef __cplusplus
 }
#endif
