/*
*
* Program Configuration
*
* Notes:
*    Sensors and controllers have their own configuration files:
*       sensors.c
*       controllers.c
*    Sensor lookup tables (if used) are defined in tables.c
*    There are some advanced configuration options in advanced.h
*/

//
// Optional components
// Set to '0' or comment out or line to disable corresponding feature
//
// Cut down the size of internal components at the expense of functionality
// If 1, use smaller structures for data in RAM
// If 2, also use smaller structures for data in ROM
#define USE_SMALL_BUILD 0
// Enable serial output
#define USE_SERIAL 0
// Enable the interactive serial terminal
#define USE_TERMINAL 0
// Enable SD card formatting from the serial terminal
#define USE_FDISK 0
// Enable external device controllers (water pumps, fans, heaters, etc)
#define USE_CONTROLLERS 1
// Enable data logging to SD cards
#define USE_LOGGING 1
//
// Supported sensor types
//
// Direct reading of voltage, measured in millivolts
#define USE_VOLT_SENSOR 1
// Direct reading of resistance, measured in ohms
#define USE_OHM_SENSOR 1
// Sensors such as thermistors with non-linear resistance changes
#define USE_LOG_BETA_SENSOR 1
// Sensors with linear resistance changes
#define USE_LINEAR_R_SENSOR 0
// Sensors with linear voltage changes
#define USE_LINEAR_V_SENSOR 0
// Sensors with resistance lookup tables
#define USE_LOOKUP_R_SENSOR 0
// Sensors with voltage lookup tables
#define USE_LOOKUP_V_SENSOR 0
// Digital pin sensors; either '1' (high) or '0' (low)
#define USE_BINARY_SENSOR 0


//
// Sensor configuration
//
// Check the status and emit warning blinks if called for every
// STATUS_CHECK_PERIOD minutes
// If 0, it's checked when the user button is pressed
#define STATUS_CHECK_PERIOD 15

// The number of sensors used
// Be sure to change the G_sensors[] definition in config.c if this changes.
// Must be >= 1 and <=255.
#define SENSOR_COUNT 4

// Number of elements in sensor lookup tables
#define LUT_SIZE 10


//
// Controller configuration
//
// Check whether any controllers need to be engaged every
// CONTROLLER_CHECK_PERIOD minutes
// If 0, they're checked when the user button is pressed.
// Individual controllers may have their own periods specified, in which case
// they ignore this.
// Press the user button and release after the third LED flash to force a
// check of all controllers, even those with their own schedules.
#define CONTROLLER_CHECK_PERIOD 15

// Number of controllers to manage
// Be sure to change the G_controllers[] definition in config.c if this changes.
// Must be >=1 and <= 255.
#define CONTROLLER_COUNT 2


//
// Data logging configuration
//
// Check the status and append it to the log every LOG_APPEND_PERIOD minutes
// If 0, it's checked when the user button is pressed.
#define LOG_APPEND_PERIOD 15

// Log file name pattern
// Names are 8.3 format (8 characters for the name + 3 for an extension), but
// can include directories.
// The path is relative to the SD card root and parent directories must exist.
// This name is modified at fixed positions (the 7th and 8th characters in) if
// LINES_PER_FILE is > 0.
#define LOGFILE_NAME_PATTERN "STATUSXX.LOG"

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

// Rotate the log file after LINES_PER_FILE lines have been written to the
// current one
// If set, this must be > 1.
// Set to 0 to disable.
#define LINES_PER_FILE ((14 * 24 * 60) / LOG_APPEND_PERIOD) // 14 days per file


//
// Pin configuration
// Platform-specific
//
// STM32F103
// Sensors like thermistors need to be on a pin that supports ADC; that means
// A0-A7 or B0-B1
//
// Pins A15, B3, and B4 are JTAG and will be pulled up or down when flashing
// and during power on, and so shouldn't be used to power anything that doesn't
// like that (like an SD card) if it will be present at those times
//
// Some pins (like A12) may have external pullups or pulldowns
//
// Sensor inputs
#define SENSOR_BAT_PIN    PA0
#define SENSOR_TOD_PIN    PA1
#define SENSOR_TID_PIN    PA2
#define SENSOR_WATER_PIN  PA3
//
// Controller outputs
#define CONTROL_FAN_PIN   PB9
#define CONTROL_WATER_PIN PB8
//
// UI pins
#define LED_PIN    PB3
#define BUTTON_PIN PB4
//
// Power pins
#define SENSOR_POWER_PIN PB11
//
// The SD card should be switched only if it uses a separate voltage regulator
// from the main board and a few mA power usage matters; powering them on and
// off is sometimes unreliable
// If a power pin has a bias, the pin is always in push-pull mode and it's
// output is set to the bias value when OFF and the reverse when ON; the
// default is high-impedence mode when OFF and output HIGH when ON
//#define SD_POWER_PIN (PB5 | BIAS_LOW)
//
// UART pins; hardware-specified; used if USE_SERIAL != 0
// PA9-PA10: UART1
#define UARTx_TX_PIN  PA9
#define UARTx_RX_PIN PA10
//
// SPI pins; hardware-specified; used if USE_LOGGING != 0
// Use SPI2 because SPI1 overlaps with the analog pins and we want to save as
// many of those as we can for sensors.
// PB12-PB15: SPI2
#define SPIx_CS_SD_PIN PB12
#define SPIx_SCK_PIN   PB13
#define SPIx_MISO_PIN  PB14
#define SPIx_MOSI_PIN  PB15
