#include "../../libraries/mpu9150/mpu9150_app.h"    // for MPU

#include <stdio.h>
#include <stdlib.h>


#include <curses.h>
#include <unistd.h>

/**
    объекты mpudata_t mpu9150, *mpu должны
    быть глобальными!
*/
mpudata_t mpu9150, *mpu = &mpu9150; // MPU data

int main(void)
{
    int i = 0, tmp;


//-----------------------------------------------
//          MPU init
    mpu9150_simple_init();
    memset(mpu, 0, sizeof(mpudata_t));
//-----------------------------------------------

    initscr();
    timeout(0);
i=0;
while(!i)
{
    usleep(1);
    i = getch();

    if(i == 'q' || i == 'Q') i = 1;
    else i = 0;
/**
            Чтение данных
*/
    do
    {
       tmp = mpu9150_read(mpu);
    }
    while(!tmp);

    linux_delay_ms(READ_DELAY);
// если всё нормально прочиталось, mpu_read
// вернет ноль. После чтения обязат. задержка

/**
            Вывод данных
*/
    system("clear");
    printf("________________________________________________\n\n\r");
    printf("MPU: \n\r");
    printf("X: %0.1f  Y: %0.1f  Z: %0.1f\n\n\r",
           mpu->fusedEuler[VEC3_X] * RAD_TO_DEGREE,
           mpu->fusedEuler[VEC3_Y] * RAD_TO_DEGREE,
           mpu->fusedEuler[VEC3_Z] * RAD_TO_DEGREE );

    printf("________________________________________________\n\r");
    printf("\nPress 'q' to exit\n\r");

    fflush(stdout);

}
    endwin();

	mpu9150_exit();

	return 0;
}
