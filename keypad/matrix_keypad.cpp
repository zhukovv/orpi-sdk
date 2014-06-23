#include "matrix_keypad.h"


int init_keypad(void)
{//поменяй местами outp/inpt
    if(!bcm2835_init()) return 1;

    bcm2835_gpio_fsel(R0, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(R1, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(R2, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(R3, BCM2835_GPIO_FSEL_OUTP);

    bcm2835_gpio_fsel(C0, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_fsel(C1, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_fsel(C2, BCM2835_GPIO_FSEL_INPT);

    return 0;
}


char get_key(void)
{
    char c = '\0';

}
