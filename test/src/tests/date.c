#include "common.h"

#if TEST_DATE

//
// Globals initialization


//
// main() initialization
void init_DATE(void) {
	err_t err;

	if (TEST_DATE_YEAR) {
		if ((err = set_date(TEST_DATE_YEAR, TEST_DATE_MONTH, TEST_DATE_DAY)) != ERR_OK) {
			PRINTF("set_date() returned %u\r\n", (uint )err);
		}
	}

	if (TEST_DATE_HOUR) {
		if ((err = set_time(TEST_DATE_HOUR, TEST_DATE_MINUTE, TEST_DATE_SECOND)) != ERR_OK) {
			PRINTF("set_time() returned %u\r\n", (uint )err);
		}
	}

	return;
}

//
// Main loop
void loop_DATE(void) {
	utime_t RTC_seconds = get_RTC_seconds();
	uint8_t year, month, day;
	uint8_t hour, min, sec;

	seconds_to_date(RTC_seconds, &year, &month, &day);
	seconds_to_time(RTC_seconds, &hour, &min, &sec);
	PRINTF("20%02u.%02u.%02u %02u:%02u:%02u (%08lu)\r\n", (uint )year, (uint )month, (uint )day, (uint )hour, (uint )min, (uint )sec, (long unsigned )RTC_seconds);

	return;
}


#endif // TEST_DATE
