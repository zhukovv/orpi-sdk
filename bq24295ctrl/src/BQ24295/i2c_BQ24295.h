#ifndef  _I2C_BQ24295_H_
#define _I2C_BQ24295_H_
#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <endian.h>
#include <pthread.h>

#include "../i2c/smbus.h"
#include "devices.h"

/*
 * i2c Address of BQ24295
 */
#define BQ24295_ADDRESS 0x6B

/*
 * BQ24295 Registers
 *
 * REG00-07 support Read and Write. REG08-0A are read only.
 *
 *
 */
typedef enum BQ24295_registers
{
	BQ2xREG_REG00=0x00,//Input Source Control Register
	BQ2xREG_REG01=0x01,//Power-On Configuration Register
	BQ2xREG_REG02=0x02,//Charge Current Control Register
	BQ2xREG_REG03=0x03,//Pre-Charge/Termination Current Control Register REG 03 (default 00010001, or 0x11)
	BQ2xREG_REG04=0x04,//Charge Voltage Control Register REG04 (default 10110010, or 0xB2)
	BQ2xREG_REG05=0x05,//Charge Termination/Timer Control Register REG05 (default 10011010, or 0x9A)
	BQ2xREG_REG06=0x06,//Boost Voltage/Thermal Regulation Control Register REG06 (default 10010011, or 0x93)
	BQ2xREG_REG07=0x07,//Misc Operation Control Register REG07 (default 01001011, or 4B)
	BQ2xREG_REG08=0x08,//System Status Register REG08
	BQ2xREG_REG09=0x09,//New Fault Register REG09
	BQ2xREG_REG0A=0x0A//Vender / Part / Revision Status Register REG0A
}
BQ2xREG_t;

/*
 * Шаблон устройства i2c
 */
#define i2c_dev_template "/dev/i2c-%d"

/*
 * Input Current Limit (Actual input current limit is the lower of I2C and ILIM)
 *
 * 000 – 100mA, 001 – 150mA, 010 – 500mA, 011 – 900mA, 100 – 1A, 101 – 1.5A, 110 – 2A, 111 – 3A
 */
typedef enum BQ2xFLG_IINLIM_data
{
	BQ2xFLG_IINLIM_0100mA=0b000,
	BQ2xFLG_IINLIM_0150mA=0b001,
	BQ2xFLG_IINLIM_0500mA=0b010,
	BQ2xFLG_IINLIM_0900mA=0b011,
	BQ2xFLG_IINLIM_1000mA=0b100,
	BQ2xFLG_IINLIM_1500mA=0b101,
	BQ2xFLG_IINLIM_2000mA=0b110,
	BQ2xFLG_IINLIM_3000mA=0b111,
} BQ2xFLG_IINLIM_t;

/*
Input Source Control Register REG00 (default 01011000, or 58)
 */
typedef struct BQ2xREG_REG00_data
{
	unsigned int IINLIM : 3;//Input Current Limit
	/*
	 * Input Voltage Limit
	 *
	 *Offset 3.88V, Range: 3.88V-5.08V
	Default: 4.76V (1011)
	 *
	 */
	unsigned int VINDPM_080mV: 1;
	unsigned int VINDPM_160mV: 1;
	unsigned int VINDPM_320mV : 1;
	unsigned int VINDPM_640mV : 1;
	/*
	 * 0 – Disable, 1 – Enable
	 */
	unsigned int EN_HIZ : 1;
} __attribute__((packed))
BQ2xREG_REG00_t;

/*
 * Power-On Configuration Register REG01 (default 00011011, or 1B)
 */
typedef struct BQ2xREG_REG01_data
{
	unsigned int reserved : 1;//Резерв

	/*
	 * Offset: 3.0V, Range 3.0V-3.7V
	 * Default: 3.5V (101)
	 *
	 */
	unsigned int SYS_MIN_0_1 : 1;
	unsigned int SYS_MIN_0_2 : 1;
	unsigned int SYS_MIN_0_4 : 1;
	/*
	 * 0- Charge Disable; 1- Charge Enable
	 * Default: Charge Battery (1)
	 */
	unsigned int CHG_CONFIG : 1;
	/*
	 * 0 – OTG Disable; 1 – OTG Enable
	 *
	 * Default: OTG Enable (1)
	 * Note: OTG_CONFIG would over-ride Charge Enable
	 * Function in CHG_CONFIG
	 */
	unsigned int OTG_CONFIG : 1;
	/*
	 * 0 – Normal ; 1 – Reset
	 * Default: Normal (0)
	 * Note: Consecutive I2C watchdog timer reset requires
	 * minimum 20uS delay
	 *
	 */
	unsigned int I2C_Watchdog_Timer_Reset : 1;
	/*
	 * 0 – Keep current register setting,
	 *
	 * 1 – Reset to default
	 *
	 */
	unsigned int Register_Reset : 1;
} __attribute__((packed))
BQ2xREG_REG01_t;

/*
 * Charge Current Control Register REG02 (default 01100000, or 60)
 */
typedef struct BQ2xREG_REG02_data
{
	/*
	 * 0 – ICHG as Fast Charge Current (REG02[7:2])
	 * and IPRECH as Pre-Charge Current (REG03[7:4]) programmed
	 * 1 – ICHG as 20% Fast Charge Current
	 * (REG02[7:2]) and IPRECH as 50% Pre-Charge
	 * Current (REG03[7:4]) programmed
	 *
	 */
	unsigned int FORCE_20PCT : 1;
	/*
	 *Set Boost Mode temperature monitor threshold
	 *voltage to disable boost mode
	 *0 – Vbcold0 (Typ. 76% of REGN or -10◦C w/ 103AT
	 * thermistor )
	 * 1 – Vbcold1 (Typ. 79% of REGN or -20◦C w/ 103AT thermistor
	 *
	 * Default: Vbcold0 (0)
	 */
	unsigned int BCOLD : 1;

	/*
	 * Fast Charge Current Limit
	 *
	 * Offset: 512mA
	 * Range: 512-3008mA (000000 - 100111)
	 * Default: 2048mA (011000)
	 * Note: ICHG higher than 3008mA is not suported
	 *
	 */
	unsigned int ICHG_0064mA : 1;
	unsigned int ICHG_0128mA : 1;
	unsigned int ICHG_0256mA : 1;
	unsigned int ICHG_0512mA : 1;
	unsigned int ICHG_1024mA : 1;
	unsigned int ICHG_2048mA : 1;
} __attribute__((packed))
BQ2xREG_REG02_t;

/*
 * Pre-Charge/Termination Current Control Register REG 03 (default 00010001, or 0x11)
 */
typedef struct BQ2xREG_REG03_data
{
	/*
	 *Termination Current Limit
	 * Offset: 128mA
Range: 128mA – 2048mA
Default: 256mA (0001)
	 */
	unsigned int ITERM_0128mA : 1;
	unsigned int ITERM_0256mA : 1;
	unsigned int ITERM_0512mA : 1;
	unsigned int ITERM_1024mA : 1;
	/*
	 *Pre-Charge Current Limit
	 *Offset: 128mA,
Range: 128mA – 2048mA
Default: 256mA (0001)
	 */
	unsigned int IPRECHG_0128mA : 1;
	unsigned int IPRECHG_0256mA : 1;
	unsigned int IPRECHG_0512mA : 1;
	unsigned int IPRECHG_1024mA : 1;
} __attribute__((packed))
BQ2xREG_REG03_t;

/*
 *Charge Voltage Control Register REG04 (default 10110010, or 0xB2)
 */
typedef struct BQ2xREG_REG04_data
{
	/*
	 * Battery Recharge Threshold (below battery regulation voltage)
	 *
	 * 0 – 100mV, 1 – 300mV Default: 100mV (0)
	 */
	unsigned int VRECHG : 1;
	/*
	 * 0 – 2.8V, 1 – 3.0V Default: 3.0V (1) (pre-charge to fast charge)
	 */
	unsigned int BATLOWV : 1;
	/*
	 * Charge Voltage Limit

Offset: 3.504V
Range: 3.504V – 4.400V
Default: 4.208V
	 */
	unsigned int VREG_016mV : 1;
	unsigned int VREG_032mV : 1;
	unsigned int VREG_064mV : 1;
	unsigned int VREG_128mV : 1;
	unsigned int VREG_256mV : 1;
	unsigned int VREG_512mV : 1;

} __attribute__((packed))
BQ2xREG_REG04_t;

/*
 * Fast Charge Timer Setting
 * 00 – 5 hrs, 01 – 8 hrs, 10 – 12 hrs, 11 – 20
 * Default: 12 hrs (10)
(See Charging Safety Timer for details)
 */
typedef enum BQ2xFLG_CHG_TIMER_data
{
	BQ2xFLG_CHG_TIMER_05hrs=0b00,
	BQ2xFLG_CHG_TIMER_08hrs=0b01,
	BQ2xFLG_CHG_TIMER_12hrs=0b10,
	BQ2xFLG_CHG_TIMER_20hrs=0b11,
} BQ2xFLG_CHG_TIMER_t;

/*
 * I2C Watchdog Timer Setting
 * 00 – Disable timer, 01 – 40s, 10 – 80s, 11 – 160s
 * Default: 40s (01)
 */
typedef enum BQ2xFLG_WATCHDOG_data
{
	BQ2xFLG_WATCHDOG_Disable_timer=0b00,
	BQ2xFLG_WATCHDOG_040s=0b01,
	BQ2xFLG_WATCHDOG_080s=0b10,
	BQ2xFLG_WATCHDOG_160s=0b11
} BQ2xFLG_WATCHDOG_t;

/*
 *
 *Charge Termination/Timer Control Register REG05 (default 10011010, or 0x9A)
 *
 */
typedef struct BQ2xREG_REG05_data
{
	unsigned int reserved0 : 1;//Резерв
	unsigned int CHG_TIMER : 2;//Fast Charge Timer Setting
	/*
	 * 0 – Disable, 1 – Enable
	 * Default: Enable (1)
	 */
	unsigned int EN_TIMER : 1;//Charging Safety Timer Enable
	/*
	 * 00 – Disable timer, 01 – 40s, 10 – 80s, 11 – 160s
	 * Default: 40s (01)
	 */
	unsigned int WATCHDOG : 2;//I2C Watchdog Timer Setting
	unsigned int reserved5 : 1;//Резерв
	/*
	 * 0 – Disable, 1 – Enable
	 * Default: Enable termination (1)
	 */
	unsigned int EN_TERM : 1;//Charging Termination Enable
} __attribute__((packed))
BQ2xREG_REG05_t;

/*
 * Thermal Regulation Threshold
 * 00 – 60°C, 01 – 80°C, 10 – 100°C, 11 –120°C
 * Default: 120°C (11)
 */
typedef enum BQ2xFLG_TREG_data
{
	BQ2xFLG_060C=0b00,
	BQ2xFLG_080C=0b01,
	BQ2xFLG_100C=0b10,
	BQ2xFLG_120C=0b11
} BQ2xFLG_TREG_t;

/*
 *Set Boost Mode temperature monitor
threshold voltage to disable boost mode
Voltage to disable boost mode
00 – V bhot1 (33% of REGN or 55◦C w/ 103AT
thermistor)
01 – V bhot0 (36% of REGN or 60◦C w/ 103AT
thermistor)
10 – V bhot2 (30% of REGN or 65◦C w/ 103AT
thermistor)
11 – Disable boost mode thermal protection.

Default: V bhot1 (00)
Note: For BHOT[1:0]=11, boost mode operates without
temperature monitor and the NTC_FAULT is generated based
on V bhot1 threshold
 */
typedef enum BQ2xFLG_BHOT_data
{
	BQ2xFLG_BHOT_33perc_of_REGN_or_55C_w_103AT_thermistor=0b00,
	BQ2xFLG_BHOT_36perc_of_REGN_or_60C_w_103AT_thermistor=0b01,
	BQ2xFLG_BHOT_30perc_of_REGN_or_65C_w_103AT_thermistor=0b10,
	BQ2xFLG_BHOT_Disable_boost_mode_thermal_protection=0b11
} BQ2xFLG_BHOT__t;


/*
 * Boost Voltage/Thermal Regulation Control Register REG06 (default 10010011, or 0x93)
 */
typedef struct BQ2xREG_REG06_data
{
	unsigned int TREG : 2;//Thermal Regulation Threshold
	unsigned int BHOT : 2;//Set Boost Mode temperature monitor
	/*
	 * Offset: 4.55V
Range: 4.55V – 5.51V
Default:5.126V(1001)
	 */
	unsigned int BOOSTV_064mV : 1;
	unsigned int BOOSTV_128mV : 1;
	unsigned int BOOSTV_256mV : 1;
	unsigned int BOOSTV_512mV : 1;
} __attribute__((packed))
BQ2xREG_REG06_t;

/*
 * Misc Operation Control Register REG07 (default 01001011, or 4B)
 */
typedef struct BQ2xREG_REG07_data
{

	/*
	 *Default: INT on BAT_FAULT (1)
	 */
	unsigned int INT_MASK_BAT_FAULT : 1;//0 – No INT during BAT_FAULT, 1 – INT on BAT_FAULT
	/*
	 * Default: INT on CHRG_FAULT (1)
	 */
	unsigned int INT_MASK_CHRG_FAULT : 1;//– No INT during CHRG_FAULT, 1 – INT on CHRG_FAULT
	unsigned int reserved2 : 3;
	/*
	 * 0 – Allow BATFET (Q4) turn on, 1 – Turn off BATFET (Q4)
	 * Default: Allow BATFET (Q4) turn on(0)
	 */
	unsigned int BATFET_Disable : 1;//Force BATFET Off
	/*
	 * Safety Timer Setting during Input DPM and Thermal Regulation
	 *
	 * 0 – Safety timer not slowed by 2X during input DPM or thermal regulation,
	 * 1 – Safety timer slowed by 2X during input DPM or thermal regulation
	 *
	 * Default: Safety timer slowed by 2X (1)
	 *
	 */
	unsigned int TMR2X_EN : 1;//Safety Timer Setting

	/*
	 * 0 – Not in D+/D– detection;
	 * 1 – Force D+/D– detection when VBUS power is presence
	 *
	 * Default: Not in D+/D– detection (0), Back to 0 after detection complete
	 */
	unsigned int DPDM_EN : 1;//Force DPDM detection
} __attribute__((packed))
BQ2xREG_REG07_t;

/*
 * 00 – Unknown (no input, or DPDM detection incomplete), 01 – USB host, 10 – Adapter port, 11 – OTG
 */
typedef enum BQ2xFLG_VBUS_STAT_data
{
	BQ2xFLG_VBUS_STAT_Unknown=0b00,
	BQ2xFLG_VBUS_STAT_USB_host=0b01,
	BQ2xFLG_VBUS_STAT_Adapter_port=0b10,
	BQ2xFLG_VBUS_STAT_OTG=0b11,
} BQ2xFLG_VBUS_STAT_t;

/*
 * 00 – Not Charging, 01 – Pre-charge (<V BATLOWV ), 10 – Fast Charging, 11 – Charge Termination Done
 */
typedef enum BQ2xFLG_CHRG_STAT_data
{
	BQ2xFLG_CHRG_STAT_Not_Charging=0b0,
	BQ2xFLG_CHRG_STAT_Pre_charge=0b1,
	BQ2xFLG_CHRG_STAT_Fast_Charging=0b10,
	BQ2xFLG_CHRG_STAT_Charge_Termination_Done=0b11,
} BQ2xFLG_CHRG_STAT_t;

/*
 *System Status Register REG08
 */
typedef struct BQ2xREG_REG08_data
{
	/*
	 * 0 – Not in VSYSMIN regulation (BAT>VSYSMIN), 1 – In VSYSMIN regulation (BAT<VSYSMIN)
	 */
	unsigned int VSYS_STAT : 1;
	/*
	 * 0 – Normal, 1 – In Thermal Regulation
	 */
	unsigned int THERM_STAT : 1;
	/*
	 *0 – Not Power Good, 1 – Power Good
	 */
	unsigned int PG_STAT : 1;
	/*
	 * 0 – Not DPM, 1 – VINDPM or IINDPM
	 */
	unsigned int DPM_STAT : 1;
	unsigned int CHRG_STAT : 2;
	unsigned int VBUS_STAT : 2;
} __attribute__((packed))
BQ2xREG_REG08_t;

/*
 * Vender / Part / Revision Status Register REG0A
 */
typedef struct BQ2xREG_REG0A_data
{
	unsigned int PN : 3;//110 (bq24295)
	unsigned int reserved3 : 2;//Резерв
	unsigned int Rev : 3;//000
} __attribute__((packed))
BQ2xREG_REG0A_t;

/*
 * New Fault Register REG09 (1) (2) (3)
 */

typedef enum BQ2xFLG_CHRG_FAULT_data
{
	BQ2xFLG_CHRG_FAULT_Normal=0b00,
	BQ2xFLG_CHRG_FAULT_Input_fault_OVP_or_bad_source=0b01,
	BQ2xFLG_CHRG_FAULT_Thermal_shutdown=0b10,
	BQ2xFLG_CHRG_FAULT_Charge_Timer_Expiration=0b11
} BQ2xFLG_CHRG_FAULT_t;

typedef struct BQ2xREG_REG09_data
{
	unsigned int NTC_FAULT_Hot_Note : 3;//0-Normal 1–Hot Note: Hot temperature threshold is different based on device operates in buck or boost mode
	unsigned int NTC_FAULT_Cold_Note : 3;//0-Normal 1–Cold Note: Cold temperature threshold is different based on device operates in buck or boost mode
	unsigned int reserved : 1;//0-Normal 1–Cold Note: Cold temperature threshold is different based on device operates in buck or boost mode
	unsigned int BAT_FAULT : 1;//0 – Normal, 1 – System OVP
	/*
	 * 00 – Normal, 01 – Input fault (OVP or bad source), 10 - Thermal shutdown, 11 – Charge Timer Expiration
	 */
	unsigned int CHRG_FAULT : 2;
	unsigned int OTG_FAULT : 1;//0 – Normal, 1 – VBUS overloaded in OTG, or VBUS OVP, or battery is too low (any conditions that we cannot start boost function)
	unsigned int WATCHDOG_FAULT : 1;//0 – Normal, 1- Watchdog timer expiration
} __attribute__((packed))
BQ2xREG_REG09_t;

typedef struct i2c_BQ24295_data
{
	/*
	 * R/W registers
	 */
	BQ2xREG_REG00_t REG00;
	BQ2xREG_REG01_t REG01;
	BQ2xREG_REG02_t REG02;
	BQ2xREG_REG03_t REG03;
	BQ2xREG_REG04_t REG04;
	BQ2xREG_REG05_t REG05;
	BQ2xREG_REG06_t REG06;
	BQ2xREG_REG07_t REG07;
}__attribute__((packed))
BQ2x_DATA_t;

typedef struct i2c_BQ24295_noncont_data
{
	/*
	 * RO registers
	 */
	BQ2xREG_REG08_t REG08;

	/*
	 * RO registers with no contiuous read
	 */
	BQ2xREG_REG09_t REG09;
	BQ2xREG_REG0A_t REG0A;
}__attribute__((packed))
BQ2x_DATA_NO_CONT_t;

typedef struct i2c_BQ24295_index
{
	BQ2xREG_t reg;
	void *ptr;
	char name[20];
} BQ2x_IDX_t;

typedef struct i2c_BQ24295_conf_index
{
	char name[90];
	int value;
}
BQ2x_CONF_IDX_t;

typedef enum
{
	BQ2x_CSAVE,
	BQ2x_CLOAD
}
BQ2x_CMODE_t;

extern char *program_invocation_short_name;

int create_BQ24295(DRV_IFACE_t driver_iface);
int destr_BQ24295(DRV_IFACE_t driver_iface);

typedef struct BQ24295_data
{
}
DRVD_BQ24295_t;

typedef struct BQ24295_settings
{
	char name[20];
	int delay;
	int bus_number;
}
DRVD_BQ24295_CONF_t;

extern DRVD_BQ24295_CONF_t conf_BQ24295;
#endif
