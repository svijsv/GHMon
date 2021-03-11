// calculate_log_beta() requires >=4 bits of precision and >=20 bits of integer
// for a halfway-decent result
#define FIXEDP_ITYPE int32_t
#define FIXEDP_MTYPE int64_t
#define FIXEDP_FRACT_BITS 8
//#define FIXEDP_DIVI(x, y) (div_s64_s64((x), (y)))

#if DEBUG
// Print a debug message
void logger(const char *format, ...)
	__attribute__ ((format(printf, 1, 2)));
#endif
