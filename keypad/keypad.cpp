#include <stdio.h>
#include "matrix_keypad.h"
#include <curses.h>
#include <unistd.h>
int main()
{
    char c;
    int i = 0;

    if(init_keypad())
    {
        printf("Keypad init error!\n");
        return 1;
    }



    initscr();
    timeout(0);
    i = 0;
    printf("Press \'q\' to exit\n\r");
while(!i)
    {
        usleep(1);
        i = getch();

        if(i == 'q' || i == 'Q') i = 1;
        else i = 0;

        c = get_key();
        if(c != '\0') printf("%c", c);
        //else printf("x");
        fflush(stdout);
    }
    endwin();
    bcm2835_close();
    return 0;
}
