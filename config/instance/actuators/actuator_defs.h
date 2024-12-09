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

//
// Section 1
// Function Definitions
//
// Generic GPIO functions
static err_t on_off_pin_init(ACTUATOR_CFG_STORAGE struct actuator_cfg_t *cfg, actuator_status_t *status) {
	output_pin_off(cfg->pin);
	status->status = 0;

	return ERR_OK;
}
static err_t on_off_pin_set(ACTUATOR_CFG_STORAGE struct actuator_cfg_t *cfg, actuator_status_t *status, ACTUATOR_STATUS_T value) {
	err_t res;

	if (value != 0) {
		res = output_pin_on(cfg->pin);
		status->status = 1;
	} else {
		res = output_pin_off(cfg->pin);
		status->status = 0;
	}
	if (res != ERR_OK) {
		status->status = -1;
	}

	return res;
}
static bool on_off_pin_is_on(ACTUATOR_CFG_STORAGE struct actuator_cfg_t *cfg, actuator_status_t *status) {
	//UNUSED(status);
	//return output_pin_is_on(cfg->pin);

	UNUSED(cfg);
	return (status->status == 1);
}

//
// Section 2
// Actuator configuration
//
ACTUATOR_CFG_STORAGE actuator_cfg_t ACTUATORS[] = {
//
// Actuator 0, irrigation control
// Turn a GPIO pin on to start irrigating and turn it off to stop
//static ACTUATOR_CFG_STORAGE actuator_cfg_t irr1 = {
{
	.name = "IRR1",
	.init = on_off_pin_init,
	.set = on_off_pin_set,
	.is_on = on_off_pin_is_on,
	.pin = IRR1_CTRL_PIN,
	.cfg_flags = ACTUATOR_CFG_FLAG_LOG
},
};
