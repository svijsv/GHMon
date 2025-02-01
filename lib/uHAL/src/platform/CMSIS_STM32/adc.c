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
// adc.c
// Manage the ADC peripheral
// NOTES:
//
#include "adc.h"

#if uHAL_USE_ADC
#include "system.h"
#include "gpio.h"


//
// Handle ADCx
// Only ADC1 can read the internal voltage reference and temperature sensor
// If this changes to ADC3, the pin-to-channel mapping will need to be updated
#define ADCx ADC1
#define ADCx_CLOCKEN RCC_PERIPH_ADC1

#if (ADCx_CLOCKEN & RCC_BUS_MASK) == RCC_BUS_APB1
# define ADCx_BUSFREQ G_freq_PCLK1
#elif (ADCx_CLOCKEN & RCC_BUS_MASK) == RCC_BUS_APB2
# define ADCx_BUSFREQ G_freq_PCLK2
#elif (ADCx_CLOCKEN & RCC_BUS_MASK) == RCC_BUS_AHB1
# define ADCx_BUSFREQ G_freq_HCLK
#else
# error "Can't determine ADC bus clock"
#endif

#if   G_freq_ADCCLK == (ADCx_BUSFREQ / 2)
# define PRESCALER ADC_PRESCALER_2
#elif G_freq_ADCCLK == (ADCx_BUSFREQ / 4)
# define PRESCALER ADC_PRESCALER_4
#elif G_freq_ADCCLK == (ADCx_BUSFREQ / 6)
# define PRESCALER ADC_PRESCALER_6
#elif G_freq_ADCCLK == (ADCx_BUSFREQ / 8)
# define PRESCALER ADC_PRESCALER_8
#else
# error "F_ADC must be F_PCLK2 / (2|4|6|8)"
#endif

DEBUG_CPP_MACRO(ADC_SAMPLE_CYCLES)
DEBUG_CPP_MACRO(ADC_SAMPLES_PER_S)
//DEBUG_CPP_MACRO(ADC_SAMPLE_TIME)
//DEBUG_CPP_MACRO(TEMP_SAMPLE_TIME)

static adc_t adc_read_channel(uint_fast32_t channel);

void adc_init(void) {
	uint32_t reg = 0;
	uint_fast8_t shift;

	clock_init(ADCx_CLOCKEN);

	// Set the sample time registers
	for (uiter_t i = 0; i < 32U; i += 3) {
		reg |= ADC_SAMPLE_TIME << i;
	}
	ADCx->SMPR1 = reg & SMPR1_MASK;
	ADCx->SMPR2 = reg & SMPR2_MASK;

	shift = ((TEMP_CHANNEL - 10U) * 3U);
	MODIFY_BITS(ADCx->SMPR1, 0b111U << shift, TEMP_SAMPLE_TIME << shift);
	shift = ((VREF_CHANNEL - 10U) * 3U);
	MODIFY_BITS(ADCx->SMPR1, 0b111U << shift, TEMP_SAMPLE_TIME << shift);

	MODIFY_BITS(ADC_PRESCALER_REG, ADC_PRESCALER_Msk, PRESCALER);

#if HAVE_STM32F1_ADC
	MODIFY_BITS(ADCx->CR2, ADC_CR2_CONT|ADC_CR2_EXTSEL|ADC_CR2_EXTTRIG,
		(0b0U   << ADC_CR2_CONT_Pos   )| // Keep at 0 for single conversion mode
		(0b111U << ADC_CR2_EXTSEL_Pos )| // Enable software start
		(0b1U   << ADC_CR2_EXTTRIG_Pos)| // Enable the trigger
		0);

	// Calibrate the ADC
	SET_BIT(ADCx->CR2, ADC_CR2_ADON);
	SET_BIT(ADCx->CR2, ADC_CR2_RSTCAL);
	while (BIT_IS_SET(ADCx->CR2, ADC_CR2_RSTCAL)) {
		// Nothing to do here
	}
	SET_BIT(ADCx->CR2, ADC_CR2_CAL);
	while (BIT_IS_SET(ADCx->CR2, ADC_CR2_CAL)) {
		// Nothing to do here
	}

#else // ! STM32F1
	MODIFY_BITS(ADCx->CR2, ADC_CR2_CONT,
		(0b0U << ADC_CR2_CONT_Pos  ) | // Keep at 0 for single conversion mode
		0);
	MODIFY_BITS(ADCx->CR1, ADC_CR1_RES, ADC_RES);
#endif // ! STM32F1

	adc_off();

	return;
}
err_t adc_on(void) {
	clock_enable(ADCx_CLOCKEN);

	// When ADON is set the first time, wake from power-down mode
	if (!BIT_IS_SET(ADCx->CR2, ADC_CR2_ADON)) {
		SET_BIT(ADCx->CR2, ADC_CR2_ADON);
	}
	// The ST HAL waits for ADON to be set to 1 after setting it to 1;
	// there's nothing in the reference manual about that though.
	while (!BIT_IS_SET(ADCx->CR2, ADC_CR2_ADON)) {
		// Nothing to do here
	}
	// Wait for stabilization
	dumb_delay_cycles(ADC_STAB_TIME_uS * (G_freq_CORE/1000000U));

	return ERR_OK;
}
err_t adc_off(void) {
	CLEAR_BIT(ADCx->CR2, ADC_CR2_ADON);
	while (BIT_IS_SET(ADCx->CR2, ADC_CR2_ADON)) {
		// Nothing to do here
	}
	clock_disable(ADCx_CLOCKEN);

	return ERR_OK;
}
bool adc_is_on(void) {
	return (clock_is_enabled(ADCx_CLOCKEN) && BIT_IS_SET(ADCx->CR2, ADC_CR2_ADON));
}

static uint8_t pin_to_channel(gpio_pin_t pin) {
	uint8_t ch;

	//
	// This mapping is valid for ADC1 and ADC2
	//
	ch = GPIO_GET_PINNO(pin);
	switch (PINID(pin)) {
#if HAVE_GPIO_PORTA
	case PINID_A0:
	case PINID_A1:
	case PINID_A2:
	case PINID_A3:
	case PINID_A4:
	case PINID_A5:
	case PINID_A6:
	case PINID_A7:
		return ch;
#endif
#if HAVE_GPIO_PORTB
	case PINID_B0:
	case PINID_B1:
		return (ch + 8U);
#endif
#if HAVE_GPIO_PORTC
	case PINID_C0:
	case PINID_C1:
	case PINID_C2:
	case PINID_C3:
	case PINID_C4:
	case PINID_C5:
		return (ch + 10U);
#endif
	}

	return 0xFFU;
}
adc_t adc_read_pin(gpio_pin_t pin) {
	uHAL_assert(GPIO_PIN_IS_VALID(pin));

#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (!GPIO_PIN_IS_VALID(pin)) {
		return ERR_ADC;
	}
#endif
#if ! uHAL_SKIP_OTHER_CHECKS
	if (!clock_is_enabled(ADCx_CLOCKEN)) {
		return ERR_ADC;
	}
#endif

	return adc_read_channel(pin_to_channel(pin));
}
static adc_t adc_read_channel(uint_fast32_t channel) {
	adcm_t adc;
#if ADC_TIMEOUT_MS
	utime_t timeout;
#endif

	// Five bits of channel selection
	if (channel > 0b11111U) {
		return ERR_ADC;
	}

	// Select the ADC channel to convert
	MODIFY_BITS(ADCx->SQR3, ADC_SQR3_SQ1_Msk,
		(channel << ADC_SQR3_SQ1_Pos)
		);

#if HAVE_STM32F1_ADC
	// Conversion can begin when ADON is set the second time after ADC power up
	// If any bit other than ADON is changed when ADON is set, no conversion is
	// triggered.
	SET_BIT(ADCx->CR2, ADC_CR2_ADON);
	while (!BIT_IS_SET(ADCx->CR2, ADC_CR2_ADON)) {
		// Nothing to do here
	}
#endif
#if ADC_TIMEOUT_MS
	timeout = SET_TIMEOUT_MS(ADC_TIMEOUT_MS);
#endif

	adc = 0;
	ADCx->SR = 0;
	for (uiter_t i = 0; i < ADC_SAMPLE_COUNT; ++i) {
		SET_BIT(ADCx->CR2, ADC_CR2_SWSTART);
		while (!BIT_IS_SET(ADCx->SR, ADC_SR_EOC)) {
			// Nothing to do here
#if ADC_TIMEOUT_MS
			if (TIMES_UP(timeout)) {
				return ERR_ADC;
			}
#endif
		}
		adc += SELECT_BITS(ADCx->DR, ADC_MAX);
	}
	adc /= ADC_SAMPLE_COUNT;

	return adc;
}

uint_fast16_t adc_read_vref_mV(void) {
	adc_t adc;
	uint_fast16_t vref;

#if ! uHAL_SKIP_OTHER_CHECKS
	if (!clock_is_enabled(ADCx_CLOCKEN)) {
		return 0;
	}
#endif

	// Enable internal VREF and temperature sensors
	SET_BIT(ADC_TSVREFE_REG, ADC_TSVREFE);
	while (!BIT_IS_SET(ADC_TSVREFE_REG, ADC_TSVREFE)) {
		// Nothing to do here
	}

	adc = adc_read_channel(VREF_CHANNEL);
	// Calculate the ADC Vref by comparing it to the internal Vref
	// adc / max = 1200mV / vref
	// (adc / max) * vref = 1200mV
	// vref = 1200mV / (adc / max)
	// vref = (1200mV * max) / adc
	vref = ((uint_fast32_t )INTERNAL_VREF_mV * (uint_fast32_t )ADC_MAX) / (uint_fast32_t )adc;

	// Disable internal VREF and temperature sensors
	CLEAR_BIT(ADC_TSVREFE_REG, ADC_TSVREFE);

	return vref;
}

/*
uint_fast16_t adc_read_internal_temp(void) {
	uint_fast32_t temp, vref;
	adc_t adc;

	// Enable internal VREF and temperature sensors
	SET_BIT(ADC_TSVREFE_REG, ADC_TSVREFE);
	while (!BIT_IS_SET(ADC_TSVREFE_REG, ADC_TSVREFE)) {
		// Nothing to do here
	}
	// Wait for stabilization
	dumb_delay_cycles(TEMP_START_TIME_uS * (G_freq_CORE/1000000U));

	vref = adc_read_channel(VREF_CHANNEL);
	vref = ((uint_fast32_t )INTERNAL_VREF_mV * (uint_fast32_t )ADC_MAX) / vref;

	temp = adc_read_channel(TEMP_CHANNEL);
	temp = (temp * vref) / ADC_MAX;
#if HAVE_STM32F1_ADC
	// Multiply by 1000 to convert mV to uV
	temp = (((TEMP_INT_T25_mV - temp) * 1000U) / TEMP_INT_SLOPE_uV) + 25U;
#else
	// Multiply by 1000 to convert mV to uV
	temp = (((temp - TEMP_INT_T25_mV) * 1000U) / TEMP_INT_SLOPE_uV) + 25U;
#endif

	// Disable internal VREF and temperature sensors
	CLEAR_BIT(ADC_TSVREFE_REG, ADC_TSVREFE);

	return temp;
}
*/

adc_t adc_read_ac_amplitude(gpio_pin_t pin, uint_fast32_t period_ms, adc_t *min, adc_t *max) {
	uint8_t channel;
	adc_t adc, adc_min, adc_max;
	utime_t timeout;

	uHAL_assert(GPIO_PIN_IS_VALID(pin));

#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (!GPIO_PIN_IS_VALID(pin)) {
		return ERR_ADC;
	}
#endif
#if ! uHAL_SKIP_OTHER_CHECKS
	if (!clock_is_enabled(ADCx_CLOCKEN)) {
		return ERR_ADC;
	}
#endif

	channel = pin_to_channel(pin);
	// Five bits of channel selection
	if (channel > 0b11111U) {
		return ERR_ADC;
	}

	// Select the ADC channel to convert
	MODIFY_BITS(ADCx->SQR3, 0b11111U << ADC_SQR3_SQ1_Pos,
		(channel << ADC_SQR3_SQ1_Pos)
		);
	// Use continuous conversion mode
	SET_BIT(ADCx->CR2, ADC_CR2_CONT);

#if HAVE_STM32F1_ADC
	// Conversion can begin when ADON is set the second time after ADC power up
	// If any bit other than ADON is changed when ADON is set, no conversion is
	// triggered.
	SET_BIT(ADCx->CR2, ADC_CR2_ADON);
	while (!BIT_IS_SET(ADCx->CR2, ADC_CR2_ADON)) {
		// Nothing to do here
	}
#endif

	ADCx->SR = 0;
	SET_BIT(ADCx->CR2, ADC_CR2_SWSTART);

	adc_min = ADC_MAX;
	adc_max = 0;
	timeout = SET_TIMEOUT_MS(period_ms);
	while (!TIMES_UP(timeout)) {
		while (!BIT_IS_SET(ADCx->SR, ADC_SR_EOC)) {
			// Nothing to do here
		}
		// Reading ADC_DR clears the EOC bit
		adc = SELECT_BITS(ADCx->DR, ADC_MAX);

		if (adc > adc_max) {
			adc_max = adc;
		} else if (adc < adc_min) {
			adc_min = adc;
		}
	}

	// Switch back to single conversion mode
	CLEAR_BIT(ADCx->CR2, ADC_CR2_CONT);
	// Wait for final conversion to finish
	while (!BIT_IS_SET(ADCx->SR, ADC_SR_EOC)) {
		// Nothing to do here
	}
	//CLEAR_BIT(ADCx->SR, ADC_SR_EOC);
	ADCx->SR = 0;

	if (min != NULL) {
		*min = adc_min;
	}
	if (max != NULL) {
		*max = adc_max;
	}
	return (adc_max - adc_min)/2U;
}


#endif // uHAL_USE_ADC
