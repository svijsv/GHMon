//
// Advanced Program Configuration
// This stuff is kept out of the user configuration file because it will rarely
// need to be changed 
//

//
// These are sub-features of USE_SMALL_BUILD
#define USE_SMALL_CODE        USE_SMALL_BUILD
#define SKIP_SAFETY_CHECKS    USE_SMALL_BUILD
#define SKIP_LIB_SAFETY_CHECKS SKIP_SAFETY_CHECKS
//
// These are sub-features of USE_SMALL_CONTROLLERS
// The configuration and status structs will change depending on these values
#define USE_CONTROLLER_SCHEDULE (!USE_SMALL_CONTROLLERS)
#define USE_CONTROLLER_INIT     1
#define USE_CONTROLLER_NAME     (!USE_SMALL_CONTROLLERS)
#define USE_CONTROLLER_DATA     (!USE_SMALL_CONTROLLERS)
#define USE_CONTROLLER_STATUS   (!USE_SMALL_CONTROLLERS)
#define USE_CONTROLLER_NEXTTIME (!USE_SMALL_CONTROLLERS)
//
// These are sub-features of USE_SMALL_SENSORS
// The configuration and status structs will change depending on these values
#define USE_SENSOR_NAME     (!USE_SMALL_SENSORS)
#define USE_SENSOR_INIT     (!USE_SMALL_SENSORS)
#define USE_SENSOR_COOLDOWN (!USE_SMALL_SENSORS)
#define USE_SENSOR_CFG_DATA (!USE_SMALL_SENSORS)
#define USE_SENSOR_CFG_PIN  (!USE_SMALL_SENSORS)
#define USE_SENSOR_DATA     (!USE_SMALL_SENSORS)

//
// Delays during status LED display
#define STATUS_LED_ERR_DELAY_MS 200U
#define STATUS_LED_ACK_DELAY_MS 200U
#define STATUS_LED_LONG_DELAY_MS 500U

//
// Hold the control button this many milliseconds to force an action (only one
// action will be taken)
// The LED will flash when pressed and then every CTRL_PRESS milliseconds, and
// the same number of times again when the button is released to indicate the
// action being taken.
// CTRL_PRESS x 0  : Take any actions with a check period of 0
// CTRL_PRESS x 1  : Sync log data to file
// CTRL_PRESS x 2  : Check device controllers
// CTRL_PRESS x 3  : Set time of day to 00:00:00 + RESET_TIME_OFFSET_MINUTES
// CTRL_PRESS x >=4: Cancel
#define CTRL_PRESS_MS (1500)
//
// Delay this many milliseconds after the first button press is registered
// to avoid reading an early release due to switch bounce
#define BUTTON_DEBOUNCE_MS 200
//
// When the button is held long enough the time is normally set to 00:00:00;
// with this that time can be offset by the given number of minutes. The
// value must be >= 0 and < 24*60
#define RESET_TIME_OFFSET_MINUTES (12 * MINUTES_PER_HOUR) // 12:00:00

//
// Every RTC_CORRECTION_PERIOD_MINUTES the RTC is adjusted by
// RTC_CORRECTION_SECONDS to account for any known clock drift. Disabled if
// either is 0.
#define RTC_CORRECTION_PERIOD_MINUTES 60
#define RTC_CORRECTION_SECONDS 0
//
// Every RTC_FINE_CORRECTION_PERIOD_MINUTES the RTC is adjusted by
// RTC_FINE_CORRECTION_SECONDS to account for any known clock drift. Disabled
// if either is 0. This is to provide for a second, finer-grained adjustment.
#define RTC_FINE_CORRECTION_PERIOD_MINUTES (60*24)
#define RTC_FINE_CORRECTION_SECONDS 0

//
// If set to 1, measure the analog voltage reference on power-up to give
// (hopefully) more accurate ADC readings
// If set to 2, measure the analog voltage reference regularly to account for
// variations
// If set to 3, measure the analog voltage reference before each use of the ADC
// This must be set to 2 to warn if the regulated voltage drops below the
// warning threshold
// The utility of this calibration depends on the accuracy of INTERNAL_VREF_mV
#define CALIBRATE_VREF 1
//
// The minimum number of minutes between voltage reference measurements when
// CALIBRATE_VREF is set to 2
#define CALIBRATE_VREF_COOLDOWN_MINUTES 15

//
// Maximum length of sensor and controller names
// Increasing this will increase the ROM space used, but depending on struct
// layout it may not be a direct relationship; typically rounding the number
// up to the next multiple of 4 will give the number of bytes used (so that
// setting it to 8 would use 12 bytes).
#define DEVICE_NAME_LEN 11
//
// The type used to store sensor reading values
#define SENSOR_READING_T int32_t
//
// The value returned when there's an error reading a sensor
#define SENSOR_BAD_VALUE INT32_MIN
//
// The type used to store sensor configuration data
#define SENSOR_CFG_DATA_T uint32_t
//
// The type used to store sensor array indexes
// The maximum sensor count is determined by this type
// The only negative effect of a larger type is correspoindingly larger code
#define SENSOR_INDEX_T int_fast8_t
//
// The default minimum period between reads of a single sensor
#define SENSOR_COOLDOWN_SECONDS 90

//
// The type used to hold controller status values
#define CONTROLLER_STATUS_T int
//
// The type used to store controller array indexes
// The maximum controller count is determined by this type
// The only negative effect of a larger type is correspoindingly larger code
#define CONTROLLER_INDEX_T int_fast8_t

//
// The number of bytes used for the SD card write buffer
// FatFS has it's own 512-byte buffer so when the size of all the data to be
// written is <= 512 bytes this just serves to minimize the number of write
// calls that need to be made to the FatFS library.
#define LOG_PRINT_BUFFER_SIZE 2048
//
// The string printed to the log for invalid values
#define LOG_INVALID_VALUE "(invalid)"
//
// The string printed to the log for fields that aren't supported
#define LOG_NO_VALUE "(unavailable)"
//
// The string printed at the end of each log line
#define LOG_LINE_END "\r\n"
