#include "../../libraries/I2C/I2C_Bus.h"         // for LTC

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <curses.h>
#include <bcm2835.h>


uint8_t ltc_regs[8];                // LTC data
uint8_t bq2x_reg8;                   // BQ data

int main()
{
    float charge_level;
    int i=0;
    char ltc_status[256] = "\0";
    char bq2x_status[256] = "\0";

    short int correct = 0;

    bcm2835_init();
    bcm2835_gpio_fsel(RPI_GPIO_P1_07, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_clr(RPI_GPIO_P1_07);
// ----------------------------------------------
//          LTC init
    I2CBus ltc2941("/dev/i2c-1");
    ltc2941.addrSet(0x64);
    ltc2941.writeByte(0x01, 0xFA);
// ----------------------------------------------

// ----------------------------------------------
//          BQ init
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
    initscr();
    timeout(0);

while(!i)
{
    usleep(1);
    i = getch();

    if(i == 'q' || i == 'Q') i = 1;
    else i = 0;
// ----------------------------------------------
//          Reading REGs
    ltc2941.readBlock(0, 8, ltc_regs);
    bq2x_reg8 = bq2x.readByte(0x08);

    system("clear");
    printf("__________________________________________\n\n\r");

    charge_level = (float)(((ltc_regs[2]<<8) & 0xFF00) | ((ltc_regs[3]) & 0x00FF));
    charge_level = charge_level * 100 / 65535.;

    switch(ltc_regs[0])
    {
        case 0x80: strcpy(ltc_status, "ok"); break;
        case 0xA0: strcpy(ltc_status, "Charge overflow/underflow"); break;
        case 0x88: strcpy(ltc_status, "Charge alert high"); break;
        case 0x84: strcpy(ltc_status, "Charge alert low"); break;
        case 0x82: strcpy(ltc_status, "V_bat alert"); break;
        case 0x81: strcpy(ltc_status, "Undervoltage lockout alert"); break;
        default: strcpy(ltc_status, "UNDEFINED!"); break;
    }
    printf("   LTC:\n\r");

    if(correct == 1)
    {
    printf("   Status: %s   Charge level: %.0f\% \n\n\r",ltc_status, charge_level);
    }
    else
    {
    printf("   Status: %s\n\r   Charge level: Undefined\n\r   Wait till the led switches \(%.0f\%\)  \n\n\r",ltc_status, charge_level);
    }

    switch(bq2x_reg8 & 0x30)
    {
        case 0x00: strcpy(bq2x_status, "Not charging");
        break;

        case 0x10: strcpy(bq2x_status, "Pre-charge");
        break;

        case 0x20: strcpy(bq2x_status, "Charging");
        break;

        case 0x30: strcpy(bq2x_status, "Charge termination done");
        bcm2835_gpio_set(RPI_GPIO_P1_07);
        delay(1);
        bcm2835_gpio_clr(RPI_GPIO_P1_07);
        correct = 1;
        break;
    }
//    if(bq2x_reg8 == 0xC0) {strcpy(bq2x_status, "Battery is disconnected");}

    printf("   BQ2x:\n\r");
    printf("   Charging status: %s\n\r", bq2x_status);
    printf("__________________________________________\n\n\r");
    printf("Press \'q\' to exit\n\r");


    fflush(stdout);
    delay(300);
}
// ----------------------------------------------
endwin();

    ltc2941.~I2CBus();
    bq2x.~I2CBus();
    bcm2835_close();
return 0;
}
