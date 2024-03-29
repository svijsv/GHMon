/*
*
* Advanced Program Configuration
*
*/

//
// Miscellaneous configuration
// This stuff is kept out of the user configuration file because it will rarely
// need to be changed 
//
// These are sub-features of USE_SMALL_BUILD and will default to that value
// when not set
//#define USE_SMALL_SENSORS     0
//#define USE_SMALL_CONTROLLERS 0
//#define USE_SMALL_CODE        0

// Hold the trigger button this many milliseconds to force an action (only one
// action will be taken)
// The LED will flash when pressed and then every CTRL_PRESS milliseconds, and
// the same number of times again when the button is released to indicate the
// action being taken.
// CTRL_PRESS x 0  : Take any actions with a check period of 0
// CTRL_PRESS x 1  : Sync log data to file
// CTRL_PRESS x 2  : Check device controllers
// CTRL_PRESS x 3  : Set time of day to 00:00:00 + RESET_TIME_OFFSET_MINUTES
// CTRL_PRESS x >=4: Cancel
#define CTRL_PRESS (2000)

// Delay this many milliseconds after the first button press is registered
// to avoid reading an early release due to switch bounce
#define BUTTON_DEBOUNCE_MS 30

// When the button is held long enough the time is normally set to 00:00:00;
// with this that time can be offset by the given number of minutes. The
// value must be >= 0 and < 24*60
#define RESET_TIME_OFFSET_MINUTES (12 * 60) // 12:00:00
// Every RTC_CORRECTION_PERIOD_MINUTES the RTC is adjusted by
// RTC_CORRECTION_SECONDS to account for any known clock drift. Disabled if
// either is 0.
#define RTC_CORRECTION_PERIOD_MINUTES 60
#define RTC_CORRECTION_SECONDS 0
// Every RTC_FINE_CORRECTION_PERIOD_MINUTES the RTC is adjusted by
// RTC_FINE_CORRECTION_SECONDS to account for any known clock drift. Disabled
// if either is 0. This is to provide for a second, finer-grained adjustment.
#define RTC_FINE_CORRECTION_PERIOD_MINUTES (60*24)
#define RTC_FINE_CORRECTION_SECONDS 0

// The UART communication parameters
#define UART_BAUDRATE 9600

// If >0, keep a ring buffer of logger() output for later replay for debugging
// purposes.
#define LOGGER_REPLAY_BUFFER_SIZE 0

// SPI communication parameters
// Target speed of the SPI bus; it will generally be somewhat higher due to
// hardware limitations
#define SPI_FREQUENCY 100000

// I2C/TWI communication parameters
// Target speed of the I2C bus
#define I2C_FREQUENCY 50000

// Maximum frequency for PWM output
#define PWM_MAX_FREQUENCY 250
// PWM duty cycle is defined as a number between 0 and PWM_DUTY_CYCLE_SCALE
// The effect of increasing the scale is hardware-dependent and may reduce the
// maximum possible frequency (as with STM32s) or have a limited effect on
// the actual resolution of a duty-cycle setting (as with ATMegas, which
// always convert to a scale of 0-255 internally)
#define PWM_DUTY_CYCLE_SCALE 100

// Maximum length of sensor and controller names
// Increasing this will increase the ROM space used, but depending on struct
// layout it may not be a direct relationship; typically rounding the number
// up to the next multiple of 4 will give the number of bytes used (so that
// setting it to 8 would use 12 bytes).
#define DEVICE_NAME_LEN 7

// Number of bits used for the signed integer holding sensor statuses
// Normally defaults to 16, but certain sensors may override that.
#define STATUS_BITS 0

// The type used for elements in lookup tables
#define LUT_T int16_t

// A controller with the CTRL_FLAG_RETRY flag set will check it's sensors'
// status at most this many additional times before stopping and issuing a
// warning
#define CONTROLLER_RETRY_MAX 2
// Wait this many seconds between attempts when CTRL_FLAG_RETRY is set
#define CONTROLLER_RETRY_DELAY_SECONDS 30
// Wait this many seconds between checking the controller 'stop' pin when it's
// set
// If set to 0, poll the stop pin continuously; nothing else will happen until
// either the stop pin goes high or the controller times out.
#define CONTROLLER_STOP_CHECK_SECONDS 5
// The number of sensors associated with each controller
// If <= 0, all controllers are triggered at their respective scheduled times.
#define CONTROLLER_INPUTS_COUNT 1
// The number of control pins associated with each controller
// Must be >=1.
#define CONTROLLER_OUTPUTS_COUNT 1

// Use the less-accurate but lower-power internal oscillator for the system
// clock where supported instead of the external crystal
// Depending on the platform this may affect the accuracy of timekeeping.
#define USE_INTERNAL_CLOCK 1

// If set to 1, measure the analog voltage reference on power-up to give
// (hopefully) more accurate ADC readings
// If set to 2, measure the analog voltage reference before each use of the
// ADC
// This must be set to 2 to warn if the regulated voltage drops below the
// warning threshold
// The utility of this calibration depends on the accuracy of INTERNAL_VREF_mV
#define CALIBRATE_VREF 0

// Number of samples to take and then average for every ADC reading
#define ADC_SAMPLE_COUNT 1

// The number of bytes used for the SD card write buffer
// If 0, an attempt is made to calculate this automatically; this should only
// really be set if there are RAM shortage issues and the auto-detection code
// isn't doing what it should.
// FatFS has it's own 512-byte buffer so when the size of all the data to be
// written is <= 512 bytes this just serves to minimize the number of write
// calls that need to be made to the FatFS library.
#define SD_PRINT_BUFFER_SIZE 0

//
// Hardware configuration
// Platform-specific
//
// The voltage of the internal reference used to calibrate the ADC reference
// If not set, use platform-specific default
//#define INTERNAL_VREF_mV 1200

// Ideal voltage output by the on-board voltage regulator
// If not set, use platform-specific default
//#define REGULATED_VOLTAGE_mV 3300

// Consider the regulated voltage low if it falls below this
// If not set, use platform-specific default
//#define REGULATED_VOLTAGE_LOW_mV 3100
