/**
 * @file  pifacecad.h
 * @brief A simple static library for controlling PiFace Control and Display.
 *
 * Copyright (C) 2013 Thomas Preston <thomas.preston@openlx.org.uk>
 * Modified 2014/06/26 John Wulff <immediatec@gmail.com> 2.6 ms delay after LCD_CLEARDISPLAY
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _PIFACECAD_H
#define _PIFACECAD_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DELAY_PULSE_NS 1000 // 1us
#define DELAY_SETTLE_NS 40000 // 40us
#define DELAY_CLEAR_NS 2600000L // 2.6ms
#define DELAY_SETUP_0_NS 15000000L // 15ms
#define DELAY_SETUP_1_NS 5000000L // 5ms
#define DELAY_SETUP_2_NS 1000000L // 1ms

// mcp23s17 GPIOB to HD44780 pin map
#define PIN_D4 0
#define PIN_D5 1
#define PIN_D6 2
#define PIN_D7 3
#define PIN_ENABLE 4
#define PIN_RW 5
#define PIN_RS 6
#define PIN_BACKLIGHT 7

// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80
#define LCD_NEWLINE 0xC0

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5X10DOTS 0x04
#define LCD_5X8DOTS 0x00

#define LCD_MAX_LINES 2
#define LCD_WIDTH 16
#define LCD_RAM_WIDTH 80 // RAM is 80 wide, split over two lines

static const uint8_t ROW_OFFSETS[] = {0, 0x40};

/**
 * Opens and initialises a PiFace Control and Display.
 * Returns a file descriptor for making raw SPI transactions to the
 * MCP23S17 (for advanced users only).
 *
 * Example:
 *
 *     pifacecad_open();
 *     int pifacedigital_fd = pifacecad_open(); // advanced
 *
 */
int pifacecad_open(void);

/**
 * Opens a PiFace Control and Display without initialising it.
 * Returns a file descriptor for making raw SPI transactions to the
 * MCP23S17 (for advanced users only).
 *
 * Example:
 *
 *     pifacecad_open_noinit();
 *     int pifacedigital_fd = pifacecad_open_noinit(); // advanced
 *
 */
int pifacecad_open_noinit(void);

/**
 * Closes a PiFace Control and Display (turns off interrupts, closes file
 * descriptor).
 *
 * Example:
 *
 *     pifacecad_close();
 *
 */
void pifacecad_close(void);

/**
 * Initialised the LCD. You will not need to call this if you have called
 * pifacecad_open.
 *
 * Example:
 *
 *     pifacecad_lcd_init();
 *
 */
void pifacecad_lcd_init(void);

/**
 * Reads the entire switch port.
 *
 * Example:
 *
 *     uint8_t switch_bits = pifacecad_read_switches();
 *
 */
uint8_t pifacecad_read_switches(void);

/**
 * Reads a single switch.
 *
 * Example (read switch 3):
 *
 *     uint8_t switch_value = pifacecad_read_switch(3);
 *
 */
uint8_t pifacecad_read_switch(uint8_t switch_num);

/**
 * Writes a message to the LCD screen starting from the current cursor
 * position. Accepts '\\n'. Returns the current cursor address.
 *
 * Example:
 *
 *     pifacecad_lcd_write("Hello, World!");
 *     pifacecad_lcd_write("Moving to a\nnew line is easy");
 *     uint8_t address = pifacecad_lcd_write("Cursor moves after\nwriting");
 *
 */
uint8_t pifacecad_lcd_write(const char * message);

/**
 * Sets the cursor position on the screen (col , row).
 *
 * Example:
 *
 *     pifacecad_lcd_set_cursor(5, 1);
 *
 */
uint8_t pifacecad_lcd_set_cursor(uint8_t col, uint8_t row);

/**
 * Sets the cursor position on the screen (address = col + row*40).
 *
 * Example:
 *
 *     pifacecad_lcd_set_cursor(41); // col=1, row=40
 *
 */
void pifacecad_lcd_set_cursor_address(uint8_t address);

/**
 * Returns the cursor position (address).
 *
 * Example:
 *
 *     uint8_t cursor_addr = pifacecad_lcd_get_cursor_address();
 *
 */
uint8_t pifacecad_lcd_get_cursor_address(void);

/**
 * Clears the screen and returns the cursor to home (0, 0).
 *
 * Example:
 *
 *     pifacecad_lcd_clear();
 *
 */
void pifacecad_lcd_clear(void);

/**
 * Returns the cursor to home (0, 0).
 *
 * Example:
 *
 *     pifacecad_lcd_home();
 *
 */
void pifacecad_lcd_home(void);

/**
 * Turns the display on or off.
 *
 * Example:
 *
 *     pifacecad_lcd_display(1);
 *
 */
void pifacecad_lcd_display(uint8_t state);

/**
 * Turns the display on.
 *
 * Example:
 *
 *     pifacecad_lcd_display_on();
 *
 */
void pifacecad_lcd_display_on(void);

/**
 * Turns the display off.
 *
 * Example:
 *
 *     pifacecad_lcd_display_off();
 *
 */
void pifacecad_lcd_display_off(void);

/**
 * Turns the blinking cursor on or off.
 *
 * Example:
 *
 *     pifacecad_lcd_blink(1);
 *
 */
void pifacecad_lcd_blink(uint8_t state);

/**
 * Turns the blink on.
 *
 * Example:
 *
 *     pifacecad_lcd_blink_on();
 *
 */
void pifacecad_lcd_blink_on(void);

/**
 * Turns the blink off.
 *
 * Example:
 *
 *     pifacecad_lcd_blink_off();
 *
 */
void pifacecad_lcd_blink_off(void);

/**
 * Turns the underline cursor on or off
 *
 * Example:
 *
 *     pifacecad_lcd_cursor(1);
 *
 */
void pifacecad_lcd_cursor(uint8_t state);

/**
 * Turns the cursor on.
 *
 * Example:
 *
 *     pifacecad_lcd_cursor_on();
 *
 */
void pifacecad_lcd_cursor_on(void);

/**
 * Turns the cursor off.
 *
 * Example:
 *
 *     pifacecad_lcd_cursor_off();
 *
 */
void pifacecad_lcd_cursor_off(void);

/**
 * Turns the backlight on.
 *
 * Example:
 *
 *     pifacecad_lcd_backlight_on();
 *
 */
void pifacecad_lcd_backlight_on(void);

/**
 * Turns the backlight off.
 *
 * Example:
 *
 *     pifacecad_lcd_backlight_off();
 *
 */
void pifacecad_lcd_backlight_off(void);

/**
 * Moves the display left.
 *
 * Example:
 *
 *     pifacecad_lcd_move_left();
 *
 */
void pifacecad_lcd_move_left(void);

/**
 * Moves the display right.
 *
 * Example:
 *
 *     pifacecad_lcd_move_right();
 *
 */
void pifacecad_lcd_move_right(void);

/**
 * The cursor will move to the right after printing causing text to read
 * left to right.
 *
 * Example:
 *
 *     pifacecad_lcd_left_to_right();
 *
 */
void pifacecad_lcd_left_to_right(void);

/**
 * The cursor will move to the left after printing causing text to read
 * right to left.
 *
 * Example:
 *
 *     pifacecad_lcd_right_to_left();
 *
 */
void pifacecad_lcd_right_to_left(void);

/**
 * The screen will follow text if it moves out of view.
 *
 * Example:
 *
 *     pifacecad_lcd_autoscroll(1);
 *
 */
void pifacecad_lcd_autoscroll(uint8_t state);

/**
 * The screen will follow text if it moves out of view.
 *
 * Example:
 *
 *     pifacecad_lcd_autoscroll_on();
 *
 */
void pifacecad_lcd_autoscroll_on(void);

/**
 * The screen will not follow text if it moves out of view.
 *
 * Example:
 *
 *     pifacecad_lcd_autoscroll_off();
 *
 */
void pifacecad_lcd_autoscroll_off(void);

/**
 * Writes the custom bitmap stored at the specified bank location to the
 * display.
 *
 * Example:
 *
 *     pifacecad_lcd_write_custom_bitmap(0);
 *
 */
void pifacecad_lcd_write_custom_bitmap(uint8_t location);

/**
 * Stores a custom bitmap to the location specified (up to 8: 0-7).
 *
 * Example:
 *
 *     uint8_t bitmap[] = {0x15, 0xa, 0x15, 0xa, 0x15, 0xa, 0x15, 0xa};
 *     pifacecad_lcd_store_custom_bitmap(0, bitmap); // store
 *     pifacecad_lcd_write_custom_bitmap(0); // write
 *
 */
void pifacecad_lcd_store_custom_bitmap(uint8_t location, uint8_t bitmap[]);

/**
 * Send a command to the HD44780.
 *
 * Example:
 *
 *     pifacecad_lcd_send_command(LCD_CLEARDISPLAY);
 *
 */
void pifacecad_lcd_send_command(uint8_t command);

/**
 * Send data to the HD44780.
 *
 * Example:
 *
 *     pifacecad_lcd_send_data('a');
 *
 */
void pifacecad_lcd_send_data(uint8_t data);

/**
 * Send a byte to the HD44780.
 *
 * Example:
 *
 *     pifacecad_lcd_send_byte(0xaa);
 *
 */
void pifacecad_lcd_send_byte(uint8_t byte);

/**
 * Set the RS pin on the HD44780.
 *
 * Example:
 *
 *     pifacecad_lcd_set_rs(1);
 *
 */
void pifacecad_lcd_set_rs(uint8_t state);

/**
 * Set the RW pin on the HD44780.
 *
 * Example:
 *
 *     pifacecad_lcd_set_rw(1);
 *
 */
void pifacecad_lcd_set_rw(uint8_t state);

/**
 * Set the enable pin on the HD44780.
 *
 * Example:
 *
 *     pifacecad_lcd_set_enable(1);
 *
 */
void pifacecad_lcd_set_enable(uint8_t state);

/**
 * Set the backlight pin on the HD44780.
 *
 * Example:
 *
 *     pifacecad_lcd_set_backlight(1);
 *
 */
void pifacecad_lcd_set_backlight(uint8_t state);

/**
 * Pulse the enable pin on the HD44780.
 *
 * Example:
 *
 *     pifacecad_lcd_set_backlight(1);
 *
 */
void pifacecad_lcd_pulse_enable(void);

/**
 * Returns an address calculated from a column and a row.
 *
 * Example:
 *
 *     uint8_t address = colrow2address(5, 1);
 *
 */
uint8_t colrow2address(uint8_t col, uint8_t row);

/**
 * Returns a column calculated from an address.
 *
 * Example:
 *
 *     uint8_t col = address2col(42); // col=2
 *
 */
uint8_t address2col(uint8_t address);

/**
 * Returns a row calculated from an address.
 *
 * Example:
 *
 *     uint8_t row = address2col(42); // row=1
 *
 */
uint8_t address2row(uint8_t address);

// int pifacecad_lcd_set_viewport_corner(int col);
// int pifacecad_lcd_see_cursor(int col);

void pifacecad_lcd_display_control(uint8_t attr, uint8_t state);
void pifacecad_lcd_entry_mode(uint8_t attr, uint8_t state);

#ifdef __cplusplus
}
#endif

#endif
