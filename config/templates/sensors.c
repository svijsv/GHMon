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
// Convert Celsius temperature to Fahrenheit by setting .scale_percent to
// C_TO_F_SCALE and .adjust to C_TO_F_ADJUST
// T(°F) = T(°C) × 1.8 + 32
#define C_TO_F_SCALE (180)
// The adjustment is applied before the scaling, so it needs to be converted
// to Celsius by dividing by 1.8
#define C_TO_F_ADJUST (DIV_ROUNDED(32.0f, 1.8f))


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
* Any resistance sensor needs a series resistor which is normally assumed
* to be on the high side of a voltage divider with the sensing pin in
* between and the sensor on the low side
*
* Any settings in a 'devcfg' block are specific to that sensor type, the
* rest are universal
*
* Mandatory configuration fields:
*    .name: The name of the sensor
*    .pin:  The GPIO pin the sensor is read on
*    .type: The type of sensor
*
* Optional configuration fields:
*    .cflags: change the manner of sensor reading, the flags can be ORd together
*       SENS_FLAG_INVERT: Invert the normal reading, for instance if the
*                         series resistor is on the low side of a voltage
*                         divider instead of the high side
*       SENS_FLAG_AC:     Measure the magnitude of an AC signal rather than
*                         simply reading it once; note that despite the name
*                         of the flag the signal voltage can't go negative
*                         without damaging the MCU
*
*    .adjust:        adjust the sensor value by a fixed amount
*    .scale_percent: scale the sensor value to some percent of itself after
*                    it's been adjusted
*    .warn_above:
*    .warn_below:    Issue a warning (flash the LED) when above/below the
*                    value
*
*
* There are more commented-out examples at the end
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
	.adjust = -273, // Convert from Kelvin to Celsius
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
//
// X: A binary (on/off) sensor, like a water level detector
{
	.name  = "BIN_SEN",
	.pin   = SENSOR_WATLOW_PIN|BIAS_LOW, // Use the internal pulldown if available
	                                     // on the pin to keep it from floating
},
//
// X: A linear voltage sensor
{
	.name  = "LIV_SEN",
	.pin   = SENSOR_BAT_PIN,

	.type = SENS_ADC_VOLT,
	.devcfg = {
		.volt.ref_value = 0,   // Reference value, here 0C
		.volt.ref_mV    = 600, // Voltage in mV at reference value
		.volt.slopeX10  = -20, // Change in tenths of a mV between value steps
		                       // Leave unset set to 0 to directly read voltage
		//.volt.sys_mv  = 5000, // The system voltage for which the above values
		                        // values are calculated, leave unset or set to
		                        // 0 if it doesn't matter
	},
},
//
// X: A linear resistance sensor
{
	.name  = "LIR_SEN",
	.pin   = SENSOR_LIGHT_PIN,

	.type = SENS_ADC_OHM,
	.devcfg = {
		.ohm.series_R_ohms = 22000, // The value of the series resistor in the
		                            // voltage divider used to detect resistance
		.ohm.ref_ohms      = 20000, // The resistance at the reference value
		.ohm.ref_value     = 20, // The reference value
		.ohm.slopeX10      = 100, // The resistance change in 1/10 ohms when the value
		                          // increases by 1
		                          // Leave unset set to 0 to directly read resistance
	},
},
//
// X: A resistance sensor with a reference point and a beta coefficient, like
// a thermistor
{
	.name  = "RBC_SEN",
	.pin   = SENSOR_TEMP_PIN,

	.type = SENS_ADC_BETA_R,
	.devcfg = {
		.beta_R.series_R_ohms = 22000, // The value of the series resistor in
		                               // the voltage divider used to detect
		                               // resistance
		.beta_R.ref_ohms      = 20000, // The resistance at the reference value
		.beta_R.ref_value     = 20,    // The reference value
		.beta_R.beta          = 3950,  // The beta coefficient
	},
},
//
// X: A lookup-table-based sensor, like a precomputed thermistor
// The table definition in tables.c includes whether it's a resistance or
// voltage table
{
	.name  = "LUT_SEN",
	.pin   = SENSOR_TEMP_PIN,

	.type = SENS_ADC_LOOKUP,
	.devcfg = {
		//.lookup.series_R_ohms = 22000, // The value of the series resistor in a
		                                 // voltage divider used to detect resistance
		                                 // Ignored for voltage tables
		.lookup.lutno = TAB_VF_3950B, // TAB_VF_3950B is defined in tables.c
	},
},
//
// X: The temperature sensor of a BMP280, with SPI communication
{
	.name  = "BTS_SEN",
	.pin   = SS_BMP280_PIN, // This is the SPI SS pin that selects the sensor
	.type  = SENS_BMx280_SPI_TEMPERATURE,
},
//
// X: The air pressure sensor of a BM[EP]280, with I2C communication
{
	.name  = "BTS_SEN",
	.pin   = I2C_BMP280_ADDR, // This is bus address of the sensor, see it's
	                          // data sheet for the possible values and how
	                          // to choose one
	.type  = SENS_BMx280_I2C_PRESSURE,
},
//
// X: The temperature sensor of a DHT11
{
	.name  = "DHT_SEN",
	.pin   = SENSOR_DHT11_PIN,
	.type  = SENS_DHT11_TEMPERATURE,
},
//
// X: The humidity sensor of a DHT11
{
	.name  = "DHH_SEN",
	.pin   = SENSOR_DHT11_PIN,
	.type  = SENS_DHT11_HUMIDITY,
},
*/
