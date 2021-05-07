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
// controllers.h
// Manage external device control
// NOTES:
//

#ifdef __cplusplus
 extern "C" {
#endif
#ifndef _CONTROLLERS_H
#define _CONTROLLERS_H

/*
* Includes
*/
#include "config.h"
#include "common.h"

#include "ulib/time.h"

#include "sensors.h"


#if USE_CONTROLLERS

/*
* Static values
*/
//
// Configuration flags for controller_static_t structs
//
// Run if ANY sensor's conditions are met (by default they all must be)
#define CTRL_FLAG_TRIGGER_ANY  0x01
// Issue a warning when the controller is engaged
#define CTRL_FLAG_WARN_WHEN_ON 0x02
// Issue a warning when a controller runs until timeout; mostly useful in
// conjuction with stop pins
#define CTRL_FLAG_WARN_WHEN_TIMEOUT 0x04
// Run the controller even if in a low-battery or low-Vcc state
#define CTRL_FLAG_IGNORE_POWER 0x08
//
// These features depend on fields removed from sensor_t in small builds
#if USE_SMALL_CONTROLLERS < 1
// After engaging, recheck the conditions and run again if required
// If the conditions are still met after CONTROLLER_RETRY_MAX attempts, the
// controller stops trying and issues a warning.
# define CTRL_FLAG_RETRY           0x10
// The polling time is an offset from 12AM rather than a period
# define CTRL_FLAG_USE_TIME_OF_DAY 0x20
#endif // USE_SMALL_CONTROLLERS < 1
#if USE_SMALL_CONTROLLERS < 2
// Monitor the stop pin continuously instead of periodic polling
# define CTRL_FLAG_STOP_CHECK_CONTINUOUS 0x40
#endif

//
// Internal flags for controller_t structs
//
// Set if the controller is currently engaged
#define CTRL_FLAG_ENGAGED     0x10
// Set if the controller has sensors attached
#define CTRL_FLAG_USES_SENSORS 0x20
// Set when the controller wants sensor data invalidated
#define CTRL_FLAG_INVALIDATE  0x40
// Set if a warning was issued by the controller
#define CTRL_FLAG_WARNING     0x80


/*
* Types
*/
//
// Description of a controller input sensor
typedef struct {
	// Consider a sensor's conditions met if it's status is above 'above' or
	// below 'below'
	// If above > below run when outside a window
	// If above < below run when inside a window
	// If above == SENS_THRESHOLD_IGNORE only run when below 'below'
	// If below == SENS_THRESHOLD_IGNORE only run when above 'above'
	status_t below;
	status_t above;

	// The index of the sensor in G_sensors[] (defined in config.c)
	// Can't be > 127.
	// Set to -1 to disable.
	int8_t si;
} controller_sens_t;
//
// Description of the user-configured portion of a controller struct
typedef struct {
	// Name used for logging
	// The size of name[] includes a trailing NUL byte.
	char name[DEVICE_NAME_LEN+1];

#if CONTROLLER_SENS_COUNT > 0
	// The sensor(s) used to determine whether the controller is activated.
	controller_sens_t inputs[CONTROLLER_SENS_COUNT];
#endif

// schedule only makes sense with individual controller wakeup times, which
// are absent when USE_SMALL_CONTROLLERS >= 1
#if USE_SMALL_CONTROLLERS < 1
	// When to check the whether the controller should be engaged
	// By default this represents the number of minutes between checks, but
	// if the CTRL_FLAG_USE_TIME_OF_DAY flag is set it represents the number
	// of minutes after midnight to check.
	uint16_t schedule_minutes;
#endif // USE_SMALL_CONTROLLERS < 1

	// Seconds to keep control_pins high after theyre turned on
	// The control pins are kept high until either this timeout is reached or
	// the stop pin goes high.
	// If 0, the pins are kept high until a future controller check stops them
	// or the stop pin goes high.
	uint8_t run_timeout_seconds;

	// Control flags
	uint8_t cflags;

	// MCU pin(s) used to engage the device(s)
	// If biased, these pins are always in push-pull mode and kept at their
	// bias state normally, then reversed when conditions are met.
	// If not biased, these pins are normally in high-impedence mode and set
	// to push-pull high when conditions are met.
	pin_t control_pins[CONTROLLER_CTRL_PIN_COUNT];
#if USE_SMALL_CONTROLLERS < 2
	// MCU pin used tell the controller that the device should stop running
	// This pin should be high when the controller should halt; the internal
	// pullup or pulldown can be used by biasing it.
	// Ignored if unset.
	pin_t stop_pin;
#endif // USE_SMALL_CONTROLLERS < 2
} controller_static_t;
//
// Description of the internal portion of a controller struct
typedef struct {
#if USE_SMALL_CONTROLLERS < 1
	// Time of next check
	utime_t next_check;
	// Time when current engagement started
	utime_t run_start;
	// Number retries that have been made for this controller
	uint8_t try_count;

	// Number of times a device has been engaged
	// Rolls over after 255 times
	uint8_t run_count;
	// Total number of seconds controller has run
	// Rolls over after ~18 hours
	uint16_t run_time_seconds;
#endif // USE_SMALL_CONTROLLERS < 1

	// Internal flags
	uint8_t iflags;
	// Index in G_controllers[]
	// At time of last test, storing 'i' in the struct used less space on flash
	// than pointer arithmetic for controllers but more for sensors; both used
	// the same amount of RAM
	//uint8_t i;
} controller_t;


/*
* Variable declarations
*/
// Array of controller structs representing available output
extern controller_t G_controllers[CONTROLLER_COUNT];
// Array of controller configuration structs representing available output
// Defined in config.c.
extern _FLASH const controller_static_t CONTROLLERS[CONTROLLER_COUNT];

/*
* Function prototypes (defined in controllers.c)
*/
// Initialize the controller subsystem
void controllers_init(void);
// Check whether a controller should be engaged
void check_controller(controller_t *c);
// Check for controller warnings
void check_controller_warnings(void);

/*
* Macros
*/
#define GET_CONTROLLER_I(c) ((uint )(c - G_controllers))
//#define GET_CONTROLLER_I(c) ((c)->i)

#else // !USE_CONTROLLERS
# define controllers_init()  ((void )0U)
# define check_controller(...) ((void )0U)
# define check_controller_warnings() ((void )0U)
#endif // USE_CONTROLLERS

#endif // _CONTROLLERS_H
#ifdef __cplusplus
 }
#endif
