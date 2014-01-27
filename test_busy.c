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
    printf("Starting\n");
    pifacecad_open_init();

    pifacecad_lcd_clear();
    pifacecad_lcd_backlight_on();
    pifacecad_lcd_write("Backlight on");
    wait_for_enter("Press enter to start");

    while (1) {
        pifacecad_lcd_clear();
        pifacecad_lcd_write("CHECKING BUSY\nCHECKING BUSY");
    }

    // pifacecad_lcd_display_off();
    // pifacecad_close();
}
