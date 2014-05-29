#include "I2C/I2C_Bus.h"            // for LTC

#include "mpu9150/mpu9150_app.h"    // for MPU

#include <stdio.h>                  // gor GPS
#include <stdlib.h>
#include "GPS/gps.h"
/**
    объекты mpudata_t mpu9150, *mpu должны
    быть глобальными!
*/
mpudata_t mpu9150, *mpu = &mpu9150; // MPU data
loc_t gps_data;                     // GPS data
uint8_t ltc_regs[8];                // LTC data


int main(void)
{
    int tmp;



//-----------------------------------------------
//          LTC init
    I2CBus ltc2941("/dev/i2c-1");
    ltc2941.addrSet(0x64);
    ltc2941.writeByte(0x01, 0xFC);
// Добавить: GPIO_07 чтобы ловить прерывания
//-----------------------------------------------

//-----------------------------------------------
//          MPU init
    mpu9150_simple_init();
    memset(mpu, 0, sizeof(mpudata_t));
//-----------------------------------------------

//-----------------------------------------------
//          GPS init
    gps_on();                   // power on
    gps_init();                 // init (9600)
//-----------------------------------------------

for(int i = 0; i < 100; i++)
{

/**
            Чтение данных
*/
    ltc2941.readBlock(0, 8, ltc_regs);
//  0-адрес начала, 8-число байт данных
//  ltc_regs - адрес массива, в котор.читаем

    /*while(!*/tmp=mpu9150_read(mpu)/*)*/ ;
    linux_delay_ms(READ_DELAY);
// если всё нормально прочиталось, mpu_read
// вернет ноль. После чтения обязат. задержка

    gps_location(&gps_data);

/**
            Вывод данных
*/
    system("clear");
    printf("---------------------------------------------------------\n");

    printf("\tLTC:\n");
    printf("\tStatus: 0x%02x   Charge: 0x%02x%02x\n\n",
            ltc_regs[0],     ltc_regs[2], ltc_regs[3]);

    printf("\t%d MPU: \n", tmp);
    printf("\tX: %0.1f  Y: %0.1f  Z: %0.1f\n\n",
           mpu->fusedEuler[VEC3_X] * RAD_TO_DEGREE,
           mpu->fusedEuler[VEC3_Y] * RAD_TO_DEGREE,
           mpu->fusedEuler[VEC3_Z] * RAD_TO_DEGREE );

    printf("\tGPS:\n");
    printf("\tLatitude: %lf     Longitude: %lf\n",
           gps_data.latitude, gps_data.longitude);

    printf("\n\ti = %d\n", i);
    printf("---------------------------------------------------------\n");

    fflush(stdout);


}

    ltc2941.~I2CBus();
	mpu9150_exit();
	gps_off();

	return 0;
}
