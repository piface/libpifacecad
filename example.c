#include "pifacecad.h"


int main(void)
{
    pifacecad_open();
    pifacecad_lcd_backlight_on();
    pifacecad_lcd_write("Hello, World!");
    pifacecad_close();
}
