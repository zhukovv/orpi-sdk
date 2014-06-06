#include "../../libraries/I2C/I2C_Bus.h"            // for LTC

#include "../../libraries/mpu9150/mpu9150_app.h"    // for MPU

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
mpudata_t mpu9150, *mpu = &mpu9150; // MPU data
loc_t gps_data;                     // GPS data
uint8_t ltc_regs[8];                // LTC data
uint8_t bq2x_reg8;//, bq2x_reg9;       // BQ2X data


int main(void)
{
    int i = 0, tmp;
    float charge_level;
    char ltc_status[256] = "\0";
    char bq2x_status[256] = "\0";
//    char c;

    bcm2835_init();
    bcm2835_gpio_fsel(RPI_GPIO_P1_07, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_clr(RPI_GPIO_P1_07);

//-----------------------------------------------
//          LTC init
    I2CBus ltc2941("/dev/i2c-1");
    ltc2941.addrSet(0x64);
    ltc2941.writeByte(0x01, 0xFA);
// Добавить: GPIO_07 чтобы ловить прерывания
//-----------------------------------------------

// ----------------------------------------------
//          BQ2x init
    I2CBus bq2x("/dev/i2c-1");
    bq2x.addrSet(0x6B);
    bq2x.writeByte(0x00, 0x5F);
    bq2x.writeByte(0x01, 0x3A);
    bq2x.writeByte(0x02, 0x80);
    bq2x.writeByte(0x03, 0x11);
    bq2x.writeByte(0x04, 0xB2);
    bq2x.writeByte(0x05, 0x9C);
    bq2x.writeByte(0x06, 0x90);

// ----------------------------------------------

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
    ltc2941.readBlock(0, 8, ltc_regs);
//  0-адрес начала, 8-число байт данных
//  ltc_regs - адрес массива, в котор.читаем

    bq2x_reg8 = bq2x.readByte(0x08);
//    bq2x_reg9 = bq2x.readByte(0x09);

    /*while(!*/tmp=mpu9150_read(mpu)/*)*/ ;
    linux_delay_ms(READ_DELAY);
// если всё нормально прочиталось, mpu_read
// вернет ноль. После чтения обязат. задержка

    gps_location(&gps_data);

/**
            Вывод данных
*/
    system("clear");
    printf("---------------------------------------------------------\n\r");

    charge_level = (float)(((ltc_regs[2]<<8) & 0xFF00) | ((ltc_regs[3]) & 0x00FF));
    charge_level = charge_level * 100 / 65535.;

    switch(ltc_regs[0])
    {
        case 0x80: strcpy(ltc_status, "Ok"); break;
        case 0xA0: strcpy(ltc_status, "Charge overflow/underflow"); break;
        case 0x88: strcpy(ltc_status, "Charge alert high"); break;
        case 0x84: strcpy(ltc_status, "Charge alert low"); break;
        case 0x82: strcpy(ltc_status, "V_bat alert"); break;
        case 0x81: strcpy(ltc_status, "Undervoltage lockout alert"); break;
        default: strcpy(ltc_status, "UNDEFINED!"); break;
    }
    printf("LTC:\n\r");
    printf("Status: %s   Charge level: %.0f%\n\n\r",
            ltc_status,     charge_level);

    switch(bq2x_reg8 & 0x30)
    {
        case 0x00: strcpy(bq2x_status, "Not_charging");
        break;

        case 0x10: strcpy(bq2x_status, "Pre-charge");
        break;

        case 0x20: strcpy(bq2x_status, "Charging");
        break;

        case 0x30: strcpy(bq2x_status, "Charge termination done");
        bcm2835_gpio_fsel(RPI_GPIO_P1_07, BCM2835_GPIO_FSEL_OUTP);
        bcm2835_gpio_set(RPI_GPIO_P1_07);
        delayMicroseconds(100);
        bcm2835_gpio_clr(RPI_GPIO_P1_07);
        break;
    }
    if(bq2x_reg8 == 0xC0) {strcpy(bq2x_status, "Battery is disconnected");}

    printf("BQ2x: \n\r");
    printf("Charging status: %s\n\n\r", bq2x_status);

    printf("MPU: \n\r");
    printf("X: %0.1f  Y: %0.1f  Z: %0.1f\n\n\r",
           mpu->fusedEuler[VEC3_X] * RAD_TO_DEGREE,
           mpu->fusedEuler[VEC3_Y] * RAD_TO_DEGREE,
           mpu->fusedEuler[VEC3_Z] * RAD_TO_DEGREE );

    printf("GPS:\n\r");
    printf("Latitude: %lf     Longitude: %lf\n\r",
           gps_data.latitude, gps_data.longitude);

 //   printf("i = %d\n\r", i);
    printf("---------------------------------------------------------\n\r");
    printf("Press 'q' to exit\n\n\r");

    fflush(stdout);

}
    endwin();

    ltc2941.~I2CBus();
    bq2x.~I2CBus();
	mpu9150_exit();
	gps_off();

	return 0;
}
