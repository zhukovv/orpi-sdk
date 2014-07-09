#include <stdio.h>
#include "../../libraries/I2C/I2C_Bus.h"
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <curses.h>
#include <bcm2835.h>

uint8_t bq2x_regs[10];                   // BQ data
int main()
{
    int i = 0;
    float tmp = 0;

    bcm2835_init();
    bcm2835_gpio_fsel(RPI_GPIO_P1_07, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_clr(RPI_GPIO_P1_07);

    I2CBus bq2x("/dev/i2c-1");
    bq2x.addrSet(0x6B);
   /* bq2x.writeByte(0x00, 0x5F);
    bq2x.writeByte(0x01, 0x3B);
    bq2x.writeByte(0x02, 0x80);
    bq2x.writeByte(0x03, 0x11);
    bq2x.writeByte(0x04, 0xB2);
    bq2x.writeByte(0x05, 0x9C);
    bq2x.writeByte(0x06, 0x90);*/

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
    bq2x.readBlock(0, 10, bq2x_regs);

    system("clear");
    /** REG_0x00 */
    /*
    printf("REG_0x00:\n\r");
    if(bq2x_regs[0]>>7 == 1)
        printf("High impedance mode: ENABLE\n\r");
    else
        printf("High impedance mode: DISABLE\n\r");
    */
    tmp = (bq2x_regs[0]>>6 & 0x01)*0.64 + (bq2x_regs[0]>>5 & 0x01)*0.32 +
            (bq2x_regs[0]>>4 & 0x01)*0.16 + (bq2x_regs[0]>>3 & 0x01)*0.08;
    printf("Input voltage limit: %.2f\n\r", 3.88 + tmp);

    printf("Input current limit: ");
    switch(bq2x_regs[0] & 0x07)
    {
        case 0b000: printf("100 mA"); break;
        case 0b001: printf("150 mA"); break;
        case 0b010: printf("500 mA"); break;
        case 0b011: printf("900 mA"); break;
        case 0b100: printf("1 A");    break;
        case 0b101: printf("1.5 A");  break;
        case 0b110: printf("2 A");    break;
        case 0b111: printf("3 A");    break;
    }
    printf("\n\r");


    /** REG_0x01 */
    /*
    printf("REG_0x01:\n\r");
    */
    printf("Minimum system voltage limit: ");

    tmp = (bq2x_regs[1]>>3 & 0x01)*0.4+(bq2x_regs[1]>>2 & 0x01)*0.2+(bq2x_regs[1]>>1 & 0x01)*0.1;
    printf("%.2f\n\r", 3 + tmp);

    tmp = (bq2x_regs[2]>>2) * 0.064;
    printf("I_charge: %.2f A\n\r", 0.512 + tmp);

    tmp = (bq2x_regs[4]>>2) * 0.016;
    printf("Charge voltage limit: %.3f V\n\r", 3.504 + tmp);

    /** ---------------STATUS---------------- */

    printf("\n\r     STATUS:     \n\r");
    printf("VBUS: ");
    switch(bq2x_regs[8] >> 6 & 0b11)
    {
        case 0b00: printf("Unknown"); break;
        case 0b01: printf("USB host"); break;
        case 0b10: printf("Adapter port"); break;
        case 0b11: printf("OTG"); break;
    }
    printf("\n\r");

    printf("CHRG: ");
    switch(bq2x_regs[8] >> 4 & 0b11)
    {
        case 0b00: printf("Not charging"); break;
        case 0b01: printf("Pre-charge"); break;
        case 0b10: printf("Fast charging"); break;
        case 0b11: printf("Charge termination done"); break;
    }
    printf("\n\r");

    printf("Input DPM: ");
    if(bq2x_regs[8] >> 3 & 0b1)
        printf("Not DPM\n\r");
    else
        printf("VINDPM or IINDPM\n\r");

    printf("PGOOD: ");
    if(bq2x_regs[8] >> 2 & 0b1)
        printf("Power good\n\r");
    else
        printf("Not power good\n\r");


    printf("VSYS: ");
    if(bq2x_regs[8] & 0b1)
        printf("Bat. < V_sys_min\n\r");
    else
        printf("Bat. > V_sys_min\n\r");


    printf("\n\r     FAULT:     \n\r");
    printf("WATCHDOG:  ");
    if(bq2x_regs[9]>>7 == 1) printf("Timer expiration\n\r");
    else printf("Normal\n\r");

    printf("OTG: ");
    if(bq2x_regs[9] >> 6 & 0b1)
        printf("VBUS ovrld or bat. is too low\n\r");
    else
        printf("Normal\n\r");

    printf("CHRG FAULT: ");
    switch(bq2x_regs[9] >> 4 & 0b11)
    {
        case 0b00: printf("Normal"); break;
        case 0b01: printf("Input fault"); break;
        case 0b10: printf("Thermal shutdown"); break;
        case 0b11: printf("Timer expiration"); break;
    }
    printf("\n\r");

    printf("BAT. FAULT: ");
    if(bq2x_regs[9]>>3 & 0b1)
        printf("System OVP\n\r");
    else
        printf("Normal\n\r");




    printf("\n\r----------------------\n\r");
    printf("R0: 0x%02X, R1: 0x%02X, R2: 0x%02X, R3: 0x%02X, R4: 0x%02X\n\r",
           bq2x_regs[0], bq2x_regs[1], bq2x_regs[2], bq2x_regs[3], bq2x_regs[4]);
    printf("R5: 0x%02X, R6: 0x%02X, R7: 0x%02X, R8: 0x%02X, R9: 0x%02X",
           bq2x_regs[5], bq2x_regs[6], bq2x_regs[7], bq2x_regs[8], bq2x_regs[9]);

    printf("\n\r----------------------\n\r");
    printf("Press \'q\' to exit ");
    fflush(stdout);
    delay(300);
}
// ----------------------------------------------
endwin();

    bq2x.~I2CBus();
    bcm2835_close();

    return 0;
}
