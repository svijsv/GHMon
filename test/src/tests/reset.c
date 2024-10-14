#include "common.h"

#if TEST_RESET

//
// Globals initialization


//
// main() initialization
void init_RESET(void) {

	return;
}

//
// Main loop
void loop_RESET(void) {
	static uint_fast8_t reset_loops = TEST_RESET_LOOPS;

	if (reset_loops == 0) {
		PRINTF("Resetting NOW!\r\n");
		platform_reset();
	}
	PRINTF("Resetting in %u loops\r\n", (uint_t )reset_loops);
	--reset_loops;

	return;
}

#endif // TEST_RESET
