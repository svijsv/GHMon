//
// uHAL configuration file for STM32 devices
//

//
// These are the standard settings
#if USE_UART_TERMINAL && (!defined(uHAL_HIBERNATE_LIMIT) || uHAL_HIBERNATE_LIMIT == 0)
# undef uHAL_HIBERNATE_LIMIT
  // The UART clock is disabled in deep sleep but we need to be able to wake up
# define uHAL_HIBERNATE_LIMIT HIBERNATE_LIGHT
#endif

#define uHAL_GPIO_SPEED OUTPUT_MEDIUM

//
// These are the instance overrides
#include GHMON_INCLUDE_CONFIG_HEADER(lib/config_CMSIS_STM32.h)

//
// These are the default settings
#include "../../lib/uHAL/config/config_CMSIS_STM32.h"
