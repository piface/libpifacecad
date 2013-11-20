/* compile with:
 * gcc -o example example.c -Isrc/ -L. -lpifacecad -L../libmcp23s17/ -lmcp23s17
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "pifacecad.h"


int main(void)
{
    pifacecad_open();
    pifacecad_lcd_backlight_on();
    pifacecad_lcd_write("Hello, World!");
    pifacecad_close();
}
