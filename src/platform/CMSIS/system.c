// SPDX-License-Identifier: GPL-3.0-only
/***********************************************************************
*                                                                      *
*                                                                      *
* Copyright 2021 svijsv                                                *
* This program is free software: you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation, version 3.                             *
*                                                                      *
* This program is distributed in the hope that it will be useful, but  *
* WITHOUT ANY WARRANTY; without even the implied warranty of           *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU    *
* General Public License for more details.                             *
*                                                                      *
* You should have received a copy of the GNU General Public License    *
* along with this program.  If not, see <http:// www.gnu.org/licenses/>.*
*                                                                      *
*                                                                      *
***********************************************************************/
// system.c
// General platform initialization
// NOTES:
//

#ifdef __cplusplus
 extern "C" {
#endif

//#define DEBUG 1
/*
* Includes
*/
#include "system.h"
#include "adc.h"
#include "gpio.h"
#include "spi.h"
#include "time.h"
#include "uart.h"
#include "i2c.h"


/*
* Static values
*/
#if defined STM32F1
# define EXTI_PREG AFIO
# define EXTI_PREG_CLOCKEN RCC_PERIPH_AFIO
# define FLASH_ACR_PRFTEN_Pos FLASH_ACR_PRFTBE_Pos
# define FLASH_ACR_PRFTEN     FLASH_ACR_PRFTBE
#else
# define EXTI_PREG SYSCFG
# define EXTI_PREG_CLOCKEN RCC_PERIPH_SYSCFG
# define RCC_BDCR_RTCSEL_NOCLOCK 0
# define RCC_BDCR_RTCSEL_LSE (0b01 << RCC_BDCR_RTCSEL_Pos)
# define RCC_BDCR_RTCSEL_LSI (0b10 << RCC_BDCR_RTCSEL_Pos)
# define RCC_BDCR_RTCSEL_HSE (0b11 << RCC_BDCR_RTCSEL_Pos)
#endif

//
// For STM32F1 use a flash latency of 0 for speeds <= 24MHz, 1 for
// 24-48MHz, and 2 for anything higher.
//
// For other STM32s, assuming 3.3V Vcc, the number of wait states varies
// by line but on the conservative end would be 0 for <= 25MHz and +1 for
// each additional 25MHz above that, or more commonly 0 for <= 30MHz and
// +1 for each additional 30MHz above that up to a line-dependendent maximum.
//
#if defined(STM32F1)
# define FLASH_WS_MAX 2
# define FLASH_WS_STEP 24000000
#elif defined(STM32F413xx) || defined(STM32F423xx)
# define FLASH_WS_MAX 6
# define FLASH_WS_STEP 25000000
#elif defined(STM32F401xC) || defined(STM32F401xE)
# define FLASH_WS_MAX 5
# define FLASH_WS_STEP 30000000
#elif defined(STM32F4)
# define FLASH_WS_MAX 6
# define FLASH_WS_STEP 30000000
#else
# error "Unsupported STM32 architecture"
#endif

//
// Determine main clock source and configuration
// Fair warning, this is a long one
//
// When all done, the following macros are defined:
//    FREQ_INPUT_HZ : The frequency of the input oscillator (HSE or HSI)
//    FREQ_OUTPUT_HZ: The frequency of the system clock
//    FREQ_OSC_HZ   : The frequency of the system oscillator (HSE, HSI, or PLL)
//
// If G_freq_HCLK can't be achieved with just the prescaler, then the PLL is
// used and the following are also defined:
//    PLL_SRC: The source of the PLL clock (PLL_SRC_{HSI,HSE}[_DIV2])
//    PLL_MUL (F1 only) : The value by which PLL_SRC is multiplied
//    PLL[MNP] (non-F1 only): The values of the corresponding RCC_PLLCFGR fields
//
#define PLL_SRC_NONE     0
#define PLL_SRC_HSE      1
#define PLL_SRC_HSE_DIV2 2
#define PLL_SRC_HSI      3
#define PLL_SRC_HSI_DIV2 4
#if USE_INTERNAL_CLOCK
# define FREQ_INPUT_HZ G_freq_HSI
#else
# define FREQ_INPUT_HZ G_freq_HSE
#endif
#define FREQ_OUTPUT_HZ G_freq_HCLK
#define CAN_USE_OSC_CLOCK(freq, osc) ( \
	((freq) *   1 == (osc)) || \
	((freq) *   2 == (osc)) || \
	((freq) *   4 == (osc)) || \
	((freq) *   8 == (osc)) || \
	((freq) *  16 == (osc)) || \
	((freq) *  64 == (osc)) || \
	((freq) * 128 == (osc)) || \
	((freq) * 256 == (osc)) || \
	((freq) * 512 == (osc)) \
	)

#if CAN_USE_OSC_CLOCK(FREQ_OUTPUT_HZ, FREQ_INPUT_HZ)
# define PLL_SRC PLL_SRC_NONE
# define FREQ_OSC_HZ FREQ_INPUT_HZ
# if DEBUG
#  pragma message "PLL_SRC is " XTRINGIZE(PLL_SRC)
#  pragma message "FREQ_OSC_HZ is " XTRINGIZE(FREQ_OSC_HZ)
# endif

#elif USE_STM32F1_PLL
  // The PLL output minimum isn't mentioned anywhere in the reference manual
  // from what I can find but *is* mentioned in the data sheet in the section
  // 'PLL characteristics'
  // There's also a maximum but I don't care about that because it's just the
  // sysclock maximum.
# define PLL_OUTPUT_MIN_HZ 16000000

  // HSI source is always divided by 2
# if ! USE_INTERNAL_CLOCK
#  define PLL_TEST_OSC FREQ_INPUT_HZ
#  include "system_PLL_STM32F1.h"
# endif
# if PLL_MUL
#  define PLL_SRC PLL_SRC_HSE
#  define FREQ_OSC_HZ (FREQ_INPUT_HZ * PLL_MUL)
# else
#  undef PLL_TEST_OSC
#  define PLL_TEST_OSC (FREQ_INPUT_HZ/2)
#  include "system_PLL_STM32F1.h"
#  if PLL_MUL
#   if USE_INTERNAL_CLOCK
#    define PLL_SRC PLL_SRC_HSI_DIV2
#   else
#    define PLL_SRC PLL_SRC_HSE_DIV2
#   endif
#   define FREQ_OSC_HZ ((FREQ_INPUT_HZ/2) * PLL_MUL)
#  else
#   error "Unsupported main clock frequency"
#  endif
# endif
# if DEBUG
#  pragma message "PLL_SRC is " XTRINGIZE(PLL_SRC)
#  pragma message "PLL_MUL is " XTRINGIZE(PLL_MUL)
#  pragma message "FREQ_OSC_HZ is " XTRINGIZE(FREQ_OSC_HZ)
# endif

// The other STM32 lines are more difficult to auto-configure than the F1s
// because they have 3 different prescalers affecting the main clock (and
// two more for other clocks but we don't use those) and two of those have
// far wider value ranges than the F1's so they can't just be listed out
// here. Instead they're determined mostly in an auto-generated external
// header that *does* list them all out.
#else // ! USE_STM32F1_PLL
# if USE_INTERNAL_CLOCK
#  define PLL_SRC PLL_SRC_HSI
# else
#  define PLL_SRC PLL_SRC_HSE
# endif
# define PLLP_DIV_2 0b00
# define PLLP_DIV_4 0b01
# define PLLP_DIV_6 0b10
# define PLLP_DIV_8 0b11

  // The PLL output minimum isn't mentioned anywhere in the reference manual
  // from what I can find but *is* mentioned in the data sheet in the section
  // 'PLL characteristics'
  // There's also a maximum but I don't care about that because it's just the
  // sysclock maximum.
# define PLL_OUTPUT_MIN_HZ 24000000
# define PLLM_MIN 2
# define PLLM_MAX 63
# define VCO_INPUT_MIN_HZ 1000000
# define VCO_INPUT_MAX_HZ 2000000
  // Acceptable VCO output is device-dependent; refer to the reference manual
  // I didn't search them all extensively I'm just defaulting to the
  // conservative number
  // The headers that detect this stuff will have to be re-generated if the
  // range changes
# define PLLN_MIN 192
# define VCO_OUTPUT_MIN_HZ 192000000
# define PLLN_MAX 432
# define VCO_OUTPUT_MAX_HZ 432000000

  // All the magic happens in here
# include "system_PLL_STM32Fx.h"
# if ! defined(PLLP)
#  error "Unsupported main clock frequency"
# endif
//# define FREQ_OSC_HZ (((FREQ_INPUT_HZ / PLLM) * PLLN) / PLLP_DIV)
# define FREQ_OSC_HZ PLL_OUTPUT_HZ

# if DEBUG
#  pragma message "PLL_SRC is " XTRINGIZE(PLL_SRC)
#  pragma message "PLLM is " XTRINGIZE(PLLM)
#  pragma message "PLLN is " XTRINGIZE(PLLN)
#  pragma message "PLLP is " XTRINGIZE(PLLP)
#  pragma message "PLLP_DIV is " XTRINGIZE(PLLP_DIV)
#  pragma message "HCLK_DIV is " XTRINGIZE(HCLK_DIV)
#  pragma message "VCO_INPUT_HZ is " XTRINGIZE(VCO_INPUT_HZ)
#  pragma message "VCO_OUTPUT_HZ is " XTRINGIZE(VCO_OUTPUT_HZ)
#  pragma message "FREQ_OSC_HZ is " XTRINGIZE(FREQ_OSC_HZ)
# endif
#endif // ! USE_STM32F1_PLL
#undef CAN_USE_OSC_CLOCK

#define SYSCLOCKSW_HSI 0b00
#define SYSCLOCKSW_HSE 0b01
#define SYSCLOCKSW_PLL 0b10
#if PLL_SRC == PLL_NONE
# if USE_INTERNAL_CLOCK
#  define SYSCLOCKEN  RCC_CR_HSION
#  define SYSCLOCKSW  SYSCLOCKSW_HSI
#  define SYSCLOCKRDY RCC_CR_HSIRDY
# else
#  define SYSCLOCKEN  RCC_CR_HSEON
#  define SYSCLOCKSW  SYSCLOCKSW_HSE
#  define SYSCLOCKRDY RCC_CR_HSERDY
# endif
#else
# define SYSCLOCKEN  RCC_CR_PLLON
# define SYSCLOCKSW  SYSCLOCKSW_PLL
# define SYSCLOCKRDY RCC_CR_PLLRDY
#endif


/*
* Types
*/


/*
* Variables
*/
#if USE_TERMINAL
// Treat the first sleep as user-initiated to make entering the terminal
// more convenient
static bool button_wakeup = true;
#endif
static uint8_t bd_write_enabled = 0;

const uint32_t G_freq_OSC = FREQ_OSC_HZ;


/*
* Local function prototypes
*/
static void light_sleep_ms(utime_t ms, uint8_t flags);
static void deep_sleep_s(utime_t s, uint8_t flags);
static void clocks_init(void);
static void enable_button_ISR(void);
static void disable_button_ISR(void);

static inline __attribute__((always_inline)) \
void sysflash(void);

/*
* Interrupt handlers
*/
#if BUTTON_PIN
OPTIMIZE_FUNCTION \
void Button_IRQHandler(void) {
	NVIC_DisableIRQ(BUTTON_IRQn);
	// The EXTI pending bit is cleared by writing 1
	SET_BIT(EXTI->PR, GPIO_GET_PINMASK(BUTTON_PIN));
	NVIC_ClearPendingIRQ(BUTTON_IRQn);

	// Let the user know we got the interrupt even if we're not going to do
	// anything for a while
	// Can't use led_flash(), we may not have delay_ms() if we're coming out of
	// hibernate_s() or sleep_ms().
	// led_flash(1, DELAY_ACK);
	sysflash();

#if USE_TERMINAL
	button_wakeup = true;
#endif

	SET_BIT(G_IRQs, BUTTON_IRQf);

	return;
}
#endif // BUTTON_PIN

/*
* Functions
*/
void platform_init(void) {
	clocks_init();

	// The AFIO clock is needed to fix a few problems, remap the inputs, and
	// to set up the button interrupt below, so enable it here and disable at
	// the end of setup when it's no longer needed
	clock_init(EXTI_PREG_CLOCKEN);

	gpio_init();
	// Violoate our separation of concerns in order to provide early user
	// feedback
	gpio_set_mode(LED_PIN, GPIO_MODE_PP, GPIO_LOW);
	// Can't led_flash() until time_init() has been called.
	// led_flash(1, DELAY_SHORT);

	time_init();
	led_flash(1, DELAY_SHORT);

#if USE_UART
	uart_init();
	led_flash(1, DELAY_SHORT);
#endif

#if USE_SPI
	spi_init();
	led_flash(1, DELAY_SHORT);
#endif

#if USE_I2C
	i2c_init();
	led_flash(1, DELAY_SHORT);
#endif

#if USE_ADC
	adc_init();
	led_flash(1, DELAY_SHORT);
#endif

	// Set interrupt priority grouping
	// Use 4 bits for group priority and 0 bits for subpriority
	// See 4.4.5 (SCB_AIRCR register) of the Cortex M3 programming manual for
	// how to determine the value passed, it looks to be '3 + number of bits
	// for subpriority'. Cortex M4 is the same.
	NVIC_SetPriorityGrouping(0b011 + 0);

#if BUTTON_PIN
	uint32_t pmask, pinno;

	// Make sure the correct port is connected to the EXTI line
#if GPIO_GET_PORTNO(BUTTON_PIN) == GPIO_PORTA
	pmask = 0b0000;
#elif GPIO_GET_PORTNO(BUTTON_PIN) == GPIO_PORTB
	pmask = 0b0001;
#else
# error "Unsupported BUTTON_PIN port"
#endif
	pinno = GPIO_GET_PINNO(BUTTON_PIN);
#if GPIO_GET_PINNO(BUTTON_PIN) <= 3
	pinno = pinno*4;
	pmask <<= (pinno);
	MODIFY_BITS(EXTI_PREG->EXTICR[0], 0b1111 << pinno, pmask);
#elif GPIO_GET_PINNO(BUTTON_PIN) <= 7
	pinno = (pinno-4)*4;
	pmask <<= (pinno);
	MODIFY_BITS(EXTI_PREG->EXTICR[1], 0b1111 << pinno, pmask);
#elif GPIO_GET_PINNO(BUTTON_PIN) <= 11
	pinno = (pinno-8)*4;
	pmask <<= (pinno);
	MODIFY_BITS(EXTI_PREG->EXTICR[2], 0b1111 << pinno, pmask);
#elif GPIO_GET_PINNO(BUTTON_PIN) <= 15
	pinno = (pinno-12)*4;
	pmask <<= (pinno);
	MODIFY_BITS(EXTI_PREG->EXTICR[3], 0b1111 << pinno, pmask);
#else
# error "Unhandled BUTTON_PIN"
#endif // GPIO_GET_PINNO(BUTTON_PIN)

	power_on_input(BUTTON_PIN);
	// Configure the EXTI interrupt for the wakeup button
	SET_BIT(EXTI->IMR,  GPIO_GET_PINMASK(BUTTON_PIN));
# if (GPIO_GET_BIAS(BUTTON_PIN) == BIAS_HIGH)
	// Falling edge
	SET_BIT(EXTI->FTSR, GPIO_GET_PINMASK(BUTTON_PIN));
# else
	// Rising edge
	SET_BIT(EXTI->RTSR, GPIO_GET_PINMASK(BUTTON_PIN));
# endif
	NVIC_SetPriority(BUTTON_IRQn, BUTTON_IRQp);
	disable_button_ISR();
#endif // BUTTON_PIN

	clock_disable(EXTI_PREG_CLOCKEN);

	return;
}
static void enable_button_ISR(void) {
#if BUTTON_PIN
	NVIC_ClearPendingIRQ(BUTTON_IRQn);
	NVIC_EnableIRQ(BUTTON_IRQn);
#endif
}
static void disable_button_ISR(void) {
#if BUTTON_PIN
	NVIC_DisableIRQ(BUTTON_IRQn);
	NVIC_ClearPendingIRQ(BUTTON_IRQn);
#endif
}
static inline __attribute__((always_inline)) \
void sysflash(void) {
	SET_GPIO_OUTPUT_HIGH(LED_PIN);
	// Hopefully ~100 ms counting overhead time @4MHz
	for (uint32_t i = 0; i < 100000; ++i) {
		// Count some clock cycles
		__asm__ volatile("" : "+g" (i) : :);
	}

	SET_GPIO_OUTPUT_LOW(LED_PIN);
	// Short delay to keep flashes from bleeding into each other
	// Hopefully ~20ms counting overhead time @4MHz
	for (uint32_t i = 0; i < 20000; ++i) {
		// Count some clock cycles
		__asm__ volatile("" : "+g" (i) : :);
	}
}
static void enable_sysclock(void) {
# if PLL_SRC == PLL_SRC_HSE
	SET_BIT(RCC->CR, RCC_CR_HSEON);
	while (!BIT_IS_SET(RCC->CR, RCC_CR_HSERDY)) {
		// Nothing to do here
	}
# endif

	SET_BIT(RCC->CR, SYSCLOCKEN);
	while (!BIT_IS_SET(RCC->CR, SYSCLOCKRDY)) {
		// Nothing to do here
	}
	MODIFY_BITS(RCC->CFGR, RCC_CFGR_SW,
		(SYSCLOCKSW << RCC_CFGR_SW_Pos) | // Set SYSCLK source
		0);
	// Wait for source to become SYSCLK
	while (GATHER_BITS(RCC->CFGR, 0b11, RCC_CFGR_SWS_Pos) != SYSCLOCKSW) {
		// Nothing to do here
	}

	return;
}
static void clocks_init(void) {
	uint32_t reg, latency;

	// Don't use clock source protection
	CLEAR_BIT(RCC->CR, RCC_CR_CSSON);

	// Always start out with the HSI to guard against problems setting the PLL
	// if it's already (somehow) enabled
	SET_BIT(RCC->CR, RCC_CR_HSION);
	while (!BIT_IS_SET(RCC->CR, RCC_CR_HSIRDY)) {
		// Nothing to do here
	}
	MODIFY_BITS(RCC->CFGR, RCC_CFGR_SW,
		(0b00 << RCC_CFGR_SW_Pos )  | // Use HSI as SYSCLK source
		0);
	// Wait for HSI to become SYSCLK
	while (GATHER_BITS(RCC->CFGR, 0b11, RCC_CFGR_SWS_Pos) != 0b00) {
		// Nothing to do here
	}
	// Make sure PLL is off
	CLEAR_BIT(RCC->CR, RCC_CR_PLLON);
	while (BIT_IS_SET(RCC->CR, RCC_CR_PLLRDY)) {
		// Nothing to do here
	}

	// Set flash latency and prefetch buffer
	//
	// The prefetch buffer must be on when using a scaler other than 1 for AHB
	//
	// This must be configured prior to setting the clock or else there may
	// be too few states
	//
	// The latency is 3 bits on the STM32F1 and 4 bits on the other lines
	latency = G_freq_HCLK / FLASH_WS_STEP;
	latency = (latency > FLASH_WS_MAX) ? FLASH_WS_MAX : latency;
	MODIFY_BITS(FLASH->ACR, FLASH_ACR_PRFTEN|FLASH_ACR_LATENCY,
		(0b1     << FLASH_ACR_PRFTEN_Pos  ) | // Enable the prefetch buffer
		(latency << FLASH_ACR_LATENCY_Pos ) |
		0);

#if PLL_SRC == PLL_SRC_NONE
# if ! USE_INTERNAL_CLOCK
	enable_sysclock();
	// Disable HSI
	// No point in doing this because it's re-enabled for stop mode anyway
	//CLEAR_BIT(RCC->CR, RCC_CR_HSION);
# endif // PLL_SRC_NONE, !USE_INTERNAL_CLOCK

#elif USE_STM32F1_PLL // !PLL_SRC_NONE
	MODIFY_BITS(RCC->CFGR, RCC_CFGR_PLLMULL|RCC_CFGR_PLLXTPRE|RCC_CFGR_PLLSRC,
		((PLL_MUL-2) << RCC_CFGR_PLLMULL_Pos) | // Multiply PLL source clock by this
		                                       // The bits for a multiplier are that multiplier - 2
# if PLL_SRC == PLL_SRC_HSE
		(0b1 << RCC_CFGR_PLLSRC_Pos)   | // Use HSE as PLL source
		(0b0 << RCC_CFGR_PLLXTPRE_Pos) | // Don't divide HSE clock
# elif PLL_SRC == PLL_SRC_HSE_DIV2
		(0b1 << RCC_CFGR_PLLSRC_Pos)   | // Use HSE as PLL source
		(0b1 << RCC_CFGR_PLLXTPRE_Pos) | // Divide HSE clock by 2
# elif PLL_SRC == PLL_SRC_HSI_DIV2
		(0b0 << RCC_CFGR_PLLSRC_Pos)   | // Use HSI divided by 2 as PLL source
# else
#  error "PLL_SRC not selected or incorrect"
# endif
		0);
	enable_sysclock();

#else // !USE_STM32F1_PLL, !PLL_SRC_NONE
	MODIFY_BITS(RCC->PLLCFGR, RCC_PLLCFGR_PLLSRC|RCC_PLLCFGR_PLLM|RCC_PLLCFGR_PLLN|RCC_PLLCFGR_PLLP,
# if PLL_SRC == PLL_SRC_HSE
		(0b1 << RCC_PLLCFGR_PLLSRC_Pos) | // Use HSE as PLL source
# elif PLL_SRC == PLL_SRC_HSI
		(0b0 << RCC_PLLCFGR_PLLSRC_Pos) | // Use HSI as PLL source
# else
#  error "PLL_SRC not selected or incorrect"
# endif
		(PLLM << RCC_PLLCFGR_PLLM_Pos) | // Set PLLM prescaler
		(PLLN << RCC_PLLCFGR_PLLN_Pos) | // Set PLLN prescaler
		(PLLP << RCC_PLLCFGR_PLLP_Pos) | // set PLLP prescaler
		0);
	enable_sysclock();
#endif // !USE_STM32F1_PLL, !PLL_SRC_NONE
#if USE_INTERNAL_CLOCK
	// Disable HSE
	CLEAR_BIT(RCC->CR, RCC_CR_HSEON);
#endif

	reg = 0;
#if G_freq_HCLK == FREQ_OSC_HZ
	reg |= HCLK_PRESCALER_1;
#elif G_freq_HCLK == (FREQ_OSC_HZ / 2)
	reg |= HCLK_PRESCALER_2;
#elif G_freq_HCLK == (FREQ_OSC_HZ / 4)
	reg |= HCLK_PRESCALER_4;
#elif G_freq_HCLK == (FREQ_OSC_HZ / 8)
	reg |= HCLK_PRESCALER_8;
#elif G_freq_HCLK == (FREQ_OSC_HZ / 16)
	reg |= HCLK_PRESCALER_16;
#elif G_freq_HCLK == (FREQ_OSC_HZ / 64)
	reg |= HCLK_PRESCALER_64;
#elif G_freq_HCLK == (FREQ_OSC_HZ / 128)
	reg |= HCLK_PRESCALER_128;
#elif G_freq_HCLK == (FREQ_OSC_HZ / 256)
	reg |= HCLK_PRESCALER_256;
#elif G_freq_HCLK == (FREQ_OSC_HZ / 512)
	reg |= HCLK_PRESCALER_512;
#else
# error "G_freq_HCLK must be a FREQ_OSC_HZ / (1|2|4|8|16|64|128|256|512)"
#endif

#if G_freq_PCLK1 == G_freq_HCLK
	reg |= PCLK1_PRESCALER_1;
#elif G_freq_PCLK1 == (G_freq_HCLK / 2)
	reg |= PCLK1_PRESCALER_2;
#elif G_freq_PCLK1 == (G_freq_HCLK / 4)
	reg |= PCLK1_PRESCALER_4;
#elif G_freq_PCLK1 == (G_freq_HCLK / 8)
	reg |= PCLK1_PRESCALER_8;
#elif G_freq_PCLK1 == (G_freq_HCLK / 16)
	reg |= PCLK1_PRESCALER_16;
#else
# error "G_freq_PCLK1 must be a G_freq_HCLK / (1|2|4|8|16)"
#endif

#if G_freq_PCLK2 == G_freq_HCLK
	reg |= PCLK2_PRESCALER_1;
#elif G_freq_PCLK2 == (G_freq_HCLK / 2)
	reg |= PCLK2_PRESCALER_2;
#elif G_freq_PCLK2 == (G_freq_HCLK / 4)
	reg |= PCLK2_PRESCALER_4;
#elif G_freq_PCLK2 == (G_freq_HCLK / 8)
	reg |= PCLK2_PRESCALER_8;
#elif G_freq_PCLK2 == (G_freq_HCLK / 16)
	reg |= PCLK2_PRESCALER_16;
#else
# error "G_freq_PCLK2 must be a G_freq_HCLK / (1|2|4|8|16)"
#endif

	MODIFY_BITS(RCC->CFGR, RCC_CFGR_HPRE|RCC_CFGR_PPRE1|RCC_CFGR_PPRE2,
		reg
		);

	// We don't use the LSI for anything
	CLEAR_BIT(RCC->CSR, RCC_CSR_LSION);

	// The LSE is configured in the backup domain so enable the power interface
	// clock and the backup domain interface clock; keep it on afterwards
	// because RTC access requires them too
	clock_enable(RCC_PERIPH_PWR);
#if RCC_PERIPH_BKP
	clock_enable(RCC_PERIPH_BKP);
#endif

	BD_write_enable();
	SET_BIT(RCC->BDCR, RCC_BDCR_LSEON);
	while (!BIT_IS_SET(RCC->BDCR, RCC_BDCR_LSERDY)) {
		// Nothing to do here
	}
	BD_write_disable();

	return;
}

OPTIMIZE_FUNCTION \
void sleep_ms(utime_t ms) {
	light_sleep_ms(ms, 0);

	return;
}
OPTIMIZE_FUNCTION \
void hibernate_s(utime_t s, uint8_t flags) {
	if (s == 0) {
		return;
	}

#if USE_SERIAL
	if (BIT_IS_SET(flags, CFG_ALLOW_INTERRUPTS)) {
		NVIC_ClearPendingIRQ(UARTx_IRQn);
		NVIC_EnableIRQ(UARTx_IRQn);
	}
#endif // USE_SERIAL
	if (BIT_IS_SET(flags, CFG_ALLOW_INTERRUPTS)) {
		enable_button_ISR();
	}

	// UART interrupts can't wake us up from deep sleep so sleep lightly for a
	// few seconds before entering deep sleep in order to detect them
	// The user will have to use the button to enter the serial terminal.
#if USE_TERMINAL
	if (button_wakeup && (BIT_IS_SET(flags, CFG_ALLOW_INTERRUPTS))) {
		if ((G_IRQs == 0) && (s > 0)) {
			uint32_t w;

			button_wakeup = false;
			w = (s > (LIGHT_SLEEP_SECONDS + MIN_DEEP_SLEEP_SECONDS)) ? LIGHT_SLEEP_SECONDS : s;
			// LOGGER("Just a %u second nap...zZz...", (uint )w);
			NOTIFY("Waiting %u seconds for UART input", (uint )w);
			light_sleep_ms(w * 1000, flags);
			s -= w;
		}
	}
#endif

	// TODO: Handle the case where the RTC isn't clocked by the LSE and the RTC
	// alarm therefore can't be used
	if ((G_IRQs == 0) && (s > 0)) {
		// LOGGER("Wake me in %u seconds...ZZz...Zzz...zzZZZZZzzzZz", (uint )sleep_s);
		LOGGER("Sleeping deeply %u seconds", (uint )s);
		deep_sleep_s(s, flags);
	}

	if (G_IRQs != 0) {
		LOGGER("Hibernation ending with G_IRQs at 0x%02X", (uint )G_IRQs);
	}

	if (BIT_IS_SET(flags, CFG_ALLOW_INTERRUPTS)) {
		disable_button_ISR();
	}
#if USE_SERIAL
	NVIC_DisableIRQ(UARTx_IRQn);
	NVIC_ClearPendingIRQ(UARTx_IRQn);
#endif // USE_SERIAL

	return;
}
OPTIMIZE_FUNCTION \
static void light_sleep_ms(utime_t ms, uint8_t flags) {
	uint32_t period;

	// The systick interrupt will wake us from sleep if left enabled
	suspend_systick();

	// Don't use deep sleep mode
	CLEAR_BIT(SCB->SCR, SCB_SCR_SLEEPDEEP_Msk);

	while (ms > 0) {
		if (ms < SLEEP_TIM_MAX_MS) {
			period = ms;
			ms = 0;
		} else {
			period = SLEEP_TIM_MAX_MS;
			ms -= SLEEP_TIM_MAX_MS;
		}

		set_sleep_alarm(period);
		while (BIT_IS_SET(RCC->APB1ENR, SLEEP_ALARM_CLOCKEN)) {
			// Wait for an interrupt
			__WFI();
			// If desired keep sleeping until the wakeup alarm triggers
			if (BIT_IS_SET(flags, CFG_ALLOW_INTERRUPTS)) {
				ms = 0;
				break;
			} else if (BIT_IS_SET(RCC->APB1ENR, SLEEP_ALARM_CLOCKEN)) {
				// Let the user know we can't do anything right now
				sysflash();
				sysflash();
			}
		}
		stop_sleep_alarm();
	}

	// Resume systick
	resume_systick();

	return;
}
OPTIMIZE_FUNCTION \
static void deep_sleep_s(utime_t s, uint8_t flags) {
	utime_t period;

	// UART can't do anything during deep sleep and there's a pulldown when it's
	// on, so turn it off until wakeup
	uart_off();

	// The systick interrupt will wake us from sleep if left enabled
	suspend_systick();

	// Use deep sleep mode
	SET_BIT(SCB->SCR, SCB_SCR_SLEEPDEEP_Msk);

	MODIFY_BITS(PWR->CR, PWR_CR_PDDS|PWR_CR_LPDS,
		(0b0 << PWR_CR_PDDS_Pos) | // Enter stop mode
		(0b1 << PWR_CR_LPDS_Pos) | // Use the low-power voltage regulator
		0);

	while (s > 0) {
		if (s < HIBERNATE_MAX_S) {
			period = s;
			s = 0;
		} else {
			period = HIBERNATE_MAX_S;
			s -= HIBERNATE_MAX_S;
		}

		set_RTC_alarm(period);
		while (BIT_IS_SET(EXTI->IMR, RTC_ALARM_EXTI_LINE)) {
			// Wait for an interrupt
			// The stop mode entry procedure will be ignored and program execution
			// continues if any of the EXTI interrupt pending flags, peripheral
			// interrupt pending flags, or RTC alarm flag are set.
			__WFI();

			// If desired keep sleeping until the wakeup alarm triggers
			if (BIT_IS_SET(flags, CFG_ALLOW_INTERRUPTS)) {
				s = 0;
				break;
			} else if (BIT_IS_SET(EXTI->IMR, RTC_ALARM_EXTI_LINE)) {
				// Let the user know we can't do anything right now
				sysflash();
				sysflash();
			}
		}
		stop_RTC_alarm();
	}

	// The SYSCLK is always HSI on wakeup
	enable_sysclock();

	// Resume systick
	resume_systick();

	// Clear wakeup flag by writing 1 to CWUF
	SET_BIT(PWR->CR, PWR_CR_CWUF);

	uart_on();

	return;
}

void clock_enable(rcc_periph_t periph_clock) {
	uint32_t mask;
	__IO uint32_t *reg, tmpreg;

	switch (SELECT_BITS(periph_clock, RCC_BUS_MASK)) {
	case RCC_BUS_AHB1:
#if defined(STM32F1)
		reg = &RCC->AHBENR;
#else
		reg = &RCC->AHB1ENR;
#endif
		break;
	case RCC_BUS_APB1:
		reg = &RCC->APB1ENR;
		break;
	case RCC_BUS_APB2:
		reg = &RCC->APB2ENR;
		break;
	default:
		return;
	}

	mask = MASK_BITS(periph_clock, RCC_BUS_MASK);
	SET_BIT(*reg, mask);

	// Delay after enabling clock; method copied from ST HAL
	tmpreg = SELECT_BITS(*reg, mask);
	while (SELECT_BITS(*reg, mask) != mask) {
		// Nothing to do here
	}
	tmpreg = tmpreg; // Shut the compiler up

	return;
}
void clock_disable(rcc_periph_t periph_clock) {
	uint32_t mask;
	__IO uint32_t *reg;

	switch (SELECT_BITS(periph_clock, RCC_BUS_MASK)) {
	case RCC_BUS_AHB1:
#if defined(STM32F1)
		reg = &RCC->AHBENR;
#else
		reg = &RCC->AHB1ENR;
#endif
		break;
	case RCC_BUS_APB1:
		reg = &RCC->APB1ENR;
		break;
	case RCC_BUS_APB2:
		reg = &RCC->APB2ENR;
		break;
	default:
		return;
	}

	mask = MASK_BITS(periph_clock, RCC_BUS_MASK);
	CLEAR_BIT(*reg, mask);
	while (SELECT_BITS(*reg, mask) != 0) {
		// Nothing to do here
	}

	return;
}
void clock_init(rcc_periph_t periph_clock) {
	uint32_t mask;
	__IO uint32_t *reg;

	clock_enable(periph_clock);

	switch (SELECT_BITS(periph_clock, RCC_BUS_MASK)) {
	// The STM32F1 series doesn't have a reset register for the AHB
#if ! defined(STM32F1)
	case RCC_BUS_AHB1:
		reg = &RCC->AHB1RSTR;
		break;
#endif
	case RCC_BUS_APB1:
		reg = &RCC->APB1RSTR;
		break;
	case RCC_BUS_APB2:
		reg = &RCC->APB2RSTR;
		break;
	default:
		return;
	}

	mask = MASK_BITS(periph_clock, RCC_BUS_MASK);
	SET_BIT(*reg, mask);
	while (SELECT_BITS(*reg, mask) != mask) {
		// Nothing to do here
	}
	CLEAR_BIT(*reg, mask);
	while (SELECT_BITS(*reg, mask) != 0) {
		// Nothing to do here
	}

	return;
}

void BD_write_enable(void) {
	if (bd_write_enabled == 0) {
		SET_BIT(PWR->CR, PWR_CR_DBP);
		while (!BIT_IS_SET(PWR->CR, PWR_CR_DBP)) {
			// Nothing to do here
		}
	}
	++bd_write_enabled;

	return;
}
void BD_write_disable(void) {
	if (bd_write_enabled != 0) {
		--bd_write_enabled;
	}
	if (bd_write_enabled == 0) {
		// Per the reference manual, backup domain write protection must remain
		// disabled if using HSE/128 as the RTC clock
		if (SELECT_BITS(RCC->BDCR, RCC_BDCR_RTCSEL) != RCC_BDCR_RTCSEL_HSE) {
			CLEAR_BIT(PWR->CR, PWR_CR_DBP);
			while (BIT_IS_SET(PWR->CR, PWR_CR_DBP)) {
				// Nothing to do here
			}
		}
	}

	return;
}


#ifdef __cplusplus
 }
#endif
