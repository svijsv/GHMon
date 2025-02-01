//
// config.h
// Configuration file for the ulib modules
//

//
// Include the GHMon configuration before anything else, it may control some
// features.
#include "config/general/config.h"

//
// These are the standard settings
#define ULIB_ENABLE_DEFAULT 0
#define ULIB_USE_STRUCT_ID 0
#define ULIB_USE_MALLOC 0
#define ULIB_DO_SAFETY_CHECKS (!SKIP_LIB_SAFETY_CHECKS)

#define USE_ULIB_ASSERT 1
#define USE_ULIB_LOCAL_ASSERT 1
#define USE_ULIB_ASSERT_OVERRIDE 1
#define USE_ULIB_PANIC 1
#define USE_ULIB_LOCAL_PANIC 1
#ifndef DEBUG_CPP_MESSAGES
# define DEBUG_CPP_MESSAGES DEBUG
#endif

#define ULIB_ENABLE_ERROR 1

#define ULIB_ENABLE_FMEM 1
//#define FMEM_STR_BYTES 96U

#define ULIB_ENABLE_ASCII 1

#define ULIB_ENABLE_BITOPS 1
#define ULIB_BITOP_ENABLE_GENERICS 1

#if !defined(HALLOC_HEAP_START_LINKER_VAR)
# define HALLOC_HEAP_START_LINKER_VAR __heap_start
#endif
#if defined(DEBUG) && DEBUG
# define HALLOC_HEAP_CANARY 0xABU
# define HALLOC_MEM_INIT_VALUE 0
#else
# define HALLOC_HEAP_CANARY 0
# define HALLOC_MEM_INIT_VALUE -1
#endif

#define ULIB_ENABLE_PRINTF 1
#define PRINTF_MAX_INT_BYTES 4
#define PRINTF_USE_o_FOR_OCTAL 0
#define PRINTF_USE_MINIMAL_FEATURE_SET 1
#define PRINTF_ALLOW_BINARY 1
#define PRINTF_ALLOW_ZERO_PADDING 1
#define PRINTF_ALLOW_PRECISION 1
#define PRINTF_ALLOW_ALT_FORMS 1

#define ULIB_ENABLE_TIME 1

#define ULIB_ENABLE_UTIL 1

//
// These are the instance overrides
#include GHMON_INCLUDE_CONFIG_HEADER(lib/ulibconfig.h)

//
// These are the conditionally-enabled features
#if !defined(ULIB_ENABLE_HALLOC) || ! ULIB_ENABLE_HALLOC
# undef ULIB_ENABLE_HALLOC
# define ULIB_ENABLE_HALLOC (USE_LOGGING)
#endif
#if !defined(ULIB_ENABLE_CSTRINGS) || ! ULIB_ENABLE_CSTRINGS
# undef ULIB_ENABLE_CSTRINGS
# define ULIB_ENABLE_CSTRINGS (USE_UART_TERMINAL || USE_LOGGING)
#endif

//
// These are the default settings
#include "../../lib/ulib/ulibconfig_template.h"
