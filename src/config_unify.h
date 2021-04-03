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
// config_unify.h
// Determine which subsystems to enable or disable based on user configuration
// NOTES:
//

#ifdef __cplusplus
 extern "C" {
#endif
#ifndef _CONFIG_UNIFY_H
#define _CONFIG_UNIFY_H

// Check if we need the ADC
#if USE_VOLT_SENSOR || USE_OHM_SENSOR || \
    USE_LOG_BETA_SENSOR || \
    USE_LINEAR_R_SENSOR || USE_LINEAR_V_SENSOR || \
    USE_LOOKUP_R_SENSOR || USE_LOOKUP_V_SENSOR || \
    CALIBRATE_VREF
# define USE_ADC 1
#endif

#if USE_LOOKUP_R_SENSOR || USE_LOOKUP_V_SENSOR
# define USE_LOOKUP_TABLES 1
#endif

#if USE_FDISK && !USE_TERMINAL
# warning "Setting USE_TERMINAL because USE_FDISK was set"
# undef USE_TERMINAL
# define USE_TERMINAL 1
#endif

#if USE_TERMINAL && !USE_SERIAL
# warning "Setting USE_SERIAL because USE_TERMINAL was set"
# undef USE_SERIAL
# define USE_SERIAL 1
#endif

#if USE_LOGGING || USE_FDISK
# define USE_SD  1
#endif

#if USE_SD
# define USE_SPI 1
#endif

#if USE_CONTROLLERS && ! CONTROLLER_COUNT
# warning "Disabling USE_CONTROLLERS because CONTROLLER_COUNT is 0"
# undef USE_CONTROLLERS
# define USE_CONTROLLERS 0
#endif

#ifndef USE_SMALL_SENSORS
# define USE_SMALL_SENSORS USE_SMALL_BUILD
#endif
#ifndef USE_SMALL_CONTROLLERS
# define USE_SMALL_CONTROLLERS USE_SMALL_BUILD
#endif
#ifndef USE_SMALL_CODE
# define USE_SMALL_CODE USE_SMALL_BUILD
#endif

#endif // _CONFIG_UNIFY_H
#ifdef __cplusplus
 }
#endif
