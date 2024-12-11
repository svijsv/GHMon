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
// Constraints to the buffer size will be available RAM and reliability of the
// power supply and log file medium.
// Set to 0 to disable buffering.
#define LOG_LINE_BUFFER_COUNT 15 // 4h with 15m intervals
//
// If set, log each sensor in SENSORS[] unless SENSOR_CFG_FLAG_NOLOG is set
// Otherwise, don't log any sensor in SENSORS[] unless SENSOR_CFG_FLAG_LOG is set
#define LOG_SENSORS_BY_DEFAULT 1
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
// These settings are for the sensor_defs.h and controller_defs.h configuration
// files
// They are specific to the implementation used here and may vary over time and
// configuration
//
// The beta coefficient of any thermistors
#define THERMISTOR_BETA_COEFFICIENT 3950U
//
// The reference value of any thermistors in degrees Celsius
#define THERMISTOR_REFERENCE_VALUE  25U
//
// The resistance at the reference value of any thermistors
#define THERMISTOR_REFERENCE_OHMS   20000U
//
// The resistance of the series resister used with any thermistors
#define THERMISTOR_SERIES_OHMS      22000U
//
// The series resistor for any voltage-divider-based moisture sensors
#define MOISTURE_SERIES_OHMS 10000U
//
// The values of the low- and high-side resistors used in the voltage divider to
// measure the battery voltage
// The relative values are what matters so e.g. 10 and 1 are the same as 10000
// and 1000
// The low side must be > 0
#define BATTERY_VDIV_HIGH_SIDE_OHMS 15150U
#define BATTERY_VDIV_LOW_SIDE_OHMS 6740U
//#define BATTERY_VDIV_HIGH_SIDE_OHMS 150U
//#define BATTERY_VDIV_LOW_SIDE_OHMS 68U
//
// If set, voltage divider-based sensors are read as having the fixed-value
// resistor on the high side and the variable resistor on the low side
#define SERIES_R_IS_HIGH_SIDE 1
//
// If set, record temperature in degrees Fahrenheit instead of Celsius
#define USE_FAHRENHEIT 1
//
// If > 1, scale temperatures by this factor (e.g. by 10 in order to track by
// tenths of a degree)
#define TEMPERATURE_SCALE 1

//
// The temperature threshold above which an inactive fan is turned on
#define COOL_ON_THRESHOLD 85
//
// The temperature threshold below which an active fan is turned off
#define COOL_OFF_THRESHOLD 75
//
// The temperature threshold below which an inactive heater is turned on
#define HEAT_ON_THRESHOLD 55
//
// The temperature threshold above which an active heater is turned off
#define HEAT_OFF_THRESHOLD 60
//
// If soil resistance is >= this many ohms, consider it dry
#define MOIST_READING_DRY 10000

//
// These settings are for the logfile.h configuration file
// They are specific to the implementation used here and may vary over time and
// configuration
// If set, write the log file to an SD card
// SPI_CS_SD_PIN must be set to the chip-select pin that controls the SD card.
#define WRITE_LOG_TO_SD 1
//
// If opening the SD log output fails and this isn't set, don't write any other
// log outputs either to avoid flushing the buffer
#define LOG_WITH_MISSING_SD 0
//
// If set, write log output to a UART port
// All log buffering is still applied.
#define WRITE_LOG_TO_UART 1
//
// If opening the uart log output fails and this isn't set, don't write any other
// log outputs either to avoid flushing the buffer
#define LOG_WITH_MISSING_UART 1
//
// The TX and RX pins the log is written to
// If 0, use the standard communication port.
#define UART_LOG_TX_PIN 0
#define UART_LOG_RX_PIN 0
//
// The UART settings for log output when not using the standard port
#define UART_LOG_BAUDRATE 9600UL
#define UART_LOG_TIMEOUT_MS 1000UL
//
// The pin used to control power to the logging device(s)
// Unused if 0.
#define LOG_POWER_PIN 0
//
// The number of milliseconds to delay after turning on the power pin to allow
// the source to stabilize
#define LOG_POWER_UP_DELAY_MS 100
//
// The number of milliseconds to delay before turning off the power pin to allow
// the devices to shut down properly.
// An SD card will need a period between when it's gone inactive and when the
// power is removed to minimize chances of corrupted data; see
// https://github.com/greiman/SdFat/issues/21
// by way of
// https://thecavepearlproject.org/2017/05/21/switching-off-sd-cards-for-low-power-data-logging/
#define LOG_POWER_DOWN_DELAY_MS 100
