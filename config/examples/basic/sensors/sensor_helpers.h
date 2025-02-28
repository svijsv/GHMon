//
// Helper functions for reading/interpreting sensors
//

#include "ulib/include/fixed_point.h"

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
// Read a voltage divider
//
typedef struct {
	sensor_reading_t reading;
} vdiv_helper_t;

sensor_reading_t* vdiv_ohms_read(SENSOR_CFG_STORAGE struct sensor_cfg_t *cfg, sensor_status_t *status) {
	uint32_t series_r = cfg->data;
	vdiv_helper_t *helper = status->data;

	bool enable_adc = (!adc_is_on());
	if (enable_adc) {
		adc_on();
	}

	adc_t adc_value = adc_read_pin(cfg->pin);

	if (enable_adc) {
		adc_off();
	}

	if (!SERIES_R_IS_HIGH_SIDE) {
		adc_value = ADC_MAX - adc_value;
	}
	if (adc_value == ADC_MAX) {
		adc_value = ADC_MAX - 1;
	}
	helper->reading.value = adc_to_resistance(adc_value, series_r);

	return &helper->reading;
}

//
// Read a thermistor
//
// https://www.daycounter.com/Calculators/Steinhart-Hart-Thermistor-Calculator.phtml
// https://www.allaboutcircuits.com/industry-articles/how-to-obtain-the-temperature-value-from-a-thermistor-measurement/
// https://www.electroniclinic.com/what-is-a-thermistor-thermistor-types-thermistor-circuits/#Thermistor_Overview
// https://www.digikey.com/en/articles/how-to-accurately-sense-temperature-using-thermistors
typedef struct {
	sensor_reading_t reading;
} thermistor_helper_t;

sensor_reading_t* thermistor_read(SENSOR_CFG_STORAGE struct sensor_cfg_t *cfg, sensor_status_t *status) {
	// The reference temperature is given in Celsius but the calculations use Kelvin
	static const fixed_point_t B_div_T0 = FIXED_POINT_DIV(FIXED_POINT_FROM_INT(THERMISTOR_BETA_COEFFICIENT), C_TO_K(FIXED_POINT_FROM_INT(THERMISTOR_REFERENCE_VALUE)));
	static fixed_point_t log_R0 = 0;

	thermistor_helper_t *helper = status->data;

	if (log_R0 == 0) {
		log_R0 = log_fixed_point(fixed_point_from_int(THERMISTOR_REFERENCE_OHMS));
	}

	bool enable_adc = (!adc_is_on());
	if (enable_adc) {
		adc_on();
	}

	adc_t adc_value = adc_read_pin(cfg->pin);

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
	// The C log() function (and ulib's log_fixed_point()) returns the natural
	// logarithm, which is more commonly abreviated as ln(). Remembering this
	// will also help avoid confusion.
	//
	// Using 1/T = 1/T0 + 1/B * ln(R/R0)
	// T = 1/(1/T0 + (ln(R/R0)/B))
	//
	// With a little work, this becomes:
	// T = B / ((B/T0) + ln(R1 / R0))
	//
	// ...thereby reducing the number of divisions by 1 AND reducing the
	// required fixed-point precision from >=18 to >=4, meaning we can fit our
	// numbers into a 32-bit integers instead of having to use the slower
	// software 64-bit divisions. To further reduce runtime, (B/T0) and log(R0)
	// can be cached at initialization.
	//
	// Step 1: (B/T0) + ln(R1 / R0)
	//     or: (B/T0) + (ln(R) - ln(R0))
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
	helper->reading.value = fixed_point_to_int_rounded(tmp);

	return &helper->reading;
}
