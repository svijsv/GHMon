/*
* Macros to aid in sensor configuration
*/
// VDIV-to-scale calculation: (100) * (Rv + Rg) / Rg
// Rv is resistance from the supply to the test point, Rg is test point to
// ground.
#define CALC_VDIV_SCALE(Rv, Rg) ((((uint64_t )100) * ((uint64_t )Rv + (uint64_t )Rg)) / (uint64_t )Rg)
// Calculate a multiplier
#define MULTIPLY_BY(mul) ((mul) * (100))
// Calculate a divider
#define DIVIDE_BY(div) ((100) / (div))
//
// Convert Celsius temperature to Fahrenheit; both of these macros need to
// be used
// T(°F) = T(°C) × 1.8 + 32
#define C_TO_F_SCALE (180)
// The adjustment is applied before the scaling, so it needs to be converted
// to Celsius
// Add half the denominator to round to nearest instead of truncating when it's
// converted to an int
#define C_TO_F_ADJUST ((32.0f + 0.9f)/1.8f)


/*
*
* Sensor definitions
* See src/sensors.h and the headers in src/sensors for structure documentation
*
* Be sure to change SENSOR_COUNT in config.h if the number of sensors changes.
*
* If using controllers, the indexes of the sensors should be tracked to make
* referencing them less error-prone. The easy way to do this is just to #define
* a name for each sensor index after it's definition.
*
* Status adjustments are made prior to scaling.
*
*/
_FLASH const sensor_static_t SENSORS[SENSOR_COUNT] = {
/*
* 0: Greenhouse temperature */ {
	.name  = "TID_SEN", // Logging name, max size 7 characters by default
	.pin   = SENSOR_TID_PIN, // SENSOR_TID_PIN defined in config/config.h

	.warn_above = 35, // Issue a warning if above 35C/95.0F
	.warn_below =  5, // Issue a warning if below 5C/41.0F

	.type = SENS_ADC_BETA_R, // Calculate the temperature with a reference point
	                         // and a beta coefficient
	.devcfg = {
		.beta_R.ref_value = 298,   // Thermistor reference temperature, 25C + 273.15K
		.beta_R.ref_ohms  = 20000, // Thermistor reference resistance
		.beta_R.beta      = 3950,  // Thermistor beta coefficient
		.beta_R.series_R_ohms = 22000, // Series resistor (Vcc -> Rs -> MCU_pin -> thermistor -> GND)
	},
	.adjust = -273, // Convert to Celsius
},
#define TEMP_TID 0

/*
* 1: Outdoor temperature */ {
	.name  = "TOD_SEN",
	.pin   = SENSOR_TOD_PIN,

	.type = SENS_ADC_BETA_R,
	.devcfg = {
		.beta_R.ref_value = 298,
		.beta_R.ref_ohms  = 20000,
		.beta_R.beta      = 3950,
		.beta_R.series_R_ohms = 22000,
	},
	.adjust = -273,
},
#define TEMP_TOD 1

/*
* 2: Soil moisture */ {
	.name  = "SM0_SEN",
	.pin   = SENSOR_WATER_PIN,

	.type = SENS_ADC_OHM, // Calculate moisture level based directly on resistance
	                      // of the sensor
	.devcfg = {
		.ohm.series_R_ohms = 47000,  // Series resistor (Vcc -> Rs -> MCU_pin -> sensor -> Gnd)
	},
},
#define WATER 2

/*
* 3: Supply voltage */ {
	.name  = "BAT_SEN",
	.pin   = SENSOR_BAT_PIN,
	.cflags = SENS_FLAG_BATTERY, // If voltage is below warn_below, go into
	                             // power-saving mode

	.warn_above = SENS_THRESHOLD_IGNORE, // Don't worry about high voltage
	.warn_below =  4000, // Warn if below 4000mV

	.type = SENS_ADC_VOLT, // Voltage sensors need no further device configuration
	.scale_percent = 200, // Scale the final value to 200%, done here to account
	                      // for the voltage divider needed to read values > Vcc.
	                      // See also the macros MULTIPLY_BY(), DIVIDE_BY(),
	                      // CALC_VDIV_SCALE() defined at the top of this file.
},
#define BATTERY 3

};

/*
// X: A linear voltage sensor, like a diode:
{
	.name  = "DIT_SEN",
	.pin   = SENSOR_TEMP_PIN,

	.type = SENS_ADC_VOLT,
	.devcfg = {
		.volt.ref_value = 0,   // Reference value, here 0C
		.volt.ref_mV = 600, // Voltage in mV at reference value
		.volt.slopeX10 = -20,  // Change in tenths of a mV between value steps
	},
},

// X: A lookup-table-based sensor, like a precomputed thermistor:
{
	.name  = "LUT_SEN",
	.pin   = SENSOR_TEMP_PIN,

	.type = SENS_ADC_LOOKUP,
	.devcfg = {
		.lookup.lutno = TAB_VF_3950B, // TAB_VF_3950B is defined in tables.c
	},
},
*/
