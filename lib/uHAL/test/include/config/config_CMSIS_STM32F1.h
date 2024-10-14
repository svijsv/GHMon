// Use PB6/PB7 instead of PA9/PA10
#undef  GPIO_REMAP_UART1
#define GPIO_REMAP_UART1 1

// Use PB8/PB9 instead of PB6/PB7 (which would conflict with remapped UART1)
#undef  GPIO_REMAP_I2C1
#define GPIO_REMAP_I2C1 1

// Use PB10/PB11 instead of PA2/PA3
#undef GPIO_REMAP_TIM2
#define GPIO_REMAP_TIM2 GPIO_REMAP_PARTIAL_2

#define LED_PIN (PINID_B11)
#define LED_PINCTRL_PIN  (PINID_A0 | GPIO_CTRL_OPENDRAIN | GPIO_CTRL_INVERT)
//#define LED_PINCTRL_PIN  (PINID_A0 | GPIO_CTRL_PUSHPULL)
#define LED_PINCTRL2_PIN LED_PINCTRL_PIN

#define PWM_LED_PIN PINID_B10

//# undef ADC_MAX
//#define ADC_MAX 0x00FF
#define ADC_TEST_PIN PINID_B1
#define ADC_TEST_PIN_GND PINID_A2
#define ADC_TEST_PIN_VCC PINID_A3

#define BUTTON_PIN_UNBIASED PINID_A1
#define BUTTON_PIN (BUTTON_PIN_UNBIASED | GPIO_CTRL_PULLUP | GPIO_CTRL_INVERT)

#undef uHAL_BACKUP_DOMAIN_RESET
#define uHAL_BACKUP_DOMAIN_RESET 0

#undef uHAL_USE_INTERNAL_OSC
#undef uHAL_USE_INTERNAL_LS_OSC
#undef F_CORE
#undef F_HCLK
#undef F_PCLK1
#undef F_PCLK2

#define F_OSC   8000000UL
#define F_CORE 48000000UL
#define uHAL_USE_INTERNAL_OSC 0
// Possible problem with LSE on test board, operates at 1/4 listed speed
#define uHAL_USE_INTERNAL_LS_OSC 1

// Theres a problem with the fake F1 when using 10MHz clock I cant pin down
//    It happens in clocks_init(), ruled out PLL and HCLK divider
#define F_HCLK  12000000UL
//#define F_PCLK1 (F_HCLK/2U)
//#define F_PCLK2 (F_HCLK)
