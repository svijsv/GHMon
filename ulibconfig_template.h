//
// config.h
// Configuration file for the ulib modules
//
// This file should only be included by configify.h

/*
* Global configuration
*/
// Default value for module enablement.
#ifndef ULIB_ENABLE_DEFAULT
# define ULIB_ENABLE_DEFAULT 1
#endif
//
// If non-zero, include an ID field in exported structs to make some debugging
// easier. Not all structs currently support this.
#ifndef ULIB_USE_STRUCT_ID
# define ULIB_USE_STRUCT_ID DEBUG
#endif
//
// If non-zero, modules with USE_MALLOC options default to using malloc.
#ifndef ULIB_USE_MALLOC
# define ULIB_USE_MALLOC 1
#endif
//
// If non-zero, default to doing additional safety checks in all modules.
#ifndef ULIB_DO_SAFETY_CHECKS
# define ULIB_DO_SAFETY_CHECKS 1
#endif


/*
* Alternate namespace module configuration
*/
// Enable this module
#ifndef ULIB_ENABLE_FMEM
# define ULIB_ENABLE_FMEM ULIB_ENABLE_DEFAULT
#endif
//
// If non-zero, enable the use of an alternate namespace for storing static
// data.
#ifndef HAVE_FMEM_NAMESPACE
# define HAVE_FMEM_NAMESPACE 0
#endif
//
// The size of the static array(s) used to return strings from FROM_STR()
#ifndef FMEM_STR_BYTES
# define FMEM_STR_BYTES 128U
#endif
//
// If non-zero, perform additional checks to handle common problems like being
// passed NULL inputs.
#ifndef DO_FMEM_SAFETY_CHECKS
# define DO_FMEM_SAFETY_CHECKS ULIB_DO_SAFETY_CHECKS
#endif


/*
* Array module configuration
*/
// Enable this module
#ifndef ULIB_ENABLE_ARRAYS
# define ULIB_ENABLE_ARRAYS ULIB_ENABLE_DEFAULT
#endif
//
// The size of arlen_t is based on the size of ARRAY_MAX_OBJECTS+1, so using
// one less than the max of an unsigned int type will result in less used
// space than using the max value itself.
#ifndef ARRAY_MAX_OBJECTS
# define ARRAY_MAX_OBJECTS (0xFFFFU - 1U)
#endif
//
// Allocated size of an array when first created.
#ifndef ARRAY_INITIAL_SIZE
# define ARRAY_INITIAL_SIZE 4U
#endif
//
// Factor by which to grow an array by when it would otherwise overflow.
#ifndef ARRAY_GROW_FACTOR
# define ARRAY_GROW_FACTOR 2U
#endif
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
#ifndef ARRAY_GROW_METHOD
# define ARRAY_GROW_METHOD ARRAY_GROW_MUL
#endif
//
// If non-zero, the array bank is grown as needed. Otherwise the bank always
// contains ARRAY_MAX_OBJECTS entries and the ARRAY_GROW_* values are ignored.
#ifndef ARRAYS_USE_MALLOC
# define ARRAYS_USE_MALLOC ULIB_USE_MALLOC
#endif
//
// If non-zero, perform additional checks to handle common problems like being
// passed NULL inputs.
#ifndef DO_ARRAY_SAFETY_CHECKS
# define DO_ARRAY_SAFETY_CHECKS ULIB_DO_SAFETY_CHECKS
#endif


/*
* ASCII character module configuration
*/
// Enable this module
#ifndef ULIB_ENABLE_ASCII
# define ULIB_ENABLE_ASCII ULIB_ENABLE_DEFAULT
#endif
//
// If non-zero, replace the ASCII module functions with their ctype.h equivalent
// Not all functions are supported
#ifndef ASCII_SUBSTITUTE_WITH_CTYPE
# define ASCII_SUBSTITUTE_WITH_CTYPE 0
#endif
//
// If non-zero, replace the ctype.h functions with their ASCII module equivalent
// Some functions may not be supported
#ifndef ASCII_SUBSTITUTE_FOR_CTYPE
# define ASCII_SUBSTITUTE_FOR_CTYPE 0
#endif
//
// If non-zero, perform additional checks to handle common problems like being
// passed NULL inputs.
#ifndef DO_ASCII_SAFETY_CHECKS
# define DO_ASCII_SAFETY_CHECKS ULIB_DO_SAFETY_CHECKS
#endif



/*
* Bitwise operation module configuration
*/
// Enable this module
#ifndef ULIB_ENABLE_BITOPS
# define ULIB_ENABLE_BITOPS ULIB_ENABLE_DEFAULT
#endif
//
// Enable the type-checked inlined function versions of the bitwise operations.
// See bits.h for details.
#ifndef ULIB_BITOP_ENABLE_INLINED_FUNCTIONS
# define ULIB_BITOP_ENABLE_INLINED_FUNCTIONS 1
#endif
//
// Enable the 64-bit versions of the above functions. Not all platforms support
// this natively.
#ifndef ULIB_BITOP_ENABLE_INLINED_64BIT_FUNCTIONS
# define ULIB_BITOP_ENABLE_INLINED_64BIT_FUNCTIONS 1
#endif
//
// Enable versions of the bitwise macros and functions that use the _Generic()
// operator. This is more type-safe and doesn't impact the size of optimized code
// but requires a compatible compiler (typically anything that supports C11 or later).
// This requires that ULIB_BITOP_ENABLE_INLINED_64BIT_FUNCTIONS be set.
#ifndef ULIB_BITOP_ENABLE_GENERICS
# define ULIB_BITOP_ENABLE_GENERICS 0
#endif


/*
* Byte buffer module configuration
*/
// Enable this module
#ifndef ULIB_ENABLE_BUFFERS
# define ULIB_ENABLE_BUFFERS ULIB_ENABLE_DEFAULT
#endif
//
// The size of buflen_t is based on the size of BUFFER_MAX_BYTES+1, so using
// one less than the max of an unsigned int type will result in less used
// space than using the max value itself.
#ifndef BUFFER_MAX_BYTES
# define BUFFER_MAX_BYTES (0xFFFFU - 1U)
#endif
//
// Size of a buffer when first created.
#ifndef BUFFER_INITIAL_SIZE
# define BUFFER_INITIAL_SIZE 32U
#endif
//
// Factor by which to grow the buffer when it would otherwise overflow.
#ifndef BUFFER_GROW_FACTOR
# define BUFFER_GROW_FACTOR 2U
#endif
//
// Method of buffer growth. See the description for ARRAY_GROW_METHOD for
// details.
#ifndef BUFFER_GROW_METHOD
# define BUFFER_GROW_METHOD BUFFER_GROW_MUL
#endif
//
// If non-zero, the buffer bank is grown as needed. Otherwise the bank is
// always BUFFER_MAX_BYTES in size and BUFFER_GROW_* values are ignored.
#ifndef BUFFERS_USE_MALLOC
# define BUFFERS_USE_MALLOC ULIB_USE_MALLOC
#endif
//
// If non-zero, perform additional checks to handle common problems like being
// passed NULL inputs.
#ifndef DO_BUFFER_SAFETY_CHECKS
# define DO_BUFFER_SAFETY_CHECKS ULIB_DO_SAFETY_CHECKS
#endif


/*
* C-style string module configuration
*/
// Enable this module
#ifndef ULIB_ENABLE_CSTRINGS
# define ULIB_ENABLE_CSTRINGS ULIB_ENABLE_DEFAULT
#endif
//
// If non-zero, perform additional checks to handle common problems like being
// passed NULL inputs.
#ifndef DO_CSTRING_SAFETY_CHECKS
# define DO_CSTRING_SAFETY_CHECKS ULIB_DO_SAFETY_CHECKS
#endif


/*
* Debugging module configuration
*/
// Enable ulib_assert(), which is used in ulib but can also be used elsewhere
#ifndef USE_ULIB_ASSERT
# define USE_ULIB_ASSERT 1
#endif
//
// Use the ulib_assert() function defined in debug.h instead of pulling in
// assert.h when NDEBUG is undefined.
// If this is enabled and NDEBUG is undefined, the function ulib_assert_failed()
// must be defined somewhere. See debug.h for the prototype.
#ifndef USE_ULIB_LOCAL_ASSERT
# define USE_ULIB_LOCAL_ASSERT 0
#endif
//
// Override assert() with ulib_assert() when USE_ULIB_LOCAL_ASSERT is also set
#ifndef USE_ULIB_ASSERT_OVERRIDE
# define USE_ULIB_ASSERT_OVERRIDE 0
#endif
//
// Enable ulib_panic(), which may be used in ulib but can also be used elsewhere
// Note that unlike assert(), there's no flag for disabling panic() in release
// builds - if enabled, theyre always enabled.
#ifndef USE_ULIB_PANIC
# define USE_ULIB_PANIC 1
#endif
//
// Use the ulib_panic() function defined in debug.h instead of pulling in
// stdlib.h and using abort().
// If this is enabled, the function ulib_panic_abort() must be defined somewhere.
// See debug.h for the prototype.
#ifndef USE_ULIB_LOCAL_PANIC
# define USE_ULIB_LOCAL_PANIC 0
#endif
//
// Enable preprocessor messages issued with DEBUG_CPP_MACRO() and DEBUG_CPP_MSG()
#ifndef DEBUG_CPP_MESSAGES
# define DEBUG_CPP_MESSAGES DEBUG
#endif


/*
* File module configuration
*/
// Enable this module
#ifndef ULIB_ENABLE_FILES
# define ULIB_ENABLE_FILES ULIB_ENABLE_DEFAULT
#endif
//
// The maximum number of levels to descend in recursive operations.
#ifndef FILE_MAX_RECURSION
# define FILE_MAX_RECURSION 255U
#endif
//
// An optional externally-defined buffer used for file operations that require
// one. Used when functions are passed NULL as their buffer.
// If defined, it must be uint8_t*.
//#define FILE_PROVIDED_BUF g_byte_buffer
//#define FILE_PROVIDED_BUF_SIZE 16384
//
// If non-zero, perform additional checks to handle common problems like being
// passed NULL inputs.
#ifndef DO_FILE_SAFETY_CHECKS
# define DO_FILE_SAFETY_CHECKS ULIB_DO_SAFETY_CHECKS
#endif
//
// These are additional checks for the internal helper functions.
#ifndef DO_FILE_EXTRA_SAFETY_CHECKS
# define DO_FILE_EXTRA_SAFETY_CHECKS ULIB_DO_SAFETY_CHECKS
#endif


/*
* Heap allocation module configuration
*/
// Enable this module
#ifndef ULIB_ENABLE_HALLOC
# define ULIB_ENABLE_HALLOC ULIB_ENABLE_DEFAULT
#endif
//
// The start address for memory allocation
// If 0, HEAP_START_LINKER_VAR must be defined.
#ifndef HALLOC_HEAP_START_ADDR
# define HALLOC_HEAP_START_ADDR 0
#endif
//
// The name of a variable whose address is the start of the heap, usually defined
// by the linker.
#ifndef HALLOC_HEAP_START_LINKER_VAR
# define HALLOC_HEAP_START_LINKER_VAR __heap_start
#endif
//
// The end address for memory allocation
// If 0, the heap is assumed to be below the stack and allowed to grow until it's
// within HALLOC_STACK_MARGIN bytes of the stack pointer.
#ifndef HALLOC_HEAP_END_ADDR
# define HALLOC_HEAP_END_ADDR 0
#endif
//
// The minimum number of bytes to maintain between the top of the heap and the
// bottom of the stack
// If > 0, the function `size_t ulib_get_stack_pointer_addr(void)` should be
// defined somewhere and return the address of the system stack pointer. There's
// a default, but it's... not very good (and probably won't work on anything but
// embedded systems).
#ifndef HALLOC_STACK_MARGIN
# define HALLOC_STACK_MARGIN 32U
#endif
//
// The alignment in bytes of all addresses returned
// If 0, use sizeof(uintptr_t).
#ifndef HALLOC_ADDR_ALIGNMENT
# define HALLOC_ADDR_ALIGNMENT 0
#endif
//
// If non-zero, write this canary value at the end of the heap and check before
// allocations to see if it's been changed, ulib_panic()ing if so.
#ifndef HALLOC_HEAP_CANARY
# define HALLOC_HEAP_CANARY 0xABU
#endif
//
// If >= 0, initialize allocated memory blocks to this value
#ifndef HALLOC_MEM_INIT_VALUE
# define HALLOC_MEM_INIT_VALUE 0
#endif
//
// If non-zero, perform additional checks to handle common problems like being
// passed NULL inputs.
#ifndef DO_HALLOC_SAFETY_CHECKS
# define DO_HALLOC_SAFETY_CHECKS ULIB_DO_SAFETY_CHECKS
#endif


/*
* Linked-list module configuration
*/
// Enable this module
#ifndef ULIB_ENABLE_LISTS
# define ULIB_ENABLE_LISTS ULIB_ENABLE_DEFAULT
#endif
//
// The size of listlen_t is based on the size of LIST_MAX_OBJECTS+1, so using
// one less than the max of an unsigned int type will result in less used
// space than using the max value itself.
#ifndef LIST_MAX_OBJECTS
# define LIST_MAX_OBJECTS (0xFFFFU - 1U)
#endif
//
// If non-zero, new entries are created with malloc. Otherwise a pointer to
// a list_entry_t struct will need to be supplied when adding to the list.
#ifndef LISTS_USE_MALLOC
# define LISTS_USE_MALLOC ULIB_USE_MALLOC
#endif
//
// If non-zero, perform additional checks to handle common problems like being
// passed NULL inputs.
#ifndef DO_LIST_SAFETY_CHECKS
# define DO_LIST_SAFETY_CHECKS ULIB_DO_SAFETY_CHECKS
#endif


/*
* Math module configuration
*/
// Enable this module
#ifndef ULIB_ENABLE_MATH
# define ULIB_ENABLE_MATH ULIB_ENABLE_DEFAULT
#endif
//
// The number of bits used in fixed-point representations. This includes the
// sign bit.
#ifndef FIXED_PNT_BITS
# define FIXED_PNT_BITS 32U
#endif
//
// The number of bits of a fixed-point representation devoted to the fraction.
#ifndef FIXED_PNT_FRACT_BITS
# define FIXED_PNT_FRACT_BITS 8U
#endif
//
// If non-zero, perform additional checks to handle common problems like
// division by 0.
#ifndef DO_MATH_SAFETY_CHECKS
# define DO_MATH_SAFETY_CHECKS ULIB_DO_SAFETY_CHECKS
#endif
//
// If set, override the macro used to divide fixed-point integers. Useful
// because a certain platform doesn't have native support for 64 bit division
// and using libc adds a full Kb to the .data section of the binary...
//#define _FIXED_PNT_DIV_PRIM(_n, _d) (div_s64_s64((_n), (_d)))


/*
* Message module configuration
*/
// Enable this module
#ifndef ULIB_ENABLE_MSG
# define ULIB_ENABLE_MSG ULIB_ENABLE_DEFAULT
#endif
//
// Maximum size of short printed strings like line prefixes and log file
// names.
#ifndef MSG_STR_BYTES
# define MSG_STR_BYTES 16U
#endif
//
// If non-zero, the msg subsystem will use malloc() to allocate memory. Otherwise
// all memory is statically-allocated.
#ifndef MSG_USE_MALLOC
# define MSG_USE_MALLOC 1
#endif
//
// If non-zero, perform additional checks to handle common problems like being
// passed NULL inputs.
#ifndef DO_MSG_SAFETY_CHECKS
# define DO_MSG_SAFETY_CHECKS ULIB_DO_SAFETY_CHECKS
#endif


/*
* Option parsing module configuration
*/
// Enable this module
#ifndef ULIB_ENABLE_GETOPT
# define ULIB_ENABLE_GETOPT ULIB_ENABLE_DEFAULT
#endif
//
// The maximum number of supported arguments and the maximum size of strings
// in argv is OPT_LEN_MAX. The type used to store the values is determined by
// OPT_LEN_MAX+1.
#ifndef OPT_LEN_MAX
# define OPT_LEN_MAX (0xFFU - 1U)
#endif
//
// If non-zero, perform additional checks to handle common problems like being
// passed NULL inputs.
#ifndef DO_OPT_SAFETY_CHECKS
# define DO_OPT_SAFETY_CHECKS ULIB_DO_SAFETY_CHECKS
#endif


/*
* Formatted printing module configuration
*/
// Enable this module
#ifndef ULIB_ENABLE_PRINTF
# define ULIB_ENABLE_PRINTF ULIB_ENABLE_DEFAULT
#endif
//
// The width in bytes of the largest integer supported in format strings
#ifndef PRINTF_MAX_INT_BYTES
# define PRINTF_MAX_INT_BYTES 4
#endif
//
// If non-zero, use '0o' as the alternate form for octal integers
#ifndef PRINTF_USE_o_FOR_OCTAL
# define PRINTF_USE_o_FOR_OCTAL 0
#endif
//
// The character to use when grouping decimal integers by thousands
#ifndef PRINTF_INT_GROUPING_CHAR
# define PRINTF_INT_GROUPING_CHAR ','
#endif
//
// If non-zero, perform additional checks to handle common problems like being
// passed NULL inputs.
#ifndef DO_PRINTF_SAFETY_CHECKS
# define DO_PRINTF_SAFETY_CHECKS ULIB_DO_SAFETY_CHECKS
#endif
//
// If set, use a reduced feature set to minimize size
// The features can also be controlled individually below
#ifndef PRINTF_USE_MINIMAL_FEATURE_SET
# define PRINTF_USE_MINIMAL_FEATURE_SET 0
#endif
//
// Try to read integers larger than the size set above
// This will usually work if the value of the variable is less than the
// maximum that can fit in PRINTF_MAX_INT_BYTES bytes
#ifndef PRINTF_TRY_LARGE_INTS
# define PRINTF_TRY_LARGE_INTS (!PRINTF_USE_MINIMAL_FEATURE_SET)
#endif
//
// Allow printing binary representations of integers
#ifndef PRINTF_ALLOW_BINARY
# define PRINTF_ALLOW_BINARY (!PRINTF_USE_MINIMAL_FEATURE_SET)
#endif
//
// Allow uncommon integer size specifiers; without this only 'l' and 'll'
// are recognized (in addition to the default integer size)
#ifndef PRINTF_ALLOW_UNCOMMON_INTS
# define PRINTF_ALLOW_UNCOMMON_INTS (!PRINTF_USE_MINIMAL_FEATURE_SET)
#endif
//
// Allow using lower-case letters in hexadecimal output
// Otherwise, only capital letters are used
#ifndef PRINTF_ALLOW_LOWERCASE_HEX
# define PRINTF_ALLOW_LOWERCASE_HEX (!PRINTF_USE_MINIMAL_FEATURE_SET)
#endif
//
// Allow printing decimal integers with thousands groups separators
#ifndef PRINTF_ALLOW_1000s_GROUPING
# define PRINTF_ALLOW_1000s_GROUPING (!PRINTF_USE_MINIMAL_FEATURE_SET)
#endif
//
// Allow printing alternate forms of integers with the '#' flag
#ifndef PRINTF_ALLOW_ALT_FORMS
# define PRINTF_ALLOW_ALT_FORMS (!PRINTF_USE_MINIMAL_FEATURE_SET)
#endif
//
// Allow left-adjusting when padding a printed value to meet the minimum width
#ifndef PRINTF_ALLOW_LEFT_ADJUST
# define PRINTF_ALLOW_LEFT_ADJUST (!PRINTF_USE_MINIMAL_FEATURE_SET)
#endif
//
// Allow padding integers with leading 0s
#ifndef PRINTF_ALLOW_ZERO_PADDING
# define PRINTF_ALLOW_ZERO_PADDING (!PRINTF_USE_MINIMAL_FEATURE_SET)
#endif
//
// Allow reading width and precision from the argument list with '*'
#ifndef PRINTF_ALLOW_VARIABLE_WIDTHS
# define PRINTF_ALLOW_VARIABLE_WIDTHS (!PRINTF_USE_MINIMAL_FEATURE_SET)
#endif
//
// Respect width (minimal print size) and precision (maximum string length)
// when printing strings
#ifndef PRINTF_ALLOW_STRING_WIDTH
# define PRINTF_ALLOW_STRING_WIDTH (!PRINTF_USE_MINIMAL_FEATURE_SET)
#endif
//
// Respect width (minimal print size) when printing chars
#ifndef PRINTF_ALLOW_CHAR_WIDTH
# define PRINTF_ALLOW_CHAR_WIDTH (!PRINTF_USE_MINIMAL_FEATURE_SET)
#endif
//
// Respect precision when printing integers and strings
#ifndef PRINTF_ALLOW_PRECISION
# define PRINTF_ALLOW_PRECISION 1
#endif
//
// Allow printing leading '+' or ' ' for positive values of signed integers
#ifndef PRINTF_ALLOW_POSITIVE_SIGNS
# define PRINTF_ALLOW_POSITIVE_SIGNS (!PRINTF_USE_MINIMAL_FEATURE_SET)
#endif


/*
* Byte FIFO buffer module configuration
*/
// Enable this module
#ifndef ULIB_ENABLE_FIFO_UINT8
# define ULIB_ENABLE_FIFO_UINT8 ULIB_ENABLE_DEFAULT
#endif
//
// The size of fifo_uint8_len_t is based on the size of FIFO_UINT8_MAX_SIZE+1, so using
// one less than the max of an unsigned int type will result in less used
// space than using the max value itself.
#ifndef FIFO_UINT8_MAX_SIZE
# define FIFO_UINT8_MAX_SIZE (0xFFFFU - 1U)
#endif
//
// If non-zero, perform additional checks to handle common problems like being
// passed NULL inputs.
#ifndef DO_FIFO_UINT8_SAFETY_CHECKS
# define DO_FIFO_UINT8_SAFETY_CHECKS ULIB_DO_SAFETY_CHECKS
#endif


/*
* String module configuration
*/
// Enable this module
#ifndef ULIB_ENABLE_STRINGS
# define ULIB_ENABLE_STRINGS ULIB_ENABLE_DEFAULT
#endif
//
// Max length of a string, excluding the terminal NUL. Keep at 255 or less
// if the total length including NUL needs to fit in a uint8_t. The type
// of strlen_t is determined by strings.h based on this value.
#ifndef STRING_MAX_BYTES
# define STRING_MAX_BYTES (0xFFFFU - 1U)
#endif
//
// If non-zero, use malloc() to re-size strings as needed. They still won't
// exceed STRING_MAX_BYTES in length excluding the NUL.
#ifndef STRINGS_USE_MALLOC
# define STRINGS_USE_MALLOC ULIB_USE_MALLOC
#endif
//
// When STRINGS_USE_MALLOC is set, grow the string in blocks of this size.
#ifndef STRING_ALLOC_BLOCK_BYTES
# define STRING_ALLOC_BLOCK_BYTES 16U
#endif
//
// If non-zero, use the internal printf() implementation for printing to strings.
// This implementation isn't thread-safe and lacks some features but may be smaller.
#ifndef STRINGS_USE_INTERNAL_PRINTF
# define STRINGS_USE_INTERNAL_PRINTF 0
#endif
//
// If non-zero, perform additional checks to handle common problems like being
// passed NULL inputs.
#ifndef DO_STRING_SAFETY_CHECKS
# define DO_STRING_SAFETY_CHECKS ULIB_DO_SAFETY_CHECKS
#endif


/*
* Time module configuration
*/
// Enable this module
#ifndef ULIB_ENABLE_TIME
# define ULIB_ENABLE_TIME ULIB_ENABLE_DEFAULT
#endif
//
// Year 0 of the internal calendar. This shouldn't be a leap year.
#ifndef TIME_YEAR_0
# define TIME_YEAR_0 2015U
#endif
//
// If non-zero, perform additional checks to handle common problems like being
// passed NULL inputs.
#ifndef DO_TIME_SAFETY_CHECKS
# define DO_TIME_SAFETY_CHECKS ULIB_DO_SAFETY_CHECKS
#endif
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
#ifndef ULIB_ENABLE_UTIL
# define ULIB_ENABLE_UTIL ULIB_ENABLE_DEFAULT
#endif
//
// If non-zero, perform additional checks to handle common problems like being
// passed NULL inputs.
#ifndef DO_UTIL_SAFETY_CHECKS
# define DO_UTIL_SAFETY_CHECKS ULIB_DO_SAFETY_CHECKS
#endif
