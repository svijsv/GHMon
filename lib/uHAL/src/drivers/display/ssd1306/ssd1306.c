// SPDX-License-Identifier: GPL-3.0-only
/***********************************************************************
*                                                                      *
*                                                                      *
* Copyright 2021, 2024 svijsv                                          *
* This program is free software: you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation, version 3.                             *
*                                                                      *
* This program is distributed in the hope that it will be useful, but  *
* WITHOUT ANY WARRANTY; without even the implied warranty of           *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU    *
* General Public License for more details.                             *
*                                                                      *
* You should have received a copy of the GNU General Public License    *
* along with this program.  If not, see <http:// www.gnu.org/licenses/>.*
*                                                                      *
*                                                                      *
***********************************************************************/
// calendar.c
// NOTES:
//
#include "common.h"
#include "include/drivers/display/ssd1306/ssd1306.h"

#if uHAL_USE_DISPLAY_SSD1306

#include "ulib/include/time.h"


#define ROWS_PER_PAGE 8U
#define COLS_PER_PAGE 128U
#define PAGES_PER_BANK 8U
#define I2C_TIMEOUT 1000U

#if SSD1306_FONT_WIDTH > 0
typedef union {
	const uint8_t *ptr;
	const uint8_t (*arr)[SSD1306_FONT_WIDTH];
} font_access_t;

# define GLYPH_WIDTH(_font) (SSD1306_FONT_WIDTH)
#else
# define GLYPH_WIDTH(_font_) ((_font_)->glyph_width)
#endif

#if SSD1306_INCLUDE_DEFAULT_FONT
# include "ssd1306_font.h"

static ssd1306_font_t _font_default = {
	(-' ')+1,
	' ',
	'~',
	0,
# if SSD1306_FONT_AUTOSCALE
	0,
	0,
# endif
# if SSD1306_FONT_WIDTH <= 0
	8,
# endif
	ssd1306_font8x8_basic
};
static const ssd1306_font_t *font_default = &_font_default;

#else
# define font_default NULL
#endif

//
// These are the possible control bytes
// Each command or write sequence begins with exactly one of these
#define SSD1306_CTRL_DATA_SINGLE 0x20U // A single data byte follows
#define SSD1306_CTRL_DATA_BATCH  0x60U // A series of data bytes followed
                                       // by a stop condition follows
#define SSD1306_CTRL_CMD_SINGLE  0x40U // A single command follows
#define SSD1306_CTRL_CMD_BATCH   0x00U // A series of commands followed by
                                       // a stop condition follows

//
// Display control commands
//
// This is a two-byte command to enable the internal charge pump
#define SSD1306_CMD_CHARGE_PUMP 0x8DU
#define SSD1306_CMD_CHARGE_PUMP_ON  0x14U
#define SSD1306_CMD_CHARGE_PUMP_OFF 0x10U
//
// Power the display on or off (enter/exit sleep mode)
#define SSD1306_CMD_PWR_ON  0xAFU
#define SSD1306_CMD_PWR_OFF 0xAEU
//
// Show or ignore the contents of the display ram
#define SSD1306_CMD_DISPLAY_RAM_ON  0xA4U
#define SSD1306_CMD_DISPLAY_RAM_OFF 0xA5U
//
// Invert the display
#define SSD1306_CMD_DISPLAY_INVERT_ON  0xA7U
#define SSD1306_CMD_DISPLAY_INVERT_OFF 0xA6U
//
// This is a two-byte command to set the display contrast, a higher number
// increases contrast
// Default is 0x7F
#define SSD1306_CMD_CONTRAST 0x81U
#define SSD1306_CMD_CONTRAST_DEFAULT 0x7FU
//
// Turn scrolling on or off
// The contents of the display buffer will need to be re-written when
// scrolling is disabled and can't be written when enabled
// The scroll configuration commands are currently not defined
#define SSD1306_CMD_SCROLL_OFF 0x2EU
#define SSD1306_CMD_SCROLL_ON  0x2FU

//
// Display configuration commands
//
// This is a one-byte command, the lower six bits are the start line
#define SSD1306_CMD_DISPLAY_START_LINE 0x40U
#define SSD1306_CMD_DISPLAY_START_LINE_DEFAULT (0x40U | 0U)
//
// Re-map column 0 to segment 127 (horizonatal flip)
// This doesn't affect already-written data
#define SSD1306_CMD_SEG_REMAP_ON  0xA0U
#define SSD1306_CMD_SEG_REMAP_OFF 0xA1U
//
// Two-byte command, set MUX ratio (i.e. number of rows) to N+1
// Minimum N is 15
#define SSD1306_CMD_MUX_RATIO 0xA8U
#define SSD1306_CMD_MUX_RATIO_DEFAULT 63U
//
// Reverse the MUX scan direction (vertical flip)
// This affects already-written data
#define SSD1306_CMD_REV_SCAN_ON  0xC8U
#define SSD1306_CMD_REV_SCAN_OFF 0xC0U
//
// Two-byte command, set vertical shift
#define SSD1306_CMD_DISPLAY_OFFSET 0xD3U
#define SSD1306_CMD_DISPLAY_OFFSET_DEFAULT 0U
//
// Two-byte command to configure the hardware, setting sequential or
// alternative COM pin configurations and enabling/disabling COM pin
// left/right remap
#define SSD1306_CMD_HW_CFG 0xDAU
#define SSD1306_CMD_HW_CFG_ALTPIN_LRREMAP   0x32U
#define SSD1306_CMD_HW_CFG_SEQPIN_LRREMAP   0x22U
#define SSD1306_CMD_HW_CFG_ALTPIN_NOLRREMAP 0x12U
#define SSD1306_CMD_HW_CFG_SEQPIN_NOLRREMAP 0x02U
#define SSD1306_CMD_HW_CFG_DEFAULT SSD1306_CMD_HW_CFG_ALTPIN_NOLRREMAP
//
// This is a two-byte command to set the display clock frequency
// The lower four bits of the argument byte is the division ratio of the
// display clock (0 == 1), the upper four bits is the oscillator frequency
#define SSD1306_CMD_DISPLAY_CLOCK_FREQ 0xD5U
#define SSD1306_CMD_DISPLAY_CLOCK_FREQ_DEFAULT 0x80U
//
// This is a two-byte command to set the pre-charge period
// The lower four bits of the argument byte set the phase 1 precharge
// and the upper four bits set the phase 2 precharge
// Neither precharge phase can be 0
#define SSD1306_CMD_PRECHARGE_PERIOD 0xD9U
#define SSD1306_CMD_PRECHARGE_PERIOD_DEFAULT 0x22U
//
// This is a two-byte command to select the deselect voltage level
#define SSD1306_CMD_VCOMM_DESELECT_LEVEL 0xDBU
#define SSD1306_CMD_VCOMM_DESELECT_LEVEL_65xVcc 0x00U // 65% of Vcc
#define SSD1306_CMD_VCOMM_DESELECT_LEVEL_77xVcc 0x20U // 77% of Vcc
#define SSD1306_CMD_VCOMM_DESELECT_LEVEL_83xVcc 0x30U // 83% of Vcc
#define SSD1306_CMD_VCOMM_DESELECT_LEVEL_DEFAULT SSD1306_CMD_VCOMM_DESELECT_LEVEL_77xVcc

//
// Addressing mode commands
//
// This is a two-byte command to set the addressing mode, see the manual for
// the differences between the modes
#define SSD1306_CMD_ADDR  0x20U
#define SSD1306_CMD_ADDR_HOR  0x00U // Horizontal addressing mode
#define SSD1306_CMD_ADDR_VER  0x01U // Vertical addressing mode
#define SSD1306_CMD_ADDR_PAGE 0x02U // Page addressing mode
#define SSD1306_CMD_ADDR_DEFAULT SSD1306_CMD_ADDR_PAGE
//
// These commands are for page addressing mode
//
// These are one-byte commands, the lower four bits are used to set the
// low and high nibbles of the start column
#define SSD1306_CMD_PAGE_START_COL_LO 0x00U
#define SSD1306_CMD_PAGE_START_COL_LO_DEFAULT (SSD1306_CMD_PAGE_START_COL_LO | 0U)
#define SSD1306_CMD_PAGE_START_COL_HI 0x10U
#define SSD1306_CMD_PAGE_START_COL_HI_DEFAULT (SSD1306_CMD_PAGE_START_COL_HI | 0U)
// This is a one-byte command, the lower three bits are the start page
#define SSD1306_CMD_PAGE_START 0xB0U
#define SSD1306_CMD_PAGE_START_DEFAULT (SSD1306_CMD_PAGE_START | 0U)
//
// These commands are for horizontal and vertial addressing modes
//
// Three-byte command, set the start and stop column
#define SSD1306_CMD_COL_RANGE 0x21U
#define SSD1306_CMD_COL_RANGE_START_DEFAULT   0U
#define SSD1306_CMD_COL_RANGE_END_DEFAULT   (COLS_PER_PAGE - 1U)
// Three-byte command, set the start and stop page
#define SSD1306_CMD_PAGE_RANGE 0x22U
#define SSD1306_CMD_PAGE_RANGE_START_DEFAULT 0U
#define SSD1306_CMD_PAGE_RANGE_END_DEFAULT   (PAGES_PER_BANK - 1U)

//
// Misc commands
//
// Don't do anything
#define SSD1306_CMD_NOP 0xE3U

//
// Helpers for verifying common function arguments
//
//#define VALID_CFG(_cfg_) (((_cfg_) != NULL) && ((_cfg_)->width > 0) && ((_cfg_)->height > 0) && (BIT_IS_SET((_cfg_)->flags, SSD1306_CFG_FLAG_SPI|SSD1306_CFG_FLAG_I2C)))
#define VALID_CFG(_cfg_) (((_cfg_) != NULL) && ((_cfg_)->width > 0) && ((_cfg_)->height > 0) && (BIT_IS_SET((_cfg_)->flags, SSD1306_CFG_FLAG_I2C)))
#define VALID_INIT(_handle_) (((_handle_) != NULL) && ((_handle_)->cfg != NULL) && BIT_IS_SET((_handle_)->flags, SSD1306_STATUS_FLAG_INITIALIZED))
#if SSD1306_FONT_WIDTH <= 0
# define VALID_FONT(_font_) (((_font_) != NULL) && ((_font_)->glyphs != NULL) && ((_font_)->glyph_width > 0))
#else
# define VALID_FONT(_font_) (((_font_) != NULL) && ((_font_)->glyphs != NULL))
#endif

#if ! uHAL_SKIP_INVALID_ARG_CHECKS
# define CHECK_INVALID_ARGS(_handle_) \
	do { \
		if ((_handle_ == NULL) || !VALID_CFG((_handle_)->cfg)) { \
				return ERR_BADARG; \
		} \
	} while (0);
#else
# define CHECK_INVALID_ARGS(_handle_) ((void )0U)
#endif

#if ! uHAL_SKIP_INIT_CHECKS
# define CHECK_INVALID_INIT(_handle_) \
	do { \
		if (!VALID_INIT(_handle_)) { \
			return ERR_INIT; \
		} \
	} while (0);
#else
# define CHECK_INVALID_INIT(_handle_) ((void )0U)
#endif

#define CHECK_STD_ARGS(_handle_) \
	do { \
		uHAL_assert(VALID_INIT(_handle_)); \
		uHAL_assert(VALID_CFG((_handle_)->cfg)); \
		CHECK_INVALID_ARGS(_handle_); \
		CHECK_INVALID_INIT(_handle_); \
	} while (0);

static err_t set_position(const ssd1306_handle_t *handle, uint8_t x_pixel, uint8_t y_row) {
	const uint8_t cmd_pos[] = {
		SSD1306_CTRL_CMD_BATCH,
		SSD1306_CMD_COL_RANGE,
		x_pixel,
		handle->cfg->width-1,
		SSD1306_CMD_PAGE_RANGE,
		y_row,
		(handle->cfg->height/ROWS_PER_PAGE)-1,
	};

	return i2c_transmit_block(handle->cfg->access.address, cmd_pos, sizeof(cmd_pos), I2C_TIMEOUT);
}

err_t ssd1306_init(ssd1306_handle_t *handle, const ssd1306_cfg_t *cfg) {
	err_t res;
	const uint8_t cmd_setup[] = {
		//
		// Addressing mode configuration
		SSD1306_CMD_ADDR,
		SSD1306_CMD_ADDR_HOR,
		/*
		// This is set below on a per-cfg basis
		SSD1306_CMD_COL_RANGE,
		SSD1306_CMD_COL_RANGE_START_DEFAULT,
		SSD1306_CMD_COL_RANGE_END_DEFAULT,
		SSD1306_CMD_PAGE_RANGE,
		SSD1306_CMD_PAGE_RANGE_START_DEFAULT,
		SSD1306_CMD_PAGE_RANGE_END_DEFAULT,
		*/

		//
		// Stuff that doesn't really need to be set because it's not usually
		// changed
#if SSD1306_FULL_INIT
		SSD1306_CMD_SCROLL_OFF,
		SSD1306_CMD_DISPLAY_START_LINE_DEFAULT,
		/*
		// These are set below on a per-cfg basis
		SSD1306_CMD_MUX_RATIO,
		SSD1306_CMD_MUX_RATIO_DEFAULT,
		SSD1306_CMD_REV_SCAN_OFF,
		SSD1306_CMD_SEG_REMAP_OFF,
		*/
		SSD1306_CMD_DISPLAY_OFFSET,
		SSD1306_CMD_DISPLAY_OFFSET_DEFAULT,
		SSD1306_CMD_HW_CFG,
		SSD1306_CMD_HW_CFG_DEFAULT,
		SSD1306_CMD_DISPLAY_CLOCK_FREQ,
		SSD1306_CMD_DISPLAY_CLOCK_FREQ_DEFAULT,
		SSD1306_CMD_PRECHARGE_PERIOD,
		SSD1306_CMD_PRECHARGE_PERIOD_DEFAULT,
		SSD1306_CMD_VCOMM_DESELECT_LEVEL,
		SSD1306_CMD_VCOMM_DESELECT_LEVEL_DEFAULT,
#endif

		//
		// Display configuration
		SSD1306_CMD_DISPLAY_INVERT_OFF,
		SSD1306_CMD_CONTRAST,
		SSD1306_CMD_CONTRAST_DEFAULT,
		SSD1306_CMD_DISPLAY_RAM_ON,
		SSD1306_CMD_CHARGE_PUMP,
		SSD1306_CMD_CHARGE_PUMP_ON,
	};

	uHAL_assert(handle != NULL);
	uHAL_assert(VALID_CFG(cfg));
#if ! uHAL_SKIP_INIT_CHECKS
#endif
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (!VALID_CFG(cfg) || (handle == NULL)) {
		return ERR_BADARG;
	}
#endif

	handle->cfg = cfg;

	if (BIT_IS_SET(cfg->flags, SSD1306_CFG_FLAG_I2C)) {
		uHAL_assert(i2c_is_on());
#if ! uHAL_SKIP_OTHER_CHECKS
		if (!i2c_is_on()) {
			i2c_on();
		}
#endif

		if ((res = i2c_transmit_block_begin(cfg->access.address, I2C_TIMEOUT)) != ERR_OK) {
			goto I2C_END;
		}

		const uint8_t cmd_start[] = {
			SSD1306_CTRL_CMD_BATCH,
			SSD1306_CMD_PWR_OFF,
		};
		if ((res = i2c_transmit_block_continue(cmd_start, sizeof(cmd_start), I2C_TIMEOUT)) != ERR_OK) {
			goto I2C_END;
		}

		if ((res = i2c_transmit_block_continue(cmd_setup, sizeof(cmd_setup), I2C_TIMEOUT)) != ERR_OK) {
			goto I2C_END;
		}

		uint8_t cmd_res[] = {
			SSD1306_CMD_MUX_RATIO,
			cfg->height-1,
			SSD1306_CMD_SEG_REMAP_OFF,
			SSD1306_CMD_REV_SCAN_OFF,
		};
		if (BIT_IS_SET(cfg->flags, SSD1306_CFG_FLAG_HFLIP)) {
			cmd_res[2] = SSD1306_CMD_SEG_REMAP_ON;
		}
		if (BIT_IS_SET(cfg->flags, SSD1306_CFG_FLAG_VFLIP)) {
			cmd_res[3] = SSD1306_CMD_REV_SCAN_ON;
		}

		if ((res = i2c_transmit_block_continue(cmd_res, sizeof(cmd_res), I2C_TIMEOUT)) != ERR_OK) {
			goto I2C_END;
		}

#if SSD1306_INIT_COMMANDS_COUNT > 0
		if ((res = i2c_transmit_block_continue(cfg->init_cmds, SSD1306_INIT_COMMANDS_COUNT, I2C_TIMEOUT)) != ERR_OK) {
			goto I2C_END;
		}
#endif

		const uint8_t cmd_pwr[] = {
			SSD1306_CMD_PWR_ON,
		};
		if ((res = i2c_transmit_block_continue(cmd_pwr, sizeof(cmd_pwr), I2C_TIMEOUT)) != ERR_OK) {
			goto I2C_END;
		}

I2C_END:
		i2c_transmit_block_end();
		if (res != ERR_OK) {
			goto END;
		}

	} else {
		res = ERR_BADARG;
		goto END;
	}

	handle->flags = SSD1306_STATUS_FLAG_INITIALIZED | SSD1306_STATUS_FLAG_DISPLAY_ON;
	handle->contrast = SSD1306_CMD_CONTRAST_DEFAULT;

	ssd1306_clear_display(handle);

	// It takes ~100ms for the power to come up
	sleep_ms(100);

END:
	return res;
}

err_t ssd1306_fill_section(const ssd1306_handle_t *handle, uint8_t byte, uint8_t xp, uint8_t wp, uint8_t yp, uint8_t hp) {
	err_t res;
	const uint8_t mode = SSD1306_CTRL_DATA_BATCH;

	CHECK_STD_ARGS(handle);

#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if ((xp >= handle->cfg->width) || (yp >= handle->cfg->height) || (xp + wp > handle->cfg->width) || (yp + hp > handle->cfg->height)) {
		return ERR_BADARG;
	}
#endif

	if (wp == 0) {
		wp = handle->cfg->width - xp;
	}
	if (hp == 0) {
		hp = handle->cfg->height - yp;
	}
	// yp/hp are the position in 8-bit high rows instead of pixels after this point
	yp /= ROWS_PER_PAGE;
	hp /= ROWS_PER_PAGE;

	if ((res = set_position(handle, xp, yp)) != ERR_OK) {
		return res;
	}

	if ((res = i2c_transmit_block_begin(handle->cfg->access.address, I2C_TIMEOUT)) != ERR_OK) {
		goto I2C_END;
	}

	if ((res = i2c_transmit_block_continue(&mode, 1, I2C_TIMEOUT)) != ERR_OK) {
		goto I2C_END;
	}

	for (uint8_t y_i = 0; y_i < hp; ++y_i) {
		for (uint8_t x_i = 0; x_i < wp; ++x_i) {
			if ((res = i2c_transmit_block_continue(&byte, 1, I2C_TIMEOUT)) != ERR_OK) {
				goto I2C_END;
			}
		}
	}

I2C_END:
	i2c_transmit_block_end();
	return res;
}
err_t ssd1306_clear_display(ssd1306_handle_t *handle) {
	return ssd1306_fill_section(handle, 0x00, 0, 0, 0, 0);
}

err_t ssd1306_toggle_inversion(ssd1306_handle_t *handle) {
	uint8_t cmd[] = {
		SSD1306_CTRL_CMD_BATCH,
		SSD1306_CMD_DISPLAY_INVERT_ON,
	};

	CHECK_STD_ARGS(handle);

	if (BIT_IS_SET(handle->flags, SSD1306_STATUS_FLAG_INVERTED)) {
		cmd[1] = SSD1306_CMD_DISPLAY_INVERT_OFF;
		CLEAR_BIT(handle->flags, SSD1306_STATUS_FLAG_INVERTED);
	} else {
		SET_BIT(handle->flags, SSD1306_STATUS_FLAG_INVERTED);
	}

	return i2c_transmit_block(handle->cfg->access.address, cmd, 2, I2C_TIMEOUT);
}
err_t ssd1306_invert_display_on(ssd1306_handle_t *handle) {
	const uint8_t cmd[] = {
		SSD1306_CTRL_CMD_BATCH,
		SSD1306_CMD_DISPLAY_INVERT_ON,
	};

	CHECK_STD_ARGS(handle);

	SET_BIT(handle->flags, SSD1306_STATUS_FLAG_INVERTED);

	return i2c_transmit_block(handle->cfg->access.address, cmd, sizeof(cmd), I2C_TIMEOUT);
}
err_t ssd1306_invert_display_off(ssd1306_handle_t *handle) {
	const uint8_t cmd[] = {
		SSD1306_CTRL_CMD_BATCH,
		SSD1306_CMD_DISPLAY_INVERT_OFF,
	};

	CHECK_STD_ARGS(handle);

	CLEAR_BIT(handle->flags, SSD1306_STATUS_FLAG_INVERTED);

	return i2c_transmit_block(handle->cfg->access.address, cmd, sizeof(cmd), I2C_TIMEOUT);
}

err_t ssd1306_toggle_display(ssd1306_handle_t *handle) {
	uint8_t cmd[] = {
		SSD1306_CTRL_CMD_BATCH,
		SSD1306_CMD_PWR_ON,
	};

	CHECK_STD_ARGS(handle);

	if (BIT_IS_SET(handle->flags, SSD1306_STATUS_FLAG_DISPLAY_ON)) {
		cmd[1] = SSD1306_CMD_PWR_OFF;
		CLEAR_BIT(handle->flags, SSD1306_STATUS_FLAG_DISPLAY_ON);
	} else {
		SET_BIT(handle->flags, SSD1306_STATUS_FLAG_DISPLAY_ON);
	}

	return i2c_transmit_block(handle->cfg->access.address, cmd, 2, I2C_TIMEOUT);
}
err_t ssd1306_display_on(ssd1306_handle_t *handle) {
	const uint8_t cmd[] = {
		SSD1306_CTRL_CMD_BATCH,
		SSD1306_CMD_PWR_ON,
	};

	CHECK_STD_ARGS(handle);

	SET_BIT(handle->flags, SSD1306_STATUS_FLAG_DISPLAY_ON);

	return i2c_transmit_block(handle->cfg->access.address, cmd, sizeof(cmd), I2C_TIMEOUT);
}
err_t ssd1306_display_off(ssd1306_handle_t *handle) {
	const uint8_t cmd[] = {
		SSD1306_CTRL_CMD_BATCH,
		SSD1306_CMD_PWR_OFF,
	};

	CHECK_STD_ARGS(handle);

	CLEAR_BIT(handle->flags, SSD1306_STATUS_FLAG_DISPLAY_ON);

	return i2c_transmit_block(handle->cfg->access.address, cmd, sizeof(cmd), I2C_TIMEOUT);
}

err_t ssd1306_set_contrast(ssd1306_handle_t *handle, uint8_t level) {
	const uint8_t cmd[] = {
		SSD1306_CTRL_CMD_BATCH,
		SSD1306_CMD_CONTRAST,
		level,
	};

	CHECK_STD_ARGS(handle);

	handle->contrast = level;

	return i2c_transmit_block(handle->cfg->access.address, cmd, sizeof(cmd), I2C_TIMEOUT);
}
err_t ssd1306_adj_contrast(ssd1306_handle_t *handle, int_t incr) {
	uint8_t cmd[] = {
		SSD1306_CTRL_CMD_BATCH,
		SSD1306_CMD_CONTRAST,
		0,
	};
	int_t level;

	CHECK_STD_ARGS(handle);

	level = handle->contrast;
	if (incr > 0) {
		if ((0xFF - incr) < level) {
			level = 0xFF;
		} else {
			level += incr;
		}
	} else {
		incr = -incr;
		if (incr > level) {
			level = 0;
		} else {
			level -= incr;
		}
	}

	handle->contrast = (uint8_t )level;
	cmd[2] = handle->contrast;

	return i2c_transmit_block(handle->cfg->access.address, cmd, sizeof(cmd), I2C_TIMEOUT);
}

static uint8_t stretch_nibble(uint_fast8_t c) {
	uint_fast8_t d = 0;

	if ((c & 0x01U) != 0U) {
		d |= 0x03U;
	}
	if ((c & 0x02U) != 0U) {
		d |= 0x0CU;
	}
	if ((c & 0x04U) != 0U) {
		d |= 0x30U;
	}
	if ((c & 0x08U) != 0U) {
		d |= 0xC0U;
	}

	return d;
}
static err_t _ssd1306_draw_text_scaled(ssd1306_handle_t *handle, const ssd1306_font_t *font, uint8_t scale_x, uint8_t scale_y, uint8_t xt, uint8_t yt, const char *text) {
	err_t res = ERR_OK;
	uint8_t max_x, max_y;

	const uint8_t cmd[] = {
		SSD1306_CTRL_DATA_BATCH,
	};

	max_x = (handle->cfg->width / GLYPH_WIDTH(font));
	max_y = (handle->cfg->height / ROWS_PER_PAGE);

	uHAL_assert((scale_y == 1) || (scale_y == 2) || (scale_y == 4));
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (!((scale_y == 1) || (scale_y == 2) || (scale_y == 4))) {
		return ERR_BADARG;
	}
	if ((xt > max_x-1) || (yt > max_y-1)) {
		return ERR_BADARG;
	}
#endif

	max_x -= xt;
	max_x /= scale_x;
	for (uiter_t row = 0; (row < scale_y) && (yt < max_y); ++row, ++yt) {
		if ((res = set_position(handle, xt * GLYPH_WIDTH(font), yt)) != ERR_OK) {
			return res;
		}

		if ((res = i2c_transmit_block_begin(handle->cfg->access.address, I2C_TIMEOUT)) != ERR_OK) {
			goto I2C_END;
		}
		if ((res = i2c_transmit_block_continue(cmd, sizeof(cmd), I2C_TIMEOUT)) != ERR_OK) {
			goto I2C_END;
		}

		for (uiter_t i = 0; ((text[i] != 0) && (i < max_x)); ++i) {
			uint8_t c = text[i];

			if ((c < font->char_min) || (c > font->char_max)) {
				c = font->char_sub;
			} else {
				c += font->char_offset;
			}

			for (uiter_t gi = 0; gi < GLYPH_WIDTH(font); ++gi) {
#if SSD1306_FONT_WIDTH <= 0
				uint8_t glyph_line = font->glyphs[(c * GLYPH_WIDTH(font)) + gi];
#else
				font_access_t acc = { .ptr = font->glyphs };
				uint8_t glyph_line = acc.arr[c][gi];
#endif

				switch (row) {
				case 0:
					switch (scale_y) {
					case 2:
						glyph_line = stretch_nibble(glyph_line);
						break;
					case 4:
						glyph_line = stretch_nibble(stretch_nibble(glyph_line));
						break;
					default:
						break;
					}
					break;

				case 1:
					switch (scale_y) {
					case 2:
						glyph_line = stretch_nibble(glyph_line >> 4U);
						break;
					case 4:
						glyph_line = stretch_nibble(stretch_nibble(glyph_line) >> 4U);
						break;
					default:
						break;
					}
					break;

				case 2:
					glyph_line = stretch_nibble(stretch_nibble(glyph_line >> 4U));
					break;

				case 3:
					glyph_line = stretch_nibble(stretch_nibble(glyph_line >> 4U) >> 4U);
					break;
				}

				for (uiter_t col = 0; col < scale_x; ++col) {
					if ((res = i2c_transmit_block_continue(&glyph_line, 1, I2C_TIMEOUT)) != ERR_OK) {
						goto I2C_END;
					}
				}
			}
		}

		i2c_transmit_block_end();
	}

I2C_END:
	i2c_transmit_block_end();

	return res;
}
err_t ssd1306_draw_text_scaled(ssd1306_handle_t *handle, const ssd1306_font_t *font, uint8_t scale_x, uint8_t scale_y, uint8_t xt, uint8_t yt, const char *text) {
	if (SSD1306_INCLUDE_DEFAULT_FONT && font == NULL) {
		font = font_default;
	}

	CHECK_STD_ARGS(handle);
	uHAL_assert(VALID_FONT(font));
	uHAL_assert(text != NULL);

#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (!VALID_FONT(font) || (text == NULL)) {
		return ERR_BADARG;
	}
#endif

#if SSD1306_FONT_AUTOSCALE
	if ((font->scale_x > 1) || (font->scale_y > 1)) {
		scale_x *= (font->scale_x > 0) ? font->scale_x : 1;
		scale_y *= (font->scale_y > 0) ? font->scale_y : 1;
	}
#endif

	return _ssd1306_draw_text_scaled(handle, font, scale_x, scale_y, xt, yt, text);
}

err_t ssd1306_draw_text(ssd1306_handle_t *handle, const ssd1306_font_t *font, uint8_t xt, uint8_t yt, const char *text) {
	err_t res;
	uint8_t max_x;

	const uint8_t cmd[] = {
		SSD1306_CTRL_DATA_BATCH,
	};

	if (SSD1306_INCLUDE_DEFAULT_FONT && font == NULL) {
		font = font_default;
	}

	uHAL_assert(VALID_FONT(font));
	uHAL_assert(text != NULL);
	CHECK_STD_ARGS(handle);
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	if (!VALID_FONT(font) || (text == NULL)) {
		return ERR_BADARG;
	}
#endif

#if SSD1306_FONT_AUTOSCALE
	if (!SSD1306_AUTOSCALE_COEXIST || ((font->scale_x > 1) || (font->scale_y > 1))) {
		uint8_t x = (font->scale_x > 0) ? font->scale_x : 1;
		uint8_t y = (font->scale_y > 0) ? font->scale_y : 1;

		return _ssd1306_draw_text_scaled(handle, font, x, y, xt, yt, text);
	}
#endif

	max_x = (handle->cfg->width / GLYPH_WIDTH(font));

#if ! uHAL_SKIP_INVALID_ARG_CHECKS
	uint8_t max_y = (handle->cfg->height / ROWS_PER_PAGE);
	if ((xt > max_x-1) || (yt > max_y-1)) {
		return ERR_BADARG;
	}
#endif

	max_x -= xt;
	if ((res = set_position(handle, xt * GLYPH_WIDTH(font), yt)) != ERR_OK) {
		return res;
	}

	if ((res = i2c_transmit_block_begin(handle->cfg->access.address, I2C_TIMEOUT)) != ERR_OK) {
		goto I2C_END;
	}
	if ((res = i2c_transmit_block_continue(cmd, sizeof(cmd), I2C_TIMEOUT)) != ERR_OK) {
		goto I2C_END;
	}

	for (uiter_t i = 0; ((text[i] != 0) && (i < max_x)); ++i) {
		uint8_t c = text[i];

		if ((c < font->char_min) || (c > font->char_max)) {
			c = font->char_sub;
		} else {
			c += font->char_offset;
		}
#if SSD1306_FONT_WIDTH <= 0
		if ((res = i2c_transmit_block_continue(&font->glyphs[c * GLYPH_WIDTH(font)], GLYPH_WIDTH(font), I2C_TIMEOUT)) != ERR_OK) {
#else
		font_access_t acc = { .ptr = font->glyphs };
		if ((res = i2c_transmit_block_continue(acc.arr[c], SSD1306_FONT_WIDTH, I2C_TIMEOUT)) != ERR_OK) {
#endif
			goto I2C_END;
		}
	}

I2C_END:
	i2c_transmit_block_end();

	return res;
}


#endif // uHAL_USE_DISPLAY_SSD1306
