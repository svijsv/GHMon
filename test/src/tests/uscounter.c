#include "common.h"

#if TEST_USCOUNTER

//
// Globals initialization


//
// main() initialization
void init_USCOUNTER(void) {


	return;
}

//
// Main loop
void loop_USCOUNTER(void) {
	utime_t hold, count1, count30, count100;

	uscounter_on();
	// Make sure we start close to the beginning of a ms period
	hold = SET_TIMEOUT_MS(1);
	while (!TIMES_UP(hold)) {
		// Nothing to do here
	}

	uscounter_start();
	++hold;
	while (!TIMES_UP(hold)) {
		// Nothing to do here
	}
	count1 = uscounter_read();

	hold += 29;
	while (!TIMES_UP(hold)) {
		// Nothing to do here
	}
	count30 = uscounter_read();

	hold += 70;
	while (!TIMES_UP(hold)) {
		// Nothing to do here
	}
	count100 = uscounter_stop();

	uscounter_off();

	PRINTF("1ms == %luus, 30ms == %lu, 100ms == %luus\r\n", (long unsigned )count1, (long unsigned )count30, (long unsigned )count100);
	if (!IS_IN_RANGE(count1, 980U, 1020U)  || !IS_IN_RANGE(count30, 28000U, 32000U) || !IS_IN_RANGE(count100, 98000U, 102000U)) {
		PRINTF("   COUNT OUT OF RANGE!\r\n");
	}

	return;
}

#endif // TEST_USCOUNTER
