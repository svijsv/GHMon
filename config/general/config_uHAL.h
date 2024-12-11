//
// uHAL configuration file
//

//
// Include the GHMon configuration before anything else, it may control some
// features.
#include "config/general/config.h"

//
// These are the standard settings
#define uHAL_USE_SMALL_CODE USE_SMALL_CODE
#define uHAL_SKIP_INVALID_ARG_CHECKS SKIP_LIB_SAFETY_CHECKS
#define uHAL_SKIP_INIT_CHECKS SKIP_LIB_SAFETY_CHECKS
#define uHAL_SKIP_OTHER_CHECKS SKIP_LIB_SAFETY_CHECKS

#define uHAL_USE_HIBERNATE 1
#define uHAL_ANNOUNCE_HIBERNATE (DEBUG)

#define TERMINAL_HAVE_EXTRA_CMDS (USE_UART_TERMINAL)
#define uHAL_USE_UART_COMM (USE_UART_OUTPUT || USE_UART_TERMINAL)
#define ENABLE_UART_LISTENING (USE_UART_TERMINAL)
#define uHAL_USE_TERMINAL (USE_UART_TERMINAL)
#define uHAL_USE_UART (uHAL_USE_UART_COMM || uHAL_USE_TERMINAL)
#define UART_COMM_BUFFER_BYTES 16U
#define TERMINAL_TIMEROUT_S 600U // 10 minutes

#define uHAL_USE_HIGH_LEVEL_GPIO 1

#define uHAL_USE_RTC 1

//
// These are the instance overrides
#include GHMON_INCLUDE_CONFIG_HEADER(lib/config_uHAL.h)

//
// These are the default settings
#include "../../lib/uHAL/config/config_uHAL.h"
