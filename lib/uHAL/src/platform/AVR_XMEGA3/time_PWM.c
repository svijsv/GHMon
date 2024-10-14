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
// time.c
// Manage the time-keeping peripherals
//
// NOTES:
//

#include "time_private.h"

#if uHAL_USE_PWM

#include "time_TCA.h"
#include "time_TCB.h"

//
// PWM stuff
//
err_t pwm_set(pwm_output_t *output, uint_fast16_t duty_cycle) {
	gpio_pin_t pin;

	assert(output != NULL);
	assert(PINID(output->pin) != 0);
	assert(duty_cycle <= PWM_DUTY_CYCLE_SCALE);

#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (output == NULL || PINID(output->pin) == 0) {
		return ERR_BADARG;
	}
#endif
	if (duty_cycle > PWM_DUTY_CYCLE_SCALE) {
		duty_cycle = PWM_DUTY_CYCLE_SCALE;
	}
	pin = output->pin;

	switch (PINID(pin)) {
#if USE_SPLIT_TCA0
	case PINID_TCA0_WO0:
		return TCA_pwm_set(&TCA0, TCA_SPLIT_LCMP0EN_bm, duty_cycle);
	case PINID_TCA0_WO1:
		return TCA_pwm_set(&TCA0, TCA_SPLIT_LCMP1EN_bm, duty_cycle);
	case PINID_TCA0_WO2:
		return TCA_pwm_set(&TCA0, TCA_SPLIT_LCMP2EN_bm, duty_cycle);
	// On the ATTiny402 WO0 and WO3 are the same output, this will differ by
	// MCU
# if PINID_TCA0_WO3 != PINID_TCA0_WO0
	case PINID_TCA0_WO3:
		return TCA_pwm_set(&TCA0, TCA_SPLIT_HCMP0EN_bm, duty_cycle);
# endif
# if PINID_TCA0_WO4
	case PINID_TCA0_WO4:
		return TCA_pwm_set(&TCA0, TCA_SPLIT_HCMP1EN_bm, duty_cycle);
# endif
# if PINID_TCA0_WO5
	case PINID_TCA0_WO5:
		return TCA_pwm_set(&TCA0, TCA_SPLIT_HCMP2EN_bm, duty_cycle);
# endif
#else
	case PINID_TCA0_WO0:
		return TCA_pwm_set(&TCA0, TCA_SINGLE_CMP0EN_bm, duty_cycle);
	case PINID_TCA0_WO1:
		return TCA_pwm_set(&TCA0, TCA_SINGLE_CMP1EN_bm, duty_cycle);
	case PINID_TCA0_WO2:
		return TCA_pwm_set(&TCA0, TCA_SINGLE_CMP2EN_bm, duty_cycle);
#endif
#if ! DISABLE_TCB0_PWM
	case PINID_TCB0_WO:
		return TCB_pwm_set(&TCB0, duty_cycle);
#endif
#if ! DISABLE_TCB1_PWM
	case PINID_TCB1_WO:
		return TCB_pwm_set(&TCB1, duty_cycle);
#endif
#if ! DISABLE_TCB2_PWM
	case PINID_TCB2_WO:
		return TCB_pwm_set(&TCB2, duty_cycle);
#endif
#if ! DISABLE_TCB3_PWM
	case PINID_TCB3_WO:
		return TCB_pwm_set(&TCB3, duty_cycle);
#endif
	}

	return ERR_NOTSUP;
}

err_t pwm_on(pwm_output_t *output, gpio_pin_t pin, uint_fast16_t duty_cycle) {
	err_t ret = ERR_NOTSUP;

	if (pin == 0 && output != NULL) {
		pin = output->pin;
	}

	assert(output != NULL);
	assert(duty_cycle <= PWM_DUTY_CYCLE_SCALE);

#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (output == NULL) {
		return ERR_BADARG;
	}
#endif
	if (duty_cycle > PWM_DUTY_CYCLE_SCALE) {
		duty_cycle = PWM_DUTY_CYCLE_SCALE;
	}
	output->pin = pin;

	switch (PINID(pin)) {
#if USE_SPLIT_TCA0
	case PINID_TCA0_WO0:
		ret = TCA_pwm_on(&TCA0, TCA_SPLIT_LCMP0EN_bm, duty_cycle);
		break;
	case PINID_TCA0_WO1:
		ret = TCA_pwm_on(&TCA0, TCA_SPLIT_LCMP1EN_bm, duty_cycle);
		break;
	case PINID_TCA0_WO2:
		ret = TCA_pwm_on(&TCA0, TCA_SPLIT_LCMP2EN_bm, duty_cycle);
		break;
# if PINID_TCA0_WO3 != PINID_TCA0_WO0
	case PINID_TCA0_WO3:
		ret = TCA_pwm_on(&TCA0, TCA_SPLIT_HCMP0EN_bm, duty_cycle);
		break;
# endif
# if PINID_TCA0_WO4
	case PINID_TCA0_WO4:
		ret = TCA_pwm_on(&TCA0, TCA_SPLIT_HCMP1EN_bm, duty_cycle);
		break;
# endif
# if PINID_TCA0_WO5
	case PINID_TCA0_WO5:
		ret = TCA_pwm_on(&TCA0, TCA_SPLIT_HCMP2EN_bm, duty_cycle);
		break;
# endif
#else
	case PINID_TCA0_WO0:
		ret = TCA_pwm_on(&TCA0, TCA_SINGLE_CMP0EN_bm, duty_cycle);
		break;
	case PINID_TCA0_WO1:
		ret = TCA_pwm_on(&TCA0, TCA_SINGLE_CMP1EN_bm, duty_cycle);
		break;
	case PINID_TCA0_WO2:
		ret = TCA_pwm_on(&TCA0, TCA_SINGLE_CMP2EN_bm, duty_cycle);
		break;
#endif
#if ! DISABLE_TCB0_PWM
	case PINID_TCB0_WO:
		ret = TCB_pwm_on(&TCB0, duty_cycle);
		break;
#endif
#if ! DISABLE_TCB1_PWM
	case PINID_TCB1_WO:
		ret = TCB_pwm_on(&TCB1, duty_cycle);
		break;
#endif
#if ! DISABLE_TCB2_PWM
	case PINID_TCB2_WO:
		ret = TCB_pwm_on(&TCB2, duty_cycle);
		break;
#endif
#if ! DISABLE_TCB3_PWM
	case PINID_TCB3_WO:
		ret = TCB_pwm_on(&TCB3, duty_cycle);
		break;
#endif
	}

	if (ret == ERR_OK) {
		gpio_set_mode(pin, GPIO_MODE_PP, GPIO_LOW);
	}

	return ret;
}
err_t pwm_off(pwm_output_t *output) {
	gpio_pin_t pin;

	assert(output != NULL);
	assert(PINID(output->pin) != 0);

#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (output == NULL || PINID(output->pin) == 0) {
		return ERR_BADARG;
	}
#endif
	pin = output->pin;

	gpio_set_mode(pin, GPIO_MODE_RESET, GPIO_FLOAT);

	switch (PINID(pin)) {
#if USE_SPLIT_TCA0
	case PINID_TCA0_WO0:
		return TCA_pwm_off(&TCA0, TCA_SPLIT_LCMP0EN_bm);
	case PINID_TCA0_WO1:
		return TCA_pwm_off(&TCA0, TCA_SPLIT_LCMP1EN_bm);
	case PINID_TCA0_WO2:
		return TCA_pwm_off(&TCA0, TCA_SPLIT_LCMP2EN_bm);
# if PINID_TCA0_WO3 != PINID_TCA0_WO0
	case PINID_TCA0_WO3:
		return TCA_pwm_off(&TCA0, TCA_SPLIT_HCMP0EN_bm);
# endif
# if PINID_TCA0_WO4
	case PINID_TCA0_WO4:
		return TCA_pwm_off(&TCA0, TCA_SPLIT_HCMP1EN_bm);
# endif
# if PINID_TCA0_WO5
	case PINID_TCA0_WO5:
		return TCA_pwm_off(&TCA0, TCA_SPLIT_HCMP2EN_bm);
# endif
#else
	case PINID_TCA0_WO0:
		return TCA_pwm_off(&TCA0, TCA_SINGLE_CMP0EN_bm);
	case PINID_TCA0_WO1:
		return TCA_pwm_off(&TCA0, TCA_SINGLE_CMP1EN_bm);
	case PINID_TCA0_WO2:
		return TCA_pwm_off(&TCA0, TCA_SINGLE_CMP2EN_bm);
#endif
#if ! DISABLE_TCB0_PWM
	case PINID_TCB0_WO:
		return TCB_pwm_off(&TCB0);
#endif
#if ! DISABLE_TCB1_PWM
	case PINID_TCB1_WO:
		return TCB_pwm_off(&TCB1);
#endif
#if ! DISABLE_TCB2_PWM
	case PINID_TCB2_WO:
		return TCB_pwm_off(&TCB2);
#endif
#if ! DISABLE_TCB3_PWM
	case PINID_TCB3_WO:
		return TCB_pwm_off(&TCB3);
#endif
	}

	return ERR_NOTSUP;
}

#endif // uHAL_USE_PWM
