//
// Generated by tools/xmega3/time_TCB_calc_PWM.sh on Sun Sep  1 06:27:09 PM EDT 2024
//
// This script requires the following macros to be set:
//    TIMER_CLOCK: The base clock rate of the timer
//    TIMER_HZ   : The desired frequency of the timer signal
//    TIMER_TOP  : The value of the timer's CCMP register
//
// This script sets the following macros:
//    TIMER_SRC  : The value of the CLKSEL bits in the timer's CTRLA register
//
// This script finds the value for TIMER_SRC that will give a frequency of no
// more than TIMER_HZ with the given value of TIMER_TOP, or if that constraint
// can't be met it will give the lowest frequency achievable
//
#undef DONE

#if !defined(DONE) && (TIMER_CLOCK / (TIMER_TOP * (TIMER_CLOCK))) <= TIMER_HZ
# define DONE 1
# define TIMER_SRC ((uint8_t )0 << TCB_CLKSEL_gp)
#endif

#if !defined(DONE) && (TIMER_CLOCK / (TIMER_TOP * (TIMER_CLOCK/2U))) <= TIMER_HZ
# define DONE 1
# define TIMER_SRC ((uint8_t )1 << TCB_CLKSEL_gp)
#endif

#if !defined(DONE)
# define TIMER_SRC (TCB_CLKSEL_CLKDIV2_gc)
#endif

#undef DONE
