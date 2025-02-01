#if defined(STM32F401xC) || defined(STM32F401xE)
# include "config_CMSIS_STM32F4.h"
#elif defined(STM32F103x6) || defined(STM32F103xB) || defined(STM32F103xE) || defined(STM32F103xG)
# include "config_CMSIS_STM32F1.h"
#else
# error "Unhandled device"
#endif

#define uHAL_BACKUP_DOMAIN_RESET 0

#include "../../lib/uHAL/config/config_CMSIS_STM32.h"
