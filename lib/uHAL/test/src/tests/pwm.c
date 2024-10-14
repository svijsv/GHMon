#include "common.h"

#if TEST_PWM

//
// Globals initialization


//
// main() initialization
void init_PWM(void) {
	return;
}

//
// Main loop
void loop_PWM(void) {
	err_t err;
	static const int_fast16_t pwm_top = PWM_DUTY_CYCLE_SCALE;
	// Don't hit '0' so we can see if there's a problem turning the LED off
	static const int_fast16_t pwm_bottom = PWM_DUTY_CYCLE_SCALE / TEST_PWM_STEPS;
	static int_fast8_t  pwm_dir = -(PWM_DUTY_CYCLE_SCALE / TEST_PWM_STEPS);
	static int_fast16_t pwm_pos = pwm_top;
	static pwm_output_t pwm_led;
	static bool turn_off = false, turn_on = true;

	if (turn_off) {
		err = pwm_off(&pwm_led);
		PRINTF("pwm_off(PWM_LED_PIN) returned %d\r\n", (int )err);
		turn_off = false;
		turn_on = true;

	} else {
		char *fn;

		if (turn_on) {
			err = pwm_on(&pwm_led, PWM_LED_PIN, pwm_pos);
			fn = "on";
			turn_on = false;
		} else {
			err = pwm_set(&pwm_led, pwm_pos);
			fn = "set";
		}
		PRINTF("pwm_%s(PWM_LED_PIN, %d) returned %d\r\n", fn, (int )pwm_pos, (int )err);

		pwm_pos += pwm_dir;
		if (pwm_pos >= pwm_top) {
			pwm_pos = pwm_top;
			pwm_dir = -pwm_dir;
		} else if (pwm_pos <= pwm_bottom) {
			pwm_pos = pwm_bottom;
			pwm_dir = -pwm_dir;
			turn_off = true;
		}
	}

	return;
}

#endif // TEST_PWM
