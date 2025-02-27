//
// Example sensor configuration
//
// Each sensor consists of at least a static configurations struct of type
// sensor_cfg_t and a read() function, with an optional init() function also
// possible.
//
// In order to keep the backend code simple while maintaining C99 compatibility,
// the functions used to initialize and read the sensors are defined in one
// section and an array of sensor configurations are defined in the second part.
//
// See src/sensors.h for details of the data structures involved.
//

//
// Sensors aren't currently used in this configuration so all of this is ignored
// by the build.
//

#include "ulib/include/fixed_point.h"

uint_fast16_t ADC_Vref_mV = REGULATED_VOLTAGE_mV;

//
// Conversions
//
INLINE uint32_t adc_to_voltage(uint32_t val, uint32_t vref) {
	return (val * vref) / ADC_MAX;
}
INLINE uint32_t adc_to_resistance(uint32_t val, uint32_t R1) {
	if (val == ADC_MAX) {
		val = ADC_MAX - 1;
	}
	return (val * R1) / (ADC_MAX - val);
}
#define C_TO_K(_t_) ((_t_) + (FIXED_POINT_FROM_INT(27315U)/100U))
#define K_TO_C(_t_) ((_t_) - (FIXED_POINT_FROM_INT(27315U)/100U))
#define C_TO_F(_t_) (FIXED_POINT_MUL((_t_), (FIXED_POINT_FROM_INT(18U)/10U)) + FIXED_POINT_FROM_INT(32U))
#define K_TO_F(_t_) (C_TO_F(K_TO_C(_t_)))

//
// Read a thermistor
//
static void thermistor_read(sensor_reading_t *ret, gpio_pin_t pin) {
	// The reference temperature is given in Celsius but the calculations use Kelvin
	static const fixed_point_t B_div_T0 = FIXED_POINT_DIV(FIXED_POINT_FROM_INT(THERMISTOR_BETA_COEFFICIENT), C_TO_K(FIXED_POINT_FROM_INT(THERMISTOR_REFERENCE_VALUE)));
	static fixed_point_t log_R0 = 0;

	if (log_R0 == 0) {
		log_R0 = log_fixed_point(fixed_point_from_int(THERMISTOR_REFERENCE_OHMS));
	}

	bool enable_adc = (!adc_is_on());
	if (enable_adc) {
		adc_on();
	}

	//adc_t adc_value = adc_read_pin(cfg->pin);
	adc_t adc_value = adc_read_pin(pin);

	if (enable_adc) {
		adc_off();
	}

	if (!SERIES_R_IS_HIGH_SIDE) {
		adc_value = ADC_MAX - adc_value;
	}
	// adc_value == ADC_MAX would cause a divide-by-0 in the log calculation
	if (adc_value == ADC_MAX) {
		adc_value = ADC_MAX - 1;
	}

	fixed_point_t therm_r = fixed_point_from_int(adc_to_resistance(adc_value, THERMISTOR_SERIES_OHMS));

	// Temperature here must be done using the Kelvin scale. Many hours of
	// confusion will be prevented by remembering that.
	//
	// Using 1/T = 1/T0 + 1/B * log(R/R0)
	// T = 1/(1/T0 + (log(R/R0)/B))
	//
	// With a little work, this becomes:
	// T = B / ((B/T0) + log(R1 / R0))
	//
	// ...thereby reducing the number of divisions by 1 AND reducing the
	// required fixed-point precision from >=18 to >=4, meaning we can fit our
	// numbers into a 32-bit integers instead of having to use the slower
	// software 64-bit divisions. To further reduce runtime, (B/T0) and log(R0)
	// can be cached at initialization.
	//
	// Step 1: (B/T0) + log(R1 / R0)
	//     or: (B/T0) + (log(R) - log(R0))
	fixed_point_t tmp = B_div_T0 + (log_fixed_point(therm_r) - log_R0);
	//
	// Step 2: B / (the above)
	if (tmp == 0) {
		// This is the smallest non-zero value our fixed-point number can be
		tmp = 1;
	}
	tmp = fixed_point_div(fixed_point_from_int(THERMISTOR_BETA_COEFFICIENT), tmp);

	if (USE_FAHRENHEIT) {
		tmp = K_TO_F(tmp);
	} else {
		tmp = K_TO_C(tmp);
	}
	if (TEMPERATURE_SCALE > 1) {
		tmp = fixed_point_mul_by_int(tmp, TEMPERATURE_SCALE);
	}
	ret->value = fixed_point_to_int_rounded(tmp);

	return;
}

//
// Section 1
// Initialization/Read Definitions
//

//
// Vcc / ADC voltage reference
//
static sensor_reading_t* VCC_read(SENSOR_CFG_STORAGE struct sensor_cfg_t *cfg, sensor_status_t *status) {
	static sensor_reading_t reading = { 0 };

	bool enable_adc = (!adc_is_on());
	if (enable_adc) {
		adc_on();
	}

	ADC_Vref_mV = reading.value = adc_read_vref_mV();

	if (enable_adc) {
		adc_off();
	}

	UNUSED(cfg);
	UNUSED(status);
	return &reading;
}
//
// Pre-regulator input voltage (e.g. battery or solar)
//
static sensor_reading_t* VIN_read(SENSOR_CFG_STORAGE struct sensor_cfg_t *cfg, sensor_status_t *status) {
	static sensor_reading_t reading = { 0 };
	const uint_fast16_t series_r1 = VIN_VDIV_HIGH_SIDE_OHMS;
	const uint_fast16_t series_r2 = VIN_VDIV_LOW_SIDE_OHMS;

	if (series_r2 == 0) {
		return NULL;
	}

	gpio_set_mode(VIN_SENSE_PIN, GPIO_MODE_AIN, GPIO_FLOAT);
	bool enable_adc = (!adc_is_on());
	if (enable_adc) {
		adc_on();
	}

	adc_t adc_value = adc_read_pin(VIN_SENSE_PIN);

	if (enable_adc) {
		adc_off();
	}
	gpio_set_mode(VIN_SENSE_PIN, GPIO_MODE_HiZ, GPIO_FLOAT);

	uint_fast16_t corrected_value = (adc_value * (series_r1 + series_r2)) / series_r2;
	reading.value = adc_to_voltage(corrected_value, ADC_Vref_mV);

	UNUSED(cfg);
	UNUSED(status);
	return &reading;
}
//
// Water reservoir temperature
//
static sensor_reading_t* water_therm_read(SENSOR_CFG_STORAGE struct sensor_cfg_t *cfg, sensor_status_t *status) {
	static sensor_reading_t reading = { 0 };

	thermistor_read(&reading, WATER_TEMP_SENSE_PIN);

	UNUSED(cfg);
	UNUSED(status);
	return &reading;
}
//
// Water reservoir level
//
static sensor_reading_t* water_level_read(SENSOR_CFG_STORAGE struct sensor_cfg_t *cfg, sensor_status_t *status) {
	static sensor_reading_t reading = { 0 };

	// The water level sense pin is already on so that if the level falls too low
	// while pumping the pin change interrupt (set up in actuator_defs.h) can force
	// the pump to turn off
	//
	// The sense pin is pulled up normally but when the water is high enough, it's
	// pulled low.
	reading.value = (gpio_get_input_state(WATER_LEVEL_SENSE_PIN) == GPIO_LOW);

	UNUSED(cfg);
	UNUSED(status);
	return &reading;
}

//
// Section 2
// Sensor configuration
//
SENSOR_CFG_STORAGE sensor_cfg_t SENSORS[] = {
//
// Vcc / ADC voltage reference
{
	//.name = "Vcc",
	//.init = NULL,
	.read = VCC_read,
},
//
// Pre-regulator input voltage
{
	//.name = "Vin",
	//.init = VIN_init,
	.read = VIN_read,
	//.cooldown_seconds = 30,
	//.pin = BATTERY_CHECK_PIN,
},
//
// Water reservoir temperature
{
	//.name = "WTR_TEMP",
	//.init = water_therm_init,
	.read = water_therm_read,
	//.cooldown_seconds = 30,
	//.pin = WATER_TEMP_SENSE_PIN,
},
//
// Water reservoir level
{
	//.name = "WTR_LEVEL",
	//.init = water_level_init,
	.read = water_level_read,
	//.pin = WATER_LEVEL_SENSE_PIN,
},
};
