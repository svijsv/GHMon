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
// Set to '0' or comment out a line to disable corresponding feature
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
// Sensors with a linear relationship between voltage output and sensor value
#define USE_VOLT_SENSORS 1
// Sensors with a linear relationship between resistance and sensor value
#define USE_OHM_SENSORS 1
// Sensors such as thermistors with reference values and beta coefficients
// used to calculate non-linear resistance changes
#define USE_BETA_R_SENSORS 1
// Sensors with resistance or voltage lookup tables
#define USE_LOOKUP_SENSORS 0
// Digital pin sensors; either '1' (high) or '0' (low)
#define USE_BINARY_SENSORS 0
// DHT11 temperature and humidity sensor
#define USE_DHT11_SENSORS 0
// BMP280 and BME280 temperature, air pressure, and humidity sensors
#define USE_BMx280_SPI_SENSORS 0
#define USE_BMx280_I2C_SENSORS 0


//
// Sensor configuration
//
// Check the status and emit warning blinks if called for every
// STATUS_CHECK_MINUTES minutes
// If 0, it's checked when the user button is pressed
#define STATUS_CHECK_MINUTES 15

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
// CONTROLLER_CHECK_MINUTES minutes
// If 0, they're checked when the user button is pressed.
// Individual controllers may have their own periods specified, in which case
// they ignore this.
// Press the user button and release after the third LED flash to force a
// check of all controllers, even those with their own schedules.
#define CONTROLLER_CHECK_MINUTES 15

// Number of controllers to manage
// Be sure to change the G_controllers[] definition in config.c if this changes.
// Must be >=1 and <= 255.
#define CONTROLLER_COUNT 2


//
// Data logging configuration
//
// Check the status and append it to the log every LOG_APPEND_MINUTES minutes
// If 0, it's checked when the user button is pressed.
#define LOG_APPEND_MINUTES 15

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
#define LINES_PER_FILE ((14 * 24 * 60) / LOG_APPEND_MINUTES) // 14 days per file


//
// Pin configuration
// Platform-specific
//
// If a power pin isn't defined or is 0, it's assumed anything it would
// normally control is always powered on
//
// If a power pin has a bias, the pin is always in push-pull mode and it's
// output is set to the bias value when OFF and the reverse when ON; the
// default is high-impedence mode when OFF and output HIGH when ON
//
// If any device on the SPI or I2C buses are power-switched then ALL devices
// on that same bus (as well as the SDA and SCL pullups in the case of I2C)
// must also be switched to prevent damage that might be caused by applying
// voltage to the IO pins of an unpowered device or current leaking through
// the IO pins
//
// The UART pins are only used if serial output is enabled
// The SPI pins are only used if logging or an SPI sensor are enabled
// The I2C pins are only used if an I2C sensor is enabled
//
//
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
#if USE_STM32
//
// Sensor inputs
#define SENSOR_BAT_PIN    PIN_A0
#define SENSOR_TOD_PIN    PIN_A1
#define SENSOR_TID_PIN    PIN_A2
#define SENSOR_WATER_PIN  PIN_A3
//
// Controller outputs
#define CONTROL_FAN_PIN   PIN_B9
#define CONTROL_WATER_PIN PIN_B8
//
// UI pins
#define LED_PIN    PIN_B3
// Using the internal pulldown removes an external component at the cost of
// very slightly increased power usage
#define BUTTON_PIN (PIN_B4|BIAS_LOW)
//
// Power pins
// Power for sensors not on the SPI or I2C buses
#define SENSOR_POWER_PIN PIN_B11
// Power for SPI devices
//#define SPI_POWER_PIN (PIN_B5 | BIAS_LOW)
// Power for I2C devices
//#define I2C_POWER_PIN (PIN_A11)
//
// UART pins; hardware-specified
// PIN_A9-PIN_A10: UART1
#define UART_TX_PIN PIN_A9
#define UART_RX_PIN PIN_A10
//
// SPI pins; hardware-specified
// PIN_B12-PIN_B15: SPI2
#define SPI_CS_SD_PIN PIN_B12
#define SPI_SCK_PIN   PIN_B13
#define SPI_MISO_PIN  PIN_B14
#define SPI_MOSI_PIN  PIN_B15
//
// I2C pins; hardware-specified
// PIN_B6-PIN_B7: I2C1
#define I2C_SCL_PIN PIN_B6
#define I2C_SDA_PIN PIN_B7

#endif // USE_STM32

//
// ATMega328
// Sensors like thermistors need to be on a pin that supports ADC; that means
// A0-A5 as well as A6 and A7 when present
//
// Pins A6 and A7 don't have digital buffers and can only be used for analog
// input
//
// There are internal pullups but no internal pulldowns
//
#if USE_AVR
//
// Sensor inputs
#define SENSOR_BAT_PIN    PIN_A0
#define SENSOR_TOD_PIN    PIN_A1
#define SENSOR_TID_PIN    PIN_A2
#define SENSOR_WATER_PIN  PIN_A3
//
// Controller outputs
#define CONTROL_FAN_PIN   PIN_4
#define CONTROL_WATER_PIN PIN_5
//
// UI pins
#define LED_PIN    PIN_2
#define BUTTON_PIN PIN_3
//
// Power pins
// Power for sensors not on the SPI or I2C buses
#define SENSOR_POWER_PIN PIN_6
// Power for SPI devices
//#define SPI_POWER_PIN   PIN_7
// Power for I2C devices
//#define I2C_POWER_PIN   PIN_8
//
// UART pins; hardware-specified
#define UART_TX_PIN PIN_TX
#define UART_RX_PIN PIN_RX
//
// SPI pins; hardware-specified
// The hardware SS pin should be used as a slave-select pin because if it's
// ever set low as an input it will force the MCU into SPI slave mode
#define SPI_SS_PIN    PIN_10
#define SPI_CS_SD_PIN SPI_SS_PIN
#define SPI_SCK_PIN   PIN_13
#define SPI_MISO_PIN  PIN_12
#define SPI_MOSI_PIN  PIN_11
//
// I2C pins; hardware-specified
// The SDA and SCL pins are the same as pins A4 and A5, even though they may
// have separate pins broken out
#define I2C_SDA_PIN PIN_A4
#define I2C_SCL_PIN PIN_A5
#endif // USE_AVR
