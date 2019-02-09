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

// current lcd state
// static int curcol = 0, currow = 0;
static uint8_t cur_address = 0;
static uint8_t cur_entry_mode = 0;
static uint8_t cur_function_set = 0;
static uint8_t cur_display_control = 0;


// static function definitions
static void sleep_ns(long nanoseconds);
static int max(int a, int b);
static int min(int a, int b);


int pifacecad_open_noinit(void)
{
    // All PiFace Digital are connected to the same SPI bus, only need 1 fd.
    if ((mcp23s17_fd = mcp23s17_open(bus, chip_select)) < 0) {
        return -1;
    }
    return mcp23s17_fd; // returns the fd in case user wants to use it
}

int pifacecad_open(void)
{
    pifacecad_open_noinit();

    // Set IO config
    const uint8_t ioconfig = BANK_OFF | \
                             INT_MIRROR_OFF | \
                             SEQOP_OFF | \
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

    return mcp23s17_fd;
}

void pifacecad_close(void)
{
    // disable interrupts if enabled
    const uint8_t intenb = mcp23s17_read_reg(GPINTENA, hw_addr, mcp23s17_fd);
    if (intenb) {
        mcp23s17_write_reg(0, GPINTENA, hw_addr, mcp23s17_fd);
        // now do some other interrupt stuff...
        // TODO
    }
    close(mcp23s17_fd);
}

void pifacecad_lcd_init(void)
{
    // setup sequence
    sleep_ns(DELAY_SETUP_0_NS);
    mcp23s17_write_reg(0x3, LCD_PORT, hw_addr, mcp23s17_fd);
    pifacecad_lcd_pulse_enable();

    sleep_ns(DELAY_SETUP_1_NS);
    mcp23s17_write_reg(0x3, LCD_PORT, hw_addr, mcp23s17_fd);
    pifacecad_lcd_pulse_enable();

    sleep_ns(DELAY_SETUP_2_NS);
    mcp23s17_write_reg(0x3, LCD_PORT, hw_addr, mcp23s17_fd);
    pifacecad_lcd_pulse_enable();

    mcp23s17_write_reg(0x2, LCD_PORT, hw_addr, mcp23s17_fd);
    pifacecad_lcd_pulse_enable();

    cur_function_set |= LCD_4BITMODE | LCD_2LINE | LCD_5X8DOTS;
    pifacecad_lcd_send_command(LCD_FUNCTIONSET | cur_function_set);

    cur_display_control |= LCD_DISPLAYOFF | LCD_CURSOROFF | LCD_BLINKOFF;
    pifacecad_lcd_send_command(LCD_DISPLAYCONTROL | cur_display_control);

    pifacecad_lcd_clear();

    cur_entry_mode |= LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
    pifacecad_lcd_send_command(LCD_ENTRYMODESET | cur_entry_mode);

    cur_display_control |= LCD_DISPLAYON | LCD_CURSORON | LCD_BLINKON;
    pifacecad_lcd_send_command(LCD_DISPLAYCONTROL | cur_display_control);
}


uint8_t pifacecad_read_switches(void)
{
    return mcp23s17_read_reg(SWITCH_PORT, hw_addr, mcp23s17_fd);
}

uint8_t pifacecad_read_switch(uint8_t switch_num)
{
    return (mcp23s17_read_reg(SWITCH_PORT,
                              hw_addr,
                              mcp23s17_fd) >> switch_num) & 1;
}


uint8_t pifacecad_lcd_write(const char * message)
{
    pifacecad_lcd_send_command(LCD_SETDDRAMADDR | cur_address);

    // for each character in the message
    while (*message) {
        if (*message == '\n') {
            pifacecad_lcd_set_cursor(0, 1);
        } else {
            pifacecad_lcd_send_data(*message);
            cur_address++;
        }
        message++;
    }
    return cur_address;
}

uint8_t pifacecad_lcd_set_cursor(uint8_t col, uint8_t row)
{
    col = max(0, min(col, (LCD_RAM_WIDTH / 2) - 1));
    row = max(0, min(row, LCD_MAX_LINES - 1));
    pifacecad_lcd_set_cursor_address(colrow2address(col, row));
    return cur_address;
}

void pifacecad_lcd_set_cursor_address(uint8_t address)
{
    cur_address = address % LCD_RAM_WIDTH;
    pifacecad_lcd_send_command(LCD_SETDDRAMADDR | cur_address);
}

uint8_t pifacecad_lcd_get_cursor_address(void)
{
    return cur_address;
}

/********************************************************************
 *  Modified 2014/06/26 John Wulff <immediatec@gmail.com>
 *  2.6 ms delay after LCD_CLEARDISPLAY
 *
 *  Execution time of the "Clear display" command is not specified
 *  in the HITACHI date sheet HD44780U, page 24. (Probably a misprint)
 *  It was measured and found to be 1.6 to 2.4 ms +- 0.2 ms
 *******************************************************************/

void pifacecad_lcd_clear(void)
{
    pifacecad_lcd_send_command(LCD_CLEARDISPLAY);
    sleep_ns(DELAY_CLEAR_NS);		/* 2.6 ms  - added JW 2014/06/26 */
    cur_address = 0;
}

/********************************************************************
 *  2.6 ms delay after LCD_RETURNHOME
 *
 *  Execution time of the "Return home" command is specified as 1.52 ms
 *  in the HITACHI date sheet HD44780U, page 24. (Probably meant for
 *  "Clear display")
 *  It was measured and found to be less than 0.8 ms and is probably
 *  37 us like all other commands.  To be safe the delay was added here
 *  also as it hardly influences performance.
 *******************************************************************/

void pifacecad_lcd_home(void)
{
    pifacecad_lcd_send_command(LCD_RETURNHOME);
    sleep_ns(DELAY_CLEAR_NS);		/* 2.6 ms  - added JW 2014/06/26 */
    cur_address = 0;
}

void pifacecad_lcd_display(uint8_t state)
{
  pifacecad_lcd_display_control( LCD_DISPLAYON, state );
}

void pifacecad_lcd_display_on(void)
{
    pifacecad_lcd_display(1);
}

void pifacecad_lcd_display_off(void)
{
    pifacecad_lcd_display(0);
}

void pifacecad_lcd_blink(uint8_t state)
{
  pifacecad_lcd_display_control( LCD_BLINKON, state );
}

void pifacecad_lcd_blink_on(void)
{
    pifacecad_lcd_blink(1);
}

void pifacecad_lcd_blink_off(void)
{
    pifacecad_lcd_blink(0);
}

void pifacecad_lcd_cursor(uint8_t state)
{
  pifacecad_lcd_display_control( LCD_CURSORON, state );
}

void pifacecad_lcd_cursor_on(void)
{
    pifacecad_lcd_cursor(1);
}

void pifacecad_lcd_cursor_off(void)
{
    pifacecad_lcd_cursor(0);
}

void pifacecad_lcd_backlight_on(void)
{
    pifacecad_lcd_set_backlight(1);
}

void pifacecad_lcd_backlight_off(void)
{
    pifacecad_lcd_set_backlight(0);
}

void pifacecad_lcd_move_left(void)
{
    pifacecad_lcd_send_command(LCD_CURSORSHIFT | \
                               LCD_DISPLAYMOVE | \
                               LCD_MOVELEFT);
}

void pifacecad_lcd_move_right(void)
{
    pifacecad_lcd_send_command(LCD_CURSORSHIFT | \
                               LCD_DISPLAYMOVE | \
                               LCD_MOVERIGHT);
}

void pifacecad_lcd_left_to_right(void)
{
    pifacecad_lcd_entry_mode( LCD_ENTRYLEFT, 1 );
}

void pifacecad_lcd_right_to_left(void)
{
    pifacecad_lcd_entry_mode( LCD_ENTRYLEFT, 0 );
}

void pifacecad_lcd_autoscroll(uint8_t state)
{
    pifacecad_lcd_entry_mode( LCD_ENTRYSHIFTINCREMENT, state );
}

// This will 'right justify' text from the cursor
void pifacecad_lcd_autoscroll_on(void)
{
    pifacecad_lcd_autoscroll(1);
}

// This will 'left justify' text from the cursor
void pifacecad_lcd_autoscroll_off(void)
{
    pifacecad_lcd_autoscroll(0);
}

void pifacecad_lcd_write_custom_bitmap(uint8_t location)
{
    pifacecad_lcd_send_command(LCD_SETDDRAMADDR | cur_address);
    pifacecad_lcd_send_data(location);
    cur_address++;
}

void pifacecad_lcd_store_custom_bitmap(uint8_t location, uint8_t bitmap[])
{
    location &= 0x7; // we only have 8 locations 0-7
    pifacecad_lcd_send_command(LCD_SETCGRAMADDR | (location << 3));
    int i;
    for (i = 0; i < 8; i++) {
        pifacecad_lcd_send_data(bitmap[i]);
    }
}

void pifacecad_lcd_display_control(uint8_t attr, uint8_t state)
{
    if ( state )
      cur_display_control |= attr;
    else
      cur_display_control &= 0xff ^ attr;
    pifacecad_lcd_send_command(LCD_DISPLAYCONTROL | cur_display_control);
}

void pifacecad_lcd_entry_mode(uint8_t attr, uint8_t state)
{
    if ( state )
      cur_entry_mode |= attr;
    else
      cur_entry_mode &= 0xff ^ attr;
    pifacecad_lcd_send_command(LCD_ENTRYMODESET | cur_entry_mode);
}

void pifacecad_lcd_send_command(uint8_t command)
{
    pifacecad_lcd_set_rs(0);
    pifacecad_lcd_send_byte(command);
    sleep_ns(DELAY_SETTLE_NS);
}

void pifacecad_lcd_send_data(uint8_t data)
{
    pifacecad_lcd_set_rs(1);
    pifacecad_lcd_send_byte(data);
    sleep_ns(DELAY_SETTLE_NS);
}

void pifacecad_lcd_send_byte(uint8_t b)
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
}

void pifacecad_lcd_set_rs(uint8_t state)
{
    mcp23s17_write_bit(state, PIN_RS, LCD_PORT, hw_addr, mcp23s17_fd);
}

void pifacecad_lcd_set_rw(uint8_t state)
{
    mcp23s17_write_bit(state, PIN_RW, LCD_PORT, hw_addr, mcp23s17_fd);
}

void pifacecad_lcd_set_enable(uint8_t state)
{
    mcp23s17_write_bit(state, PIN_ENABLE, LCD_PORT, hw_addr, mcp23s17_fd);
}

void pifacecad_lcd_set_backlight(uint8_t state)
{
    mcp23s17_write_bit(state, PIN_BACKLIGHT, LCD_PORT, hw_addr, mcp23s17_fd);
}

/* pulse the enable pin */
void pifacecad_lcd_pulse_enable(void)
{
    pifacecad_lcd_set_enable(1);
    sleep_ns(DELAY_PULSE_NS);
    pifacecad_lcd_set_enable(0);
    sleep_ns(DELAY_PULSE_NS);
}

uint8_t colrow2address(uint8_t col, uint8_t row)
{
    return col + ROW_OFFSETS[row];
}

uint8_t address2col(uint8_t address)
{
    return address % ROW_OFFSETS[1];
}

uint8_t address2row(uint8_t address)
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
    return a > b ? a : b;
}

static int min(int a, int b)
{
    return a < b ? a : b;
}
