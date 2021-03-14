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

#define PRESCALER_DIV2 0b00 // PCLK2/2
#define PRESCALER_DIV4 0b01 // PCLK2/4
#define PRESCALER_DIV6 0b10 // PCLK2/6
#define PRESCALER_DIV8 0b11 // PCLK2/8

#define SAMPLE_TIME_1_5   0b000 //  1.5 cycles
#define SAMPLE_TIME_7_5   0b001 //  7.5 cycles
#define SAMPLE_TIME_13_5  0b010 // 13.5 cycles
#define SAMPLE_TIME_28_5  0b011 // 28.5 cycles
#define SAMPLE_TIME_41_5  0b100 // 41.5 cycles
#define SAMPLE_TIME_55_5  0b101 // 55.5 cycles
#define SAMPLE_TIME_71_5  0b110 // 71.5 cycles
#define SAMPLE_TIME_239_5 0b111 // 239.5 cycles


/*
* Types
*/


/*
* Variables
*/
uint32_t G_freq_ADC;


/*
* Local function prototypes
*/
static uint32_t calculate_prescaler(uint32_t max_hz);
static uint32_t calculate_sample_rate(uint32_t uS);
static adc_t adc_read_channel(uint32_t channel);


/*
* Interrupt handlers
*/


/*
* Functions
*/
void adc_init(void) {
	uint32_t tmp, reg;

	// Set ADC prescaler
	// On the STM32F1, ADC input clock must not exceed 14MHz
	MODIFY_BITS(RCC->CFGR, RCC_CFGR_ADCPRE,
		calculate_prescaler(14000000)
		);
	switch (GATHER_BITS(RCC->CFGR, 0b11, RCC_CFGR_ADCPRE_Pos)) {
	case PRESCALER_DIV2:
		G_freq_ADC = G_freq_PCLK2/2;
		break;
	case PRESCALER_DIV4:
		G_freq_ADC = G_freq_PCLK2/4;
		break;
	case PRESCALER_DIV6:
		G_freq_ADC = G_freq_PCLK2/6;
		break;
	case PRESCALER_DIV8:
		G_freq_ADC = G_freq_PCLK2/8;
		break;
	default:
		break;
	}

	clock_init(&RCC->APB2ENR, &RCC->APB2RSTR, RCC_APB2ENR_ADCxEN);

	MODIFY_BITS(ADCx->CR2, ADC_CR2_CONT|ADC_CR2_EXTSEL,
		(0b0   << ADC_CR2_CONT_Pos  ) | // Keep at 0 for single conversion mode
		(0b111 << ADC_CR2_EXTSEL_Pos) | // Enable software start
		0);

	// Make things simpler by setting the whole sample time registers at once
	// Be sure G_freq_ADC is set before calculating the sample rate.
	reg = 0;
	tmp = calculate_sample_rate(ADC_SAMPLE_TIME);
	for (uiter_t i = 0; i < 11; ++i) {
		SET_BIT(reg, tmp << (i*3));
	}
	ADCx->SMPR1 = reg & 0x00FFFFFF; // Only 24 bits used
	ADCx->SMPR2 = reg & 0x3FFFFFFF; // Only 30 bits used

#if ADCx_IS_ADC1
	tmp = calculate_sample_rate(TEMP_SAMPLE_uS);
	MODIFY_BITS(ADCx->SMPR1, (0b111 << ((TEMP_CHANNEL-10)*3)),
		(tmp << ((TEMP_CHANNEL-10)*3)) |
		0);
#endif // ADCx_IS_ADC1

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

	return;
}
void adc_off(void) {
	clock_disable(&RCC->APB2ENR, RCC_APB2ENR_ADCxEN);

	return;
}
static uint32_t calculate_prescaler(uint32_t max_hz) {
	if ((G_freq_PCLK2 / 2) < max_hz) {
		return (PRESCALER_DIV2 << RCC_CFGR_ADCPRE_Pos);
	}
	if ((G_freq_PCLK2 / 4) < max_hz) {
		return (PRESCALER_DIV4 << RCC_CFGR_ADCPRE_Pos);
	}
	if ((G_freq_PCLK2 / 6) < max_hz) {
		return (PRESCALER_DIV6 << RCC_CFGR_ADCPRE_Pos);
	}
	return ((uint32_t )PRESCALER_DIV8 << RCC_CFGR_ADCPRE_Pos);
}
static uint32_t calculate_sample_rate(uint32_t uS) {
	uint32_t tmp;

	tmp = (G_freq_ADC * uS) / 1000000;

	if (tmp > 72) {
		tmp = SAMPLE_TIME_239_5;
	} else if (tmp > 56) {
		tmp = SAMPLE_TIME_71_5;
	} else if (tmp > 42) {
		tmp = SAMPLE_TIME_55_5;
	} else if (tmp > 29) {
		tmp = SAMPLE_TIME_41_5;
	} else if (tmp > 14) {
		tmp = SAMPLE_TIME_28_5;
	} else if (tmp > 8) {
		tmp = SAMPLE_TIME_13_5;
	} else if (tmp > 2) {
		tmp = SAMPLE_TIME_7_5;
	} else {
		tmp = SAMPLE_TIME_1_5;
	}

	return tmp;
}

adc_t adc_read_pin(pin_t pin) {
	uint32_t channel;
	pin_t pinno;

	channel = 0; // Shut the compiler up

	pinno = GPIO_GET_PINNO(pin);
	switch (pin) {
	case (GPIO_PORTA|GPIO_PINNO(0)):
	case (GPIO_PORTA|GPIO_PINNO(1)):
	case (GPIO_PORTA|GPIO_PINNO(2)):
	case (GPIO_PORTA|GPIO_PINNO(3)):
	case (GPIO_PORTA|GPIO_PINNO(4)):
	case (GPIO_PORTA|GPIO_PINNO(5)):
	case (GPIO_PORTA|GPIO_PINNO(6)):
	case (GPIO_PORTA|GPIO_PINNO(7)):
		channel = pinno;
		break;
	case (GPIO_PORTB|GPIO_PINNO(0)):
	case (GPIO_PORTB|GPIO_PINNO(1)):
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

	adc = 0;

	// Select the ADC channel to convert
	MODIFY_BITS(ADCx->SQR3, 0b11111 << ADC_SQR3_SQ1_Pos,
		(channel << ADC_SQR3_SQ1_Pos)
		);

	// When ADON is set the first time, wake from power-down mode
	if (!BIT_IS_SET(ADCx->CR2, ADC_CR2_ADON)) {
		SET_BIT(ADCx->CR2, ADC_CR2_ADON);
	}
	// The ST HAL waits for ADON to be set to 1 after setting it to 1;
	// there's nothing in the reference manual about that though.
	while (!BIT_IS_SET(ADCx->CR2, ADC_CR2_ADON)) {
		// Nothing to do here
	}
	dumber_delay(ADC_STAB_TIME_uS * (G_freq_HCLK/1000000U));

	// Conversion can begin when ADON is set the second time after ADC power up
	// If any bit other than ADON is changed when ADON is set, no conversion is
	// triggered.
	SET_BIT(ADCx->CR2, ADC_CR2_ADON);
	SET_BIT(ADCx->CR2, ADC_CR2_EXTTRIG);

	for (uiter_t i = 0; i < ADC_SAMPLE_COUNT; ++i) {
		SET_BIT(ADCx->CR2, ADC_CR2_SWSTART);
		while (!BIT_IS_SET(ADCx->SR, ADC_SR_EOC)) {
			// Nothing to do here
		}
		adc += SELECT_BITS(ADCx->DR, ADC_MAX);
	}
	adc /= ADC_SAMPLE_COUNT;

	CLEAR_BIT(ADCx->CR2, ADC_CR2_ADON|ADC_CR2_EXTTRIG);

	return adc;
}

void adc_read_internals(int16_t *vref, int16_t *tempCx10) {
	adc_t adc;

	assert(vref != NULL);
	assert(tempCx10 != NULL);

#if ADCx_IS_ADC1
	// Enable internal VREF and temperature sensors
	SET_BIT(ADCx->CR2, ADC_CR2_TSVREFE);
	// Rely on the overhead between here and measuring for the startup delay
	// dumber_delay(TEMP_START_TIME_US * (freq_CoreCLK/1000000U));

	adc = adc_read_channel(VREF_CHANNEL);
	// Calculate the ADC Vref by comparing it to the internal Vref
	// adc / max = 1200mV / vref
	// (adc / max) * vref = 1200mV
	// vref = 1200mV / (adc / max)
	// vref = (1200mV * max) / adc
	*vref = (INTERNAL_VREF * ADC_MAX) / (uint32_t )adc;

	adc = adc_read_channel(TEMP_CHANNEL);
	// adc/max = v/vref
	// (adc/max)*vref = v
	// v = (adc*vref)/max
	// From the reference manual:
	// tempC = ((V25-Vsense)/avg_slope)+25
	*tempCx10 = (((TEMP_INT_V25 - ((adc * *vref) / ADC_MAX)) * 1000) / TEMP_INT_SLOPE) + 25;

	CLEAR_BIT(ADCx->CR2, ADC_CR2_TSVREFE);

#else // !ADCx_IS_ADC1
	*vref = REGULATED_VOLTAGE;
	*tempCx10 = 0;
#endif // ADCx_IS_ADC1

	return;
}

#endif // USE_ADC

#ifdef __cplusplus
 }
#endif
