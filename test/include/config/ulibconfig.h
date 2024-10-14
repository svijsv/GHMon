//
// config.h
// Configuration file for the ulib modules
//
// This file should only be included by configify.h

/*
* Global configuration
*/
// Default value for module enablement.
#define ULIB_ENABLE_DEFAULT 0
//
// If non-zero, include an ID field in exported structs to make some debugging
// easier. Not all structs currently support this.
#define ULIB_USE_STRUCT_ID 0
//
// If non-zero, modules with USE_MALLOC options default to using malloc.
#define ULIB_USE_MALLOC 0
//
// If non-zero, default to doing additional safety checks in all modules.
//#define ULIB_DO_SAFETY_CHECKS DEBUG
#define ULIB_DO_SAFETY_CHECKS 0


/*
* Alternate namespace module configuration
*/
// Enable this module
#define ULIB_ENABLE_FMEM 1
//
// If non-zero, enable the use of an alternate namespace for storing static
// data.
#ifdef HAVE_AVR_MEGA
# define HAVE_FMEM_NAMESPACE 1
#else
# define HAVE_FMEM_NAMESPACE 0
#endif
//
// The size of the static array(s) used to return strings from FROM_STR()
#define FMEM_STR_BYTES 96U
//
// If non-zero, perform additional checks to handle common problems like being
// passed NULL inputs.
#define DO_FMEM_SAFETY_CHECKS ULIB_DO_SAFETY_CHECKS


/*
* Array module configuration
*/
// Enable this module
#define ULIB_ENABLE_ARRAYS ULIB_ENABLE_DEFAULT
//
// The size of arlen_t is based on the size of ARRAY_MAX_OBJECTS+1, so using
// one less than the max of an unsigned int type will result in less used
// space than using the max value itself.
#define ARRAY_MAX_OBJECTS (0xFFFFU - 1U)
//
// Allocated size of an array when first created.
#define ARRAY_INITIAL_SIZE 4
//
// Factor by which to grow an array by when it would otherwise overflow.
#define ARRAY_GROW_FACTOR 2
//
// Method of array growth.
//    ARRAY_GROW_NONE: The array is never grown - if an overflow would occur,
//                     excess objects are simply not added.
//    ARRAY_GROW_FRAC: The increase is by allocated/factor - e.g. if
//                     ARRAY_GROW_FACTOR is 2, the new size is 1.5 times the
//                     previous one (with an error margin on account of integer
//                     division).
//    ARRAY_GROW_ADD: Add ARRAY_GROW_FACTOR to the allocated size.
//    ARRAY_GROW_MUL: Multiply the allocated size by ARRAY_GROW_FACTOR.
#define ARRAY_GROW_METHOD ARRAY_GROW_MUL
//
// If non-zero, the array bank is grown as needed. Otherwise the bank always
// contains ARRAY_MAX_OBJECTS entries and the ARRAY_GROW_* values are ignored.
#define ARRAYS_USE_MALLOC ULIB_USE_MALLOC
//
// If non-zero, perform additional checks to handle common problems like being
// passed NULL inputs.
#define DO_ARRAY_SAFETY_CHECKS ULIB_DO_SAFETY_CHECKS


/*
* ASCII character module configuration
*/
// Enable this module
#define ULIB_ENABLE_ASCII 1
//
// If non-zero, replace the ASCII module functions with their ctype.h equivalent
// Not all functions are supported
#define ASCII_SUBSTITUTE_WITH_CTYPE 0
//
// If non-zero, replace the ctype.h functions with their ASCII module equivalent
// Some functions may not supported
#define ASCII_SUBSTITUTE_FOR_CTYPE 0
//
// If non-zero, perform additional checks to handle common problems like being
// passed NULL inputs.
#define DO_ASCII_SAFETY_CHECKS ULIB_DO_SAFETY_CHECKS


/*
* Bitwise operation module configuration
*/
// Enable this module
#define ULIB_ENABLE_BITOPS 1
//
// Enable the type-checked inlined function versions of the bitwise operations.
// See bits.h for details.
#define ULIB_BITOP_ENABLE_INLINED_FUNCTIONS 1
//
// Enable the 64-bit versions of the above functions. Not all platforms support
// this natively.
#define ULIB_BITOP_ENABLE_INLINED_64BIT_FUNCTIONS 1
//
// Enable versions of the bitwise macros and functions that use the _Generic()
// operator. This is more type-safe and doesn't impact the size of optimized code
// but requires a compatible compiler (typically anything that supports C11 or later).
// This requires that ULIB_BITOP_ENABLE_INLINED_64BIT_FUNCTIONS be set.
#define ULIB_BITOP_ENABLE_GENERICS 1


/*
* Byte buffer module configuration
*/
// Enable this module
#define ULIB_ENABLE_BUFFERS ULIB_ENABLE_DEFAULT
//
// The size of buflen_t is based on the size of BUFFER_MAX_BYTES+1, so using
// one less than the max of an unsigned int type will result in less used
// space than using the max value itself.
//
#define BUFFER_MAX_BYTES (0xFFFFU - 1U)
//
// Size of a buffer when first created.
#define BUFFER_INITIAL_SIZE 32
//
// Factor by which to grow the buffer by when it would otherwise overflow.
#define BUFFER_GROW_FACTOR 2
//
// Method of buffer growth. See the description for ARRAY_GROW_METHOD for
// details.
#define BUFFER_GROW_METHOD BUFFER_GROW_MUL
//
// If non-zero, the buffer bank is grown as needed. Otherwise the bank is
// always BUFFER_MAX_BYTES in size and BUFFER_GROW_* values are ignored.
#define BUFFERS_USE_MALLOC ULIB_USE_MALLOC
//
// If non-zero, perform additional checks to handle common problems like being
// passed NULL inputs.
#define DO_BUFFER_SAFETY_CHECKS ULIB_DO_SAFETY_CHECKS


/*
* C-style string module configuration
*/
// Enable this module
#define ULIB_ENABLE_CSTRINGS 1
//
//
// If non-zero, perform additional checks to handle common problems like being
// passed NULL inputs.
#define DO_CSTRING_SAFETY_CHECKS ULIB_DO_SAFETY_CHECKS


/*
* Debugging module configuration
*/
// Enable ulib_assert(), which is used in ulib but can also be used elsewhere
#define USE_ULIB_ASSERT 1
//
// Use the ulib_assert() function defined in debug.h instead of pulling in
// assert.h when NDEBUG is undefined.
// If this is enabled and NDEBUG is undefined, the function ulib_assert_failed()
// must be defined somewhere. See debug.h for the prototype.
#define USE_ULIB_LOCAL_ASSERT 1
//
// Override assert() with ulib_assert() when USE_ULIB_LOCAL_ASSERT is also set
#define USE_ULIB_ASSERT_OVERRIDE 1
//
// Enable ulib_panic(), which may be used in ulib but can also be used elsewhere
// Note that unlike assert(), there's no flag for disabling panic() in release
// builds - if enabled, theyre always enabled.
#define USE_ULIB_PANIC 0
//
// Use the ulib_panic() function defined in debug.h instead of pulling in
// stdlib.h and using abort().
// If this is enabled, the function ulib_panic_abort() must be defined somewhere.
// See debug.h for the prototype.
#define USE_ULIB_LOCAL_PANIC 0
//
// Enable preprocessor messages issued with DEBUG_CPP_MACRO() and DEBUG_CPP_MSG()
//#define DEBUG_CPP_MESSAGES DEBUG


/*
* File module configuration
*/
// Enable this module
#define ULIB_ENABLE_FILES ULIB_ENABLE_DEFAULT
//
// The maximum number of levels to descend in recursive operations.
#define FILE_MAX_RECURSION 255U
//
// An optional externally-defined buffer used for file operations that require
// one. Used when functions are passed NULL as their buffer.
// If defined, it must be uint8_t*.
//#define FILE_PROVIDED_BUF g_byte_buffer
//#define FILE_PROVIDED_BUF_SIZE 16384
//
// If non-zero, perform additional checks to handle common problems like being
// passed NULL inputs.
#define DO_FILE_SAFETY_CHECKS ULIB_DO_SAFETY_CHECKS
//
// These are additional checks for the internal helper functions.
#define DO_FILE_EXTRA_SAFETY_CHECKS ULIB_DO_SAFETY_CHECKS


/*
* Linked-list module configuration
*/
// Enable this module
#define ULIB_ENABLE_LISTS ULIB_ENABLE_DEFAULT
//
// The size of listlen_t is based on the size of LIST_MAX_OBJECTS+1, so using
// one less than the max of an unsigned int type will result in less used
// space than using the max value itself.
#define LIST_MAX_OBJECTS (0xFFFFU - 1U)
//
// If non-zero, new entries are created with malloc. Otherwise a pointer to
// a list_entry_t struct will need to be supplied when adding to the list.
#define LISTS_USE_MALLOC ULIB_USE_MALLOC
//
// If non-zero, perform additional checks to handle common problems like being
// passed NULL inputs.
#define DO_LIST_SAFETY_CHECKS ULIB_DO_SAFETY_CHECKS


/*
* Math module configuration
*/
// Enable this module
#define ULIB_ENABLE_MATH 1
//
// The number of bits used in fixed-point representations. This includes the
// sign bit.
#define FIXED_PNT_BITS 32
//
// The number of bits of a fixed-point representation devoted to the fraction.
#define FIXED_PNT_FRACT_BITS 8
//
// If non-zero, perform additional checks to handle common problems like
// division by 0.
#define DO_MATH_SAFETY_CHECKS ULIB_DO_SAFETY_CHECKS
//
// If set, override the macro used to divide fixed-point integers. Useful
// because a certain platform doesn't have native support for 64 bit division
// and using libc adds a full Kb to the .data section of the binary...
//#define _FIXED_PNT_DIV_PRIM(_n, _d) (div_s64_s64((_n), (_d)))


/*
* Message module configuration
*/
// Enable this module
#define ULIB_ENABLE_MSG ULIB_ENABLE_DEFAULT
//
// Maximum size of short printed strings like line prefixes and log file
// names.
#define MSG_STR_BYTES 16U
//
// If non-zero, the msg subsystem will use malloc() to allocate memory. Otherwise
// all memory is statically-allocated.
#define MSG_USE_MALLOC 1
//
// If non-zero, perform additional checks to handle common problems like being
// passed NULL inputs.
#define DO_MSG_SAFETY_CHECKS ULIB_DO_SAFETY_CHECKS


/*
* Option parsing module configuration
*/
// Enable this module
#define ULIB_ENABLE_GETOPT ULIB_ENABLE_DEFAULT
//
// The maximum number of supported arguments and the maximum size of strings
// in argv is OPT_LEN_MAX. The type used to store the values is determined by
// OPT_LEN_MAX+1.
#define OPT_LEN_MAX (0xFFU - 1U)
//
// If non-zero, perform additional checks to handle common problems like being
// passed NULL inputs.
#define DO_OPT_SAFETY_CHECKS ULIB_DO_SAFETY_CHECKS


/*
* Formatted printing module configuration
*/
// Enable this module
#define ULIB_ENABLE_PRINTF 1
//
// The width in bytes of the largest integer supported in format strings
#define PRINTF_MAX_INT_BYTES 4
//
// If non-zero, use '0o' as the alternate form for octal integers
#define PRINTF_USE_o_FOR_OCTAL 0
//
// The character to use when grouping decimal integers by thousands
#define PRINTF_INT_GROUPING_CHAR ','
//
// If non-zero, perform additional checks to handle common problems like being
// passed NULL inputs.
#define DO_PRINTF_SAFETY_CHECKS ULIB_DO_SAFETY_CHECKS
//
// If set, use a reduced feature set to minimize size
// The individual settings can also be controlled below
#define PRINTF_USE_MINIMAL_FEATURE_SET 1
//
// Try to read integers larger than the size set above
// This will usually work if the value of the variable is less than the
// maximum that can fit in PRINTF_MAX_INT_BYTES bytes
#define PRINTF_TRY_LARGE_INTS (!PRINTF_USE_MINIMAL_FEATURE_SET)
//
// Allow printing binary representations of integers
#define PRINTF_ALLOW_BINARY 1
//
// Allow printing decimal integers with thousands groups separators
#define PRINTF_ALLOW_1000s_GROUPING  (!PRINTF_USE_MINIMAL_FEATURE_SET)
//
// Respect width (minimal print size) and precision (maximum string length)
// when printing strings
#define PRINTF_ALLOW_STRING_WIDTH (!PRINTF_USE_MINIMAL_FEATURE_SET)
//
// Respect width (minimal print size) when printing chars
#define PRINTF_ALLOW_CHAR_WIDTH (!PRINTF_USE_MINIMAL_FEATURE_SET)
//
// Allow left-adjusting when padding a printed value to meet the minimum width
#define PRINTF_ALLOW_LEFT_ADJUST (!PRINTF_USE_MINIMAL_FEATURE_SET)
//
// Allow padding integers with leading 0s
#define PRINTF_ALLOW_ZERO_PADDING 1
//
// Allow using lower-case letters in hexadecimal output
// Otherwise, only capital letters are used
#define PRINTF_ALLOW_LOWERCASE_HEX (!PRINTF_USE_MINIMAL_FEATURE_SET)
//
// Respect precision when printing integers and strings
#define PRINTF_ALLOW_PRECISION 1
//
// Allow printing alternate forms of integers with the '#' flag
#define PRINTF_ALLOW_ALT_FORMS 1
//
// Allow printing leading '+' or ' ' for positive values of signed integers
#define PRINTF_ALLOW_POSITIVE_SIGNS (!PRINTF_USE_MINIMAL_FEATURE_SET)
//
// Allow reading width and precision from the argument list with '*'
#define PRINTF_ALLOW_VARIABLE_WIDTHS (!PRINTF_USE_MINIMAL_FEATURE_SET)
//
// Allow uncommon integer size specifiers; without this only 'l' and 'll'
// are recognized (in addition to the default integer size)
#define PRINTF_ALLOW_UNCOMMON_INTS  1


/*
* Byte FIFO buffer module configuration
*/
// Enable this module
#define ULIB_ENABLE_FIFO_UINT8 ULIB_ENABLE_DEFAULT
//
// The size of fifo_uint8_len_t is based on the size of FIFO_UINT8_MAX_SIZE+1, so using
// one less than the max of an unsigned int type will result in less used
// space than using the max value itself.
#define FIFO_UINT8_MAX_SIZE (0xFFFFU - 1U)
//
// If non-zero, perform additional checks to handle common problems like being
// passed NULL inputs.
#define DO_FIFO_UINT8_SAFETY_CHECKS ULIB_DO_SAFETY_CHECKS


/*
* String module configuration
*/
// Enable this module
#define ULIB_ENABLE_STRINGS ULIB_ENABLE_DEFAULT
//
// Max length of a string, excluding the terminal NUL. Keep at 255 or less
// if the total length including NUL needs to fit in a uint8_t. The type
// of strlen_t is determined by strings.h based on this value.
#define STRING_MAX_BYTES (0xFFU - 1U)
//
// If non-zero, use malloc() to re-size strings as needed. They still won't
// exceed STRING_MAX_BYTES in length excluding the NUL.
#define STRINGS_USE_MALLOC ULIB_USE_MALLOC
//
// When STRINGS_USE_MALLOC is set, grow the string in blocks of this size.
#define STRING_ALLOC_BLOCK_BYTES 16U
//
// If non-zero, use the internal printf() implementation for printing to strings.
// This implementation isn't thread-safe and lacks some features but may be smaller.
#define STRINGS_USE_INTERNAL_PRINTF 1
//
// If non-zero, perform additional checks to handle common problems like being
// passed NULL inputs.
#define DO_STRING_SAFETY_CHECKS ULIB_DO_SAFETY_CHECKS


/*
* Time module configuration
*/
// Enable this module
#define ULIB_ENABLE_TIME 1
//
// Year 0 of the internal calendar. This shouldn't be a leap year.
#define TIME_YEAR_0 2015U
//
// If non-zero, perform additional checks to handle common problems like being
// passed NULL inputs.
#define DO_TIME_SAFETY_CHECKS ULIB_DO_SAFETY_CHECKS
//
// A macro or function used to get the current value of the systick timer.
// Only needed when SET_TIMEOUT_MS() is used. This can be set in the calling
// code (and redefined as needed) because it's only used in a macro.
//#define GET_SYSTICKS_MS()
//
// Like GET_SYSTICKS_MS but used by SET_RTC_TIMEOUT_S().
//#define GET_RTCTICKS_S() (time(null))


/*
* Utility module configuration
*/
// Enable this module
#define ULIB_ENABLE_UTIL 1
//
// If non-zero, perform additional checks to handle common problems like being
// passed NULL inputs.
#define DO_UTIL_SAFETY_CHECKS ULIB_DO_SAFETY_CHECKS
