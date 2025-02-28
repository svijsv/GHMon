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
// If set, measure the ADC voltage reference instead of using REGULATED_VOLTAGE_mV
// to facilitate unregulated power sources (e.g. batteries)
#define CALIBRATE_VREF 0

//
// Sensor configuration
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
// The values of the low- and high-side resistors used in the voltage divider to
// measure the pre-regulator input voltage
// The relative values are what matters so e.g. 10 and 1 are the same as 10000
// and 1000
// These are managed with 16-bit ints so keep the values small
// The low side must be > 0
#define VIN_VDIV_HIGH_SIDE_OHMS 100U
#define VIN_VDIV_LOW_SIDE_OHMS 33U
//
// If set, voltage divider-based sensors are read as having the fixed-value
// resistor on the high side and the variable resistor on the low side
#define SERIES_R_IS_HIGH_SIDE 1

//
// Controller configuration
//
// If the thermistor reading the water temperature is <= this value, the water is
// considered warm enough for pumping
// If this is 0, calculate the temperature using the thermistor values defined
// above and PUMP_MIN_WATER_TEMP below.
//#define WATER_TEMP_OK_OHMS 46850U // 7C, ~45F
#define WATER_TEMP_OK_OHMS 40350U // 10C, 50F
//#define WATER_TEMP_OK_OHMS 27500U // 18C, ~65F
//
// The minimum water reservoir temperature required to enable pumping
// Only used if WATER_TEMP_OK_OHMS is 0.
#define PUMP_MIN_WATER_TEMP 45U
//
// The minimum Vin required to enable pumping
#if defined(DEBUG) && DEBUG
# define PUMP_MINIMUM_VIN_mV (4000)
#else
# define PUMP_MINIMUM_VIN_mV (1200U * 4U)
#endif
//
// If set, the water level is considered OK when the sense pin is pulled down.
// Otherwise it's considered OK when pulled up.
#define WATER_LEVEL_OK_LOW 1
