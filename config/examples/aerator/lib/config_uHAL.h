//
// uHAL settings that should be overridden globally go in here
#define uHAL_USE_ADC 1
#define ADC_SAMPLE_uS 10U
#define ADC_SAMPLE_COUNT 1U
#define ADC_TIMEOUT_MS 100U

#define uHAL_USE_PWM 0
#define PWM_FREQUENCY_HZ 1000
#define PWM_DUTY_CYCLE_SCALE 100U

#if USE_LOGGING && WRITE_LOG_TO_SD
# define uHAL_USE_SPI 1
# define uHAL_USE_FATFS 1
#endif
