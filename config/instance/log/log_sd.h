#if WRITE_LOG_TO_SD

#include "FatFS/ff.h"

static bool print_to_SD = false;
static uint8_t write_errors;

static FATFS fs;
static FIL fh;

#define SD_IS_MOUNTED() (fs.fs_type != 0)
#define SD_FILE_IS_OPEN() (fh.obj.fs != NULL)

static err_t FRESULT_to_err_t(FRESULT fres) {
	if (!USE_SMALL_CODE) {
		switch (fres) {
		case FR_OK:
			return ERR_OK;
		case FR_DISK_ERR:
			return ERR_IO;
		case FR_NOT_READY:
			return ERR_NODEV;
		case FR_NO_FILE:
		case FR_NO_PATH:
		case FR_INVALID_NAME:
		case FR_INVALID_DRIVE:
		case FR_INVALID_PARAMETER:
			return ERR_BADARG;
		case FR_DENIED:
		case FR_EXIST:
		case FR_WRITE_PROTECTED:
		case FR_LOCKED:
			return ERR_PERM;
		case FR_INVALID_OBJECT:
		case FR_NOT_ENABLED:
		case FR_NO_FILESYSTEM:
			return ERR_INIT;
		case FR_TIMEOUT:
			return ERR_TIMEOUT;
		case FR_NOT_ENOUGH_CORE:
			return ERR_NOMEM;
		//case FR_INT_ERR:
		//case FR_MKFS_ABORTED:
		//case FR_TOO_MANY_OPEN_FILES:
		default:
			return ERR_UNKNOWN;
		}
	}

	return (fres == FR_OK) ? ERR_OK : ERR_UNKNOWN;
}

static void init_log_SD(void) {
	output_pin_on(SPI_CS_SD_PIN);
	return;
}

static err_t open_SD_file(const char *path) {
	FRESULT fres;

	if (!SD_IS_MOUNTED()) {
		if (SKIP_SAFETY_CHECKS || !print_to_SD) {
			return ERR_OK;
		} else {
			return ERR_INIT;
		}
	}

	print_to_SD = true;
	if ((fres = f_open(&fh, path, FA_WRITE|FA_OPEN_APPEND)) != FR_OK) {
		LOGGER("f_open(): FatFS error %u", (uint )fres);
		print_to_SD = false;
	}

	return FRESULT_to_err_t(fres);
}
static err_t open_SD(void) {
	FRESULT fres;

	spi_on();

	if ((fres = f_mount(&fs, "", 1)) != FR_OK) {
		LOGGER("f_mount(): FatFS error %u", (uint )fres);
		spi_off();
	}

	return FRESULT_to_err_t(fres);
}

static err_t close_SD_file(void) {
	FRESULT fres;

	print_to_SD = false;

	if (!SD_FILE_IS_OPEN()) {
		return ERR_OK;
	}

	if ((fres = f_close(&fh)) != FR_OK) {
		LOGGER("f_close(): FatFS error %u", (uint )fres);
	}

	return FRESULT_to_err_t(fres);
}
static err_t close_SD(void) {
	FRESULT fres;

	if (!SD_IS_MOUNTED()) {
		return ERR_OK;
	}

	close_SD_file();

	if (write_errors != 0) {
		LOGGER("%u log SD write errors", (uint )write_errors);
		write_errors = 0;
	}
	if ((fres = f_unmount("")) != FR_OK) {
		LOGGER("f_unmount(): FatFS error %u", (uint )fres);
	}
	spi_off();

	return FRESULT_to_err_t(fres);
}

static err_t write_buffer_to_SD(uint8_t *buf, print_buffer_size_t bytes) {
	FRESULT fres;
	UINT bw = 0;

	if (!print_to_SD) {
		return ERR_OK;
	}
	if (!SKIP_SAFETY_CHECKS && (!SD_IS_MOUNTED() || !SD_FILE_IS_OPEN())) {
		return ERR_INIT;
	}

	if ((fres = f_write(&fh, buf, bytes, &bw)) != FR_OK) {
		// Not much else we can do about problems here
		++write_errors;
		LOGGER("f_write(): FatFS error %u (%u of %u bytes written)", (uint )fres, (uint )bw, (uint )bytes);
	}

	return FRESULT_to_err_t(fres);
}

static err_t SD_file_is_available(const char *path) {
	FILINFO st;

	//
	// If the file doesn't exist already, the name is available (ERR_OK).
	// If the file exists, we should check the next option (ERR_UNKNOWN).
	// For all other errors, abort for now and try again later (ERR_RETRY).
	switch (f_stat(path, &st)) {
	case FR_NO_FILE:
		return ERR_OK;
	case FR_OK:
		return ERR_UNKNOWN;
	default:
		return ERR_RETRY;
	}
}

#else // WRITE_LOG_TO_SD
static void init_log_SD(void) {
	return;
}
static err_t open_SD_file(const char *path) {
	UNUSED(path);
	return ERR_OK;
}
static err_t open_SD(void) {
	return ERR_OK;
}
static err_t close_SD_file(void) {
	return ERR_OK;
}
static err_t close_SD(void) {
	return ERR_OK;
}
static err_t write_buffer_to_SD(uint8_t *buf, print_buffer_size_t bytes) {
	UNUSED(buf);
	UNUSED(bytes);
	return ERR_OK;
}
static err_t SD_file_is_available(const char *path) {
	UNUSED(path);
	return ERR_OK;
}
#endif // WRITE_LOG_TO_SD
