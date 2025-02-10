//
// These are the hooks and related variables used to control writing out the log
//

//
// Convert a sensor type ID into a string
static const char* sensor_type_to_name(uint8_t type) {
	if (type != 0) {
		static char type_name[7] = "(T:";
		type_name[3] = '0' + (type / 10);
		type_name[4] = '0' + (type % 10);
		type_name[5] = ')';
		return type_name;
	}

	return NULL;
}

//
// Print extra at the end of each log line
// To disable this functionality, return NULL.
//
// These just print the buffer index; doing so serves no practical purpose,
// it's for demonstation only.
static const char* print_header_extra(void) {
	return "log_buffer_index";
}
// Because of the way printing is implemented, we need to treat the final (nominally
// unbuffered) line as though it were buffered.
static log_line_buffer_size_t log_line_indexes[LOG_LINE_BUFFER_COUNT+1];
static void buffer_line_extra(log_line_buffer_size_t index) {
	log_line_indexes[index] = index;
	return;
}
static const char* print_line_extra(log_line_buffer_size_t index) {
	static char buf[] = "index_XX";

	if (index == LOG_LINE_BUFFER_COUNT) {
		return "(unbuffered)";
	}
	buf[6] = '0' + (index / 10);
	buf[7] = '0' + (index % 10);

	return buf;
}

//
// We have two supported (optional) outputs, UART and an SPI-controlled SD card,
// each defined in their own header for convenience.
#include "log_uart.h"
#include "log_sd.h"

//
// Initialize the output device
static void init_output_device(void) {
	init_log_UART();
	init_log_SD();
	return;
}
//
// Open the output device for writing
// If this returns anything other than ERR_OK then nothing is written, the
// oldest entry in the log buffer is overwritten, and close_output_buffer()
// is not called.
static err_t open_output_device(void) {
	err_t res = ERR_OK;
	bool abort_logging = false;

	if (LOG_POWER_PIN != 0) {
		//output_pin_on(LOG_POWER_PIN);
		gpio_set_output_state(LOG_POWER_PIN, GPIO_HIGH);
		if (LOG_POWER_UP_DELAY_MS > 0) {
			delay_ms(LOG_POWER_UP_DELAY_MS);
		}
	}

	if (WRITE_LOG_TO_UART && !abort_logging) {
		if ((res = open_UART()) != ERR_OK) {
			PRINTF("Failed to open log UART: error %d", (int )res);
			if (!LOG_WITH_MISSING_UART) {
				abort_logging = true;
			}
		}
	}

	if (WRITE_LOG_TO_SD && !abort_logging) {
		if ((res = open_SD()) != ERR_OK) {
			PRINTF("Failed to open log SD: error %d", (int )res);
			if (!LOG_WITH_MISSING_SD) {
				abort_logging = true;
				close_UART();
			}
		}
	}

	return (abort_logging) ? res : ERR_OK;
}
//
// Close the output device
// The return value is currently ignored
static err_t close_output_device(void) {
	close_UART();
	close_SD();

	if (LOG_POWER_PIN != 0) {
		if (LOG_POWER_DOWN_DELAY_MS > 0) {
			delay_ms(LOG_POWER_DOWN_DELAY_MS);
		}
		//output_pin_off(LOG_POWER_PIN);
		gpio_set_output_state(LOG_POWER_PIN, GPIO_LOW);
	}

	return ERR_OK;
}
//
// Open a file on the output device for writing
// 'path' is NULL if USE_LOG_FILE_NAME isn't set
static err_t open_output_file(const char *path) {
	return open_SD_file(path);
}
//
// Close the open file on the output device
static err_t close_output_file(void) {
	return close_SD_file();
}
//
// Check if a file name on the output device can be used for a new log file
// 'path' is NULL if USE_LOG_FILE_NAME isn't set
// Return values:
//    ERR_OK   : The file name is available
//    ERR_RETRY: The device is unavailable and the file name is tried again later
//    Anything else: The file is unavailable and another will be tried
static err_t output_file_is_available(const char *path) {
	return SD_file_is_available(path);
}
//
// Write a block of bytes to the output device
static err_t write_buffer_to_storage(uint8_t *buf, print_buffer_size_t bytes) {
	write_buffer_to_UART(buf, bytes);
	return write_buffer_to_SD(buf, bytes);
}
//
// Write a single byte to the output device
static err_t write_byte_to_storage(uint8_t c) {
	return write_buffer_to_storage(&c, 1);
}
//
// If this returns true, skip scheduled writes to the log file
// Forced writes and buffered lines are unaffected
static bool skip_log_writes(void) {
	const uint8_t warnings = (WARN_BATTERY_LOW | WARN_VCC_LOW);

	return (BIT_IS_SET(ghmon_warnings, warnings));
}
