//
// If non-zero, any GPIO port which the device header files expose will be
// enabled. Otherwise, any port used needs to be explicitly enabled by setting
// HAVE_GPIO_PORTx to non-zero either on the command line or in this configuration
// file
#define HAVE_GPIO_PORT_DEFAULT 0
#define HAVE_GPIO_PORTA 1
#define HAVE_GPIO_PORTB 1

//
// UART serial console pins
// USART1:
#define UART_COMM_TX_PIN PINID_B6 // A9, Alt: B6
#define UART_COMM_RX_PIN PINID_B7 // A10, Alt: B7

//
// SPI pins
// SPI1:
#define SPI_SS_PIN    PINID_A4 // A4, Alt: A15
#define SPI_SCK_PIN   PINID_A5 // A5, Alt: B3
#define SPI_MISO_PIN  PINID_A6 // A6, Alt: B4
#define SPI_MOSI_PIN  PINID_A7 // A7, Alt: B5
#define SPI_CS_SD_PIN SPI_SS_PIN

//
// I2C pins
// I2C1:
#define I2C_SDA_PIN PINID_B9 // B7, Alt: B9
#define I2C_SCL_PIN PINID_B8 // B6, Alt: B8

#define STATUS_LED_PIN  (PINID_A11 | GPIO_CTRL_OPENDRAIN | GPIO_CTRL_INVERT)
#define CTRL_BUTTON_PIN (PINID_A3  | GPIO_CTRL_PULLUP | GPIO_CTRL_INVERT)
