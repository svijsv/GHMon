//
// uHAL settings that should be overridden for AVR XMega3 devices go in here

//
// Find the pin definitions
#if defined(__AVR_ATtiny402__)
# include GHMON_INCLUDE_CONFIG_HEADER(pindefs/attiny402.h)
# define uHAL_USE_INTERNAL_OSC 1
# define uHAL_USE_INTERNAL_LS_OSC 1
#else
# error "Unhandled device"
#endif

//
// Some devices don't support PWM or certain other peripherals in standby mode
// If using such a device and PWM is needed while sleeping, uncomment this block.
/*
#if uHAL_USE_PWM && (!defined(uHAL_HIBERNATE_LIMIT) || uHAL_HIBERNATE_LIMIT == 0)
# undef uHAL_HIBERNATE_LIMIT
# define uHAL_HIBERNATE_LIMIT HIBERNATE_LIGHT
#endif
*/
