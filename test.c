#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "pifacecad.h"


void wait_for_enter(const char * message)
{
    printf("%s", message);
    while (getchar() != '\n');
}

int main(void)
{
    pifacecad_open();

    pifacecad_lcd_backlight_on();
    wait_for_enter("Backlight on");

    pifacecad_lcd_write("Hello, World!");
    wait_for_enter("write");

    pifacecad_lcd_set_cursor(5, 1);
    wait_for_enter("set cursor to 5, 1");

    int address = pifacecad_lcd_get_cursor_address();
    pifacecad_lcd_set_cursor_address(address + 1);
    wait_for_enter("set cursor by address + 1");

    pifacecad_lcd_clear();
    wait_for_enter("clear");

    pifacecad_lcd_write("Screw you guys\nI'm going home");
    pifacecad_lcd_home();
    wait_for_enter("home and newline");

    pifacecad_lcd_display_off();
    wait_for_enter("display off");

    pifacecad_lcd_display_on();
    wait_for_enter("display on");

    pifacecad_lcd_blink_off();
    wait_for_enter("blink off");
    pifacecad_lcd_blink_on();
    wait_for_enter("blink on");

    pifacecad_lcd_cursor_off();
    wait_for_enter("cursor off");
    pifacecad_lcd_cursor_on();
    wait_for_enter("cursor on");

    pifacecad_lcd_backlight_off();
    wait_for_enter("backlight off");

    pifacecad_lcd_move_left();
    wait_for_enter("move left");

    pifacecad_lcd_move_right();
    wait_for_enter("move right");

    pifacecad_lcd_clear();
    pifacecad_lcd_left_to_right();
    pifacecad_lcd_write("left 2 right");
    wait_for_enter("left to right");

    pifacecad_lcd_clear();
    pifacecad_lcd_set_cursor(15, 1);
    pifacecad_lcd_right_to_left();
    pifacecad_lcd_write("right 2 left");
    wait_for_enter("right to left");

    pifacecad_lcd_clear();
    pifacecad_lcd_left_to_right();
    uint8_t bitmap[8] = {0x10, 0x8, 0x7, 0xf, 0xf, 0xe, 0x0, 0x0};
    pifacecad_lcd_store_custom_bitmap(0, bitmap);
    pifacecad_lcd_write_custom_bitmap(0);
    wait_for_enter("custom bitmap");

    pifacecad_lcd_clear();
    pifacecad_lcd_display_off();
    pifacecad_close();
}
