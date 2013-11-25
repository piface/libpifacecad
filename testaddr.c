#include <pifacecad.h>
#include <stdio.h>


int main(void)
{
    pifacecad_open();
    pifacecad_lcd_write("hello");

    int vals[] = {40, 65};

    int i;
    for (i = 0; i < 2; i++) {
        pifacecad_lcd_set_cursor_address(vals[i]);
        printf("%d\n", vals[i]);
        while (getchar() != '\n');
    }

    pifacecad_close();
}