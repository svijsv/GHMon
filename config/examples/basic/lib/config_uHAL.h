//
// uHAL settings that should be overridden globally go in here
#define uHAL_USE_ADC 1
#define ADC_SAMPLE_uS 100U
#define ADC_SAMPLE_COUNT 3U

#define uHAL_USE_HIGH_LEVEL_GPIO 1

#if USE_LOGGING && WRITE_LOG_TO_SD
# define uHAL_USE_SPI 1
# define uHAL_USE_FATFS 1
#endif
