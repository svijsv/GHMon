/*
* Find a PLL multiplier to achieve a given HCLK frequency on an STM32F1
*
* Connectivity-line devices have a different internal layout and this will
* give the wrong results.
*/

#if ! defined(PLL_TEST_OSC)
# error "Expected PLL_TEST_OSC to be defined"
#endif

#define MUL_IS_OK(m) (((PLL_TEST_OSC * m) >= PLL_OUTPUT_MIN_HZ) && CAN_USE_OSC_CLOCK(FREQ_OUTPUT_HZ, PLL_TEST_OSC * m))

#if MUL_IS_OK(2)
# define PLL_MUL 2
#elif MUL_IS_OK(3)
# define PLL_MUL 3
#elif MUL_IS_OK(4)
# define PLL_MUL 4
#elif MUL_IS_OK(5)
# define PLL_MUL 5
#elif MUL_IS_OK(6)
# define PLL_MUL 6
#elif MUL_IS_OK(7)
# define PLL_MUL 7
#elif MUL_IS_OK(8)
# define PLL_MUL 8
#elif MUL_IS_OK(9)
# define PLL_MUL 9
#elif MUL_IS_OK(10)
# define PLL_MUL 10
#elif MUL_IS_OK(11)
# define PLL_MUL 11
#elif MUL_IS_OK(12)
# define PLL_MUL 12
#elif MUL_IS_OK(13)
# define PLL_MUL 13
#elif MUL_IS_OK(14)
# define PLL_MUL 14
#elif MUL_IS_OK(15)
# define PLL_MUL 15
#elif MUL_IS_OK(16)
# define PLL_MUL 16
#endif

#undef MUL_IS_OK
