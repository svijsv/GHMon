//
// Program Configuration
//
// Notes:
//    There are some advanced configuration options in advanced.h
//

//
// Optional components
// Set to '0' to disable corresponding feature
//
// Enable sensors
#define USE_SENSORS 1
//
// Enable controllers
#define USE_CONTROLLERS 1
//
// Enable actuators
#define USE_ACTUATORS 1
//
// Cut down the size of internal components at the expense of functionality
// These can be controlled in more detail in advanced.h
#ifndef USE_SMALL_BUILD
# define USE_SMALL_BUILD 0
#endif
#ifndef USE_SMALL_SENSORS
# define USE_SMALL_SENSORS 0
#endif
#ifndef USE_SMALL_CONTROLLERS
# define USE_SMALL_CONTROLLERS 0
#endif
#ifndef USE_SMALL_ACTUATORS
# define USE_SMALL_ACTUATORS 0
#endif
//
// Enable data logging
#define USE_LOGGING 1
//
// Use the control button
#define USE_CTRL_BUTTON 1
//
// Use the status LED
#define USE_STATUS_LED 1
//
// Enable UART serial output
#define USE_UART_OUTPUT 1
//
// Enable UART serial terminal
#define USE_UART_TERMINAL 1

//
// Sensor configuration
//
// Check the status and emit warning blinks if called for every
// STATUS_CHECK_MINUTES minutes
// If 0, it's checked only when the user button is pressed
#define STATUS_CHECK_MINUTES 15
//
// If set and USE_STATUS_LED is also set, the status LED is lighted during sleep
// when there's a warning instead of only flashing when the status is checked
#define STATUS_LED_LIGHTS_ON_WARNING 0

//
// Controller configuration
//
// Check whether any controllers need to be run every CONTROLLER_CHECK_MINUTES
// If 0, they're only checked when requested by way of the control button.
// Individual controllers may have their own periods specified, in which case
// they ignore this.
#define CONTROLLER_CHECK_MINUTES 15
//
// If a time-of-day controller would be scheduled to run fewer than this many
// minutes in the past (because e.g. the clock changed or the device restarted),
// run it immediately instead of waiting for tomorrow
// Be careful using this in conjunction with controller_cfg_t.next_run_time().
#define CONTROLLER_SCHEDULE_SKEW_WINDOW_MINUTES 15

//
// Data logging configuration
//
// Check the status and append it to the log every LOG_APPEND_MINUTES minutes
// If 0, it's only updated when requested by way of the control button.
#define LOG_APPEND_MINUTES 15
//
// Instead of writing to storage every time, keep this many readings in memory
// and write them all when the buffer is full.
// With 10 minute intervals, a 5-reading buffer will write once an hour (5
// buffered + the newest).
// The log buffer is allocated at run-time so any RAM usage report offered by the
// build system is going to be wrong. Generally you can expect 36 bytes per
// device for each line on 32-bit devices with the default build settings or 26
// bytes for 8/16 bit devices.
// Set to 0 to disable buffering.
#define LOG_LINE_BUFFER_COUNT 15 // 4h with 15m intervals
//
// If set, log each sensor in SENSORS[] unless SENSOR_CFG_FLAG_NOLOG is set
// Otherwise, don't log any sensor in SENSORS[] unless SENSOR_CFG_FLAG_LOG is set
#define LOG_SENSORS_BY_DEFAULT 1
//
// If set, sensors are read to update their values for logging
// Otherwise whatever values were last read are used.
#define LOG_UPDATES_SENSORS 1
//
// If set, print the type of each sensor value
#define LOG_PRINT_SENSOR_TYPE 1
//
// If set, log each controller in CONTROLLERS[] unless CONTROLLER_CFG_FLAG_NOLOG is set
// Otherwise, don't log any controller in CONTROLLERS[] unless CONTROLLER_CFG_FLAG_LOG is set
#define LOG_CONTROLLERS_BY_DEFAULT 1
//
// If set, log each actuator in ACTUATORS[] unless ACTUATOR_CFG_FLAG_NOLOG is set
// Otherwise, don't log any actuator in ACTUATORS[] unless ACTUATOR_CFG_FLAG_LOG is set
#define LOG_ACTUATORS_BY_DEFAULT 0
//
// Rotate the log file after LOG_LINES_PER_FILE lines have been written to the
// current one
// If set, this must be > 1.
// Set to 0 to disable.
#define LOG_LINES_PER_FILE ((7 * 24 * 60) / LOG_APPEND_MINUTES) // 7 days per file
//
// Log file name pattern
// Names are 8.3 format (8 characters for the name + 3 for an extension), but
// can include directories.
// The path is relative to the storage root and any parent directories must exist.
// The maximum path size is 255 bytes.
// If LOG_LINES_PER_FILE is > 0, this name is modified at the 5th and 6th places
// before the end and those characters must be 'XX'.
#define LOG_FILE_NAME_PATTERN "STATUSXX.LOG"
//
// If set, keep track of the current log file name
// Without this LOGFILE_NAME_PATTERN does nothing and LOG_LINES_PER_FILE just
// reprints the header when the file would otherwise be rotated.
#define USE_LOG_FILE_NAME (WRITE_LOG_TO_SD)

//
// The user-defined code handling sensor/controller/actuator/logging definitions
// also needs to be configured but the settings involved will vary depending on
// implementation. To keep things organized, those are all placed in a separate
// configuration file which is included here.
#include "config_impl.h"
