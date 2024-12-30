//
// These are the settings controlling the use-defined code handling sensors,
// controllers, actuators, and logging. These particular settings are used by
// the example implementation.
//

//
// General configuration
//
// If set, record temperature in degrees Fahrenheit instead of Celsius
#define USE_FAHRENHEIT 1
//
// If > 1, scale temperatures by this factor (e.g. by 10 in order to track by
// tenths of a degree)
#define TEMPERATURE_SCALE 1

//
// sensor_defs.h configuration
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
// controller_defs.h configuration
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
// logfile.h configuration
//
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
