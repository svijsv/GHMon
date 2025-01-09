//
// Example controller configuration
//
// Each controller consists of at least a static configurations struct of type
// controller_cfg_t and a run() function, with optional init() and next_run_time()
// functions also possible.
//
// In order to keep the backend code simple while maintaining C99 compatibility,
// the functions used to initialize and run the controllers are defined in one
// section and an array of controller configurations are defined in the second part.
//
// See src/controllers.h for details of the data structures involved.
//

//
// Section 0, helper functions
//

#include "ulib/include/math.h"

//
// Conversions
//
#define C_TO_K(_t_) ((_t_) + (FIXED_PNT_FROM_INT(27315U)/100U))
#define K_TO_C(_t_) ((_t_) - (FIXED_PNT_FROM_INT(27315U)/100U))
#define C_TO_F(_t_) (FIXED_PNT_MUL((_t_), (FIXED_PNT_FROM_INT(18U)/10U)) + FIXED_PNT_FROM_INT(32U))
#define K_TO_F(_t_) (C_TO_F(K_TO_C(_t_)))

INLINE uint_fast16_t adc_to_voltage(uint32_t val, uint32_t vref) {
	return (val * vref) / ADC_MAX;
}
INLINE uint32_t adc_to_resistance(uint32_t val, uint32_t R1) {
	if (val == ADC_MAX) {
		val = ADC_MAX - 1;
	}
	return (val * R1) / (ADC_MAX - val);
}

//
// Read a thermistor
//
static uint_fast16_t read_thermistor(gpio_pin_t pin) {
	// The reference temperature is given in Celsius but the calculations use Kelvin
	static const fixed_pnt_t B_div_T0 = FIXED_PNT_DIV(FIXED_PNT_FROM_INT(THERMISTOR_BETA_COEFFICIENT), C_TO_K(FIXED_PNT_FROM_INT(THERMISTOR_REFERENCE_VALUE)));
	static fixed_pnt_t log_R0 = 0;

	if (log_R0 == 0) {
		log_R0 = log_fixed_pnt(fixed_pnt_from_int(THERMISTOR_REFERENCE_OHMS));
	}

	adc_t adc_value = adc_read_pin(pin);
	if (adc_value == ERR_ADC) {
		return 0;
	}

	if (!SERIES_R_IS_HIGH_SIDE) {
		adc_value = ADC_MAX - adc_value;
	}
	fixed_pnt_t therm_r = fixed_pnt_from_int(adc_to_resistance(adc_value, THERMISTOR_SERIES_OHMS));
	// therm_r == 0 would cause a divide-by-0 in the log calculation
	if (therm_r == 0) {
		return 0;
	}

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
	fixed_pnt_t tmp = B_div_T0 + (log_fixed_pnt(therm_r) - log_R0);

	//
	// Step 2: B / (the above)
	if (tmp == 0) {
		// This is the smallest non-zero value our fixed-point number can be
		tmp = 1;
	}
	tmp = fixed_pnt_div(fixed_pnt_from_int(THERMISTOR_BETA_COEFFICIENT), tmp);

	if (USE_FAHRENHEIT) {
		tmp = K_TO_F(tmp);
	} else {
		tmp = K_TO_C(tmp);
	}
	if (TEMPERATURE_SCALE > 1) {
		tmp = fixed_pnt_mul_by_int(tmp, TEMPERATURE_SCALE);
	}

	return fixed_pnt_to_int_rounded(tmp);
}

//
// Section 1
// Function Definitions
//

//
// Aerator pump control
//
static err_t pump_init(CONTROLLER_CFG_STORAGE controller_cfg_t *cfg, controller_status_t *status) {
	gpio_set_mode(PUMP_CTRL_PIN, GPIO_MODE_PP, GPIO_LOW);
	//status->status = 0;

	UNUSED(cfg);
	UNUSED(status);
	return ERR_OK;
}
static err_t pump_run(CONTROLLER_CFG_STORAGE controller_cfg_t *cfg, controller_status_t *status) {
	bool water_level_ok, water_temp_ok, voltage_ok;

	if (WATER_LEVEL_SENSE_PIN != 0) {
		gpio_state_t pull_dir = (WATER_LEVEL_OK_LOW) ? GPIO_HIGH : GPIO_LOW;
		gpio_state_t ok_state = (WATER_LEVEL_OK_LOW) ? GPIO_LOW  : GPIO_HIGH;

		gpio_set_mode(WATER_LEVEL_SENSE_PIN, GPIO_MODE_IN, pull_dir);
		//
		// Delay long enough for the pin pullup to take effect
		sleep_ms(50);
		water_level_ok = (gpio_get_input_state(WATER_LEVEL_SENSE_PIN) == ok_state);
		gpio_set_mode(WATER_LEVEL_SENSE_PIN, GPIO_MODE_RESET, GPIO_FLOAT);
	} else {
		water_level_ok = true;
	}

	bool set_analog_mode = (GPIO_MODE_RESET_ALIAS != GPIO_MODE_AIN);
	bool enable_adc = false;
	if (WATER_TEMP_SENSE_PIN != 0 || VIN_SENSE_PIN != 0) {
		enable_adc = (!adc_is_on());
	}
	if (enable_adc) {
		adc_on();
	}

	if (WATER_TEMP_SENSE_PIN != 0) {
		if (WATER_TEMP_OK_OHMS != 0) {
			adc_t tmp = adc_read_pin(WATER_TEMP_SENSE_PIN);
			if (!SERIES_R_IS_HIGH_SIDE) {
				tmp = ADC_MAX - tmp;
			}

			uint32_t R = adc_to_resistance(tmp, THERMISTOR_SERIES_OHMS);
			water_temp_ok = R <= WATER_TEMP_OK_OHMS;

		} else {
			uint_fast16_t water_temp;

			if (set_analog_mode) {
				gpio_set_mode(WATER_TEMP_SENSE_PIN, GPIO_MODE_AIN, GPIO_FLOAT);
			}

			water_temp = read_thermistor(WATER_TEMP_SENSE_PIN);

			if (set_analog_mode) {
				gpio_set_mode(WATER_TEMP_SENSE_PIN, GPIO_MODE_RESET, GPIO_FLOAT);
			}

			water_temp_ok = water_temp >= PUMP_MIN_WATER_TEMP;
		}
	} else {
		water_temp_ok = true;
	}

	if (VIN_SENSE_PIN != 0) {
		uint_fast16_t voltage;

		if (set_analog_mode) {
			gpio_set_mode(VIN_SENSE_PIN, GPIO_MODE_AIN, GPIO_FLOAT);
		}

		adcm_t tmp = adc_read_pin(VIN_SENSE_PIN);
		tmp = (tmp * (VIN_VDIV_HIGH_SIDE_OHMS + VIN_VDIV_LOW_SIDE_OHMS)) / VIN_VDIV_LOW_SIDE_OHMS;
		voltage = adc_to_voltage(tmp, REGULATED_VOLTAGE_mV);

		if (set_analog_mode) {
			gpio_set_mode(VIN_SENSE_PIN, GPIO_MODE_RESET, GPIO_FLOAT);
		}

		voltage_ok = voltage >= PUMP_MINIMUM_VIN_mV;
	} else {
		voltage_ok = true;
	}

	if (enable_adc) {
		adc_off();
	}

	bool want_on = (voltage_ok && water_temp_ok && water_level_ok);

	if (want_on) {
		//set_actuator_by_index(0, 1);
		gpio_set_mode(PUMP_CTRL_PIN, GPIO_MODE_PP, GPIO_HIGH);
		//status->status = 1;
	} else {
		//set_actuator_by_index(0, 0);
		// No pulldown on AVRs
		//gpio_set_mode(PUMP_CTRL_PIN, GPIO_MODE_IN, GPIO_LOW);
		//gpio_set_mode(PUMP_CTRL_PIN, GPIO_MODE_HiZ, GPIO_FLOAT);
		gpio_set_mode(PUMP_CTRL_PIN, GPIO_MODE_PP, GPIO_LOW);
		//status->status = 0;
	}

	UNUSED(cfg);
	UNUSED(status);
	return ERR_OK;
}

//
// Section 2
// Controller configuration
//
CONTROLLER_CFG_STORAGE controller_cfg_t CONTROLLERS[] = {
//
// Aerator pump control
{
	//.name = "PUMP_CTRL",
	.init = pump_init,
	.run = pump_run,
	//.schedule_minutes = 15,
	//.schedule_minutes = 1,
	//.schedule_minutes = 0,
},
};
