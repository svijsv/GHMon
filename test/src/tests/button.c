#include "common.h"

#if TEST_BUTTON

//DEBUG_CPP_MACRO(BUTTON_PIN);

#if HAVE_STM32
# if GPIO_GET_PINNO(BUTTON_PIN) == 0
#  define BUTTON_ISR EXTI0_IRQHandler
# elif GPIO_GET_PINNO(BUTTON_PIN) == 1
#  define BUTTON_ISR EXTI1_IRQHandler
# elif GPIO_GET_PINNO(BUTTON_PIN) == 2
#  define BUTTON_ISR EXTI2_IRQHandler
# elif GPIO_GET_PINNO(BUTTON_PIN) == 3
#  define BUTTON_ISR EXTI3_IRQHandler
# elif GPIO_GET_PINNO(BUTTON_PIN) == 4
#  define BUTTON_ISR EXTI4_IRQHandler
# elif (GPIO_GET_PINNO(BUTTON_PIN) >= 5) && (GPIO_GET_PINNO(BUTTON_PIN) <= 9)
#  define BUTTON_ISR EXTI9_5_IRQHandler
# elif (GPIO_GET_PINNO(BUTTON_PIN) >= 10) && (GPIO_GET_PINNO(BUTTON_PIN) <= 15)
#  define BUTTON_ISR EXTI15_10_IRQHandler
# else
#  error "Can't determine user button"
# endif // BUTTON_BIN number
# define CLEAR_BUTTON_ISR() (void )0U
#endif // HAVE_STM32

#if HAVE_AVR_XMEGA3
# include <avr/interrupt.h> // For button interrupt handling

# if (GPIO_GET_PORTNO(BUTTON_PIN) == GPIO_PORTA)
#  define BUTTON_ISR PORTA_PORT_vect
#  define BUTTON_PORT PORTA
# elif (GPIO_GET_PORTNO(BUTTON_PIN) == GPIO_PORTB)
#  define BUTTON_ISR PORTB_PORT_vect
#  define BUTTON_PORT PORTB
# else
#  error "BUTTON_PIN is unhandled"
# endif
  // Write '1' to a flag to clear it
# define CLEAR_BUTTON_ISR() do { BUTTON_PORT.INTFLAGS = GPIO_GET_PINMASK(BUTTON_PIN); } while (0)
#endif // HAVE_AVR_XMEGA3

DEBUG_CPP_MACRO(BUTTON_ISR);


//
// Globals initialization
static volatile uint8_t button_pressed = 0;
static gpio_listen_t button_listen_handle;


//
// Misc Functions
ISR(BUTTON_ISR) {
	CLEAR_BUTTON_ISR();

	// Need to turn the interrupt off to clear interrupt and keep it off so button
	// bounce doesn't re-trigger
	gpio_listen_off(&button_listen_handle);

	button_pressed = 1;
	uHAL_SET_STATUS(uHAL_FLAG_IRQ);

	return;
}

static void button_handler(void) {
	utime_t timeout;

	//button_pressed = 1;
	// First timeout is shorter to account for time taken to wake up
	timeout = SET_TIMEOUT_MS(TEST_BUTTON_PRESS_PERIOD_MS-50);

#if TEST_BUTTON_DEBOUNCE_MS
	delay_ms(TEST_BUTTON_DEBOUNCE_MS);
#endif

	// Use a delay of 10ms to limit the influence of delay_ms() overhead
	do {
		if (TIMES_UP(timeout)) {
			++button_pressed;
			// button_pressed is always '1' when we enter this function so setting
			// the max count to 0 will automatically disable it (at least until
			// button_pressed rolls over)
			if (button_pressed == TEST_BUTTON_PRESS_PERIOD_MAX_COUNT) {
				PRINTF("Max button duration\r\n");
				break;
			}
			timeout = SET_TIMEOUT_MS(TEST_BUTTON_PRESS_PERIOD_MS);
		}
		delay_ms(10);
	} while (input_pin_is_on(BUTTON_PIN));

	// It bounces when released too
#if TEST_BUTTON_DEBOUNCE_MS
	delay_ms(TEST_BUTTON_DEBOUNCE_MS);
#endif

	uHAL_CLEAR_STATUS(uHAL_FLAG_IRQ);
	return;
}


//
// main() initialization
void init_BUTTON(void) {
	input_pin_on(BUTTON_PIN);
	input_pin_listen_init(&button_listen_handle, BUTTON_PIN);
	input_pin_listen_on(&button_listen_handle);

	return;
}


//
// Main loop
void loop_BUTTON(void) {
	if (button_pressed) {
		button_handler();
		PRINTF("GOT BUTTON PRESS lasting %u intervals\r\n", (uint_t )button_pressed);
		button_pressed = 0;
		input_pin_listen_on(&button_listen_handle);
	}

	return;
}


#endif // TEST_BUTTON
