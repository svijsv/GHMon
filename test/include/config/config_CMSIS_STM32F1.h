//
// F1-line STM32s only allow remapping peripheral pins in batches. In order
// to use the remapped pins, set the corresponding flag. Check the datasheet
// for pin mappings.
// Possible values are GPIO_REMAP_{DEFAULT,FULL,PARTIAL,PARTIAL_1,PARTIAL_2}
// Other STM32 lines don't use these flags.
#define GPIO_REMAP_SPI1  0
#define GPIO_REMAP_I2C1  1 // Use PB8/PB9 instead of PB6/PB7 (which would conflict with remapped UART1)
#define GPIO_REMAP_UART1 1 // Use PB6/PB7 instead of PA9/PA10
#define GPIO_REMAP_UART2 0
#define GPIO_REMAP_UART3 0
#define GPIO_REMAP_TIM1  0
#define GPIO_REMAP_TIM2  GPIO_REMAP_PARTIAL_2 // Use PB10/PB11 instead of PA2/PA3
#define GPIO_REMAP_TIM3  0
#define GPIO_REMAP_TIM4  0
#define GPIO_REMAP_TIM9  0
#define GPIO_REMAP_TIM10 0
#define GPIO_REMAP_TIM11 0
#define GPIO_REMAP_TIM13 0
#define GPIO_REMAP_TIM14 0
//
// If non-zero, any GPIO port which the device header files expose will be
// enabled. Otherwise, any port used needs to be explicitly enabled by setting
// HAVE_GPIO_PORTx to non-zero either on the command line or in this configuration
// file
#define HAVE_GPIO_PORT_DEFAULT 0
#define HAVE_GPIO_PORTA 1
#define HAVE_GPIO_PORTB 1
//
// When non-zero, disable PWM outputs on a given port
// This has the effect of reducing flash memory usage by 48 bytes per disabled
// port on non-F1 devices
// It's not necessary to explicitly disable PWM on a port if the port wasn't
// enabled above for general GPIO usage
#define DISABLE_PORTA_PWM 0
#define DISABLE_PORTB_PWM 0
//
// UART serial console pins
#define UART_COMM_TX_PIN PINID_UART1_TX
#define UART_COMM_RX_PIN PINID_UART1_RX
//
// SPI pins
#define SPI_SS_PIN    PINID_SPI1_NSS
#define SPI_SCK_PIN   PINID_SPI1_SCK
#define SPI_MISO_PIN  PINID_SPI1_MISO
#define SPI_MOSI_PIN  PINID_SPI1_MOSI
#define SPI_CS_SD_PIN SPI_SS_PIN
//
// I2C pins
#define I2C_SDA_PIN PINID_I2C1_SDA
#define I2C_SCL_PIN PINID_I2C1_SCL


#define LED_PIN (PINID_B11)
#define LED_PINCTRL_PIN  (PINID_A0 | GPIO_CTRL_OPENDRAIN | GPIO_CTRL_INVERT)
//#define LED_PINCTRL_PIN  (PINID_A0 | GPIO_CTRL_PUSHPULL)
#define LED_PINCTRL2_PIN LED_PINCTRL_PIN

#define PWM_LED_PIN PINID_B10

//#define ADC_MAX 0x00FF
#define ADC_TEST_PIN PINID_B1
#define ADC_TEST_PIN_GND PINID_A2
#define ADC_TEST_PIN_VCC PINID_A3

#define BUTTON_PIN_UNBIASED PINID_A1
#define BUTTON_PIN (BUTTON_PIN_UNBIASED | GPIO_CTRL_PULLUP | GPIO_CTRL_INVERT)

#define F_OSC   8000000UL
#define F_CORE 48000000UL
#define uHAL_USE_INTERNAL_OSC 0
// Problem with LSE on test board, operates at 1/4 listed speed
#define uHAL_USE_INTERNAL_LS_OSC 1

// Theres a problem with the fake F1 when using 10MHz clock I cant pin down
//    It happens in clocks_init(), ruled out PLL and HCLK divider
#define F_HCLK  12000000UL
//#define F_PCLK1 (F_HCLK/2U)
//#define F_PCLK2 (F_HCLK)
