// SPDX-License-Identifier: GPL-3.0-only
/***********************************************************************
*                                                                      *
*                                                                      *
* Copyright 2024 svijsv                                                *
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
/// @file
/// @brief SSD1306 LCD Display Driver
///
#ifndef _uHAL_DRIVERS_DISPLAY_SSD1306_H
#define _uHAL_DRIVERS_DISPLAY_SSD1306_H

#include "interface.h"

///
/// Font definition structure for SSD1306 devices
///
typedef struct {
	int8_t char_offset; ///< Character values are adjusted by this to map
	                    ///<  them to the glyph array
	uint8_t char_min;   ///< Minimum value of displayable characters
	uint8_t char_max;   ///< Maximum value of displayable characters
	uint8_t char_sub;   ///< Substitute this glyph for absent characters
#if SSD1306_FONT_AUTOSCALE || __HAVE_DOXYGEN__
	uint8_t scale_x;    ///< Scale font on the x axis.
	                    ///< @note This member is only present when @c SSD1306_FONT_AUTOSCALE is set.
	uint8_t scale_y;    ///< Scale font on the y axis - must be 0, 1, 2, or 4
	                    ///< @note This member is only present when @c SSD1306_FONT_AUTOSCALE is set.
#endif
#if SSD1306_FONT_WIDTH <= 0 || __HAVE_DOXYGEN__
	uint8_t glyph_width; ///< The width in pixels of glyphs in the font
	                    ///< @note This member is only present when @c SSD1306_FONT_WIDTH is <= 0.
#endif
	const uint8_t *glyphs; ///< The array of glyphs
} ssd1306_font_t;

#if SSD1306_INCLUDE_DEFAULT_FONT || __HAVE_DOXYGEN__
///
/// If @c SSD1306_INCLUDE_DEFAULT_FONT is set, this is the default font.
extern const uint8_t ssd1306_font8x8_basic[];
#endif


///
/// @name Device Configuration
/// @{
//
/// Configuration structure for SSD1306 devices
///
typedef struct {
	union {
		gpio_pin_t cs_pin; ///< CS pin in SPI mode
		uint8_t address;   ///< I2C address in I2C mode
	} access; ///< The I2C address or SPI CS pin of the device
	          ///< @attention SPI support is not implemented
	uint8_t height; ///< Height of display in pixels
	uint8_t width;  ///< Width of display in pixels
	uint8_t flags;  ///< Configuration flags (see the @c SSD1306_CFG_FLAG_* macros)
#if SSD1306_INIT_COMMANDS_COUNT > 0 || __HAVE_DOXYGEN__
	/// Additional initialization commands, see ssd1306.c for definitions
	/// @note This member is only present when @c SSD1306_INIT_COMMANDS_COUNT is > 0.
	uint8_t init_cmds[SSD1306_INIT_COMMANDS_COUNT];
#endif
} ssd1306_cfg_t;
//
// Configuration flags
//
//#define SSD1306_CFG_FLAG_SPI   0x01U // Use SPI communication (unimplemented)
#define SSD1306_CFG_FLAG_I2C   0x02U ///< Use I2C communication
#define SSD1306_CFG_FLAG_VFLIP 0x04U ///< Flip screen vertically
#define SSD1306_CFG_FLAG_HFLIP 0x08U ///< Flip screen horizontally
/// @}

///
/// @name Device Management
/// @{
//
///
/// Device handle structure for SSD1306 devices
///
typedef struct {
	uint8_t flags;    ///< Status flags, see the @c SSD1306_STATUS_FLAG_* macros
	uint8_t contrast; ///< Screen contrast
	const ssd1306_cfg_t *cfg; ///< Device configuration, saved from @c ssd1306_init()
} ssd1306_handle_t;
//
// Status flags
//
#define SSD1306_STATUS_FLAG_INITIALIZED 0x80U ///< Device has been initialized
#define SSD1306_STATUS_FLAG_DISPLAY_ON  0x40U ///< Display is currently on
#define SSD1306_STATUS_FLAG_INVERTED    0x10U ///< Display is currently inverted
/// @}

///
/// Initialize an SSD1306 device handle
///
/// @attention
/// The passed ssd1306_cfg_t struct can be re-used for other devices and
/// modified but must remain a valid configuration as long as the handle is in
/// use
///
/// @param cfg The structure describing the device configuration
/// @param handle The handle used to manage the device after initialization
///
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t ssd1306_init(ssd1306_handle_t *handle, const ssd1306_cfg_t *cfg);

///
/// Reset an SSD1306 device
///
/// @param handle The handle used to manage the device
///
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
INLINE err_t ssd1306_reset(ssd1306_handle_t *handle) {
	return ssd1306_init(handle, handle->cfg);
}

///
/// Invert the colors
///
/// @param handle The handle used to manage the device
///
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t ssd1306_invert_display_on(ssd1306_handle_t *handle);
///
/// Un-Invert the colors
///
/// @param handle The handle used to manage the device
///
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t ssd1306_invert_display_off(ssd1306_handle_t *handle);
///
/// Toggle color inversion
///
/// @param handle The handle used to manage the device
///
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t ssd1306_toggle_inversion(ssd1306_handle_t *handle);

///
/// Turn the display on
///
/// @param handle The handle used to manage the device
///
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t ssd1306_display_on(ssd1306_handle_t *handle);
///
/// Turn the display off
///
/// @param handle The handle used to manage the device
///
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t ssd1306_display_off(ssd1306_handle_t *handle);
///
/// Toggle display power
///
/// @param handle The handle used to manage the device
///
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t ssd1306_toggle_power(ssd1306_handle_t *handle);

///
/// Set device contrast to absolute level
///
/// @param handle The handle used to manage the device
/// @param level The new contrast level, 0-255
///
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t ssd1306_set_contrast(ssd1306_handle_t *handle, uint8_t level);
///
/// Adjust device contrast level
///
/// @param handle The handle used to manage the device
/// @param incr The amount by which to adjust contrast
///
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t ssd1306_adj_contrast(ssd1306_handle_t *handle, int_t incr);

///
/// Fill a section the screen with a bit pattern
///
/// @note
/// The coordinates are in pixels, but the y coordinates are adjusted to use
/// the native 8-bit-tall sections
///
/// @param handle The handle used to manage the device
/// @param byte The byte pattern to use for the fill
/// @param xp The X start position in pixels
/// @param wp The width of the fill block in pixels
/// @param yp The Y start position in pixels
/// @param hp The height of the fill block in pixels
///
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t ssd1306_fill_section(const ssd1306_handle_t *handle, uint8_t byte, uint8_t xp, uint8_t wp, uint8_t yp, uint8_t hp);

///
/// Blank the display
///
/// @param handle The handle used to manage the device
///
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t ssd1306_clear_display(ssd1306_handle_t *handle);

///
/// Draw text on the screen
///
/// If @c SSD1306_FONT_AUTOSCALE is set, this will auto-scale the text
/// based on the @c .scale_x and @c .scale_y fields of @c font
///
/// @param handle The handle used to manage the device
/// @param font The font to use for the text
/// @param xt The X start position in unscaled-glyph-width blocks
/// @param yt The Y start position in unscaled-glyph-height blocks
/// @param text The text to print
///
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t ssd1306_draw_text(ssd1306_handle_t *handle, const ssd1306_font_t *font, uint8_t xt, uint8_t yt, const char *text);
///
/// Draw scaled text on the screen
///
/// @param handle The handle used to manage the device
/// @param font The font to use for the text
/// @param scale_x The X scaling factor
/// @param scale_y The Y scaling factor, must be 1, 2, or 4
/// @param xt The X start position in unscaled-glyph-width blocks
/// @param yt The Y start position in unscaled-glyph-height blocks
/// @param text The text to print
///
/// @returns ERR_OK if successful, otherwise an error code indicating
///  the nature of the problem encountered.
err_t ssd1306_draw_text_scaled(ssd1306_handle_t *handle, const ssd1306_font_t *font, uint8_t scale_x, uint8_t scale_y, uint8_t xt, uint8_t yt, const char *text);

#endif // _uHAL_DRIVERS_DISPLAY_SSD1306_H
