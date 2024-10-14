// SPDX-License-Identifier: GPL-3.0-only
/***********************************************************************
*                                                                      *
*                                                                      *
* Copyright 2021, 2024 svijsv                                          *
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
#include "system.h"
#include "adc.h"
#include "gpio.h"
#include "spi.h"
#include "time.h"
#include "uart.h"
#include "i2c.h"


void pre_hibernate_hook_caller(utime_t *s, sleep_mode_t *sleep_mode, uHAL_flags_t flags);
void post_hibernate_hook_caller(utime_t s, sleep_mode_t sleep_mode, uHAL_flags_t flags);


DEBUG_CPP_MACRO(G_freq_CORE)
DEBUG_CPP_MACRO(G_freq_HCLK)
DEBUG_CPP_MACRO(G_freq_PCLK1)
DEBUG_CPP_MACRO(G_freq_PCLK2)

//
// Duration of system LED flashes
#define SYSFLASH_MS (500U)
#define SYSFLASH_BUF_MS (200U)
#define SYSALERT_MS (SYSFLASH_MS*2U)

//
// Determine main clock source and configuration
// Fair warning, this is a long one
//
// When all done, the following macros are defined:
//    FREQ_INPUT_HZ : The frequency of the input oscillator (HSE or HSI)
//    FREQ_OUTPUT_HZ: The frequency of the system clock
//    FREQ_OSC_HZ   : The frequency of the system oscillator (HSE, HSI, or PLL)
//
// If the PLL is used the following are also defined:
//    PLL_SRC: The source of the PLL clock (PLL_SRC_{HSI,HSE}[_DIV2])
//    PLL_MUL (F1 only) : The value by which PLL_SRC is multiplied
//    PLL[MNP] (non-F1 only): The values of the corresponding RCC_PLLCFGR fields
//
#define PLL_SRC_NONE     0
#define PLL_SRC_HSE      1
#define PLL_SRC_HSE_DIV2 2
#define PLL_SRC_HSI      3
#define PLL_SRC_HSI_DIV2 4
#if uHAL_USE_INTERNAL_OSC
# define FREQ_INPUT_HZ G_freq_HSI
#else
# define FREQ_INPUT_HZ G_freq_HSE
#endif
#define FREQ_OUTPUT_HZ G_freq_CORE
#define CAN_USE_OSC_CLOCK(_test, _osc) ((_test) == (_osc))

#if CAN_USE_OSC_CLOCK(FREQ_OUTPUT_HZ, FREQ_INPUT_HZ)
# define PLL_SRC PLL_SRC_NONE
# define FREQ_OSC_HZ FREQ_INPUT_HZ
  DEBUG_CPP_MACRO(PLL_SRC)
  DEBUG_CPP_MACRO(FREQ_OSC_HZ)
  DEBUG_CPP_MACRO(FREQ_OUTPUT_HZ)

#elif HAVE_STM32F1_PLL
  // HSI source is always divided by 2
# if ! uHAL_USE_INTERNAL_OSC
#  define PLL_TEST_OSC FREQ_INPUT_HZ
#  include "system_PLL_STM32F1.h"
# endif
# ifdef PLL_MUL
#  define PLL_SRC PLL_SRC_HSE
#  define FREQ_OSC_HZ (FREQ_INPUT_HZ * PLL_MUL)
# else
#  undef PLL_TEST_OSC
#  define PLL_TEST_OSC (FREQ_INPUT_HZ/2U)
#  include "system_PLL_STM32F1.h"
#  ifdef PLL_MUL
#   if uHAL_USE_INTERNAL_OSC
#    define PLL_SRC PLL_SRC_HSI_DIV2
#   else
#    define PLL_SRC PLL_SRC_HSE_DIV2
#   endif
#   define FREQ_OSC_HZ ((FREQ_INPUT_HZ/2U) * PLL_MUL)
#  else
#   error "Unsupported main clock frequency"
#  endif
# endif
  DEBUG_CPP_MACRO(PLL_SRC)
  DEBUG_CPP_MACRO(PLL_MUL)
  DEBUG_CPP_MACRO(FREQ_OSC_HZ)
  DEBUG_CPP_MACRO(FREQ_INPUT_HZ)
  DEBUG_CPP_MACRO(FREQ_OUTPUT_HZ)

// The other STM32 lines are more difficult to auto-configure than the F1s
// because they have 3 different prescalers affecting the main clock (and
// two more for other clocks but we don't use those) and two of those have
// far wider value ranges than the F1's so they can't just be listed out
// here. Instead they're determined mostly in a script-generated external
// header that *does* list them all out.
#else // HAVE_STM32F1_PLL
# if uHAL_USE_INTERNAL_OSC
#  define PLL_SRC PLL_SRC_HSI
# else
#  define PLL_SRC PLL_SRC_HSE
# endif
# define PLLP_DIV_2 0b00U
# define PLLP_DIV_4 0b01U
# define PLLP_DIV_6 0b10U
# define PLLP_DIV_8 0b11U

// The PLL_* values are defined in the device-specific platform header
# define PLLM_MIN (PLL_PLLM_MIN)
# define PLLM_MAX (PLL_PLLM_MAX)
# define VCO_INPUT_MIN_HZ (PLL_VCO_INPUT_MIN_HZ)
# define VCO_INPUT_MAX_HZ (PLL_VCO_INPUT_MAX_HZ)
# define PLLN_MIN (PLL_PLLN_MIN)
# define PLLN_MAX (PLL_PLLN_MAX)
# define VCO_OUTPUT_MIN_HZ (PLL_VCO_OUTPUT_MIN_HZ)
# define VCO_OUTPUT_MAX_HZ (PLL_VCO_OUTPUT_MAX_HZ)

  // All the magic happens in here
# include "system_PLL_STM32Fx.h"
# if ! defined(PLLP)
#  error "Unsupported main clock frequency"
# endif
//# define FREQ_OSC_HZ (((FREQ_INPUT_HZ / PLLM) * PLLN) / PLLP_DIV)
# define FREQ_OSC_HZ PLL_OUTPUT_HZ
  DEBUG_CPP_MACRO(PLL_SRC)
  DEBUG_CPP_MACRO(PLLM)
  DEBUG_CPP_MACRO(PLLN)
  DEBUG_CPP_MACRO(PLLP)
  DEBUG_CPP_MACRO(PLLP_DIV)
  DEBUG_CPP_MACRO(VCO_INPUT_HZ)
  DEBUG_CPP_MACRO(VCO_OUTPUT_HZ)
  DEBUG_CPP_MACRO(FREQ_OSC_HZ)
  DEBUG_CPP_MACRO(FREQ_INPUT_HZ)
  DEBUG_CPP_MACRO(FREQ_OUTPUT_HZ)

#endif // HAVE_STM32F1_PLL
#undef CAN_USE_OSC_CLOCK

#if PLL_SRC == PLL_SRC_NONE
# if uHAL_USE_INTERNAL_OSC
#  define SYSCLOCKON  RCC_CR_HSION
#  define SYSCLOCKSW  RCC_CFGR_SW_HSI
#  define SYSCLOCKRDY RCC_CR_HSIRDY
# else
#  define SYSCLOCKON  RCC_CR_HSEON
#  define SYSCLOCKSW  RCC_CFGR_SW_HSE
#  define SYSCLOCKRDY RCC_CR_HSERDY
# endif
#else
# define SYSCLOCKON  RCC_CR_PLLON
# define SYSCLOCKSW  RCC_CFGR_SW_PLL
# define SYSCLOCKRDY RCC_CR_PLLRDY
#endif
// SW is used to set the clock source, SWS is used to check the clock source
#if RCC_CFGR_SW_Pos < RCC_CFGR_SWS_Pos
//# define SYSCLOCKSWS (SYSCLOCKSW << (RCC_CFGR_SWS_Pos - RCC_CFGR_SW_Pos))
# define SYSCLOCK_SWS_SHIFT(_sw_) ((_sw_) << (RCC_CFGR_SWS_Pos - RCC_CFGR_SW_Pos))
#else
//# define SYSCLOCKSWS (SYSCLOCKSW >> (RCC_CFGR_SW_Pos - RCC_CFGR_SWS_Pos))
# define SYSCLOCK_SWS_SHIFT(_sw_) ((_sw_) >> (RCC_CFGR_SW_Pos - RCC_CFGR_SWS_Pos))
#endif


#define IRQ_IS_REQUESTED ((uHAL_CHECK_STATUS(uHAL_FLAG_IRQ)))
#define IRQ_IS_WAITING(_flags_) (BIT_IS_SET(_flags_, uHAL_CFG_ALLOW_INTERRUPTS) && IRQ_IS_REQUESTED)


static uint_fast8_t bd_write_enabled = 0;


static void clocks_init(void);

void platform_reset(void) {
	pre_reset_hook();
	NVIC_SystemReset();
}

void platform_init(void) {
	clocks_init();

	// The AFIO clock is needed to fix a few problems and remap the inputs, so
	// enable it here and disable at the end of setup when it's no longer needed
	clock_init(EXTI_PREG_CLOCKEN);

	gpio_init();

	time_init();

#if uHAL_USE_UART_COMM
	const uart_port_cfg_t uart_cfg = {
		.rx_pin = UART_COMM_RX_PIN,
		.tx_pin = UART_COMM_TX_PIN,
		.baud_rate = UART_COMM_BAUDRATE,
	};

	if (uart_init_port(UART_COMM_PORT, &uart_cfg) == ERR_OK) {
		uart_on(UART_COMM_PORT);
		serial_init();
	} else {
		error_state_crude();
	}
#endif

#if uHAL_USE_SPI
	spi_init();
#endif

#if uHAL_USE_I2C
	i2c_init();
#endif

#if uHAL_USE_ADC
	adc_init();
#endif

	// Set interrupt priority grouping
	// Use 4 bits for group priority and 0 bits for subpriority
	// See 4.4.5 (SCB_AIRCR register) of the Cortex M3 programming manual for
	// how to determine the value passed, it looks to be '3 + number of bits
	// for subpriority'. Cortex M4 is the same.
	NVIC_SetPriorityGrouping(0b011U + 0U);

	clock_disable(EXTI_PREG_CLOCKEN);

	return;
}

static void set_sysclock_src(uint32_t rcc_cfgr_sw, uint32_t rcc_cr_on, uint32_t rcc_cr_rdy) {
	uint32_t sws;

	assert((rcc_cfgr_sw == RCC_CFGR_SW_HSI) || (rcc_cfgr_sw == RCC_CFGR_SW_HSE) || (rcc_cfgr_sw == RCC_CFGR_SW_PLL));
	assert((rcc_cr_on == RCC_CR_HSION) || (rcc_cr_on == RCC_CR_HSEON) || (rcc_cr_on == RCC_CR_PLLON));
	assert((rcc_cr_rdy == RCC_CR_HSIRDY) || (rcc_cr_rdy == RCC_CR_HSERDY) || (rcc_cr_rdy == RCC_CR_PLLRDY));

	// Turn the new clock on
	SET_BIT(RCC->CR, rcc_cr_on);
	// Wait for the clock to come up
	while (!BIT_IS_SET(RCC->CR, rcc_cr_rdy)) {
		// Nothing to do here
	}

	// Set the system clock
	MODIFY_BITS(RCC->CFGR, RCC_CFGR_SW, rcc_cfgr_sw);
	// Wait for the system clock to switch over
	sws = SYSCLOCK_SWS_SHIFT(rcc_cfgr_sw);
	while (SELECT_BITS(RCC->CFGR, RCC_CFGR_SWS) != sws) {
		// Nothing to do here
	}

	return;
}
static void enable_sysclock(void) {
# if PLL_SRC == PLL_SRC_HSE || PLL_SRC == PLL_SRC_HSE_DIV2
	SET_BIT(RCC->CR, RCC_CR_HSEON);
	while (!BIT_IS_SET(RCC->CR, RCC_CR_HSERDY)) {
		// Nothing to do here
	}
# endif

	set_sysclock_src(SYSCLOCKSW, SYSCLOCKON, SYSCLOCKRDY);

	return;
}
static void clocks_init(void) {
	uint32_t reg, latency;

	// Don't use clock source protection
	CLEAR_BIT(RCC->CR, RCC_CR_CSSON);

	// Always start out with the HSI to guard against problems setting the PLL
	// if it's already (somehow) enabled
	set_sysclock_src(RCC_CFGR_SW_HSI, RCC_CR_HSION, RCC_CR_HSIRDY);

	// Make sure PLL is off for configuration
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
		(0b1U    << FLASH_ACR_PRFTEN_Pos  ) | // Enable the prefetch buffer
		(latency << FLASH_ACR_LATENCY_Pos ) |
		0);

#if PLL_SRC == PLL_SRC_NONE
# if ! uHAL_USE_INTERNAL_OSC
	enable_sysclock();
	// Disable HSI
	// No point in doing this because it's re-enabled for stop mode anyway
	//CLEAR_BIT(RCC->CR, RCC_CR_HSION);
# elif uHAL_HSICAL_TRIM
	// The default value of HSITRIM is always the middle of the possible range,
	// but the range varies between device lines
	//uint32_t trim = (((RCC_CR_HSITRIM_Msk+1U)/2U) + uHAL_HSI_TRIM) << RCC_CR_HSITRIM_Pos;

	MODIFY_BITS(RCC->CR, RCC_CR_HSITRIM_Msk, ((uint32_t )uHAL_HSI_TRIM << RCC_CR_HSITRIM_Pos));
# endif

#elif HAVE_STM32F1_PLL // !PLL_SRC_NONE
	MODIFY_BITS(RCC->CFGR, RCC_CFGR_PLLMULL|RCC_CFGR_PLLXTPRE|RCC_CFGR_PLLSRC,
		((PLL_MUL-2U) << RCC_CFGR_PLLMULL_Pos) | // Multiply PLL source clock by this
		                                         // The bits for a multiplier are that multiplier - 2
# if PLL_SRC == PLL_SRC_HSE
		(0b1U << RCC_CFGR_PLLSRC_Pos)   | // Use HSE as PLL source
		(0b0U << RCC_CFGR_PLLXTPRE_Pos) | // Don't divide HSE clock
# elif PLL_SRC == PLL_SRC_HSE_DIV2
		(0b1U << RCC_CFGR_PLLSRC_Pos)   | // Use HSE as PLL source
		(0b1U << RCC_CFGR_PLLXTPRE_Pos) | // Divide HSE clock by 2
# elif PLL_SRC == PLL_SRC_HSI_DIV2
		(0b0U << RCC_CFGR_PLLSRC_Pos)   | // Use HSI divided by 2 as PLL source
# else
#  error "PLL_SRC not selected or incorrect"
# endif
		0);
	enable_sysclock();

#else // !HAVE_STM32F1_PLL, !PLL_SRC_NONE
	MODIFY_BITS(RCC->PLLCFGR, RCC_PLLCFGR_PLLSRC|RCC_PLLCFGR_PLLM|RCC_PLLCFGR_PLLN|RCC_PLLCFGR_PLLP,
# if PLL_SRC == PLL_SRC_HSE
		(0b1U << RCC_PLLCFGR_PLLSRC_Pos) | // Use HSE as PLL source
# elif PLL_SRC == PLL_SRC_HSI
		(0b0U << RCC_PLLCFGR_PLLSRC_Pos) | // Use HSI as PLL source
# else
#  error "PLL_SRC not selected or incorrect"
# endif
		(PLLM << RCC_PLLCFGR_PLLM_Pos) | // Set PLLM prescaler
		(PLLN << RCC_PLLCFGR_PLLN_Pos) | // Set PLLN prescaler
		(PLLP << RCC_PLLCFGR_PLLP_Pos) | // set PLLP prescaler
		0);
	enable_sysclock();
#endif // !HAVE_STM32F1_PLL, !PLL_SRC_NONE
#if uHAL_USE_INTERNAL_OSC
	// Disable HSE (if on) to save power
	CLEAR_BIT(RCC->CR, RCC_CR_HSEON);
#endif

	reg = 0;
#if G_freq_HCLK == G_freq_CORE
	reg |= RCC_CFGR_HPRE_DIV1;
#elif (G_freq_HCLK * 2) == G_freq_CORE
	reg |= RCC_CFGR_HPRE_DIV2;
#elif (G_freq_HCLK * 4) == G_freq_CORE
	reg |= RCC_CFGR_HPRE_DIV4;
#elif (G_freq_HCLK * 8) == G_freq_CORE
	reg |= RCC_CFGR_HPRE_DIV8;
#elif (G_freq_HCLK * 16) == G_freq_CORE
	reg |= RCC_CFGR_HPRE_DIV16;
#elif (G_freq_HCLK * 64) == G_freq_CORE
	reg |= RCC_CFGR_HPRE_DIV64;
#elif (G_freq_HCLK * 128) == G_freq_CORE
	reg |= RCC_CFGR_HPRE_DIV128;
#elif (G_freq_HCLK * 256) == G_freq_CORE
	reg |= RCC_CFGR_HPRE_DIV256;
#elif (G_freq_HCLK * 512) == G_freq_CORE
	reg |= RCC_CFGR_HPRE_DIV512;
#else
# error "F_HCLK must be F_CORE / (1|2|4|8|16|64|128|256|512)"
#endif

#if G_freq_PCLK1 == G_freq_HCLK
	reg |= RCC_CFGR_PPRE1_DIV1;
#elif (G_freq_PCLK1 * 2) == G_freq_HCLK
	reg |= RCC_CFGR_PPRE1_DIV2;
#elif (G_freq_PCLK1 * 4) == G_freq_HCLK
	reg |= RCC_CFGR_PPRE1_DIV4;
#elif (G_freq_PCLK1 * 8) == G_freq_HCLK
	reg |= RCC_CFGR_PPRE1_DIV8;
#elif (G_freq_PCLK1 * 16) == G_freq_HCLK
	reg |= RCC_CFGR_PPRE1_DIV16;
#else
# error "F_PCLK1 must be F_HCLK / (1|2|4|8|16)"
#endif

#if G_freq_PCLK2 == G_freq_HCLK
	reg |= RCC_CFGR_PPRE2_DIV1;
#elif (G_freq_PCLK2 * 2) == G_freq_HCLK
	reg |= RCC_CFGR_PPRE2_DIV2;
#elif (G_freq_PCLK2 * 4) == G_freq_HCLK
	reg |= RCC_CFGR_PPRE2_DIV4;
#elif (G_freq_PCLK2 * 8) == G_freq_HCLK
	reg |= RCC_CFGR_PPRE2_DIV8;
#elif (G_freq_PCLK2 * 16) == G_freq_HCLK
	reg |= RCC_CFGR_PPRE2_DIV16;
#else
# error "F_PCLK2 must be F_HCLK / (1|2|4|8|16)"
#endif

	MODIFY_BITS(RCC->CFGR, RCC_CFGR_HPRE|RCC_CFGR_PPRE1|RCC_CFGR_PPRE2,
		reg
		);

	// Turn on the power interface clock and the backup domain interface
	// clock to allow access to the RTC
	clock_enable(RCC_PERIPH_PWR);
# if defined(RCC_PERIPH_BKP) && RCC_PERIPH_BKP > 0
	clock_enable(RCC_PERIPH_BKP);
# endif

#if uHAL_BACKUP_DOMAIN_RESET
	BD_write_enable();
	SET_BIT(RCC->BDCR, RCC_BDCR_BDRST);
	while (!BIT_IS_SET(RCC->BDCR, RCC_BDCR_BDRST)) {
		// Nothing to do here
	}
	CLEAR_BIT(RCC->BDCR, RCC_BDCR_BDRST);
	while (BIT_IS_SET(RCC->BDCR, RCC_BDCR_BDRST)) {
		// Nothing to do here
	}
	BD_write_disable();
#endif

#if uHAL_USE_INTERNAL_LS_OSC
	SET_BIT(RCC->CSR, RCC_CSR_LSION);
	while (!BIT_IS_SET(RCC->CSR, RCC_CSR_LSIRDY)) {
		// Nothing to do here
	}

#else // uHAL_USE_INTERNAL_LS_OSC
	CLEAR_BIT(RCC->CSR, RCC_CSR_LSION);

	// The LSE is configured in the backup domain so enable the power interface
	// clock and the backup domain interface clock; keep it on afterwards
	// because RTC access requires them too
	/*
	clock_enable(RCC_PERIPH_PWR);
# if RCC_PERIPH_BKP
	clock_enable(RCC_PERIPH_BKP);
# endif
	*/

	BD_write_enable();
	SET_BIT(RCC->BDCR, RCC_BDCR_LSEON);
	while (!BIT_IS_SET(RCC->BDCR, RCC_BDCR_LSERDY)) {
		// Nothing to do here
	}
	BD_write_disable();
#endif // uHAL_USE_INTERNAL_LS_OSC

	return;
}

#if uHAL_USE_HIBERNATE
static void light_sleep_ms(utime_t ms, uint_fast8_t flags, uint_t *wakeups) {
	uint32_t period;
	uint_t wu = 0;

	if (IRQ_IS_WAITING(flags)) {
		return;
	}

	// The systick interrupt will wake us from sleep if left enabled
	disable_systick();

	// Don't use deep sleep mode
	CLEAR_BIT(SCB->SCR, SCB_SCR_SLEEPDEEP_Msk);

	while (ms > 0) {
		period = set_sleep_alarm(ms);
#if ! uHAL_SKIP_OTHER_CHECKS
		if (period == 0) {
			// This isn't accurate, but we don't have a systick for delay_ms()
			dumb_delay_ms(ms);
			ms = 0;
			break;
		}
#endif
		ms = (ms > period) ? (ms - period) : 0;

		wu = 0;
		while (sleep_alarm_is_set() && !IRQ_IS_WAITING(flags)) {
			// Wait for an interrupt
			__WFI();

			// If required keep sleeping until the wakeup alarm triggers
			if (sleep_alarm_is_set()) {
				++wu;
				if (IRQ_IS_WAITING(flags)) {
					ms = 0;
					break;
				}
			}
		}
		stop_sleep_alarm();
	}

	// Resume systick
	enable_systick();

	if (wakeups != NULL) {
		*wakeups = wu;
	}

	return;
}

static void deep_sleep_s(utime_t s, sleep_mode_t sleep_mode, uint_fast8_t flags, uint_t *wakeups) {
	utime_t period;
	uint_t wu = 0;
	uint32_t pwr_cr = 0;

	switch (sleep_mode) {
	case HIBERNATE_LIGHT:
		CLEAR_BIT(SCB->SCR, SCB_SCR_SLEEPDEEP_Msk);
		break;
	case HIBERNATE_MAX:
		SET_BIT(pwr_cr, 1U << PWR_CR_PDDS_Pos);
		//fallthrough
	default:
	//case HIBERNATE_DEEP:
		SET_BIT(pwr_cr, PWR_CR_SLOW_WAKE_BITS);
		SET_BIT(SCB->SCR, SCB_SCR_SLEEPDEEP_Msk);
		MODIFY_BITS(PWR->CR, PWR_CR_PDDS|PWR_CR_SLOW_WAKE_MASK, pwr_cr);
		break;
	}

	// The systick interrupt will wake us from sleep if left enabled
	disable_systick();

	while (s > 0) {
		if (s < HIBERNATE_MAX_S) {
			period = s;
			s = 0;
		} else {
			period = HIBERNATE_MAX_S;
			s -= HIBERNATE_MAX_S;
		}

		set_RTC_alarm(period);
		while (RTC_alarm_is_set() && !IRQ_IS_WAITING(flags)) {
			// Wait for an interrupt
			// The stop mode entry procedure will be ignored and program execution
			// continues if any of the EXTI interrupt pending flags, peripheral
			// interrupt pending flags, or RTC alarm flag are set.
			__WFI();

			// If required keep sleeping until the wakeup alarm triggers
			if (RTC_alarm_is_set()) {
				++wu;
				if (IRQ_IS_WAITING(flags)) {
					s = 0;
					break;
				}
			}
		}
		stop_RTC_alarm();
	}

	if (sleep_mode != HIBERNATE_LIGHT) {
		// The SYSCLK is always HSI on wakeup from stop mode
		enable_sysclock();
	}

	// Resume systick
	enable_systick();

	// Clear wakeup flag by writing 1 to CWUF
	SET_BIT(PWR->CR, PWR_CR_CWUF);

	if (wakeups != NULL) {
		*wakeups = wu;
	}

	return;
}
void sleep_ms(utime_t ms) {
	light_sleep_ms(ms, 0, NULL);

	return;
}
void hibernate_s(utime_t s, sleep_mode_t sleep_mode, uHAL_flags_t flags) {
	uint_t wu = 0;

	pre_hibernate_hook_caller(&s, &sleep_mode, flags);
	const utime_t begin_s = s;

	if (uHAL_CHECK_STATUS(uHAL_FLAG_INHIBIT_HIBERNATION)) {
		sleep_mode = HIBERNATE_LIGHT;
	}
#if uHAL_HIBERNATE_LIMIT
	if (sleep_mode > uHAL_HIBERNATE_LIMIT) {
		sleep_mode = uHAL_HIBERNATE_LIMIT;
	}
#endif

	if (s == 0) {
		return;
	}

	if (!IRQ_IS_WAITING(flags) && (s > 0)) {
		deep_sleep_s(s, sleep_mode, flags, &wu);
	}

	if (IRQ_IS_REQUESTED) {
		LOGGER("Hibernation ending with uHAL_status at 0x%02X", (uint_t )uHAL_status);
	}
	if (wu > 1) {
		LOGGER("Hibernation was interrupted %u times", (uint_t )wu);
	}

	post_hibernate_hook_caller(begin_s, sleep_mode, flags);

	return;
}
#endif // uHAL_USE_HIBERNATE

void hibernate(sleep_mode_t sleep_mode, uHAL_flags_t flags) {
	uint32_t pwr_cr = 0;

	pre_hibernate_hook_caller(NULL, &sleep_mode, flags);

	if (uHAL_CHECK_STATUS(uHAL_FLAG_INHIBIT_HIBERNATION)) {
		sleep_mode = HIBERNATE_LIGHT;
	}
#if uHAL_HIBERNATE_LIMIT
	if (sleep_mode > uHAL_HIBERNATE_LIMIT) {
		sleep_mode = uHAL_HIBERNATE_LIMIT;
	}
#endif

	switch (sleep_mode) {
	case HIBERNATE_LIGHT:
		CLEAR_BIT(SCB->SCR, SCB_SCR_SLEEPDEEP_Msk);
		break;
	case HIBERNATE_MAX:
		SET_BIT(pwr_cr, 1U << PWR_CR_PDDS_Pos);
		//fallthrough
	default:
	//case HIBERNATE_DEEP:
		SET_BIT(pwr_cr, PWR_CR_SLOW_WAKE_BITS);
		SET_BIT(SCB->SCR, SCB_SCR_SLEEPDEEP_Msk);
		MODIFY_BITS(PWR->CR, PWR_CR_PDDS|PWR_CR_SLOW_WAKE_MASK, pwr_cr);
		break;
	}

	// The systick interrupt will wake us from sleep if left enabled
	disable_systick();

	// Wait for an interrupt
	// The stop mode entry procedure will be ignored and program execution
	// continues if any of the EXTI interrupt pending flags, peripheral
	// interrupt pending flags, or RTC alarm flag are set.
	__WFI();

	if (sleep_mode != HIBERNATE_LIGHT) {
		// The SYSCLK is always HSI on wakeup from stop mode
		enable_sysclock();
	}

	// Resume systick
	enable_systick();

	// Clear wakeup flag by writing 1 to CWUF
	SET_BIT(PWR->CR, PWR_CR_CWUF);

	post_hibernate_hook_caller(0, sleep_mode, flags);

	return;
}

bool clock_is_enabled(rcc_periph_t periph_clock) {
	uint32_t mask;
	uint32_t reg;

	switch (SELECT_BITS(periph_clock, RCC_BUS_MASK)) {
	case RCC_BUS_AHB1:
#if ! HAVE_AHB2
		reg = RCC->AHBENR;
#else
		reg = RCC->AHB1ENR;
#endif
		break;
	case RCC_BUS_APB1:
		reg = RCC->APB1ENR;
		break;
	case RCC_BUS_APB2:
		reg = RCC->APB2ENR;
		break;
	default:
		return false;
	}

	mask = MASK_BITS(periph_clock, RCC_BUS_MASK);

	return BITS_ARE_SET(reg, mask);
}
void clock_enable(rcc_periph_t periph_clock) {
	uint32_t mask;
	__IO uint32_t *reg, tmpreg;

	switch (SELECT_BITS(periph_clock, RCC_BUS_MASK)) {
	case RCC_BUS_AHB1:
#if ! HAVE_AHB2
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
#if ! HAVE_AHB2
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
#if HAVE_AHB_RESET
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
		// disabled if using HSE/128 as the RTC clock on the STM32F1 line
		// Edit: But we don't support HSE for the RTC
/*
#if HAVE_STM32F1_RTC
		if (SELECT_BITS(RCC->BDCR, RCC_BDCR_RTCSEL) != RCC_BDCR_RTCSEL_HSE) {
#else
		if (true) {
#endif
*/
		if (true) {
			CLEAR_BIT(PWR->CR, PWR_CR_DBP);
			while (BIT_IS_SET(PWR->CR, PWR_CR_DBP)) {
				// Nothing to do here
			}
		}
	}

	return;
}
