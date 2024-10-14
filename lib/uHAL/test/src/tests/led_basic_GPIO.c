#include "common.h"

#if TEST_LED

//
// Globals initialization


//
// main() initialization
void init_LED(void) {
	gpio_set_mode(LED_PIN, GPIO_MODE_PP, GPIO_HIGH);
}

//
// Main loop
void loop_LED(void) {
	gpio_toggle_output_state(LED_PIN);
}

#endif // TEST_LED
