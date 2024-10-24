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
#define DEVICE_NAME_LEN 7
//
// The signed type used to store sensor statuses, this may need to be larger
// for certain sensor types
#define SENSOR_STATUS_T int16_t
//
// The default minimum period between reads of a single sensor, used to prevent
// sensitive sensors from being continuously powered on and off
#define SENSOR_DEFAULT_COOLDOWN_SECONDS 0

//
// The type used to hold controller status values
#define CONTROLLER_STATUS_T int

//
// The number of bytes used for the SD card write buffer
// FatFS has it's own 512-byte buffer so when the size of all the data to be
// written is <= 512 bytes this just serves to minimize the number of write
// calls that need to be made to the FatFS library.
#define SD_PRINT_BUFFER_SIZE 0

//
// Wait this many milliseconds for device power to come up when it's turned on
// In testing, 20ms was too short for reliable AC amplitude measurement and
// may have been the cause of unreliable single-measurement ADC values
// 50ms was much better but not as good as 75ms and 100ms wasn't signifantly
// better than 75ms
#define POWER_UP_DELAY_MS 75
