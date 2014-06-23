#include <stdio.h>
#include "matrix_keypad.h"

int main()
{
    char c;

    if(!init_keypad())
    {
        printf("Keypad init error!\n");
        return 1;
    }


    while(1)
    {
        c = get_key();
        if(c != '\0') printf("%c", c);
    }

    // TODO: bcm2835_close()
    return 0;
}
