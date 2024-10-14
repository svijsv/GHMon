typedef struct {
	uint8_t tim_id  : 5;
	uint8_t channel : 3;
} pwm_tim_map_t;

#if HAVE_GPIO_PORTA_PWM
DEBUG_CPP_MSG("Using port A PWM outputs");
static const pwm_tim_map_t port_A_pwm_options_map[16][3] = {
	{{ TIMER_2, 1 }, {  TIMER_5, 1 }, {       0, 0 }}, // 0
	{{ TIMER_2, 2 }, {  TIMER_5, 2 }, {       0, 0 }}, // 1
	{{ TIMER_2, 3 }, {  TIMER_5, 3 }, { TIMER_9, 1 }}, // 2
	{{ TIMER_2, 4 }, {  TIMER_5, 4 }, { TIMER_9, 2 }}, // 3
	{{       0, 0 }, {        0, 0 }, {       0, 0 }}, // 4
	{{ TIMER_2, 1 }, {        0, 0 }, {       0, 0 }}, // 5
	{{ TIMER_3, 1 }, { TIMER_13, 1 }, {       0, 0 }}, // 6
	{{ TIMER_3, 2 }, { TIMER_14, 1 }, {       0, 0 }}, // 7
	{{ TIMER_1, 1 }, {        0, 0 }, {       0, 0 }}, // 8
	{{ TIMER_1, 2 }, {        0, 0 }, {       0, 0 }}, // 9
	{{ TIMER_1, 3 }, {        0, 0 }, {       0, 0 }}, // 10
	{{ TIMER_1, 4 }, {        0, 0 }, {       0, 0 }}, // 11
	{{       0, 0 }, {        0, 0 }, {       0, 0 }}, // 12
	{{       0, 0 }, {        0, 0 }, {       0, 0 }}, // 13
	{{       0, 0 }, {        0, 0 }, {       0, 0 }}, // 14
	{{ TIMER_2, 1 }, {        0, 0 }, {       0, 0 }}, // 15
};
#endif

#if HAVE_GPIO_PORTB_PWM
DEBUG_CPP_MSG("Using port B PWM outputs");
static const pwm_tim_map_t port_B_pwm_options_map[16][3] = {
	{{  TIMER_3, 3 }, {        0, 0 }, {       0, 0 }}, // 0
	{{  TIMER_3, 4 }, {        0, 0 }, {       0, 0 }}, // 1
	{{        0, 0 }, {        0, 0 }, {       0, 0 }}, // 2
	{{  TIMER_2, 2 }, {        0, 0 }, {       0, 0 }}, // 3
	{{  TIMER_3, 1 }, {        0, 0 }, {       0, 0 }}, // 4
	{{  TIMER_3, 2 }, {        0, 0 }, {       0, 0 }}, // 5
	{{  TIMER_4, 1 }, {        0, 0 }, {       0, 0 }}, // 6
	{{  TIMER_4, 2 }, {        0, 0 }, {       0, 0 }}, // 7
	{{  TIMER_4, 3 }, { TIMER_10, 1 }, {       0, 0 }}, // 8
	{{  TIMER_4, 4 }, { TIMER_11, 1 }, {       0, 0 }}, // 9
	{{  TIMER_2, 3 }, {        0, 0 }, {       0, 0 }}, // 10
	{{  TIMER_2, 4 }, {        0, 0 }, {       0, 0 }}, // 11
	{{        0, 0 }, {        0, 0 }, {       0, 0 }}, // 12
	{{        0, 0 }, {        0, 0 }, {       0, 0 }}, // 13
	{{ TIMER_12, 1 }, {        0, 0 }, {       0, 0 }}, // 14
	{{ TIMER_12, 2 }, {        0, 0 }, {       0, 0 }}, // 15
};
#endif

#if HAVE_GPIO_PORTC_PWM
DEBUG_CPP_MSG("Using port C PWM outputs");
static const pwm_tim_map_t port_C_pwm_options_map[16][3] = {
	{{       0, 0 }, {       0, 0 }, {       0, 0 }}, // 0
	{{       0, 0 }, {       0, 0 }, {       0, 0 }}, // 1
	{{       0, 0 }, {       0, 0 }, {       0, 0 }}, // 2
	{{       0, 0 }, {       0, 0 }, {       0, 0 }}, // 3
	{{       0, 0 }, {       0, 0 }, {       0, 0 }}, // 4
	{{       0, 0 }, {       0, 0 }, {       0, 0 }}, // 5
	{{ TIMER_3, 1 }, { TIMER_8, 1 }, {       0, 0 }}, // 6
	{{ TIMER_3, 2 }, { TIMER_8, 2 }, {       0, 0 }}, // 7
	{{ TIMER_3, 3 }, { TIMER_8, 3 }, {       0, 0 }}, // 8
	{{ TIMER_3, 4 }, { TIMER_8, 4 }, {       0, 0 }}, // 9
	{{       0, 0 }, {       0, 0 }, {       0, 0 }}, // 10
	{{       0, 0 }, {       0, 0 }, {       0, 0 }}, // 11
	{{       0, 0 }, {       0, 0 }, {       0, 0 }}, // 12
	{{       0, 0 }, {       0, 0 }, {       0, 0 }}, // 13
	{{       0, 0 }, {       0, 0 }, {       0, 0 }}, // 14
	{{       0, 0 }, {       0, 0 }, {       0, 0 }}, // 15
};
#endif

#if HAVE_GPIO_PORTD_PWM
DEBUG_CPP_MSG("Using port D PWM outputs");
static const pwm_tim_map_t port_D_pwm_options_map[16][3] = {
	{{       0, 0 }, {       0, 0 }, {       0, 0 }}, // 0
	{{       0, 0 }, {       0, 0 }, {       0, 0 }}, // 1
	{{       0, 0 }, {       0, 0 }, {       0, 0 }}, // 2
	{{       0, 0 }, {       0, 0 }, {       0, 0 }}, // 3
	{{       0, 0 }, {       0, 0 }, {       0, 0 }}, // 4
	{{       0, 0 }, {       0, 0 }, {       0, 0 }}, // 5
	{{       0, 0 }, {       0, 0 }, {       0, 0 }}, // 6
	{{       0, 0 }, {       0, 0 }, {       0, 0 }}, // 7
	{{       0, 0 }, {       0, 0 }, {       0, 0 }}, // 8
	{{       0, 0 }, {       0, 0 }, {       0, 0 }}, // 9
	{{       0, 0 }, {       0, 0 }, {       0, 0 }}, // 10
	{{       0, 0 }, {       0, 0 }, {       0, 0 }}, // 11
	{{ TIMER_4, 1 }, {       0, 0 }, {       0, 0 }}, // 12
	{{ TIMER_4, 2 }, {       0, 0 }, {       0, 0 }}, // 13
	{{ TIMER_4, 3 }, {       0, 0 }, {       0, 0 }}, // 14
	{{ TIMER_4, 4 }, {       0, 0 }, {       0, 0 }}, // 15
};
#endif

#if HAVE_GPIO_PORTE_PWM
DEBUG_CPP_MSG("Using port E PWM outputs");
static const pwm_tim_map_t port_E_pwm_options_map[16][3] = {
	{{       0, 0 }, {       0, 0 }, {       0, 0 }}, // 0
	{{       0, 0 }, {       0, 0 }, {       0, 0 }}, // 1
	{{       0, 0 }, {       0, 0 }, {       0, 0 }}, // 2
	{{       0, 0 }, {       0, 0 }, {       0, 0 }}, // 3
	{{       0, 0 }, {       0, 0 }, {       0, 0 }}, // 4
	{{ TIMER_9, 1 }, {       0, 0 }, {       0, 0 }}, // 5
	{{ TIMER_9, 2 }, {       0, 0 }, {       0, 0 }}, // 6
	{{       0, 0 }, {       0, 0 }, {       0, 0 }}, // 7
	{{       0, 0 }, {       0, 0 }, {       0, 0 }}, // 8
	{{ TIMER_1, 1 }, {       0, 0 }, {       0, 0 }}, // 9
	{{       0, 0 }, {       0, 0 }, {       0, 0 }}, // 10
	{{ TIMER_1, 2 }, {       0, 0 }, {       0, 0 }}, // 11
	{{       0, 0 }, {       0, 0 }, {       0, 0 }}, // 12
	{{ TIMER_1, 3 }, {       0, 0 }, {       0, 0 }}, // 13
	{{ TIMER_1, 4 }, {       0, 0 }, {       0, 0 }}, // 14
	{{       0, 0 }, {       0, 0 }, {       0, 0 }}, // 15
};
#endif

#if HAVE_GPIO_PORTF_PWM
DEBUG_CPP_MSG("Using port F PWM outputs");
static const pwm_tim_map_t port_F_pwm_options_map[16][3] = {
	{{        0, 0 }, {       0, 0 }, {       0, 0 }}, // 0
	{{        0, 0 }, {       0, 0 }, {       0, 0 }}, // 1
	{{        0, 0 }, {       0, 0 }, {       0, 0 }}, // 2
	{{        0, 0 }, {       0, 0 }, {       0, 0 }}, // 3
	{{        0, 0 }, {       0, 0 }, {       0, 0 }}, // 4
	{{        0, 0 }, {       0, 0 }, {       0, 0 }}, // 5
	{{ TIMER_10, 1 }, {       0, 0 }, {       0, 0 }}, // 6
	{{ TIMER_11, 1 }, {       0, 0 }, {       0, 0 }}, // 7
	{{ TIMER_13, 1 }, {       0, 0 }, {       0, 0 }}, // 8
	{{ TIMER_14, 1 }, {       0, 0 }, {       0, 0 }}, // 9
	{{        0, 0 }, {       0, 0 }, {       0, 0 }}, // 10
	{{        0, 0 }, {       0, 0 }, {       0, 0 }}, // 11
	{{        0, 0 }, {       0, 0 }, {       0, 0 }}, // 12
	{{        0, 0 }, {       0, 0 }, {       0, 0 }}, // 13
	{{        0, 0 }, {       0, 0 }, {       0, 0 }}, // 14
	{{        0, 0 }, {       0, 0 }, {       0, 0 }}, // 15
};
#endif

#if HAVE_GPIO_PORTH_PWM
DEBUG_CPP_MSG("Using port H PWM outputs");
static const pwm_tim_map_t port_H_pwm_options_map[16][3] = {
	{{        0, 0 }, {       0, 0 }, {       0, 0 }}, // 0
	{{        0, 0 }, {       0, 0 }, {       0, 0 }}, // 1
	{{        0, 0 }, {       0, 0 }, {       0, 0 }}, // 2
	{{        0, 0 }, {       0, 0 }, {       0, 0 }}, // 3
	{{        0, 0 }, {       0, 0 }, {       0, 0 }}, // 4
	{{        0, 0 }, {       0, 0 }, {       0, 0 }}, // 5
	{{ TIMER_12, 1 }, {       0, 0 }, {       0, 0 }}, // 6
	{{        0, 0 }, {       0, 0 }, {       0, 0 }}, // 7
	{{        0, 0 }, {       0, 0 }, {       0, 0 }}, // 8
	{{ TIMER_12, 2 }, {       0, 0 }, {       0, 0 }}, // 9
	{{  TIMER_5, 1 }, {       0, 0 }, {       0, 0 }}, // 10
	{{  TIMER_5, 2 }, {       0, 0 }, {       0, 0 }}, // 11
	{{  TIMER_5, 3 }, {       0, 0 }, {       0, 0 }}, // 12
	{{        0, 0 }, {       0, 0 }, {       0, 0 }}, // 13
	{{        0, 0 }, {       0, 0 }, {       0, 0 }}, // 14
	{{        0, 0 }, {       0, 0 }, {       0, 0 }}, // 15
};
#endif

#if HAVE_GPIO_PORTI_PWM
DEBUG_CPP_MSG("Using port I PWM outputs");
static const pwm_tim_map_t port_I_pwm_options_map[16][3] = {
	{{ TIMER_5, 4 }, {       0, 0 }, {       0, 0 }}, // 0
	{{       0, 0 }, {       0, 0 }, {       0, 0 }}, // 1
	{{ TIMER_8, 4 }, {       0, 0 }, {       0, 0 }}, // 2
	{{       0, 0 }, {       0, 0 }, {       0, 0 }}, // 3
	{{       0, 0 }, {       0, 0 }, {       0, 0 }}, // 4
	{{ TIMER_8, 1 }, {       0, 0 }, {       0, 0 }}, // 5
	{{ TIMER_8, 2 }, {       0, 0 }, {       0, 0 }}, // 6
	{{ TIMER_8, 3 }, {       0, 0 }, {       0, 0 }}, // 7
	{{       0, 0 }, {       0, 0 }, {       0, 0 }}, // 8
	{{       0, 0 }, {       0, 0 }, {       0, 0 }}, // 9
	{{       0, 0 }, {       0, 0 }, {       0, 0 }}, // 10
	{{       0, 0 }, {       0, 0 }, {       0, 0 }}, // 11
	{{       0, 0 }, {       0, 0 }, {       0, 0 }}, // 12
	{{       0, 0 }, {       0, 0 }, {       0, 0 }}, // 13
	{{       0, 0 }, {       0, 0 }, {       0, 0 }}, // 14
	{{       0, 0 }, {       0, 0 }, {       0, 0 }}, // 15
};
#endif


static TIM_TypeDef* get_pwm_tim_from_id(uint_fast8_t tim_id) {
	switch (tim_id) {
#if USE_TIMER1_PWM
	case TIMER_1:
		return TIM1;
#endif
#if USE_TIMER2_PWM
	case TIMER_2:
		return TIM2;
#endif
#if USE_TIMER3_PWM
	case TIMER_3:
		return TIM3;
#endif
#if USE_TIMER4_PWM
	case TIMER_4:
		return TIM4;
#endif
#if USE_TIMER5_PWM
	case TIMER_5:
		return TIM5;
#endif
#if USE_TIMER6_PWM
	case TIMER_6:
		return TIM6;
#endif
#if USE_TIMER7_PWM
	case TIMER_7:
		return TIM7;
#endif
#if USE_TIMER8_PWM
	case TIMER_8:
		return TIM8;
#endif
#if USE_TIMER9_PWM
	case TIMER_9:
		return TIM9;
#endif
#if USE_TIMER10_PWM
	case TIMER_10:
		return TIM10;
#endif
#if USE_TIMER11_PWM
	case TIMER_11:
		return TIM11;
#endif
#if USE_TIMER12_PWM
	case TIMER_12:
		return TIM12;
#endif
#if USE_TIMER13_PWM
	case TIMER_13:
		return TIM13;
#endif
#if USE_TIMER14_PWM
	case TIMER_14:
		return TIM14;
#endif
	}

	return NULL;
}

static TIM_TypeDef* find_available_pin_pwm_tim(gpio_pin_t pin, uint_fast8_t *tim_id, uint_fast8_t *channel, gpio_af_t *af_code, rcc_periph_t *rcc) {
	TIM_TypeDef *TIMx = NULL;
	gpio_pin_t pinno;
	uint32_t enr_bit = 0;
	const pwm_tim_map_t (*map)[16][3];

	pinno = GPIO_GET_PINNO(pin);
	switch (GPIO_GET_PORTMASK(pin)) {
#if HAVE_GPIO_PORTA_PWM
	case GPIO_PORTA_MASK:
		map = &port_A_pwm_options_map;
		break;
#endif
#if HAVE_GPIO_PORTB_PWM
	case GPIO_PORTB_MASK:
		map = &port_B_pwm_options_map;
		break;
#endif
#if HAVE_GPIO_PORTC_PWM
	case GPIO_PORTC_MASK:
		map = &port_C_pwm_options_map;
		break;
#endif
#if HAVE_GPIO_PORTD_PWM
	case GPIO_PORTD_MASK:
		map = &port_D_pwm_options_map;
		break;
#endif
#if HAVE_GPIO_PORTE_PWM
	case GPIO_PORTE_MASK:
		map = &port_E_pwm_options_map;
		break;
#endif
#if HAVE_GPIO_PORTF_PWM
	case GPIO_PORTF_MASK:
		map = &port_F_pwm_options_map;
		break;
#endif
#if HAVE_GPIO_PORTH_PWM
	case GPIO_PORTH_MASK:
		map = &port_H_pwm_options_map;
		break;
#endif
#if HAVE_GPIO_PORTI_PWM
	case GPIO_PORTI_MASK:
		map = &port_I_pwm_options_map;
		break;
#endif
		default:
			return NULL;
	}

	for (uiter_t i = 0; i < 3; ++i) {
		uint_fast8_t t, c;

		t = (*map)[pinno][i].tim_id;
		TIMx = get_pwm_tim_from_id(t);
		if (TIMx == NULL) {
			continue;
		}

		c = (*map)[pinno][i].channel;
		switch (c) {
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

		if (!BIT_IS_SET(TIMx->CCER, enr_bit)) {
			if (tim_id != NULL) {
				*tim_id = t;
			}
			if (channel != NULL) {
				*channel = c;
			}
			if (af_code != NULL) {
				*af_code = get_af_from_id(t);
			}
			if (rcc != NULL) {
				*rcc = get_rcc_from_id(t);
			}
			return TIMx;
		}
	}

	return NULL;
}
