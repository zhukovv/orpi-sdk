#include "matrix_keypad.h"
uint8_t matrix[][3] = {
                    {'1', '2', '3'},
                    {'4', '5', '6'},
                    {'7', '8', '9'},
                    {'*', '0', '#'}
                    };

int col[] = {C0, C1, C2};
int row[] = {R0, R1, R2, R3};


int init_keypad(void)
{
    int i;
    if(!bcm2835_init()) return 1;

    for(i = 0; i < 3; i++)
    {
        bcm2835_gpio_fsel(col[i], BCM2835_GPIO_FSEL_INPT);
        bcm2835_gpio_set_pud(col[i], BCM2835_GPIO_PUD_UP);
    }

    for(i = 0; i < 4; i++)
    {
        bcm2835_gpio_fsel(row[i], BCM2835_GPIO_FSEL_OUTP);
        bcm2835_gpio_set(row[i]);
    }

    return 0;
}


uint8_t get_key(void)
{
    char c = '\0';
    int i, j;

    for(i = 0; i < 4; i++)
    {
        bcm2835_gpio_clr(row[i]);
        for(j = 0; j < 3; j++)
        {
            if(bcm2835_gpio_lev(col[j]) == 0)
            {
                bcm2835_delay(60);

                while(bcm2835_gpio_lev(col[j]) == 0)
                {;}

                //return matrix[i][j];
                c = matrix[i][j];
            }
        }
        bcm2835_gpio_set(row[i]);
    }
    bcm2835_delay(20);
    return c;

}
