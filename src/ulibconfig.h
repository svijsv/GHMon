// calculate_log_beta() requires >=4 bits of precision and >=20 bits of integer
// for a halfway-decent result
#define FIXEDP_ITYPE int32_t
#define FIXEDP_MTYPE int64_t
#define FIXEDP_FRACT_BITS 8
//#define FIXEDP_DIVI(x, y) (div_s64_s64((x), (y)))

// The biggest string FROM_FSTR() might be used with that I could find was
// the help message in terminal.c at 86 characters + the NUL
#define _FLASH_TMP_SIZE 96

#if DEBUG
// Print a debug message
void logger(const char *format, ...)
	__attribute__ ((format(printf, 1, 2)));
#endif
