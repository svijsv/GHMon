/*
* Macros to aid in controller configuration
*/
// Set a schedule using a more natural notation than bare minutes
#define CLOCK_TIME(hours, minutes) (((hours) * 60) + (minutes))

/*
*
* Controller definitions
* See src/controllers.h for structure documentation
*
* Be sure to change CONTROLLER_COUNT in config.h if the number of controllers
* changes.
*/
_FLASH const controller_static_t CONTROLLERS[CONTROLLER_COUNT] = {
/*
* 0: Fan control */ {
	.name = "FAN_CTL", // Logging name, max size 7 characters by default
	.control_pins = { CONTROL_FAN_PIN }, // CONTROL_FAN_PIN is defined in config.h

	.run_timeout_seconds = 0,  // Once triggered, run continuously until the
	                           // temperature is low enough
	.schedule_minutes    = 15, // Check the input sensor every 15 minutes

	// Base decision to run on this sensor
	.inputs = {
		{
			.si = TEMP_TID, // Use this index in SENSORS[], TEMP_TID is defined
			                // in config/sensors.c
			.above = 30, // Turn on above 30C (86F)
			.below = SENS_THRESHOLD_IGNORE, // Don't turn on below anything
		}
	},
},

/*
* 1: Irrigation control */ {
	.name = "WAT_CTL",
	.control_pins = { CONTROL_WATER_PIN },

	.run_timeout_seconds = 30, // Run for 30 seconds when triggered
	.schedule_minutes = CLOCK_TIME(17, 30), // Check daily at 5:30PM (internal time)

	.cflags = CTRL_FLAG_USE_TIME_OF_DAY // The schedule is a specific time
	                                    // rather than a time period
	        | CTRL_FLAG_RETRY, // Re-check the input again after the timeout

	.inputs = {
		{
			.si = WATER,
			.above = 50000, // Trigger when sensor resistance is > 50KR
			.below = SENS_THRESHOLD_IGNORE,
		}
	},
},

};

/*
// X: An everythings-OK alarm, goes off every minute as long as everything's OK
{
	.name = "OK_ALRM",
	.control_pins = { CONTROL_OKALARM_PIN },
	.stop_pin = CONTROL_NOTOK_PIN, // Don't run the alarm if something is holding
	                               // CONTROL_NOTOK_PIN high

	.run_timeout_seconds = 5,
	.schedule_minutes    = 1,
	.cflags = CTRL_FLAG_IGNORE_POWER, // It's important to know that everything's
	                                  // OK, so run the alarm even when power
	                                  // is low.

	.inputs = {
		{
			.si = TEMP_TID,
			// If above < below, the values are taken as a window to operate
			// inside of.
			.above = 10, // Turn on above 10C (50F)
			.below = 30, // Turn on below 30C (86F)
		},
		// More than one input can be attached to controllers by changing
		// CONTROLLER_INPUTS_COUNT in config/advanced.h
		{
			.si = WATER,
			.above = SENS_THRESHOLD_IGNORE,
			.below = 50000,
		},
	},
},
*/
