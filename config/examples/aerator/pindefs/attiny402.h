#define HAVE_GPIO_PORT_DEFAULT 1

#define GPIO_REMAP_I2C0   0
#define GPIO_REMAP_SPI0   0
#define GPIO_REMAP_UART0  0
#define GPIO_REMAP_TCA0W0 0
#define GPIO_REMAP_TCA0W1 0
#define GPIO_REMAP_TCA0W2 0
#define GPIO_REMAP_TCA0W3 0
#define GPIO_REMAP_TCB0   0

//
// Map IC pins to internal pin IDs
// Pin 1 is VCC, Pin 8 is GND, pin 6 is UPDI
#define PIN_2 PINID_A6
#define PIN_3 PINID_A7
#define PIN_4 PINID_A1
#define PIN_5 PINID_A2
#define PIN_7 PINID_A3

//
// Attiny402 available pins:
//    PA1 ADC, USART (alt), I2C, TCA0 PWM
//    PA2 ADC, USART (alt), I2C, TCA0 PWM
//    PA3 ADC,                   TCA0 PWM
//    PA6 ADC, USART (def),      TCB0 PWM
//    PA7 ADC, USART (def)

// Voltage input sense pin, set to 0 to disable check
// Analog input
#define VIN_SENSE_PIN (PINID_A3) // Pin 7

// Water temperature sense pin, set to 0 to disable check
// Analog input
#define WATER_TEMP_SENSE_PIN (PINID_A2) // Pin 5

// Water level sense pin, set to 0 to disable check
// Digital input with pullup
#define WATER_LEVEL_SENSE_PIN (PINID_A7) // Pin 3

// Pump motor control pin
// Digital output
#define PUMP_CTRL_PIN (PINID_A6) // Pin 2
//#define USE_STDBY_TCB 1
