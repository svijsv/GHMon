//
// uHAL settings that should be overridden for STM32 devices go in here

#define uHAL_USE_INTERNAL_OSC 0
#define uHAL_USE_INTERNAL_LS_OSC 0

//
// Find the pin definitions
#if defined(STM32F103x6) || defined(STM32F103xB) || defined(STM32F103xE) || defined(STM32F103xG)
# if ! uHAL_USE_INTERNAL_OSC && !defined(F_OSC)
#  define F_OSC 8000000UL
# endif
# include GHMON_INCLUDE_CONFIG_HEADER(pindefs/stm32f103.h)

#elif defined(STM32F401xC) || defined(STM32F401xE)
# if ! uHAL_USE_INTERNAL_OSC && !defined(F_OSC)
#  define F_OSC 25000000UL
# endif
# include GHMON_INCLUDE_CONFIG_HEADER(pindefs/stm32f401.h)

#else
# error "Unhandled device"
#endif
