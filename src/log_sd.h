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

static void log_init_SD(void) {
	output_pin_on(SPI_CS_SD_PIN);
	return;
}

static err_t open_SD_file(void) {
	FRESULT fres;

	if (!print_to_SD) {
		return ERR_OK;
	}

	if ((fres = f_open(&fh, logfile_name, FA_WRITE|FA_OPEN_APPEND)) != FR_OK) {
		LOGGER("f_open(): FatFS error %u", (uint )fres);
		SET_BIT(ghmon_warnings, WARN_LOG_ERROR);
		print_to_SD = false;
	}

	return FRESULT_to_err_t(fres);
}
static err_t open_SD(void) {
	FRESULT fres;

	spi_on();
	print_to_SD = true;

	if ((fres = f_mount(&fs, "", 1)) != FR_OK) {
		LOGGER("f_mount(): FatFS error %u", (uint )fres);
		SET_BIT(ghmon_warnings, WARN_LOG_ERROR);
		spi_off();
		print_to_SD = false;
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
		SET_BIT(ghmon_warnings, WARN_LOG_ERROR);
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
		SET_BIT(ghmon_warnings, WARN_LOG_ERROR);
		LOGGER("%u log SD write errors", (uint )write_errors);
		write_errors = 0;
	}
	if ((fres = f_unmount("")) != FR_OK) {
		LOGGER("f_unmount(): FatFS error %u", (uint )fres);
		SET_BIT(ghmon_warnings, WARN_LOG_ERROR);
	}
	spi_off();

	return FRESULT_to_err_t(fres);
}

static err_t _write_to_SD(uint8_t *buf, print_buffer_size_t bytes) {
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
		LOGGER("f_write(): FatFS error %u (%u bytes written)", (uint )fres, (uint )bw);
	}

	return FRESULT_to_err_t(fres);
}
static err_t write_buffer_to_SD(void) {
	return _write_to_SD(print_buffer.buffer, print_buffer.size);
}
static err_t write_char_to_SD(uint8_t c) {
	return _write_to_SD(&c, 1);
}

static bool SD_file_exists(const char *path) {
	FILINFO st;

	return (f_stat(path, &st) == FR_OK);
}


#else // WRITE_LOG_TO_SD
# define log_init_SD() (void )0U
# define write_buffer_to_SD() (ERR_OK)
# define write_char_to_SD(_c_) (ERR_OK)
# define open_SD() (ERR_OK)
# define open_SD_file() (ERR_OK)
# define close_SD() (ERR_OK)
# define close_SD_file() (ERR_OK)
#endif // WRITE_LOG_TO_SD
