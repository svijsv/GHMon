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
#if USE_STM32F1_ADC
# define TEMP_CHANNEL 16
#else
# define TEMP_CHANNEL 18
#endif

#if defined(STM32F1)
// Datasheet value "Avg_Slope" converted from mV to uV
# define TEMP_INT_SLOPE_uV 4300
// Datasheet value "V25" converted from V to mV
# define TEMP_INT_T25_MV   1430
// Temperature sensor startup time specified as Tstart in the datasheet
# define TEMP_START_TIME_u S 10
// The recommended sample time for the internal temperature
# define TEMP_SAMPLE_uS      17

// ADC stabilization time is specified under Tstab in the datasheet
# define ADC_STAB_TIME_uS 1

#else // ! STM32F1
# define TEMP_INT_SLOPE_uV  2500
# define TEMP_INT_T25_mV     760
# define TEMP_START_TIME_uS   10
// Datasheet gives a minimum sample time of 10uS but no max
# define TEMP_SAMPLE_uS       20

# define ADC_STAB_TIME_uS 3
#endif // ! STM32F1


#if USE_STM32F1_ADC
# define SAMPLE_TIME_1_5   0b000 //  1.5 cycles
# define SAMPLE_TIME_7_5   0b001 //  7.5 cycles
# define SAMPLE_TIME_13_5  0b010 // 13.5 cycles
# define SAMPLE_TIME_28_5  0b011 // 28.5 cycles
# define SAMPLE_TIME_41_5  0b100 // 41.5 cycles
# define SAMPLE_TIME_55_5  0b101 // 55.5 cycles
# define SAMPLE_TIME_71_5  0b110 // 71.5 cycles
# define SAMPLE_TIME_239_5 0b111 // 239.5 cycles
// The value of a register set entirely to the default sample time
// 239.5 cycles:
//# define SAMPLE_TIME_REG 0x3FFFFFFF
// 71.5 cycles:
# define SAMPLE_TIME_REG 0x36DB6DB6
# define SMPR1_MASK 0x00FFFFFF
# define SMPR2_MASK 0x3FFFFFFF

#else
# define SAMPLE_TIME_3   0b000
# define SAMPLE_TIME_15  0b001
# define SAMPLE_TIME_28  0b010
# define SAMPLE_TIME_56  0b011
# define SAMPLE_TIME_84  0b100
# define SAMPLE_TIME_112 0b101
# define SAMPLE_TIME_144 0b110
# define SAMPLE_TIME_480 0b111
// The value of a register set entirely to the default sample time
// 480 cycles:
//# define SAMPLE_TIME_REG 0x3FFFFFFF
// 84 cycles:
# define SAMPLE_TIME_REG 0x24924924
# define SMPR1_MASK 0x07FFFFFF
# define SMPR2_MASK 0x3FFFFFFF
#endif

#if   G_freq_ADC == (ADCx_BUSFREQ / 2)
# define PRESCALER ADC_PRESCALER_2
#elif G_freq_ADC == (ADCx_BUSFREQ / 4)
# define PRESCALER ADC_PRESCALER_4
#elif G_freq_ADC == (ADCx_BUSFREQ / 6)
# define PRESCALER ADC_PRESCALER_6
#elif G_freq_ADC == (ADCx_BUSFREQ / 8)
# define PRESCALER ADC_PRESCALER_8
#else
# error "G_freq_ADC must be a G_freq_PCLK2 / (2|4|6|8)"
#endif


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
	clock_init(ADCx_CLOCKEN);

	// Set the sample time registers
	// If the sample time might be shorter than the minimum temperature sample
	// time and the temperature might be read, it's sample time should be set
	// separately.
	ADCx->SMPR1 = SAMPLE_TIME_REG & SMPR1_MASK;
	ADCx->SMPR2 = SAMPLE_TIME_REG & SMPR2_MASK;

	MODIFY_BITS(ADC_PRESCALER_REG, ADC_PRESCALER_Msk, PRESCALER);

#if USE_STM32F1_ADC
	MODIFY_BITS(ADCx->CR2, ADC_CR2_CONT|ADC_CR2_EXTSEL|ADC_CR2_EXTTRIG,
		(0b0   << ADC_CR2_CONT_Pos  ) | // Keep at 0 for single conversion mode
		(0b111 << ADC_CR2_EXTSEL_Pos) | // Enable software start
		(0b1   << ADC_CR2_EXTTRIG_Pos)| // Enable the trigger
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
		(0b0 << ADC_CR2_CONT_Pos  ) | // Keep at 0 for single conversion mode
		0);
#endif // ! STM32F1

	adc_off();

	return;
}
void adc_on(void) {
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
	dumb_delay_cycles(ADC_STAB_TIME_uS * (G_freq_HCLK/1000000U));

	return;
}
void adc_off(void) {
	CLEAR_BIT(ADCx->CR2, ADC_CR2_ADON);
	clock_disable(ADCx_CLOCKEN);

	return;
}
static uint8_t pin_to_channel(pin_t pin) {
	uint8_t pinno;

	// This is cheating, but we know that channels 0-7 are the first 8 pins
	// on port A and 8 and 9 are the first 2 on port B so we can just do this
	// to get from pin to channel
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
		//return pinno;
		break;
	case PINID_B0:
	case PINID_B1:
		//return (pinno + 8);
		pinno += + 8;
		break;
	default:
		LOGGER("Attempted ADC read on invalid pin 0x%02X", (uint )pin);
		pinno = 0xFF;
		break;
	}

	return pinno;
}
adc_t adc_read_pin(pin_t pin) {
	return adc_read_channel(pin_to_channel(pin));
}
static adc_t adc_read_channel(uint32_t channel) {
	adcm_t adc;

	// Five bits of channel selection
	if (channel > 0b11111) {
		return 0;
	}

	// Select the ADC channel to convert
	MODIFY_BITS(ADCx->SQR3, 0b11111 << ADC_SQR3_SQ1_Pos,
		(channel << ADC_SQR3_SQ1_Pos)
		);

#if USE_STM32F1_ADC
	// Conversion can begin when ADON is set the second time after ADC power up
	// If any bit other than ADON is changed when ADON is set, no conversion is
	// triggered.
	SET_BIT(ADCx->CR2, ADC_CR2_ADON);
#endif

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

	// Enable internal VREF and temperature sensors
#if USE_STM32F1_ADC
	SET_BIT(ADCx->CR2, ADC_CR2_TSVREFE);
#else
	SET_BIT(ADCx_COMMON->CCR, ADC_CCR_TSVREFE);
#endif

	adc = adc_read_channel(VREF_CHANNEL);
	// Calculate the ADC Vref by comparing it to the internal Vref
	// adc / max = 1200mV / vref
	// (adc / max) * vref = 1200mV
	// vref = 1200mV / (adc / max)
	// vref = (1200mV * max) / adc
	vref = (INTERNAL_VREF_mV * ADC_MAX) / (uint32_t )adc;

	// Disable internal VREF and temperature sensors
#if USE_STM32F1_ADC
	CLEAR_BIT(ADCx->CR2, ADC_CR2_TSVREFE);
#else
	CLEAR_BIT(ADCx_COMMON->CCR, ADC_CCR_TSVREFE);
#endif

	return vref;
}

adc_t adc_read_ac_amplitude(pin_t pin, uint32_t period_ms) {
	uint8_t channel;
	adc_t adc, adc_min, adc_max;
	adcm_t adcm_min, adcm_max;
	utime_t timeout;

	channel = pin_to_channel(pin);
	// Five bits of channel selection
	if (channel > 0b11111) {
		return 0;
	}

	// Select the ADC channel to convert
	MODIFY_BITS(ADCx->SQR3, 0b11111 << ADC_SQR3_SQ1_Pos,
		(channel << ADC_SQR3_SQ1_Pos)
		);
	// Use continuous conversion mode
	SET_BIT(ADCx->CR2, ADC_CR2_CONT);

#if USE_STM32F1_ADC
	// Conversion can begin when ADON is set the second time after ADC power up
	// If any bit other than ADON is changed when ADON is set, no conversion is
	// triggered.
	SET_BIT(ADCx->CR2, ADC_CR2_ADON);
#endif

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
	CLEAR_BIT(ADCx->CR2, ADC_CR2_CONT);
	// Wait for final conversion to finish
	while (!BIT_IS_SET(ADCx->SR, ADC_SR_EOC)) {
		// Nothing to do here
	}
	CLEAR_BIT(ADCx->SR, ADC_SR_EOC);

	adcm_max /= ADC_SAMPLE_COUNT;
	adcm_min /= ADC_SAMPLE_COUNT;
	return (adcm_max - adcm_min)/2;
}



#endif // USE_ADC

#ifdef __cplusplus
 }
#endif
