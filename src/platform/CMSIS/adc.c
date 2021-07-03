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
// adc.c
// Manage the ADC peripheral
// NOTES:
//

#ifdef __cplusplus
 extern "C" {
#endif

//#define DEBUG 1
/*
* Includes
*/
#include "adc.h"
#include "system.h"
#include "gpio.h"


#if USE_ADC

/*
* Static values
*/
#define VREF_CHANNEL 17
#define TEMP_CHANNEL 16

// Datasheet value "Avg_Slope" converted from mV to uV
#define TEMP_INT_SLOPE 4300
// Datasheet value "V25" converted from V to mV
#define TEMP_INT_V25   1430
// Temperature sensor startup time specified as Tstart in the datasheet is
// 4-10us
#define TEMP_START_TIME_uS 10
// The recommended sample time for the internal temperature is 17.1us
#define TEMP_SAMPLE_uS     17

// ADC stabilization time is specified under Tstab in the datasheet as
// 1us max
#define ADC_STAB_TIME_uS 1

#define SAMPLE_TIME_1_5   0b000 //  1.5 cycles
#define SAMPLE_TIME_7_5   0b001 //  7.5 cycles
#define SAMPLE_TIME_13_5  0b010 // 13.5 cycles
#define SAMPLE_TIME_28_5  0b011 // 28.5 cycles
#define SAMPLE_TIME_41_5  0b100 // 41.5 cycles
#define SAMPLE_TIME_55_5  0b101 // 55.5 cycles
#define SAMPLE_TIME_71_5  0b110 // 71.5 cycles
#define SAMPLE_TIME_239_5 0b111 // 239.5 cycles
// The value of a register set entirely to the default sample time
// 239.5 cycles:
//#define SAMPLE_TIME_REG 0x3FFFFFFF
// 71.5 cycles:
#define SAMPLE_TIME_REG 0x36DB6DB6

/*
* Types
*/


/*
* Variables
*/


/*
* Local function prototypes
*/
static adc_t adc_read_channel(uint32_t channel);


/*
* Interrupt handlers
*/


/*
* Functions
*/
void adc_init(void) {
	clock_init(&RCC->APB2ENR, &RCC->APB2RSTR, RCC_APB2ENR_ADCxEN);

	MODIFY_BITS(ADCx->CR2, ADC_CR2_CONT|ADC_CR2_EXTSEL|ADC_CR2_EXTTRIG,
		(0b0   << ADC_CR2_CONT_Pos  ) | // Keep at 0 for single conversion mode
		(0b111 << ADC_CR2_EXTSEL_Pos) | // Enable software start
		(0b1   << ADC_CR2_EXTTRIG_Pos)| // Enable the trigger
		0);

	// Set the sample time registers
	// If the sample time might bit shorter than the minimum temperature sample
	// time and the temperature might be read, it's sample time should be set
	// separately.
	ADCx->SMPR1 = SAMPLE_TIME_REG & 0x00FFFFFF; // Only 24 bits used
	ADCx->SMPR2 = SAMPLE_TIME_REG & 0x3FFFFFFF; // Only 30 bits used

	// When ADON is set the first time, wake from power-down mode
	SET_BIT(ADCx->CR2, ADC_CR2_ADON);

	// Calibrate the ADC
	SET_BIT(ADCx->CR2, ADC_CR2_RSTCAL);
	while (BIT_IS_SET(ADCx->CR2, ADC_CR2_RSTCAL)) {
		// Nothing to do here
	}
	SET_BIT(ADCx->CR2, ADC_CR2_CAL);
	while (BIT_IS_SET(ADCx->CR2, ADC_CR2_CAL)) {
		// Nothing to do here
	}

	CLEAR_BIT(ADCx->CR2, ADC_CR2_ADON);
	clock_disable(&RCC->APB2ENR, RCC_APB2ENR_ADCxEN);

	return;
}
void adc_on(void) {
	clock_enable(&RCC->APB2ENR, RCC_APB2ENR_ADCxEN);

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
	dumb_delay_cycles(ADC_STAB_TIME_uS * (G_freq_HCLK/1000000U));

	return;
}
void adc_off(void) {
	CLEAR_BIT(ADCx->CR2, ADC_CR2_ADON);

	clock_disable(&RCC->APB2ENR, RCC_APB2ENR_ADCxEN);

	return;
}

adc_t adc_read_pin(pin_t pin) {
	uint32_t channel;
	pin_t pinno;

	channel = 0; // Shut the compiler up

	pinno = GPIO_GET_PINNO(pin);
	switch (PINID(pin)) {
	case PINID_A0:
	case PINID_A1:
	case PINID_A2:
	case PINID_A3:
	case PINID_A4:
	case PINID_A5:
	case PINID_A6:
	case PINID_A7:
		channel = pinno;
		break;
	case PINID_B0:
	case PINID_B1:
		channel = pinno + 8;
		break;
	default:
		return 0;
		break;
	}

	return adc_read_channel(channel);
}
static adc_t adc_read_channel(uint32_t channel) {
	adcm_t adc;

	// Five bits of channel selection
	assert(channel <= 0b11111);

	// Select the ADC channel to convert
	MODIFY_BITS(ADCx->SQR3, 0b11111 << ADC_SQR3_SQ1_Pos,
		(channel << ADC_SQR3_SQ1_Pos)
		);

	// Conversion can begin when ADON is set the second time after ADC power up
	// If any bit other than ADON is changed when ADON is set, no conversion is
	// triggered.
	SET_BIT(ADCx->CR2, ADC_CR2_ADON);

	adc = 0;
	for (uiter_t i = 0; i < ADC_SAMPLE_COUNT; ++i) {
		SET_BIT(ADCx->CR2, ADC_CR2_SWSTART);
		while (!BIT_IS_SET(ADCx->SR, ADC_SR_EOC)) {
			// Nothing to do here
		}
		adc += SELECT_BITS(ADCx->DR, ADC_MAX);
	}
	adc /= ADC_SAMPLE_COUNT;

	return adc;
}

int16_t adc_read_vref_mV(void) {
	adc_t adc;
	uint16_t vref;

#if ADCx_IS_ADC1
	// Enable internal VREF and temperature sensors
	SET_BIT(ADCx->CR2, ADC_CR2_TSVREFE);

	adc = adc_read_channel(VREF_CHANNEL);
	// Calculate the ADC Vref by comparing it to the internal Vref
	// adc / max = 1200mV / vref
	// (adc / max) * vref = 1200mV
	// vref = 1200mV / (adc / max)
	// vref = (1200mV * max) / adc
	vref = (INTERNAL_VREF_mV * ADC_MAX) / (uint32_t )adc;

	CLEAR_BIT(ADCx->CR2, ADC_CR2_TSVREFE);

#else // !ADCx_IS_ADC1
	vref = REGULATED_VOLTAGE_mV;
#endif // ADCx_IS_ADC1

	return vref;
}

adc_t adc_read_ac_amplitude(pin_t pin, uint32_t period_ms) {
	uint32_t channel = 0;
	pin_t pinno;
	adc_t adc, adc_min, adc_max;
	adcm_t adcm_min, adcm_max;
	utime_t timeout;

	pinno = GPIO_GET_PINNO(pin);
	switch (PINID(pin)) {
	case PINID_A0:
	case PINID_A1:
	case PINID_A2:
	case PINID_A3:
	case PINID_A4:
	case PINID_A5:
	case PINID_A6:
	case PINID_A7:
		channel = pinno;
		break;
	case PINID_B0:
	case PINID_B1:
		channel = pinno + 8;
		break;
	default:
		return 0;
		break;
	}

	// Use continuous conversion mode
	MODIFY_BITS(ADCx->CR2, ADC_CR2_CONT,
		(0b1   << ADC_CR2_CONT_Pos  )
		);
	// Select the ADC channel to convert
	MODIFY_BITS(ADCx->SQR3, 0b11111 << ADC_SQR3_SQ1_Pos,
		(channel << ADC_SQR3_SQ1_Pos)
		);

	// Conversion can begin when ADON is set the second time after ADC power up
	// If any bit other than ADON is changed when ADON is set, no conversion is
	// triggered.
	SET_BIT(ADCx->CR2, ADC_CR2_ADON);

	adcm_max = 0;
	adcm_min = 0;
	SET_BIT(ADCx->CR2, ADC_CR2_SWSTART);
	for (uiter_t i = 0; i < ADC_SAMPLE_COUNT; ++i) {
		adc_min = ADC_MAX;
		adc_max = 0;
		timeout = SET_TIMEOUT(period_ms);
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
		adcm_max += adc_max;
		adcm_min += adc_min;
	}

	// Switch back to single conversion mode
	MODIFY_BITS(ADCx->CR2, ADC_CR2_CONT,
		(0b0   << ADC_CR2_CONT_Pos  )
		);
	// Wait for final conversion to finish
	while (!BIT_IS_SET(ADCx->SR, ADC_SR_EOC)) {
		// Nothing to do here
	}
	CLEAR_BIT(ADCx->SR, ADC_SR_EOC);

	adcm_max /= ADC_SAMPLE_COUNT;
	adcm_min /= ADC_SAMPLE_COUNT;
	return (adcm_max - adcm_min);
}



#endif // USE_ADC

#ifdef __cplusplus
 }
#endif
