#include "common.h"

#if TEST_SSD1306

#include "uHAL/include/drivers/display/ssd1306/ssd1306.h"
//#include "uHAL/src/drivers/display/ssd1306/ssd1306_font.h"
//#include "ssd1306_font.h"

#include "ulib/include/cstrings.h"
#include <string.h>

//
// Globals initialization
static ssd1306_cfg_t ssd1306_cfg_real = {
	{ TEST_SSD1306_ADDR },
	64,
	128,
	SSD1306_CFG_FLAG_I2C | SSD1306_CFG_FLAG_VFLIP,
};
static ssd1306_cfg_t *ssd1306_cfg = &ssd1306_cfg_real;
static ssd1306_handle_t ssd1306_status = { 0 };
static ssd1306_font_t font = {
	(-' ')+1,
	' ',
	'~',
	0,
#if SSD1306_FONT_AUTOSCALE
	0,
	2,
#endif
#if SSD1306_FONT_WIDTH <= 0
	8,
#endif
	ssd1306_font8x8_basic
};
static ssd1306_font_t font_small = {
	(-' ')+1,
	' ',
	'~',
	0,
#if SSD1306_FONT_AUTOSCALE
	0,
	0,
#endif
#if SSD1306_FONT_WIDTH <= 0
	8,
#endif
	ssd1306_font8x8_basic
};
static char ssd1306_loopno[8];


//
// main() initialization
void init_SSD1306(void) {
	err_t err;

	err = i2c_on();
	if (err != ERR_OK) {
		PRINTF("i2c_on() failed: error 0x%02X\r\n", (uint )err);
	} else {
		err = ssd1306_init(&ssd1306_status, ssd1306_cfg);
		if (err != ERR_OK) {
			PRINTF("ssd1306_init() failed: error 0x%02X\r\n", (uint )err);
		}
	}

	return;
}


//
// Main loop
void loop_SSD1306(void) {
	static uint_t i = 0, scale = 4;
	uint_t xscale, yscale;

	if (!BIT_IS_SET(ssd1306_status.flags, SSD1306_STATUS_FLAG_INITIALIZED)) {
		PRINTF("ssd1306_status not initialized.\r\n");
		return;
	}

	memset(ssd1306_loopno, ' ', 7);
	uint_t s = cstring_from_uint(ssd1306_loopno, sizeof(ssd1306_loopno), i, 10) - 1;
	ssd1306_loopno[s] = ' ';
	ssd1306_loopno[7] = 0;

	++i;
	if (i > 100) {
		i = 0;
	}
	switch (scale) {
	case 1:
		xscale = 1;
		yscale = 2;
		break;
	case 2:
		xscale = 2;
		yscale = 1;
		break;
	case 3:
		xscale = 2;
		yscale = 2;
		break;
	default:
		xscale = 1;
		yscale = 1;
		scale = 0;
		break;
	}
	++scale;

	ssd1306_draw_text_scaled(&ssd1306_status, &font,       xscale, yscale, 0, 0, "ABCDEFGHIJKLMNOP");
	ssd1306_draw_text_scaled(&ssd1306_status, &font,            1,      1, 0, 5, "loop no: ");
	ssd1306_draw_text_scaled(&ssd1306_status, &font,            1,      1, 9, 5, ssd1306_loopno);
	ssd1306_draw_text_scaled(&ssd1306_status, &font_small,      1,      1, 0, 7, "loop no: ");
	ssd1306_draw_text_scaled(&ssd1306_status, &font_small,      1,      1, 9, 7, ssd1306_loopno);

	return;
}


#endif // TEST_SSD1306
