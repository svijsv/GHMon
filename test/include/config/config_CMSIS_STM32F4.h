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
#define UART_COMM_TX_PIN PINID_B6 // B6, A9
#define UART_COMM_RX_PIN PINID_B7 // B7, A10
//
// SPI pins
#define SPI_SS_PIN    PINID_SPI1_NSS
#define SPI_SCK_PIN   PINID_SPI1_SCK
#define SPI_MISO_PIN  PINID_SPI1_MISO
#define SPI_MOSI_PIN  PINID_SPI1_MOSI
#define SPI_CS_SD_PIN SPI_SS_PIN
//
// I2C pins
#define I2C_SDA_PIN PINID_B9 // B9, B7
#define I2C_SCL_PIN PINID_B8 // B8, B6


#define LED_PIN (PINID_A12)
#define LED_PINCTRL_PIN  (PINID_A11 | GPIO_CTRL_OPENDRAIN)
//#define LED_PINCTRL_PIN  (PINID_A11 | GPIO_CTRL_OPENDRAIN | GPIO_CTRL_INVERT)
//#define LED_PINCTRL_PIN  (PINID_A0 | GPIO_CTRL_PUSHPULL)
#define LED_PINCTRL2_PIN LED_PINCTRL_PIN

#define PWM_LED_PIN PINID_A8
//#define PWM_LED_PIN PINID_A3

//#define ADC_MAX 0x00FF
#define ADC_TEST_PIN PINID_A0 // Vcc -> 22K -> 8.1K -> Gnd
#define ADC_TEST_PIN_GND PINID_A1
#define ADC_TEST_PIN_VCC PINID_A2

#define BUTTON_PIN_UNBIASED PINID_A3
#define BUTTON_PIN (BUTTON_PIN_UNBIASED | GPIO_CTRL_PULLUP | GPIO_CTRL_INVERT)

#define F_OSC  25000000UL
#define F_CORE 48000000UL
#define uHAL_USE_INTERNAL_OSC 0
#define uHAL_USE_INTERNAL_LS_OSC 0

#define F_HCLK  48000000UL
//#define F_PCLK1 (F_HCLK/2U)
//#define F_PCLK2 (F_HCLK)

//#define SLEEP_ALARM_TIMER 3
