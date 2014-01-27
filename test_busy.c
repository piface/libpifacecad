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

    while (1) {
        pifacecad_lcd_write("CHECKING BUSY\nCHECKING BUSY");
        pifacecad_lcd_clear();
    }

    // pifacecad_lcd_display_off();
    // pifacecad_close();
}
