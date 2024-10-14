#include "common.h"

#if TEST_LED_PINCTRL

//
// Globals initialization


//
// main() initialization
void init_LED_PINCTRL(void) {
	output_pin_on(LED_PINCTRL_PIN);
}

//
// Main loop
void loop_LED_PINCTRL(void) {
	output_pin_toggle(LED_PINCTRL_PIN);
}

#endif // TEST_LED_PINCTRL
