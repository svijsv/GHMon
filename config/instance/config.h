//
// Program Configuration
//
// Notes:
//    There are some advanced configuration options in advanced.h
//

//
// Optional components
// Set to '0' or comment out a line to disable corresponding feature
//
// Cut down the size of internal components at the expense of functionality
#ifndef USE_SMALL_BUILD
# define USE_SMALL_BUILD 0
#endif
//
// Enable sensors
#define USE_SENSORS 0
//
// Enable external device controllers (water pumps, fans, heaters, etc)
#define USE_CONTROLLERS 0
//
// Enable data logging to SD cards
#define USE_LOGGING 0
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
// The number of sensors used
// Be sure to change the G_sensors[] definition in config.c if this changes.
// Must be >= 1 and <=255.
#define SENSOR_COUNT 4
//
// The number of elements in sensor lookup tables
#define LUT_SIZE 10

//
// Controller configuration
//
// Check whether any controllers need to be engaged every
// CONTROLLER_CHECK_MINUTES minutes
// If 0, they're checked when the user button is pressed.
// Individual controllers may have their own periods specified, in which case
// they ignore this.
// Press the user button and release after the third LED flash to force a
// check of all controllers, even those with their own schedules.
#define CONTROLLER_CHECK_MINUTES 15
//
// If a controller was scheduled to run fewer than this many minutes in the past
// but for some reason wasn't (e.g. because the clock changed or the device
// restarted), run it now
#define CONTROLLER_SCHEDULE_SCEW_WINDOW_MINUTES 15
//
// The number of controllers to manage
// Be sure to change the G_controllers[] definition in config.c if this changes.
// Must be >=1 and <= 255.
#define CONTROLLER_COUNT 2

//
// Data logging configuration
//
// Check the status and append it to the log every LOG_APPEND_MINUTES minutes
// If 0, it's checked only when the user button is pressed.
#define LOG_APPEND_MINUTES 15
//
// Log file name pattern
// Names are 8.3 format (8 characters for the name + 3 for an extension), but
// can include directories.
// The path is relative to the SD card root and parent directories must exist.
// This name is modified at the 5th and 6th places before the end if
// LINES_PER_FILE is > 0.
#define LOGFILE_NAME_PATTERN "STATUSXX.LOG"
//
// Instead of writing to disk every time, store this many readings into memory
// and write them all to disk when the buffer is full.
// With 10 minute intervals, a 5-reading buffer will write once an hour (5
// buffered + the newest).
// Constraints to the buffer size will be available RAM and reliability of the
// power supply and log file medium.
// The maximum buffer size is 65,535 (0xFFFF) lines.
// Set to 0 to disable buffering.
// Press the user button and release after the second LED flash to force a
// write to the SD card.
#define LOGFILE_BUFFER_COUNT 15 // 4h with 15m intervals
//
// Rotate the log file after LINES_PER_FILE lines have been written to the
// current one
// If set, this must be > 1.
// Set to 0 to disable.
#define LINES_PER_FILE ((7 * 24 * 60) / LOG_APPEND_MINUTES) // 7 days per file
