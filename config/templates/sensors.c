/*
*
* Sensor definitions
* See sensors.h for structure documentation
*
*/
// Be sure to change SENSOR_COUNT in config.h if the number of sensors changes.
// If using controllers, the indexes of the sensors should be tracked to make
// referencing them less error-prone. The easy way to do this is just to #define
// a name for each sensor after it's definition.
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
	.scale = 200, // Scale the final value to 200%, done here to account for the
	              // voltage divider needed to read values > Vcc.
	              // See also the macros MULTIPLY_BY(), DIVIDE_BY(), and
	              // CALC_VDIV_SCALE() defined in sensors.h.
},
#define BATTERY 3

};

/*
// X: A linear voltage sensor, like a diode:
{
	.name  = "DIT_SEN",
	.pin   = SENSOR_TEMP_PIN,

	.type = SENS_ADC_LINEAR,
	.cflags = SENS_FLAG_VOLTS,
	.devcfg = {
		.linear.ref_value = 0,   // Reference value, here 0C
		.linear.ref_input = 600, // Voltage in mV at reference value
		.linear.slopeX10 = -20,  // Change in tenths of a mV between value steps
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
