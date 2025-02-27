//
// Example actuator configuration
//
// Each actuator consists of at least a static configuration struct of type
// actuator_cfg_t and a set() function, with optional init() and is_on() functions
// also possible.
//
// In order to keep the backend code simple while maintaining C99 compatibility,
// the functions used to initialize and set the actuators are defined in one
// section and an array of actuator configurations are defined in the second part.
//
// See src/actuators.h for details of the data structures involved.
//

//
// Actuators aren't currently used in this configuration so all of this is ignored
// by the build.
//

extern uint_fast16_t ADC_Vref_mV;

//static pwm_output_t pump_ctrl_output;

//
// Find the required ISR for the interrupt on the water level sensor pin
#include <avr/interrupt.h>

#if (GPIO_GET_PORTNO(WATER_LEVEL_SENSE_PIN) == GPIO_PORTA)
# define WATER_LEVEL_SENSE_ISR PORTA_PORT_vect
# define WATER_LEVEL_SENSE_PORT PORTA
#elif (GPIO_GET_PORTNO(WATER_LEVEL_SENSE_PIN) == GPIO_PORTB)
# define WATER_LEVEL_SENSE_ISR PORTB_PORT_vect
# define WATER_LEVEL_SENSE_PORT PORTB
#else
# error "WATER_LEVEL_SENSE_PIN is unhandled"
#endif
// Write '1' to a flag to clear it
#define CLEAR_WATER_LEVEL_SENSE_ISR() do { WATER_LEVEL_SENSE_PORT.INTFLAGS = GPIO_GET_PINMASK(WATER_LEVEL_SENSE_PIN); } while (0)

//
// Section 1
// Function Definitions
//

//
// Read a voltage on an analog pin to decide how hard to run the motor
static uint_fast8_t get_pump_on_level(void) {
	bool enable_adc = (!adc_is_on());
	if (enable_adc) {
		adc_on();
	}

	adc_t ctrl_volts = adc_read_pin(PUMP_LEVEL_CTRL_PIN);
	ctrl_volts = (ctrl_volts * ADC_Vref_mV) / ADC_MAX;

	if (enable_adc) {
		adc_off();
	}

	if (ctrl_volts < PUMP_LEVEL_CTRL_1) {
		return 1;
	} else if (ctrl_volts < PUMP_LEVEL_CTRL_2) {
		return 2;
	} else if (ctrl_volts < PUMP_LEVEL_CTRL_3) {
		return 3;
	} else if (ctrl_volts < PUMP_LEVEL_CTRL_4) {
		return 4;
	}
	return 5;
}
//
// When the pump starts the water level may drop far enough to leave it dry,
// so set up an interrupt on the rising edge (pin is low when water is high)
// to make sure the pump stops
static void water_level_monitor_init(void) {
	gpio_listen_cfg_t irq_cfg = {
		WATER_LEVEL_SENSE_PIN,
		GPIO_TRIGGER_RISING
	};
	// We're never going to use this handle again so it doesn't need to be static
	gpio_listen_t irq_handle;

	gpio_set_mode(WATER_LEVEL_SENSE_PIN, GPIO_MODE_IN, GPIO_HIGH);
	gpio_listen_init(&irq_handle, &irq_cfg);
	gpio_listen_on(&irq_handle);

	return;
}
ISR(WATER_LEVEL_SENSE_ISR) {
	CLEAR_WATER_LEVEL_SENSE_ISR();
	pwm_set(&pump_ctrl_output, 0);
	actuators[0].status = 0;
}
//
// Aerator pump control
static err_t pump_ctrl_init(ACTUATOR_CFG_STORAGE struct actuator_cfg_t *cfg, actuator_status_t *status) {
	UNUSED(cfg);
	UNUSED(status);
	water_level_monitor_init();
	return pwm_on(&pump_ctrl_output, PUMP_CTRL_PIN, 0);
}
static err_t pump_ctrl_set(ACTUATOR_CFG_STORAGE struct actuator_cfg_t *cfg, actuator_status_t *status, ACTUATOR_STATUS_T value) {
	if (value == 0) {
		pwm_set(&pump_ctrl_output, 0);
		status->status = 0;

	} else {
		uint_fast8_t level = get_pump_on_level();

		if (status->status != level) {
			uint_fast16_t dc = (PUMP_PWM_MIN + ((level-1) * PUMP_PWM_STEP));
			if (dc > PWM_DUTY_CYCLE_SCALE) {
				dc = PWM_DUTY_CYCLE_SCALE;
			}
			pwm_set(&pump_ctrl_output, dc);
			status->status = level;
		}
	}

	UNUSED(cfg);
	return ERR_OK;
}

//
// Section 2
// Actuator configuration
//
ACTUATOR_CFG_STORAGE actuator_cfg_t ACTUATORS[] = {
//
// Aerator pump control
{
	//.name = "PUMP_ACT",
	.init = pump_ctrl_init,
	.set = pump_ctrl_set,
	//.pin = PUMP_CTRL_PIN,
},
};
