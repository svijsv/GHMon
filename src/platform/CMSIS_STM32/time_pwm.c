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
// time_*_.c
// Manage the PWM timers
// NOTES:
//    Timers 6 & 7 have no possible PWM outputs
//

#include "time_private.h"
#include "gpio.h"
#include "system.h"

#if uHAL_USE_PWM

#if USE_TIMER1_PWM
 DEBUG_CPP_MSG("Using timer 1 PWM")
#endif
#if USE_TIMER2_PWM
 DEBUG_CPP_MSG("Using timer 2 PWM")
#endif
#if USE_TIMER3_PWM
 DEBUG_CPP_MSG("Using timer 3 PWM")
#endif
#if USE_TIMER4_PWM
 DEBUG_CPP_MSG("Using timer 4 PWM")
#endif
#if USE_TIMER5_PWM
 DEBUG_CPP_MSG("Using timer 5 PWM")
#endif
#if USE_TIMER8_PWM
 DEBUG_CPP_MSG("Using timer 8 PWM")
#endif
#if USE_TIMER9_PWM
 DEBUG_CPP_MSG("Using timer 9 PWM")
#endif
#if USE_TIMER10_PWM
 DEBUG_CPP_MSG("Using timer 10 PWM")
#endif
#if USE_TIMER11_PWM
 DEBUG_CPP_MSG("Using timer 11 PWM")
#endif
#if USE_TIMER12_PWM
 DEBUG_CPP_MSG("Using timer 12 PWM")
#endif
#if USE_TIMER13_PWM
 DEBUG_CPP_MSG("Using timer 13 PWM")
#endif
#if USE_TIMER14_PWM
 DEBUG_CPP_MSG("Using timer 14 PWM")
#endif


// A PWM signal period is the total time it takes for the counter to go from
// 0 to PWM_DUTY_CYCLE_SCALE, therefore the frequency of the timer's counter
// is the signal frequency times PWM_DUTY_CYCLE_SCALE and the maximum supported
// frequency is the base clock divided by PWM_DUTY_CYCLE_SCALE
#define PWM_APB1_HZ (PWM_FREQUENCY_HZ * PWM_DUTY_CYCLE_SCALE)
#if PWM_APB1_HZ > TIM_APB1_MAX_HZ
# undef  PWM_APB1_HZ
# define PWM_APB1_HZ TIM_APB1_MAX_HZ
#endif
#define PWM_APB2_HZ (PWM_FREQUENCY_HZ * PWM_DUTY_CYCLE_SCALE)
#if PWM_APB2_HZ > TIM_APB2_MAX_HZ
# undef  PWM_APB2_HZ
# define PWM_APB2_HZ TIM_APB2_MAX_HZ
#endif
DEBUG_CPP_MACRO(PWM_APB1_HZ)
DEBUG_CPP_MACRO(PWM_APB2_HZ)

// PWM_MAX_CNT could be up to 32 bits with timers 2-5 on non-F1 devices, but
// that gets complicated and who needs that much resolution?
#if PWM_DUTY_CYCLE_SCALE <= TIM_MAX_CNT
# define PWM_MAX_CNT PWM_DUTY_CYCLE_SCALE
# define SET_PWM_CH(_ccr_, _dc_) ((_ccr_) = (_dc_))
#else
# define PWM_MAX_CNT TIM_MAX_CNT
# define SET_PWM_CH(_ccr_, _dc_) ((_ccr_) = ((uint64_t )(_dc_) * PWM_MAX_CNT) / PWM_DUTY_CYCLE_SCALE)
#endif

#define PWM_OUTPUT_IS_VALID(_o_) ((_o_) != NULL && GPIO_PIN_IS_VALID((_o_)->pin) && (_o_)->TIMx != NULL && (_o_)->channel > 0 && (_o_)->channel <= 4)

#if HAVE_STM32F1_GPIO
# include "time_pwm_find_ch_f1.h"
#else
# include "time_pwm_find_ch_fx_table.h"
#endif

static void configure_pwm_timer(uint_fast8_t tim_id, uint16_t psc) {
	TIM_TypeDef *TIMx = get_tim_from_id(tim_id);
	const uint16_t ccmr =
		// PWM mode 1 is high while TIMx_CNT < TIMx_CCRx
		// PWM mode 2 is high while TIMx_CNT > TIMx_CCRx
		(0b00  << TIM_CCMR1_CC1S_Pos)  | // Channel 1/3, output mode
		(0b110 << TIM_CCMR1_OC1M_Pos)  | // Channel 1/3, PWM mode 1
		(0b1   << TIM_CCMR1_OC1PE_Pos) | // Channel 1/3, preload register enabled
		(0b00  << TIM_CCMR1_CC2S_Pos)  | // Channel 2/4, output mode
		(0b110 << TIM_CCMR1_OC2M_Pos)  | // Channel 2/4, PWM mode 1
		(0b1   << TIM_CCMR1_OC2PE_Pos) | // Channel 2/4, preload register enabled
		0;

	assert(TIMx != NULL);
	if (TIMx == NULL) {
		return;
	}

	TIMx->CR1 = TIM_CR1_ARPE;
	TIMx->PSC = psc;

	// The output stops when it hits the CCR, so 100% duty cycle would actually
	// be a cycle short if we just set the max to the duty cycle max
#if (PWM_DUTY_CYCLE_SCALE <= PWM_MAX_CNT)
	TIMx->ARR = (PWM_DUTY_CYCLE_SCALE)-1U;
#else
	TIMx->ARR = PWM_MAX_CNT-1U;
#endif

	TIMx->CCMR1 = ccmr;
	// Not all timers actually use this register, but it doesn't hurt to set it
	// (I hope)
	TIMx->CCMR2 = ccmr;

	// Main output enable, only needed for advanced timers (1 and 8)
	// Not all timers actually use this register, but it doesn't hurt to set it
	// (I hope)
	TIMx->BDTR = TIM_BDTR_MOE;

	// Generate an update event to load the shadow registers
	SET_BIT(TIMx->EGR, TIM_EGR_UG);
	// Clear all event flags
	TIMx->SR = 0;

	return;
}

void pwm_init(void) {
	rcc_periph_t apb1_enr = 0, apb2_enr = 0;
	uint16_t psc_apb1, psc_apb2;
	const uint8_t timers[] = {
#if USE_TIMER1_PWM
		TIMER_1,
#endif
#if USE_TIMER2_PWM
		TIMER_2,
#endif
#if USE_TIMER3_PWM
		TIMER_3,
#endif
#if USE_TIMER4_PWM
		TIMER_4,
#endif
#if USE_TIMER5_PWM
		TIMER_5,
#endif
#if USE_TIMER8_PWM
		TIMER_8,
#endif
#if USE_TIMER9_PWM
		TIMER_9,
#endif
#if USE_TIMER10_PWM
		TIMER_10,
#endif
#if USE_TIMER11_PWM
		TIMER_11,
#endif
#if USE_TIMER12_PWM
		TIMER_12,
#endif
#if USE_TIMER13_PWM
		TIMER_13,
#endif
#if USE_TIMER14_PWM
		TIMER_14,
#endif
	};

	// Timers 2-7 and 12-14 are on APB1
	// Timers 1 and 8-11 are on APB2
	// Not all timers are present on all hardware
#if USE_TIMER2_PWM
	apb1_enr |= RCC_PERIPH_TIM2;
#endif
#if USE_TIMER3_PWM
	apb1_enr |= RCC_PERIPH_TIM3;
#endif
#if USE_TIMER4_PWM
	apb1_enr |= RCC_PERIPH_TIM4;
#endif
#if USE_TIMER5_PWM
	apb1_enr |= RCC_PERIPH_TIM5;
#endif
#if USE_TIMER12_PWM
	apb1_enr |= RCC_PERIPH_TIM12;
#endif
#if USE_TIMER13_PWM
	apb1_enr |= RCC_PERIPH_TIM13;
#endif
#if USE_TIMER14_PWM
	apb1_enr |= RCC_PERIPH_TIM14;
#endif

#if USE_TIMER1_PWM
	apb2_enr |= RCC_PERIPH_TIM1;
#endif
#if USE_TIMER8_PWM
	apb2_enr |= RCC_PERIPH_TIM8;
#endif
#if USE_TIMER9_PWM
	apb2_enr |= RCC_PERIPH_TIM9;
#endif
#if USE_TIMER10_PWM
	apb2_enr |= RCC_PERIPH_TIM10;
#endif
#if USE_TIMER11_PWM
	apb2_enr |= RCC_PERIPH_TIM11;
#endif

	clock_init(apb1_enr);
	clock_init(apb2_enr);

	psc_apb1 = calculate_TIM_prescaler(RCC_BUS_APB1, PWM_APB1_HZ);
	psc_apb2 = calculate_TIM_prescaler(RCC_BUS_APB2, PWM_APB2_HZ);

	for (uiter_t i = 0; i < SIZEOF_ARRAY(timers); ++i) {
		const uint_fast8_t tim_id = timers[i];

		configure_pwm_timer(tim_id, is_apb1_tim(tim_id) ? psc_apb1 : psc_apb2);
	}

	clock_disable(apb1_enr);
	clock_disable(apb2_enr);

	return;
}

static err_t _pwm_set(TIM_TypeDef *TIMx, uint_fast8_t channel, uint_fast16_t duty_cycle) {
	assert(TIMx != NULL);
	assert(duty_cycle <= PWM_DUTY_CYCLE_SCALE);

	if (duty_cycle > PWM_DUTY_CYCLE_SCALE) {
		duty_cycle = PWM_DUTY_CYCLE_SCALE;
	}

	switch (channel) {
	case 1:
		SET_PWM_CH(TIMx->CCR1, duty_cycle);
		//ccr = &(TIMx->CCR1);
		break;
	case 2:
		SET_PWM_CH(TIMx->CCR2, duty_cycle);
		//ccr = &(TIMx->CCR2);
		break;
	case 3:
		SET_PWM_CH(TIMx->CCR3, duty_cycle);
		//ccr = &(TIMx->CCR3);
		break;
	case 4:
		SET_PWM_CH(TIMx->CCR4, duty_cycle);
		//ccr = &(TIMx->CCR4);
		break;
	}
	//SET_PWM_CH(*ccr, duty_cycle);

	return ERR_OK;
}
err_t pwm_set(pwm_output_t *output, uint_fast16_t duty_cycle) {
	assert(duty_cycle <= PWM_DUTY_CYCLE_SCALE);
	assert(PWM_OUTPUT_IS_VALID(output));

#if ! uHAL_SKIP_INIT_CHECKS
	if (!PWM_OUTPUT_IS_VALID(output)) {
		return ERR_INIT;
	}
#endif
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (!GPIO_PIN_IS_VALID(output->pin)) {
		return ERR_BADARG;
	}
	/*
	if (duty_cycle > PWM_DUTY_CYCLE_SCALE) {
		duty_cycle = PWM_DUTY_CYCLE_SCALE;
	}
	*/
#endif

	return _pwm_set(output->TIMx, output->channel, duty_cycle);
}
err_t pwm_on(pwm_output_t *output, gpio_pin_t pin, uint_fast16_t duty_cycle) {
	uint_fast8_t tim_id = 0, channel = 0;
	gpio_af_t af = 0;
	rcc_periph_t TIMxEN = 0;
	TIM_TypeDef *TIMx;
	uint32_t en_bit = 0;

	if (pin == 0 && output != NULL) {
		pin = output->pin;
	}

	assert(duty_cycle <= PWM_DUTY_CYCLE_SCALE);
	assert(GPIO_PIN_IS_VALID(pin));
	assert(output != NULL);

#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (output == NULL || !GPIO_PIN_IS_VALID(pin)) {
		return ERR_BADARG;
	}
	/*
	if (duty_cycle > PWM_DUTY_CYCLE_SCALE) {
		duty_cycle = PWM_DUTY_CYCLE_SCALE;
	}
	*/
#endif

	TIMx = find_available_pin_pwm_tim(pin, &tim_id, &channel, &af, &TIMxEN);
	if (TIMx == NULL) {
		return ERR_NOTSUP;
	}

	clock_enable(TIMxEN);
	switch (channel) {
	case 1:
		en_bit = TIM_CCER_CC1E;
		break;
	case 2:
		en_bit = TIM_CCER_CC2E;
		break;
	case 3:
		en_bit = TIM_CCER_CC3E;
		break;
	case 4:
		en_bit = TIM_CCER_CC4E;
		break;
	}
	_pwm_set(TIMx, channel, duty_cycle);
	SET_BIT(TIMx->CCER, en_bit);
	SET_BIT(TIMx->CR1, TIM_CR1_CEN);

	// Enable output by setting the pin to AF PP mode
	// See section 9.1.11 (GPIO configurations for device peripherals) of the
	// STM32F1 reference manual
	gpio_set_AF(pin, af);
	gpio_set_mode(pin, GPIO_MODE_PP_AF, GPIO_FLOAT);

	output->pin = pin;
	output->TIMx = TIMx;
	output->TIMxEN = TIMxEN;
	output->channel = channel;

	return ERR_OK;
}
err_t pwm_off(pwm_output_t *output) {
	uint32_t enr_bit = 0;

	assert(PWM_OUTPUT_IS_VALID(output));

#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (!PWM_OUTPUT_IS_VALID(output)) {
		return ERR_BADARG;
	}
#endif

	switch (output->channel) {
	case 1:
		enr_bit = TIM_CCER_CC1E;
		break;
	case 2:
		enr_bit = TIM_CCER_CC2E;
		break;
	case 3:
		enr_bit = TIM_CCER_CC3E;
		break;
	case 4:
		enr_bit = TIM_CCER_CC4E;
		break;
	}
	CLEAR_BIT(output->TIMx->CCER, enr_bit);

	if (SELECT_BITS(output->TIMx->CCER, TIM_CCER_CC1E|TIM_CCER_CC2E|TIM_CCER_CC3E|TIM_CCER_CC4E) == 0) {
		CLEAR_BIT(output->TIMx->CR1, TIM_CR1_CEN);
		clock_disable(output->TIMxEN);
	}
	gpio_set_mode(output->pin, GPIO_MODE_RESET, GPIO_FLOAT);

	return ERR_OK;
}


#endif // uHAL_USE_PWM
