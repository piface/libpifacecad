#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <unistd.h>
#include <mcp23s17.h>
#include "pifacecad.h"


// PiFace Control and Display is always at /dev/spidev0.1, hw_addr = 0
static const int bus = 0, chip_select = 1, hw_addr = 0;
static int mcp23s17_fd = 0; // MCP23S17 SPI file descriptor

static const int SWITCH_PORT = GPIOA;
static const int LCD_PORT = GPIOB;

static int curcol = 0, currow = 0;

static uint8_t display_control = 0;


// static function definitions
static void sleep_ns(long nanoseconds);
static int max(int a, int b);
static int min(int a, int b);


int pifacecad_open_noinit()
{
    // All PiFace Digital are connected to the same SPI bus, only need 1 fd.
    if ((mcp23s17_fd = mcp23s17_open(bus, chip_select)) < 0) {
        return -1;
    }
    return mcp23s17_fd; // returns the fd in case user wants to use it
}

int pifacecad_open()
{
    pifacecad_open_noinit();

    // Set IO config
    const uint8_t ioconfig = BANK_OFF | \
                             INT_MIRROR_OFF | \
                             SEQOP_ON | \
                             DISSLW_OFF | \
                             HAEN_ON | \
                             ODR_OFF | \
                             INTPOL_LOW;
    mcp23s17_write_reg(ioconfig, IOCON, hw_addr, mcp23s17_fd);

    // Set GPIO Port A as inputs (switches)
    mcp23s17_write_reg(0xff, IODIRA, hw_addr, mcp23s17_fd);
    mcp23s17_write_reg(0xff, GPPUA, hw_addr, mcp23s17_fd);

    // Set GPIO Port B as outputs (connected to HD44780)
    mcp23s17_write_reg(0x00, IODIRB, hw_addr, mcp23s17_fd);

    // enable interrupts
    mcp23s17_write_reg(0xFF, GPINTENA, hw_addr, mcp23s17_fd);

    pifacecad_lcd_init();

    return 0;
}

int pifacecad_close()
{
    // disable interrupts if enabled
    const uint8_t intenb = mcp23s17_read_reg(GPINTENA, hw_addr, mcp23s17_fd);
    if (intenb) {
        mcp23s17_write_reg(0, GPINTENA, hw_addr, mcp23s17_fd);
        // now do some other interrupt stuff...
        // TODO
    }
    return close(mcp23s17_fd);
}

int pifacecad_lcd_init()
{
    // setup sequence
    sleep_ns(DELAY_SETUP_0);
    mcp23s17_write_reg(0x3, LCD_PORT, hw_addr, mcp23s17_fd);
    pifacecad_lcd_pulse_enable();

    sleep_ns(DELAY_SETUP_1);
    mcp23s17_write_reg(0x3, LCD_PORT, hw_addr, mcp23s17_fd);
    pifacecad_lcd_pulse_enable();

    sleep_ns(DELAY_SETUP_2);
    mcp23s17_write_reg(0x3, LCD_PORT, hw_addr, mcp23s17_fd);
    pifacecad_lcd_pulse_enable();

    mcp23s17_write_reg(0x2, LCD_PORT, hw_addr, mcp23s17_fd);
    pifacecad_lcd_pulse_enable();

    pifacecad_lcd_update_function_set(LCD_4BITMODE | LCD_2LINE | LCD_5X8DOTS);
    pifacecad_lcd_update_display_control(LCD_DISPLAYOFF);
    pifacecad_lcd_clear();
    pifacecad_lcd_update_entry_mode(LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT);
    pifacecad_lcd_update_display_control(
        LCD_DISPLAYON | LCD_CURSORON | LCD_BLINKON);

    return 0;
}


uint8_t pifacecad_read_switches()
{
    return mcp23s17_read_reg(SWITCH_PORT, hw_addr, mcp23s17_fd);
}

uint8_t pifacecad_read_switch(uint8_t switch_num)
{
    return (mcp23s17_read_reg(SWITCH_PORT,
                              hw_addr,
                              mcp23s17_fd) >> switch_num) & 1;
}


int pifacecad_lcd_write(const char * message)
{
    pifacecad_lcd_set_ddram_address(colrow2address(curcol, currow));

    // for each character in the message
    while (*message) {
        if (*message == '\n') {
            pifacecad_lcd_set_cursor(0, 1);
        } else {
            pifacecad_lcd_send_data(*message);
            curcol++;;
        }
        message++;
    }
    return 0;
}


int pifacecad_lcd_set_cursor(int col, int row)
{
    // if (col == 0 && row == 1) {
    //     pifacecad_lcd_send_command(LCD_NEWLINE);
    // } else {
    //     col = max(0, min(col, LCD_RAM_WIDTH - 1));
    //     row = max(0, min(row, LCD_MAX_LINES - 1));
    //     pfcad_set_ddram_address(colrow2address(col, row));
    // }
    // curcol = col;
    // currow = row;
    col = max(0, min(col, LCD_RAM_WIDTH - 1));
    row = max(0, min(row, LCD_MAX_LINES - 1));
    return pifacecad_lcd_set_cursor_address(colrow2address(col, row));
}

int pifacecad_lcd_set_cursor_address(int address)
{
    pifacecad_lcd_set_ddram_address(address);
    curcol = address2col(address);
    currow = address2row(address);
    return 0;
}

int pifacecad_lcd_get_cursor_address()
{
    return colrow2address(curcol, currow);
}


int pifacecad_lcd_clear()
{
    pifacecad_lcd_send_command(LCD_CLEARDISPLAY);
    curcol = 0;
    currow = 0;
    return 0;
}

int pifacecad_lcd_home()
{
    pifacecad_lcd_send_command(LCD_RETURNHOME);
    curcol = 0;
    currow = 0;
    return 0;
}


int pifacecad_lcd_display_on()
{
    return pifacecad_lcd_update_display_control(display_control | LCD_DISPLAYON);
}

int pifacecad_lcd_display_off()
{
    return pifacecad_lcd_update_display_control(display_control & ~LCD_DISPLAYON);
}

int pifacecad_lcd_blink_on()
{
    return pifacecad_lcd_update_display_control(display_control | LCD_BLINKON);
}

int pifacecad_lcd_blink_off()
{
    return pifacecad_lcd_update_display_control(display_control & ~LCD_BLINKOFF);
}

int pifacecad_lcd_cursor_on()
{
    return pifacecad_lcd_update_display_control(display_control | LCD_CURSORON);
}

int pifacecad_lcd_cursor_off()
{
    return pifacecad_lcd_update_display_control(display_control & ~LCD_CURSOROFF);
}

int pifacecad_lcd_backlight_on()
{
    return pifacecad_lcd_set_backlight(1);
}

int pifacecad_lcd_backlight_off()
{
    return pifacecad_lcd_set_backlight(0);
}


int pifacecad_lcd_move_left()
{
    return -1;
}

int pifacecad_lcd_move_right()
{
    return -1;
}

int pifacecad_lcd_left_justify()
{
    return -1;
}

int pifacecad_lcd_right_justify()
{
    return -1;
}

int pifacecad_lcd_left_to_right()
{
    return -1;
}

int pifacecad_lcd_right_to_left()
{
    return -1;
}

int pifacecad_lcd_set_cgram_address(int address)
{
    return -1;
}

int pifacecad_lcd_set_ddram_address(int address)
{
    return pifacecad_lcd_send_command(LCD_SETDDRAMADDR | address);
}

int pifacecad_lcd_update_function_set(uint8_t state)
{
    return pifacecad_lcd_send_command(LCD_FUNCTIONSET | state);
}

int pifacecad_lcd_update_entry_mode(uint8_t state)
{
    return pifacecad_lcd_send_command(LCD_ENTRYMODESET | state);
}

int pifacecad_lcd_update_display_control(uint8_t state)
{
    return pifacecad_lcd_send_command(LCD_DISPLAYCONTROL | state);
}

int pifacecad_lcd_send_command(uint8_t command)
{
    pifacecad_lcd_set_rs(0);
    pifacecad_lcd_send_byte(command);
    sleep_ns(DELAY_SETTLE_NS);
    return 0;
}

int pifacecad_lcd_send_data(uint8_t data)
{
    pifacecad_lcd_set_rs(1);
    pifacecad_lcd_send_byte(data);
    sleep_ns(DELAY_SETTLE_NS);
    return 0;
}

int pifacecad_lcd_send_byte(uint8_t b)
{
    // get current lcd port state and clear the data bits
    uint8_t current_state = mcp23s17_read_reg(LCD_PORT, hw_addr, mcp23s17_fd);
    current_state &= 0xF0; // clear the data bits

    // send first nibble (0bXXXX0000)
    uint8_t new_byte = current_state | ((b >> 4) & 0xF);  // set nibble
    mcp23s17_write_reg(new_byte, LCD_PORT, hw_addr, mcp23s17_fd);
    pifacecad_lcd_pulse_enable();

    // send second nibble (0b0000XXXX)
    new_byte = current_state | (b & 0xF);  // set nibble
    mcp23s17_write_reg(new_byte, LCD_PORT, hw_addr, mcp23s17_fd);
    pifacecad_lcd_pulse_enable();
    return 0;
}

int pifacecad_lcd_set_rs(uint8_t state)
{
    mcp23s17_write_bit(state, PIN_RS, LCD_PORT, hw_addr, mcp23s17_fd);
    return 0;
}

int pifacecad_lcd_set_rw(uint8_t state)
{
    mcp23s17_write_bit(state, PIN_RW, LCD_PORT, hw_addr, mcp23s17_fd);
    return 0;
}

int pifacecad_lcd_set_enable(uint8_t state)
{
    mcp23s17_write_bit(state, PIN_ENABLE, LCD_PORT, hw_addr, mcp23s17_fd);
    return 0;
}

int pifacecad_lcd_set_backlight(uint8_t state)
{
    mcp23s17_write_bit(state, PIN_BACKLIGHT, LCD_PORT, hw_addr, mcp23s17_fd);
    return 0;
}

/* pulse the enable pin */
int pifacecad_lcd_pulse_enable()
{
    pifacecad_lcd_set_enable(1);
    sleep_ns(DELAY_PULSE_NS);
    pifacecad_lcd_set_enable(0);
    sleep_ns(DELAY_PULSE_NS);
    return 0;
}


int colrow2address(int col, int row)
{
    return col + ROW_OFFSETS[row];
}

int address2col(int address)
{
    return address % ROW_OFFSETS[1];
}

int address2row(int address)
{
    return address > ROW_OFFSETS[1] ? 1 : 0;
}

static void sleep_ns(long nanoseconds)
{
    struct timespec time0, time1;
    time0.tv_sec = 0;
    time0.tv_nsec = nanoseconds;
    nanosleep(&time0 , &time1);
}

static int max(int a, int b)
{
    if (a > b) {
        return a;
    } else {
        return b;
    }
}

static int min(int a, int b)
{
    if (a < b) {
        return a;
    } else {
        return b;
    }
}
