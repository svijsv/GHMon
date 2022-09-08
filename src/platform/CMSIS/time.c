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
// time.c
// Manage the time-keeping peripherals
// NOTES:
//    The timers are defined in time.h
//

#ifdef __cplusplus
 extern "C" {
#endif

//#define DEBUG 1
/*
* Includes
*/
#include "time.h"
#include "system.h"
#include "gpio.h"


#if DEBUG
# pragma message "SLEEP_TIM_MS_TICKS is " XTRINGIZE(SLEEP_TIM_MS_TICKS)
# pragma message "SLEEP_TIM_MAX_MS is " XTRINGIZE(SLEEP_TIM_MAX_MS)
#endif


/*
* Static values
*/
// Divide the number of cycles per ms by this in a dumb delay to account for
// overhead
#define DUMB_DELAY_DIV 7

// If any prescaler other than 1 is used on a timer's bus clock, the timer's
// frequency is doubled
#if G_freq_PCLK1 == G_freq_HCLK
# define TIM_APB1_MAX_FREQUENCY G_freq_PCLK1
#else
# define TIM_APB1_MAX_FREQUENCY (G_freq_PCLK1 * 2)
#endif
#if G_freq_PCLK2 == G_freq_HCLK
# define TIM_APB2_MAX_FREQUENCY G_freq_PCLK2
#else
# define TIM_APB2_MAX_FREQUENCY (G_freq_PCLK2 * 2)
#endif

// A PWM signal period is the total time it takes for the counter to go from
// 0 to PWM_DUTY_CYCLE_SCALE-1, therefore the frequency of the timer's counter
// is the signal frequency times PWM_DUTY_CYCLE_SCALE and the maximum supported
// frequency is the base clock divided by PWM_DUTY_CYCLE_SCALE
#define PWM_APB1_FREQUENCY (PWM_MAX_FREQUENCY * PWM_DUTY_CYCLE_SCALE)
#if PWM_APB1_FREQUENCY > (TIM_APB1_MAX_FREQUENCY / PWM_DUTY_CYCLE_SCALE)
# undef  PWM_APB1_FREQUENCY
# define PWM_APB1_FREQUENCY (TIM_APB1_MAX_FREQUENCY / PWM_DUTY_CYCLE_SCALE)
#endif
#define PWM_APB2_FREQUENCY (PWM_MAX_FREQUENCY * PWM_DUTY_CYCLE_SCALE)
#if PWM_APB2_FREQUENCY > (TIM_APB2_MAX_FREQUENCY / PWM_DUTY_CYCLE_SCALE)
# undef  PWM_APB2_FREQUENCY
# define PWM_APB2_FREQUENCY (TIM_APB2_MAX_FREQUENCY / PWM_DUTY_CYCLE_SCALE)
#endif


/*
* Types
*/


/*
* Variables
*/
// System tick count, milliseconds
volatile utime_t G_sys_msticks = 0;


/*
* Local function prototypes
*/
static uint16_t calculate_TIM_prescaler(rcc_periph_t tim, uint32_t hz);
static void systick_init(void);
// Defined in time_RTC_*.h
static void RTC_init(void);
static void timers_init(void);


/*
* Interrupt handlers
*/
OPTIMIZE_FUNCTION \
void SysTick_Handler(void) {
	++G_sys_msticks;
	return;
}
OPTIMIZE_FUNCTION \
void SleepAlarm_IRQHandler(void) {
	NVIC_DisableIRQ(SLEEP_ALARM_IRQn);
	NVIC_ClearPendingIRQ(SLEEP_ALARM_IRQn);

	//CLEAR_BIT(SLEEP_ALARM_TIM->SR, TIM_SR_UIF);
	SLEEP_ALARM_TIM->SR = 0;
	// Configured to disable itself
	//CLEAR_BIT(SLEEP_ALARM_TIM->CR1, TIM_CR1_CEN);
	clock_disable(SLEEP_ALARM_CLOCKEN);

	return;
}


/*
* Functions
*/
void time_init(void) {
	systick_init();
	RTC_init();
	timers_init();

	return;
}
//
// Manage the systick timer
static void systick_init(void) {
	uint32_t period;

	G_sys_msticks = 0;
	// G_sys_sticks = 0;

	// Re-implement SysTick_Config() from CMSIS/ARM/core_cm3.h so we can use
	// HCLK/8 as our clock source (but why?...)
	period = (G_freq_HCLK/8) / 1000;
	assert(period > 0);
	assert(SysTick_LOAD_RELOAD_Msk >= (period-1));

	SysTick->LOAD = (uint32_t )(period - 1);
	NVIC_SetPriority(SysTick_IRQn, SYSTICK_IRQp);
	SysTick->VAL = 0;
	MODIFY_BITS(SysTick->CTRL, SysTick_CTRL_CLKSOURCE_Msk|SysTick_CTRL_TICKINT_Msk|SysTick_CTRL_ENABLE_Msk,
		(0b0 << SysTick_CTRL_CLKSOURCE_Pos) | // Keep 0 for HCLCK/8; set to 1 for HCLK
		(0b1 << SysTick_CTRL_TICKINT_Pos  ) | // Enable the interrupt
		(0b1 << SysTick_CTRL_ENABLE_Pos   ) | // Enable the counter
		0);

	return;
}
void suspend_systick(void) {
	CLEAR_BIT(SysTick->CTRL, SysTick_CTRL_TICKINT_Msk|SysTick_CTRL_ENABLE_Msk);
	return;
}
void resume_systick(void) {
	SET_BIT(SysTick->CTRL, SysTick_CTRL_TICKINT_Msk|SysTick_CTRL_ENABLE_Msk);
	return;
}
//
// Manage the RTC
// This is handled in other files to avoid cluttering this one because the
// RTC differs so much between STM32 lines
#define INCLUDED_BY_TIME_C 1
#if USE_STM32F1_RTC
# include "time_RTC_STM32F1.c"
#else
# include "time_RTC_STM32Fx.c"
#endif
//
// Manage the standard timers
#define CONFIGURE_PWM_TIMER(TIMx) do { \
	(TIMx)->CR1 = cr1; \
	if (PWM_DUTY_CYCLE_SCALE <= 0xFFFF) { \
		(TIMx)->ARR = (PWM_DUTY_CYCLE_SCALE - 1); \
	} else { \
		(TIMx)->ARR = 0xFFFF; \
	} \
	(TIMx)->CCMR1 = ccmr; \
	(TIMx)->CCMR2 = ccmr; \
	/* Generate an update event to load the shadow registers */ \
	SET_BIT((TIMx)->EGR, TIM_EGR_UG); \
	/* Clear all event flags */ \
	(TIMx)->SR = 0; \
	} while (0);

static void timers_init(void) {
	rcc_periph_t apb1_enr = 0, apb2_enr = 0;
	uint16_t psc_apb1, psc_apb2, cr1, ccmr;

	// Timers 2-7 and 12-14 are on APB1
	// Timers 1 and 8-11 are on APB2
	// Not all timers are present on all hardware
#ifdef TIM2
	apb1_enr |= RCC_PERIPH_TIM2;
#endif
#ifdef TIM3
	apb1_enr |= RCC_PERIPH_TIM3;
#endif
#ifdef TIM4
	apb1_enr |= RCC_PERIPH_TIM4;
#endif

#ifdef TIM1
	apb2_enr |= RCC_PERIPH_TIM1;
#endif

#if SELECT_BITS(SLEEP_ALARM_CLOCKEN, RCC_BUS_MASK) == RCC_BUS_APB1
	apb1_enr |= SLEEP_ALARM_CLOCKEN;
#else
	apb2_enr |= SLEEP_ALARM_CLOCKEN;
#endif
#if SELECT_BITS(USCOUNTER_CLOCKEN, RCC_BUS_MASK) == RCC_BUS_APB1
	apb1_enr |= USCOUNTER_CLOCKEN;
#else
	apb2_enr |= USCOUNTER_CLOCKEN;
#endif

	clock_init(apb1_enr);
	clock_init(apb2_enr);

	//
	// PWM timers
	//
	psc_apb1 = calculate_TIM_prescaler(RCC_BUS_APB1, PWM_APB1_FREQUENCY);
	psc_apb2 = calculate_TIM_prescaler(RCC_BUS_APB2, PWM_APB2_FREQUENCY);
	// Auto-reload register buffer is suggested by the reference manual for
	// reliability in PWM mode
	cr1 = TIM_CR1_ARPE;
	// PWM mode 1 is high while TIMx_CNT < TIMx_CCRx
	// PWM mode 2 is high while TIMx_CNT > TIMx_CCRx
	ccmr =
		(0b00  << TIM_CCMR1_CC1S_Pos)  | // Channel 1/3, output mode
		(0b110 << TIM_CCMR1_OC1M_Pos)  | // Channel 1/3, PWM mode 1
		(0b1   << TIM_CCMR1_OC1PE_Pos) | // Channel 1/3, preload register enabled
		(0b00  << TIM_CCMR1_CC2S_Pos)  | // Channel 2/4, output mode
		(0b110 << TIM_CCMR1_OC2M_Pos)  | // Channel 2/4, PWM mode 1
		(0b1   << TIM_CCMR1_OC2PE_Pos) | // Channel 2/4, preload register enabled
		0;
#if USE_TIMER1_PWM
	TIM1->PSC = psc_apb2;
	// Main output enable, only needed for advanced timers (1 and 8)
	TIM1->BDTR = TIM_BDTR_MOE;
	CONFIGURE_PWM_TIMER(TIM1);
#endif
#if USE_TIMER2_PWM
	TIM2->PSC = psc_apb1;
	CONFIGURE_PWM_TIMER(TIM2);
#endif
#if USE_TIMER3_PWM
	TIM3->PSC = psc_apb1;
	CONFIGURE_PWM_TIMER(TIM3);
#endif
#if USE_TIMER4_PWM
	TIM4->PSC = psc_apb1;
	CONFIGURE_PWM_TIMER(TIM4);
#endif

	//
	// Sleep timer
	//
	MODIFY_BITS(SLEEP_ALARM_TIM->CR1, TIM_CR1_ARPE|TIM_CR1_CMS|TIM_CR1_DIR|TIM_CR1_OPM,
		(0b0  << TIM_CR1_ARPE_Pos) | // 0 to disable reload register buffer
		(0b00 << TIM_CR1_CMS_Pos ) | // 0 to disable bidirectional counting
		(0b0 << TIM_CR1_DIR_Pos  ) | // 0 to use as an upcounter
		(0b1 << TIM_CR1_OPM_Pos  ) | // 1 to automatically disable on update events
		0);
	G_sleep_alarm_psc = calculate_TIM_prescaler(SLEEP_ALARM_CLOCKEN, 1000*SLEEP_TIM_MS_TICKS);
	SLEEP_ALARM_TIM->PSC = G_sleep_alarm_psc;
	NVIC_SetPriority(SLEEP_ALARM_IRQn, SLEEP_ALARM_IRQp);

	//
	// USCOUNTER timer
	//
	G_uscounter_psc = calculate_TIM_prescaler(USCOUNTER_CLOCKEN, 1000000);

	clock_disable(apb1_enr);
	clock_disable(apb2_enr);

	return;
}
// hz is the target rate of counter increase, e.g. 1000 to go up 1 each
// millisecond (or 1000 a second)
// Clocks:
// Timers 2-7 and 12-14
//   PCLK1*1 if PCLK1 prescaler is 1
//   PCLK1*2 otherwise
// Timers 1 and 8-11
//   PCLK2*1 if PCLK2 prescaler is 1
//   PCLK2*2 otherwise
static uint16_t calculate_TIM_prescaler(rcc_periph_t tim, uint32_t hz) {
	uint16_t prescaler;
	uint32_t pclk;

	assert(hz != 0);

	switch (SELECT_BITS(tim, RCC_BUS_MASK)) {
	case RCC_BUS_APB1:
		pclk = G_freq_PCLK1;
		prescaler = (G_freq_PCLK1 == G_freq_HCLK) ? 1 : 2;
		break;
	case RCC_BUS_APB2:
		pclk = G_freq_PCLK2;
		prescaler = (G_freq_PCLK2 == G_freq_HCLK) ? 1 : 2;
		break;
	default:
		// Shouldn't reach this point
		assert(false);
		return 0;
	}
	assert(pclk > hz);
	assert((pclk/hz) <= (0xFFFF/prescaler));

	prescaler *= pclk / hz;
	// A prescaler of 0 divides by 1 so we need to adjust.
	--prescaler;

	return prescaler;
}
void set_sleep_alarm(uint32_t ms) {
	assert(ms != 0);
	assert(ms <= SLEEP_TIM_MAX_MS);

	clock_enable(SLEEP_ALARM_CLOCKEN);

	// Set the reload value and generate an update event to load it
	SLEEP_ALARM_TIM->ARR = ms * SLEEP_TIM_MS_TICKS;
	SET_BIT(SLEEP_ALARM_TIM->EGR, TIM_EGR_UG);

	SLEEP_ALARM_TIM->SR = 0x0000;                 // Clear all event flags
	SET_BIT(SLEEP_ALARM_TIM->DIER, TIM_DIER_UIE); // Enable update interrupts
	NVIC_ClearPendingIRQ(SLEEP_ALARM_IRQn);
	NVIC_EnableIRQ(SLEEP_ALARM_IRQn);
	SET_BIT(SLEEP_ALARM_TIM->CR1, TIM_CR1_CEN);   // Enable the timer

	return;
}
void stop_sleep_alarm(void) {
	NVIC_DisableIRQ(SLEEP_ALARM_IRQn);
	NVIC_ClearPendingIRQ(SLEEP_ALARM_IRQn);

	// Disable update interrupts
	CLEAR_BIT(SLEEP_ALARM_TIM->DIER, TIM_DIER_UIE);
	// Clear all event flags
	SLEEP_ALARM_TIM->SR = 0;
	// Configured to disable itself, but this may be called before the alarm
	// goes off
	CLEAR_BIT(SLEEP_ALARM_TIM->CR1, TIM_CR1_CEN);

	clock_disable(SLEEP_ALARM_CLOCKEN);

	return;
}
void uscounter_on(void) {
	clock_enable(USCOUNTER_CLOCKEN);
	return;
}
void uscounter_off(void) {
	clock_disable(USCOUNTER_CLOCKEN);
	return;
}

void delay_ms(utime_t ms) {
	utime_t timer;

#if DEBUG && USE_UART_COMM
	uint32_t systick_mask = SysTick_CTRL_TICKINT_Msk|SysTick_CTRL_ENABLE_Msk;
	if (SELECT_BITS(SysTick->CTRL, systick_mask) != systick_mask) {
		uart_on(comm_port);
		LOGGER("Someone is using delay_ms() without systick...");
		dumb_delay_ms(ms);
		return;
	}
#endif

	timer = SET_TIMEOUT(ms);
	while (!TIMES_UP(timer)) {
		// Nothing to do here
	}

	return;
}
// https:// stackoverflow.com/questions/7083482/how-to-prevent-gcc-from-optimizing-out-a-busy-wait-loop
void dumb_delay_ms(utime_t ms) {
	uint32_t cycles;

	cycles = ms * (G_freq_HCLK/(1000*DUMB_DELAY_DIV));

	for (uint32_t i = 0; i < cycles; ++i) {
		// Count some clock cycles
		__asm__ volatile("" : "+g" (i) : :);
	}

	return;
}
void dumb_delay_cycles(uint32_t cycles) {
	for (uint32_t i = 0; i < cycles; ++i) {
		// Count some clock cycles
		__asm__ volatile("" : "+g" (i) : :);
	}

	return;
}
//
// PWM management
#if PWM_DUTY_CYCLE_SCALE <= 0xFFFF
# define SET_PWM_CH(ccr, duty) do { (ccr) = (duty); } while (0)
#else
# define SET_PWM_CH(ccr, duty) do { (ccr) = ((uint64_t )(duty) * 0xFFFF) / PWM_DUTY_CYCLE_SCALE; } while (0)
#endif
static TIM_TypeDef* find_pwm_tim(pin_t pin, uint8_t *channel, gpio_af_t *altpin, rcc_periph_t *periph) {
	uint8_t ch;
	gpio_af_t af;
	rcc_periph_t TIMxEN;
	TIM_TypeDef *TIMx;

	ch = 4;
	switch (PINID(pin)) {
#if USE_TIMER1_PWM
	case PINID_TIM1_CH1:
		--ch;
		// fall through
	case PINID_TIM1_CH2:
		--ch;
		// fall through
	case PINID_TIM1_CH3:
		--ch;
		// fall through
	case PINID_TIM1_CH4:
		TIMx = TIM1;
		TIMxEN = RCC_PERIPH_TIM1;
		af = AF_TIM1;
		break;
#endif
#if USE_TIMER2_PWM
	case PINID_TIM2_CH1:
		--ch;
		// fall through
	case PINID_TIM2_CH2:
		--ch;
		// fall through
	case PINID_TIM2_CH3:
		--ch;
		// fall through
	case PINID_TIM2_CH4:
		TIMx = TIM2;
		TIMxEN = RCC_PERIPH_TIM2;
		af = AF_TIM2;
		break;
#endif
#if USE_TIMER3_PWM
	case PINID_TIM3_CH1:
		--ch;
		// fall through
	case PINID_TIM3_CH2:
		--ch;
		// fall through
	case PINID_TIM3_CH3:
		--ch;
		// fall through
	case PINID_TIM3_CH4:
		TIMx = TIM3;
		TIMxEN = RCC_PERIPH_TIM3;
		af = AF_TIM3;
		break;
#endif
#if USE_TIMER4_PWM
	case PINID_TIM4_CH1:
		--ch;
		// fall through
	case PINID_TIM4_CH2:
		--ch;
		// fall through
	case PINID_TIM4_CH3:
		--ch;
		// fall through
	case PINID_TIM4_CH4:
		TIMx = TIM4;
		TIMxEN = RCC_PERIPH_TIM4;
		af = AF_TIM4;
		break;
#endif

	default:
		LOGGER("Attempted PWM with incapable pin 0x%02X", (uint )pin);
		return NULL;
		break;
	}

	if (channel != NULL) {
		*channel = ch;
	}
	if (altpin != NULL) {
		*altpin = af;
	}
	if (periph != NULL) {
		*periph = TIMxEN;
	}
	return TIMx;
}
void pwm_set(pin_t pin, uint16_t duty_cycle) {
	uint8_t channel = 0;
	TIM_TypeDef *TIMx;

	assert(duty_cycle <= PWM_DUTY_CYCLE_SCALE);

	duty_cycle -= 1;

	TIMx = find_pwm_tim(pin, &channel, NULL, NULL);
	if (TIMx == NULL) {
		return;
	}

	switch (channel) {
	case 1:
		SET_PWM_CH(TIMx->CCR1, duty_cycle);
		break;
	case 2:
		SET_PWM_CH(TIMx->CCR2, duty_cycle);
		break;
	case 3:
		SET_PWM_CH(TIMx->CCR3, duty_cycle);
		break;
	case 4:
		SET_PWM_CH(TIMx->CCR4, duty_cycle);
		break;
	}

	return;
}
void pwm_on(pin_t pin, uint16_t duty_cycle) {
	uint8_t channel = 0;
	gpio_af_t af = 0;
	rcc_periph_t TIMxEN = 0;
	TIM_TypeDef *TIMx;

	assert(duty_cycle <= PWM_DUTY_CYCLE_SCALE);

	duty_cycle -= 1;

	TIMx = find_pwm_tim(pin, &channel, &af, &TIMxEN);
	if (TIMx == NULL) {
		return;
	}

	clock_enable(TIMxEN);
	switch (channel) {
	case 1:
		SET_PWM_CH(TIMx->CCR1, duty_cycle);
		SET_BIT(TIMx->CCER, TIM_CCER_CC1E);
		break;
	case 2:
		SET_PWM_CH(TIMx->CCR2, duty_cycle);
		SET_BIT(TIMx->CCER, TIM_CCER_CC2E);
		break;
	case 3:
		SET_PWM_CH(TIMx->CCR3, duty_cycle);
		SET_BIT(TIMx->CCER, TIM_CCER_CC3E);
		break;
	case 4:
		SET_PWM_CH(TIMx->CCR4, duty_cycle);
		SET_BIT(TIMx->CCER, TIM_CCER_CC4E);
		break;
	}
	SET_BIT(TIMx->CR1, TIM_CR1_CEN);

	// Enable output by setting the pin to AF PP mode
	// See section 9.1.11 (GPIO configurations for device peripherals) of the
	// STM32F1 reference manual
	gpio_set_AF(pin, af);
	gpio_set_mode(pin, GPIO_MODE_PP_AF, GPIO_FLOAT);

	return;
}
void pwm_off(pin_t pin) {
	uint8_t channel;
	rcc_periph_t TIMxEN;
	TIM_TypeDef *TIMx;

	TIMx = find_pwm_tim(pin, &channel, NULL, &TIMxEN);
	if (TIMx == NULL) {
		return;
	}

	switch (channel) {
	case 1:
		CLEAR_BIT(TIMx->CCER, TIM_CCER_CC1E);
		break;
	case 2:
		CLEAR_BIT(TIMx->CCER, TIM_CCER_CC2E);
		break;
	case 3:
		CLEAR_BIT(TIMx->CCER, TIM_CCER_CC3E);
		break;
	case 4:
		CLEAR_BIT(TIMx->CCER, TIM_CCER_CC4E);
		break;
	}

	//if (SELECT_BITS(TIMx->CCER, TIM_CCER_CC1E|TIM_CCER_CC2E|TIM_CCER_CC3E|TIM_CCER_CC4E) == 0) {
	if (TIMx->CCER == 0) {
		CLEAR_BIT(TIMx->CR1, TIM_CR1_CEN);
		clock_disable(TIMxEN);
	}

	return;
}

#ifdef __cplusplus
 }
#endif
