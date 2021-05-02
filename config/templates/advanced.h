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
// CTRL_PRESS x 3  : Set time of day to 12:00:00
// CTRL_PRESS x >=4: Cancel
#define CTRL_PRESS (2000)

// The UART communication parameters
#define UART_BAUDRATE 9600

// SPI communication parameters
// Target speed of the SPI bus; it will generally be somewhat higher due to
// hardware limitations
#define SPI_FREQUENCY 100000

// I2C/TWI communication parameters
// Target speed of the I2C bus
#define I2C_FREQUENCY 50000

// Maximum length of sensor and controller names
// Increasing this will increase the ROM space used, but depending on struct
// layout it may not be a direct relationship; typically rounding the number
// up to the next multiple of 4 will give the number of bytes used (so that
// setting it to 8 would use 12 bytes).
#define DEVICE_NAME_LEN 7

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
#define CONTROLLER_STOP_CHECK_SECONDS 5
// The number of sensors associated with each controller.
// Must be >= 1 and <= 255.
#define CONTROLLER_SENS_COUNT 1

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
