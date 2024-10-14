#include "common.h"

#if TEST_SD

#include FATFS_FF_H_PATH

//
// Globals initialization


//
// main() initialization
void init_SD(void) {
	gpio_set_mode(SPI_CS_SD_PIN, GPIO_MODE_PP, GPIO_HIGH);
	spi_on();

	return;
}

//
// Main loop
void loop_SD(void) {
	FRESULT fs_err = FR_OK;
	FATFS fs = { 0 };
	FIL fh = { 0 };
	bool is_opened = false;
	static const TCHAR* test_file_name = TEST_SD_FILE_NAME;
	static const char* test_write_string = ".";
	static const UINT test_write_string_len = 1;
	char SD_read_buf[TEST_SD_READ_BUF_SIZE];
	UINT SD_bytes = 0;

	if ((fs_err = f_mount(&fs, "", 1)) != FR_OK) {
		PRINTF("Mount error: fatfs %u\r\n", (uint )fs_err);
		goto SD_END;
	}

	//
	// Test SD read
	//
	if ((fs_err = f_open(&fh, test_file_name, FA_READ)) != FR_OK) {
		PRINTF("%s: read open error: fatfs %u\r\n", test_file_name, (uint )fs_err);
		goto SD_END;
	}
	is_opened = true;
	if ((fs_err = f_read(&fh, &SD_read_buf, TEST_SD_READ_BUF_SIZE-1, &SD_bytes)) != FR_OK) {
		PRINTF("%s: read error: fatfs %u\r\n", test_file_name, (uint )fs_err);
		goto SD_END;
	}
	SD_read_buf[SD_bytes] = 0;
	PRINTF("Read %u bytes from SD: '%s'\r\n", (uint )SD_bytes, SD_read_buf);
	// Set is_opened to false before closing so that we don't try to close again
	// if we fail and go to SD_END
	is_opened = false;
	if ((fs_err = f_close(&fh)) != FR_OK) {
		PRINTF("%s: close error: fatfs %u\r\n", test_file_name, (uint )fs_err);
		goto SD_END;
	}

	//
	// Test SD write
	//
	if ((fs_err = f_open(&fh, test_file_name, FA_WRITE|FA_OPEN_APPEND)) != FR_OK) {
		PRINTF("%s: write open error: fatfs %u\r\n", test_file_name, (uint )fs_err);
		goto SD_END;
	}
	is_opened = true;
	if ((fs_err = f_write(&fh, test_write_string, test_write_string_len, &SD_bytes)) != FR_OK) {
		PRINTF("%s:  append error: fatfs %u\r\n", test_file_name, (uint )fs_err);
		goto SD_END;
	}
	/*
	if (SD_bytes != test_write_string_len) {
		PRINTF("SD written bytes differs from string size: %u vs %u\r\n", (uint )SD_bytes, (uint )test_write_string_len);
	}
	*/
	PRINTF("Wrote %u bytes to SD: '%s'\r\n", (uint )SD_bytes, test_write_string);

SD_END:
	if (is_opened && ((fs_err = f_close(&fh)) != FR_OK)) {
		PRINTF("%s: close error: fatfs %u\r\n", test_file_name, (uint )fs_err);
	}
	if ((fs.fs_type != 0) && ((fs_err = f_unmount("")) != FR_OK)) {
		PRINTF("unmount error: fatfs %u\r\n", (uint )fs_err);
	}

	return;
}

#endif // TEST_SD
