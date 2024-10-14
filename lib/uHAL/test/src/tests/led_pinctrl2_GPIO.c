#include "common.h"

#if TEST_LED_PINCTRL2

//
// Globals initialization
static pinctrl_handle_t led_pin;

//
// main() initialization
void init_LED_PINCTRL2(void) {
/*
	pinctrl_cfg_t lcfg = {
		.pin = LED_PIN,
		.as_input = 0,
		.hi_is_on = 1,
		.hi_active = 1,
		.lo_active = 1,
		.turn_on = 1
	};
	pinctrl_init(&led_pin, &lcfg);
*/
	pinctrl_init2_output(&led_pin, LED_PINCTRL2_PIN);
}

//
// Main loop
void loop_LED_PINCTRL2(void) {
	pinctrl_toggle(&led_pin);
}

#endif // TEST_LED_PINCTRL2
