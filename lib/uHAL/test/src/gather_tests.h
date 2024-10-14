#if TEST_LED
# if TEST_LED_PINCTRL && LED_PIN == LED_PINCTRL_PIN
#  error "TEST_LED and TEST_LED_PINCTRL can't be run at the same time when they use the same pin."
# endif
# if TEST_LED_PINCTRL2 && LED_PIN == LED_PINCTRL2_PIN
#  error "TEST_LED and TEST_LED_PINCTRL2 can't be run at the same time when they use the same pin."
# endif
# if TEST_PWM && LED_PIN == PWM_LED_PIN
#  error "TEST_LED and TEST_PWM can't be run at the same time when they use the same pin."
# endif
#endif

#if TEST_LED_PINCTRL
# if TEST_LED_PINCTRL2 && LED_PINCTRL_PIN == LED_PINCTRL2_PIN
#  error "TEST_LED_PINCTRL and TEST_LED_PINCTRL2 can't be run at the same time when they use the same pin."
# endif
# if TEST_PWM && LED_PINCTRL_PIN == PWM_LED_PIN
#  error "TEST_PINCTRL_LED and TEST_PWM can't be run at the same time when they use the same pin."
# endif
#endif

#if TEST_LED_PINCTRL2
# if TEST_PWM && LED_PINCTRL2_PIN == PWM_LED_PIN
#  error "TEST_PINCTRL2_LED and TEST_PWM can't be run at the same time when they use the same pin."
# endif
#endif


#if TEST_LED
  void init_LED(void);
  void loop_LED(void);
#else
# define init_LED() (void )0U
# define loop_LED() (void )0U
#endif

#if TEST_LED_PINCTRL
  void init_LED_PINCTRL(void);
  void loop_LED_PINCTRL(void);
#else
# define init_LED_PINCTRL() (void )0U
# define loop_LED_PINCTRL() (void )0U
#endif

#if TEST_LED_PINCTRL2
  void init_LED_PINCTRL2(void);
  void loop_LED_PINCTRL2(void);
#else
# define init_LED_PINCTRL2() (void )0U
# define loop_LED_PINCTRL2() (void )0U
#endif

#if TEST_PWM
  void init_PWM(void);
  void loop_PWM(void);
#else
# define init_PWM() (void )0U
# define loop_PWM() (void )0U
#endif

#if TEST_BUTTON
  void init_BUTTON(void);
  void loop_BUTTON(void);
#else
# define init_BUTTON() (void )0U
# define loop_BUTTON() (void )0U
#endif

#if TEST_USCOUNTER
  void init_USCOUNTER(void);
  void loop_USCOUNTER(void);
#else
# define init_USCOUNTER() (void )0U
# define loop_USCOUNTER() (void )0U
#endif

#if TEST_DATE
  void init_DATE(void);
  void loop_DATE(void);
#else
# define init_DATE() (void )0U
# define loop_DATE() (void )0U
#endif

#if TEST_ADC
  void init_ADC(void);
  void loop_ADC(void);
#else
# define init_ADC() (void )0U
# define loop_ADC() (void )0U
#endif

#if TEST_SD
  void init_SD(void);
  void loop_SD(void);
#else
# define init_SD() (void )0U
# define loop_SD() (void )0U
#endif

#if TEST_UART_LISTEN
  void init_UART_LISTEN(void);
  void loop_UART_LISTEN(void);
#else
# define init_UART_LISTEN() (void )0U
# define loop_UART_LISTEN() (void )0U
#endif

#if TEST_TERMINAL
  void init_TERMINAL(void);
  void loop_TERMINAL(void);
#else
# define init_TERMINAL() (void )0U
# define loop_TERMINAL() (void )0U
#endif

#if TEST_SSD1306
  void init_SSD1306(void);
  void loop_SSD1306(void);
#else
# define init_SSD1306() (void )0U
# define loop_SSD1306() (void )0U
#endif

#if TEST_RESET
  void init_RESET(void);
  void loop_RESET(void);
#else
# define init_RESET() (void )0U
# define loop_RESET() (void )0U
#endif
