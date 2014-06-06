#include <stdio.h>                  // gor GPS
#include <stdlib.h>
#include "../../libraries/GPS/gps.h"

#include <curses.h>
#include <unistd.h>
#include <bcm2835.h>
/**
    объекты mpudata_t mpu9150, *mpu должны
    быть глобальными!
*/
loc_t gps_data;                     // GPS data
int main(void)
{
    int i = 0, tmp;

//-----------------------------------------------
//          GPS init
    gps_on();                   // power on
    gps_init();                 // init (9600)
//-----------------------------------------------

    initscr();
    timeout(0);

while(!i)
{
    usleep(1);
    i = getch();

    if(i == 'q' || i == 'Q') i = 1;
    else i = 0;
/**
            Чтение данных
*/
    gps_location(&gps_data);
    delay(1);

/**
            Вывод данных
*/
    system("clear");
    printf("_________________________________________\n\n\r");


    printf("GPS:\n\r");
    printf("Latitude: %lf     Longitude: %lf\n\r",
           gps_data.latitude, gps_data.longitude);

 //   printf("i = %d\n\r", i);
    printf("_________________________________________\n\n\r");
    printf("Press 'q' to exit\n\r");

    fflush(stdout);

}
    endwin();

	gps_off();

	return 0;
}
