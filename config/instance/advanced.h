//
// Advanced Program Configuration
// This stuff is kept out of the user configuration file because it will rarely
// need to be changed 
//

//
// These are sub-features of USE_SMALL_BUILD
#define USE_SMALL_SENSORS     USE_SMALL_BUILD
#define USE_SMALL_CONTROLLERS USE_SMALL_BUILD
#define USE_SMALL_CODE        USE_SMALL_BUILD
#define SKIP_SAFETY_CHECKS    USE_SMALL_BUILD
#define SKIP_LIB_SAFETY_CHECKS SKIP_SAFETY_CHECKS

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
// Number of bits used for the signed integer holding sensor statuses
// Normally defaults to 16, but certain sensors may override that.
#define STATUS_BITS 0
//
// The type used for elements in lookup tables
#define LUT_T int16_t

//
// A controller with the CTRL_FLAG_RETRY flag set will check it's sensors'
// status at most this many additional times before stopping and issuing a
// warning
#define CONTROLLER_RETRY_MAX 2
//
// Wait this many seconds between attempts when CTRL_FLAG_RETRY is set
#define CONTROLLER_RETRY_DELAY_SECONDS 30
//
// Wait this many seconds between checking the controller 'stop' pin when it's
// set
// If set to 0, poll the stop pin continuously; nothing else will happen until
// either the stop pin goes high or the controller times out.
#define CONTROLLER_STOP_CHECK_SECONDS 5
//
// The number of sensors associated with each controller
// If <= 0, all controllers are triggered at their respective scheduled times.
#define CONTROLLER_INPUTS_COUNT 1
//
// The number of control pins associated with each controller
// Must be >=1.
#define CONTROLLER_OUTPUTS_COUNT 1

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
