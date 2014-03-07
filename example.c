#include <stdio.h>
#include "pifacecad.h"


int main(void)
{
    pifacecad_open();

    /**
     * Enable interrupt processing (only required for all
     * blocking/interrupt methods)
     */
    int intenable = pifacecad_enable_interrupts();
    if (intenable == 0) {
        printf("Interrupts enabled.\n");
    } else {
        printf("Could not enable interrupts. Try running using sudo to "
               "enable PiFaceDigital interrupts.\n");
    }


    pifacecad_lcd_backlight_on();
    pifacecad_lcd_write("Hello, World!");

    /**
     * Wait for input change interrupt
     */
    if(intenable) {
        printf("Interrupts disabled, skipping interrupt tests.\n");
    }
    else {
        printf("Waiting for switch (press any button on PiFace Control "
               "and Display)\n");
        uint8_t switches = pifacecad_wait_for_switch(-1);
        printf("Switches: 0x%x\n", switches);
        // pifacecad_disable_interrupts();
    }

    pifacecad_close();
}
