/*
 ============================================================================
 Name        : i2c_BQ24295.c
 Author      :
 Version     :
 Copyright   : GPL
 Description :
 Created on  : 07.04.2014
 ============================================================================

 The bq24295 is a highly-integrated switch-mode battery charge management and system power path
management devices for 1 cell Li-Ion and Li-polymer battery in a wide range of power bank, tablet and other
portable device applications. Its low impedance power path optimizes switch-mode operation efficiency, reduces
battery charging time and extends battery life during discharging phase. The I2C serial interface with charging
and system settings makes the device a truly flexible solution.

 */
//#include <rios/devices/nav/i2c_BQ24295.h>
#include "i2c_BQ24295.h"

static int fd;
static pthread_t is_running;
static FILE *conff;

static DRV_IFACE_t drv;
static DRVD_BQ24295_t *outdata;

static DRVD_BQ24295_CONF_t *cfg = &conf_BQ24295;
/*
 * Continuous r/w data
 */
static BQ2x_DATA_t data;
/*
 * No continuous data
 */
static BQ2x_DATA_NO_CONT_t ncdata;

static BQ2x_IDX_t db[] =
{
		{ BQ2xREG_REG00,&data.REG00,"REG00"},
		{ BQ2xREG_REG01,&data.REG01,"REG01"},
		{ BQ2xREG_REG02,&data.REG02,"REG02"},
		{ BQ2xREG_REG03,&data.REG03,"REG03"},
		{ BQ2xREG_REG04,&data.REG04,"REG04"},
		{ BQ2xREG_REG05,&data.REG05,"REG05"},
		{ BQ2xREG_REG06,&data.REG06,"REG06"},
		{ BQ2xREG_REG07,&data.REG07,"REG07"},
		{ BQ2xREG_REG08,&ncdata.REG08,"REG08"},
		{ BQ2xREG_REG09,&ncdata.REG09,"REG09"},
		{ BQ2xREG_REG0A,&ncdata.REG0A,"REG0A"}
};

static BQ2x_CONF_IDX_t dbc[] =
{
		{"REG00.EN_HIZ",0},
		{"REG00.IINLIM",0},
		{"REG00.VINDPM_080mV",0},
		{"REG00.VINDPM_160mV",0},
		{"REG00.VINDPM_320mV",0},
		{"REG00.VINDPM_640mV",0},

		{"REG01.CHG_CONFIG",0},
		{"REG01.I2C_Watchdog_Timer_Reset",0},
		{"REG01.OTG_CONFIG",0},
		{"REG01.Register_Reset",0},
		{"REG01.SYS_MIN_0_1",0},
		{"REG01.SYS_MIN_0_2",0},
		{"REG01.SYS_MIN_0_4",0},

		{"REG02.BCOLD",0},
		{"REG02.FORCE_20PCT",0},
		{"REG02.ICHG_0064mA",0},
		{"REG02.ICHG_0128mA",0},
		{"REG02.ICHG_0256mA",0},
		{"REG02.ICHG_0512mA",0},
		{"REG02.ICHG_1024mA",0},
		{"REG02.ICHG_2048mA",0},

		{"REG03.IPRECHG_0128mA",0},
		{"REG03.IPRECHG_0256mA",0},
		{"REG03.IPRECHG_0512mA",0},
		{"REG03.IPRECHG_1024mA",0},
		{"REG03.ITERM_0128mA",0},
		{"REG03.ITERM_0256mA",0},
		{"REG03.ITERM_0512mA",0},
		{"REG03.ITERM_1024mA",0},

		{"REG04.BATLOWV",0},
		{"REG04.VRECHG",0},
		{"REG04.VREG_016mV",0},
		{"REG04.VREG_032mV",0},
		{"REG04.VREG_064mV",0},
		{"REG04.VREG_128mV",0},
		{"REG04.VREG_256mV",0},
		{"REG04.VREG_512mV",0},
		{"REG04.VRECHG",0},

		{"REG05.CHG_TIMER",0},
		{"REG05.EN_TERM",0},
		{"REG05.EN_TIMER",0},
		{"REG05.WATCHDOG",0},

		{"REG06.BHOT",0},
		{"REG06.BOOSTV_064mV",0},
		{"REG06.BOOSTV_128mV",0},
		{"REG06.BOOSTV_256mV",0},
		{"REG06.BOOSTV_512mV",0},

		{"REG07.BATFET_Disable",0},
		{"REG07.DPDM_EN",0},
		{"REG07.INT_MASK_BAT_FAULT",0},
		{"REG07.INT_MASK_CHRG_FAULT",0},
		{"REG07.TMR2X_EN",0}
};

/*
 * Save or load config
 */
static void config_proc(BQ2x_CONF_IDX_t *v,BQ2x_CMODE_t m)
{
	if(strcmp(v->name,"REG00.EN_HIZ")==0){(m==BQ2x_CSAVE)?(v->value = data.REG00.EN_HIZ):(data.REG00.EN_HIZ=v->value);}
	if(strcmp(v->name,"REG00.IINLIM")==0){(m==BQ2x_CSAVE)?(v->value = data.REG00.IINLIM):(data.REG00.IINLIM=v->value);}
	if(strcmp(v->name,"REG00.VINDPM_080mV")==0){(m==BQ2x_CSAVE)?(v->value = data.REG00.VINDPM_080mV):(data.REG00.VINDPM_080mV=v->value);}
	if(strcmp(v->name,"REG00.VINDPM_160mV")==0){(m==BQ2x_CSAVE)?(v->value = data.REG00.VINDPM_160mV):(data.REG00.VINDPM_160mV=v->value);}
	if(strcmp(v->name,"REG00.VINDPM_320mV")==0){(m==BQ2x_CSAVE)?(v->value = data.REG00.VINDPM_320mV):(data.REG00.VINDPM_320mV=v->value);}
	if(strcmp(v->name,"REG00.VINDPM_640mV")==0){(m==BQ2x_CSAVE)?(v->value = data.REG00.VINDPM_640mV):(data.REG00.VINDPM_640mV=v->value);}

	if(strcmp(v->name,"REG01.CHG_CONFIG")==0){(m==BQ2x_CSAVE)?(v->value = data.REG01.CHG_CONFIG):(data.REG01.CHG_CONFIG=v->value);}
	if(strcmp(v->name,"REG01.I2C_Watchdog_Timer_Reset")==0){(m==BQ2x_CSAVE)?(v->value = data.REG01.I2C_Watchdog_Timer_Reset):(data.REG01.I2C_Watchdog_Timer_Reset=v->value);}
	if(strcmp(v->name,"REG01.OTG_CONFIG")==0){(m==BQ2x_CSAVE)?(v->value = data.REG01.OTG_CONFIG):(data.REG01.OTG_CONFIG=v->value);}
	if(strcmp(v->name,"REG01.Register_Reset")==0){(m==BQ2x_CSAVE)?(v->value = data.REG01.Register_Reset):(data.REG01.Register_Reset=v->value);}
	if(strcmp(v->name,"REG01.SYS_MIN_0_1")==0){(m==BQ2x_CSAVE)?(v->value = data.REG01.SYS_MIN_0_1):(data.REG01.SYS_MIN_0_1=v->value);}
	if(strcmp(v->name,"REG01.SYS_MIN_0_2")==0){(m==BQ2x_CSAVE)?(v->value = data.REG01.SYS_MIN_0_2):(data.REG01.SYS_MIN_0_2=v->value);}
	if(strcmp(v->name,"REG01.SYS_MIN_0_4")==0){(m==BQ2x_CSAVE)?(v->value = data.REG01.SYS_MIN_0_4):(data.REG01.SYS_MIN_0_4=v->value);}

	if(strcmp(v->name,"REG02.BCOLD")==0){(m==BQ2x_CSAVE)?(v->value = data.REG02.BCOLD):(data.REG02.BCOLD=v->value);}
	if(strcmp(v->name,"REG02.FORCE_20PCT")==0){(m==BQ2x_CSAVE)?(v->value = data.REG02.FORCE_20PCT):(data.REG02.FORCE_20PCT=v->value);}
	if(strcmp(v->name,"REG02.ICHG_0064mA")==0){(m==BQ2x_CSAVE)?(v->value = data.REG02.ICHG_0064mA):(data.REG02.ICHG_0064mA=v->value);}
	if(strcmp(v->name,"REG02.ICHG_0128mA")==0){(m==BQ2x_CSAVE)?(v->value = data.REG02.ICHG_0128mA):(data.REG02.ICHG_0128mA=v->value);}
	if(strcmp(v->name,"REG02.ICHG_0256mA")==0){(m==BQ2x_CSAVE)?(v->value = data.REG02.ICHG_0256mA):(data.REG02.ICHG_0256mA=v->value);}
	if(strcmp(v->name,"REG02.ICHG_0512mA")==0){(m==BQ2x_CSAVE)?(v->value = data.REG02.ICHG_0512mA):(data.REG02.ICHG_0512mA=v->value);}
	if(strcmp(v->name,"REG02.ICHG_1024mA")==0){(m==BQ2x_CSAVE)?(v->value = data.REG02.ICHG_1024mA):(data.REG02.ICHG_1024mA=v->value);}
	if(strcmp(v->name,"REG02.ICHG_2048mA")==0){(m==BQ2x_CSAVE)?(v->value = data.REG02.ICHG_2048mA):(data.REG02.ICHG_2048mA=v->value);}

	if(strcmp(v->name,"REG03.IPRECHG_0128mA")==0){(m==BQ2x_CSAVE)?(v->value = data.REG03.IPRECHG_0128mA):(data.REG03.IPRECHG_0128mA=v->value);}
	if(strcmp(v->name,"REG03.IPRECHG_0256mA")==0){(m==BQ2x_CSAVE)?(v->value = data.REG03.IPRECHG_0256mA):(data.REG03.IPRECHG_0256mA=v->value);}
	if(strcmp(v->name,"REG03.IPRECHG_0512mA")==0){(m==BQ2x_CSAVE)?(v->value = data.REG03.IPRECHG_0512mA):(data.REG03.IPRECHG_0512mA=v->value);}
	if(strcmp(v->name,"REG03.IPRECHG_1024mA")==0){(m==BQ2x_CSAVE)?(v->value = data.REG03.IPRECHG_1024mA):(data.REG03.IPRECHG_1024mA=v->value);}
	if(strcmp(v->name,"REG03.ITERM_0128mA")==0){(m==BQ2x_CSAVE)?(v->value = data.REG03.ITERM_0128mA):(data.REG03.ITERM_0128mA=v->value);}
	if(strcmp(v->name,"REG03.ITERM_0256mA")==0){(m==BQ2x_CSAVE)?(v->value = data.REG03.ITERM_0256mA):(data.REG03.ITERM_0256mA=v->value);}
	if(strcmp(v->name,"REG03.ITERM_0512mA")==0){(m==BQ2x_CSAVE)?(v->value = data.REG03.ITERM_0512mA):(data.REG03.ITERM_0512mA=v->value);}
	if(strcmp(v->name,"REG03.ITERM_1024mA")==0){(m==BQ2x_CSAVE)?(v->value = data.REG03.ITERM_1024mA):(data.REG03.ITERM_1024mA=v->value);}

	if(strcmp(v->name,"REG04.BATLOWV")==0){(m==BQ2x_CSAVE)?(v->value = data.REG04.BATLOWV):(data.REG04.BATLOWV=v->value);}
	if(strcmp(v->name,"REG04.VRECHG")==0){(m==BQ2x_CSAVE)?(v->value = data.REG04.VRECHG):(data.REG04.VRECHG=v->value);}
	if(strcmp(v->name,"REG04.VREG_016mV")==0){(m==BQ2x_CSAVE)?(v->value = data.REG04.VREG_016mV):(data.REG04.VREG_016mV=v->value);}
	if(strcmp(v->name,"REG04.VREG_032mV")==0){(m==BQ2x_CSAVE)?(v->value = data.REG04.VREG_032mV):(data.REG04.VREG_032mV=v->value);}
	if(strcmp(v->name,"REG04.VREG_064mV")==0){(m==BQ2x_CSAVE)?(v->value = data.REG04.VREG_064mV):(data.REG04.VREG_064mV=v->value);}
	if(strcmp(v->name,"REG04.VREG_128mV")==0){(m==BQ2x_CSAVE)?(v->value = data.REG04.VREG_128mV):(data.REG04.VREG_128mV=v->value);}
	if(strcmp(v->name,"REG04.VREG_256mV")==0){(m==BQ2x_CSAVE)?(v->value = data.REG04.VREG_256mV):(data.REG04.VREG_256mV=v->value);}
	if(strcmp(v->name,"REG04.VREG_512mV")==0){(m==BQ2x_CSAVE)?(v->value = data.REG04.VREG_512mV):(data.REG04.VREG_512mV=v->value);}
	if(strcmp(v->name,"REG04.VRECHG")==0){(m==BQ2x_CSAVE)?(v->value = data.REG04.VRECHG):(data.REG04.VRECHG=v->value);}

	if(strcmp(v->name,"REG05.CHG_TIMER")==0){(m==BQ2x_CSAVE)?(v->value = data.REG05.CHG_TIMER):(data.REG05.CHG_TIMER=v->value);}
	if(strcmp(v->name,"REG05.EN_TERM")==0){(m==BQ2x_CSAVE)?(v->value = data.REG05.EN_TERM):(data.REG05.EN_TERM=v->value);}
	if(strcmp(v->name,"REG05.EN_TIMER")==0){(m==BQ2x_CSAVE)?(v->value = data.REG05.EN_TIMER):(data.REG05.EN_TIMER=v->value);}
	if(strcmp(v->name,"REG05.WATCHDOG")==0){(m==BQ2x_CSAVE)?(v->value = data.REG05.WATCHDOG):(data.REG05.WATCHDOG=v->value);}

	if(strcmp(v->name,"REG06.BHOT")==0){(m==BQ2x_CSAVE)?(v->value = data.REG06.BHOT):(data.REG06.BHOT=v->value);}
	if(strcmp(v->name,"REG06.BOOSTV_064mV")==0){(m==BQ2x_CSAVE)?(v->value = data.REG06.BOOSTV_064mV):(data.REG06.BOOSTV_064mV=v->value);}
	if(strcmp(v->name,"REG06.BOOSTV_128mV")==0){(m==BQ2x_CSAVE)?(v->value = data.REG06.BOOSTV_128mV):(data.REG06.BOOSTV_128mV=v->value);}
	if(strcmp(v->name,"REG06.BOOSTV_256mV")==0){(m==BQ2x_CSAVE)?(v->value = data.REG06.BOOSTV_256mV):(data.REG06.BOOSTV_256mV=v->value);}
	if(strcmp(v->name,"REG06.BOOSTV_512mV")==0){(m==BQ2x_CSAVE)?(v->value = data.REG06.BOOSTV_512mV):(data.REG06.BOOSTV_512mV=v->value);}

	if(strcmp(v->name,"REG07.BATFET_Disable")==0){(m==BQ2x_CSAVE)?(v->value = data.REG07.BATFET_Disable):(data.REG07.BATFET_Disable=v->value);}
	if(strcmp(v->name,"REG07.DPDM_EN")==0){(m==BQ2x_CSAVE)?(v->value = data.REG07.DPDM_EN):(data.REG07.DPDM_EN=v->value);}
	if(strcmp(v->name,"REG07.INT_MASK_BAT_FAULT")==0){(m==BQ2x_CSAVE)?(v->value = data.REG07.INT_MASK_BAT_FAULT):(data.REG07.INT_MASK_BAT_FAULT=v->value);}
	if(strcmp(v->name,"REG07.INT_MASK_CHRG_FAULT")==0){(m==BQ2x_CSAVE)?(v->value = data.REG07.INT_MASK_CHRG_FAULT):(data.REG07.INT_MASK_CHRG_FAULT=v->value);}
	if(strcmp(v->name,"REG07.TMR2X_EN")==0){(m==BQ2x_CSAVE)?(v->value = data.REG07.TMR2X_EN):(data.REG07.TMR2X_EN=v->value);}
}

static int write_start_address(BQ2xREG_t address)
{
	uint8_t addr = address;
	if ((write(fd, &addr, sizeof(addr))) != sizeof(addr) )
	{
		fprintf(stderr, "%s:%s Error write_start_adress. Errno = %d: %s.\n",program_invocation_short_name,cfg->name,errno,strerror(errno));
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

static int single_read_registers(void)
{
	write_start_address(BQ2xREG_REG00);
	if (read(fd, &data, sizeof(data)) != sizeof(data))
	{
		fprintf(stderr, "%s:%s Error read. Errno = %d: %s.\n",program_invocation_short_name,cfg->name,errno,strerror(errno));
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int init_config(void);
int dest_config(void);

static int dump_registers(void)
{
	fprintf(stdout, "%s:%s dump_registers():\n",program_invocation_short_name,cfg->name);
	unsigned int cnt;
	for (cnt = 0; cnt < sizeof(db)/sizeof(BQ2x_IDX_t); ++cnt)
	{
		fprintf(stdout, "0x%02x:0x%02x:%s\n",*(uint8_t *)db[cnt].ptr,db[cnt].reg,db[cnt].name);
	}
	return EXIT_SUCCESS;
}


static int save_config(void)
{
	if(conff==NULL){return EXIT_FAILURE;};
	single_read_registers();
	dump_registers();

	unsigned int cnt;
	for (cnt = 0; cnt < sizeof(dbc)/sizeof(BQ2x_CONF_IDX_t); ++cnt)
	{
		config_proc(&dbc[cnt],BQ2x_CSAVE);
		if(fprintf(conff,"%d %s\n",dbc[cnt].value,dbc[cnt].name)<0)
		{
			fprintf(stderr, "%s:%s:save_config() error. Errno = %d: %s.\n",program_invocation_short_name,cfg->name,errno,strerror(errno));
		}
	}
	dest_config();
	init_config();
	return EXIT_SUCCESS;
}

int init_config(void)
{
	char cfgf[] = "BQ24295.conf";
	int ret = access(cfgf,R_OK);
	if(ret != 0)
	{
		conff = fopen(cfgf,"w+");
		if(conff == NULL)
		{
			fprintf(stderr, "%s:%s:init_log(%s) error. Errno = %d: %s.\n",program_invocation_short_name,cfg->name,cfgf,errno,strerror(errno));
			return EXIT_FAILURE;
		}
		save_config();
	}
	else
	{
		conff = fopen(cfgf,"r");
		if(conff == NULL)
		{
			fprintf(stderr, "%s:%s:init_log(%s) error. Errno = %d: %s.\n",program_invocation_short_name,cfg->name,cfgf,errno,strerror(errno));
			return EXIT_FAILURE;
		}
	}
	return EXIT_SUCCESS;
}

int dest_config(void)
{
	if(fclose(conff)<0)
	{
		fprintf(stderr, "%s:%s:dest_log() error. Errno = %d: %s.\n",program_invocation_short_name,cfg->name,errno,strerror(errno));
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

static int load_config(void)
{
	if(conff==NULL){return EXIT_FAILURE;};

	unsigned int cnt;
	for (cnt = 0; cnt < sizeof(dbc)/sizeof(BQ2x_CONF_IDX_t); ++cnt)
	{
		BQ2x_CONF_IDX_t val;
		int ret = fscanf(conff,"%i %s\n",&val.value,val.name);
		unsigned int cntc;
		int valid_conf = 0;
		for (cntc = 0; cntc < sizeof(dbc)/sizeof(BQ2x_CONF_IDX_t); ++cntc)
		{
			if(strcmp(dbc[cntc].name,val.name) == 0)
			{
				valid_conf = 1;
			}
		}
		if(valid_conf != 1 || val.value > 0b1111/*максимальное значение аргумента конфигурации*/)
		{
			fprintf(stderr, "%s:load_config() invalid config %s %d.\n",program_invocation_short_name,val.name,val.value);
			return EXIT_FAILURE;
		}
		else
		{
			dbc[cnt].value = val.value;
			config_proc(&dbc[cnt],BQ2x_CLOAD);
		}

		if(ret == EOF)
		{
			fprintf(stderr, "%s:load_config() wrong number of lines.\n",program_invocation_short_name);
		}
	}

	return EXIT_SUCCESS;
}

static int read_registers(void)
{
	unsigned int cnt;
	for (cnt = 0; cnt < sizeof(db)/sizeof(BQ2x_IDX_t); ++cnt)
	{
		write_start_address(db[cnt].reg);
		if (read(fd, db[cnt].ptr, sizeof(uint8_t)) != sizeof(uint8_t))
		{
			fprintf(stderr, "%s:%s Error read. Errno = %d: %s.\n",program_invocation_short_name,cfg->name,errno,strerror(errno));
			return EXIT_FAILURE;
		}
	}
	return EXIT_SUCCESS;
}

static int write_registers(void)
{
	unsigned int cnt;
	for (cnt = 0; cnt < sizeof(db)/sizeof(BQ2x_IDX_t); ++cnt)
	{
		uint8_t addr = db[cnt].reg;
		int ret = i2c_smbus_write_byte_data(fd, addr,*(uint8_t *)db[cnt].ptr);
		if (ret != 0)
		{
			fprintf(stderr, "%s:%s Error write. Errno = %d: %s.\n",program_invocation_short_name,cfg->name,errno,strerror(errno));
			fprintf(stderr, "%s:%s Return = %d: %s.\n",program_invocation_short_name,cfg->name,ret,strerror(ret));
			return EXIT_FAILURE;
		}
	}
	return EXIT_SUCCESS;
}

static void display_IINLIM_mode(void)
{
	switch (data.REG00.IINLIM) {
	case BQ2xFLG_IINLIM_0100mA:
		fprintf(stdout, "%s:%s set_mode() BQ2xFLG_IINLIM_0100mA.\n",program_invocation_short_name,cfg->name);
		break;
	case BQ2xFLG_IINLIM_0500mA:
		fprintf(stdout, "%s:%s set_mode() BQ2xFLG_IINLIM_0500mA.\n",program_invocation_short_name,cfg->name);
		break;
	case BQ2xFLG_IINLIM_0900mA:
		fprintf(stdout, "%s:%s set_mode() BQ2xFLG_IINLIM_0900mA.\n",program_invocation_short_name,cfg->name);
		break;
	case BQ2xFLG_IINLIM_1000mA:
		fprintf(stdout, "%s:%s set_mode() BQ2xFLG_IINLIM_1000mA.\n",program_invocation_short_name,cfg->name);
		break;
	case BQ2xFLG_IINLIM_1500mA:
		fprintf(stdout, "%s:%s set_mode() BQ2xFLG_IINLIM_1500mA.\n",program_invocation_short_name,cfg->name);
		break;
	case BQ2xFLG_IINLIM_2000mA:
		fprintf(stdout, "%s:%s set_mode() BQ2xFLG_IINLIM_2000mA.\n",program_invocation_short_name,cfg->name);
		break;
	case BQ2xFLG_IINLIM_3000mA:
		fprintf(stdout, "%s:%s set_mode() BQ2xFLG_IINLIM_3000mA.\n",program_invocation_short_name,cfg->name);
		break;
	}
}

static int check_fault(void)
{
	write_start_address(BQ2xREG_REG09);
	if (read(fd, &ncdata.REG09, sizeof(ncdata.REG09)) != sizeof(ncdata.REG09))
	{
		fprintf(stderr, "%s:%s Error read. Errno = %d: %s.\n",program_invocation_short_name,cfg->name,errno,strerror(errno));
		return EXIT_FAILURE;
	}

	if(ncdata.REG09.BAT_FAULT)
	{
		fprintf(stdout, "%s:%s check_fault() BAT_FAULT.\n",program_invocation_short_name,cfg->name);
	}
	if(ncdata.REG09.CHRG_FAULT)
	{
		fprintf(stdout, "%s:%s check_fault() CHRG_FAULT.\n",program_invocation_short_name,cfg->name);
	}
	if(ncdata.REG09.NTC_FAULT_Cold_Note)
	{
		fprintf(stdout, "%s:%s check_fault() NTC_FAULT_Cold_Note.\n",program_invocation_short_name,cfg->name);
	}
	if(ncdata.REG09.NTC_FAULT_Hot_Note)
	{
		fprintf(stdout, "%s:%s check_fault() NTC_FAULT_Hot_Note.\n",program_invocation_short_name,cfg->name);
	}
	if(ncdata.REG09.OTG_FAULT)
	{
		fprintf(stdout, "%s:%s check_fault() OTG_FAULT.\n",program_invocation_short_name,cfg->name);
	}
	if(ncdata.REG09.WATCHDOG_FAULT)
	{
		fprintf(stdout, "%s:%s check_fault() WATCHDOG_FAULT.\n",program_invocation_short_name,cfg->name);
	}

	return EXIT_SUCCESS;
}

static int check_status(void)
{
	static BQ2xREG_REG08_t status_prev;
	write_start_address(BQ2xREG_REG08);
	if (read(fd, &ncdata.REG08, sizeof(ncdata.REG08)) != sizeof(ncdata.REG08))
	{
		fprintf(stderr, "%s:%s Error read. Errno = %d: %s.\n",program_invocation_short_name,cfg->name,errno,strerror(errno));
		return EXIT_FAILURE;
	}
	if(memcmp(&ncdata.REG08,&status_prev,sizeof(ncdata.REG08)) == 0 )
	{
		return EXIT_SUCCESS;
	}

	if(ncdata.REG08.CHRG_STAT != status_prev.CHRG_STAT)
	{
		BQ2xFLG_CHRG_STAT_t CHRG_STAT = (BQ2xFLG_CHRG_STAT_t)ncdata.REG08.CHRG_STAT;
		switch (CHRG_STAT) {
		case BQ2xFLG_CHRG_STAT_Charge_Termination_Done:
			fprintf(stdout, "%s:%s check_status() CHRG_STAT Charge_Termination_Done.\n",program_invocation_short_name,cfg->name);
			break;
		case BQ2xFLG_CHRG_STAT_Fast_Charging:
			fprintf(stdout, "%s:%s check_status() CHRG_STAT Fast_Charging.\n",program_invocation_short_name,cfg->name);
			break;
		case BQ2xFLG_CHRG_STAT_Not_Charging:
			fprintf(stdout, "%s:%s check_status() CHRG_STAT Not_Charging.\n",program_invocation_short_name,cfg->name);
			break;
		case BQ2xFLG_CHRG_STAT_Pre_charge:
			fprintf(stdout, "%s:%s check_status() CHRG_STAT Pre_charge.\n",program_invocation_short_name,cfg->name);
			break;
		default:
			break;
		}
	}

	if(ncdata.REG08.DPM_STAT != status_prev.DPM_STAT)
	{
		if(ncdata.REG08.DPM_STAT)
		{
			fprintf(stdout, "%s:%s check_status() DPM_STAT VINDPM or IINDPM.\n",program_invocation_short_name,cfg->name);
		}
		else
		{
			fprintf(stdout, "%s:%s check_status() DPM_STAT Not DPM.\n",program_invocation_short_name,cfg->name);
		}
	}

	if(ncdata.REG08.PG_STAT != status_prev.PG_STAT)
	{
		if(ncdata.REG08.PG_STAT)
		{
			fprintf(stdout, "%s:%s check_status() PG_STAT Power Good.\n",program_invocation_short_name,cfg->name);
		}
		else
		{
			fprintf(stdout, "%s:%s check_status() PG_STAT Not Power Good.\n",program_invocation_short_name,cfg->name);
		}
	}

	if(ncdata.REG08.THERM_STAT != status_prev.THERM_STAT)
	{
		if(ncdata.REG08.THERM_STAT)
		{
			fprintf(stdout, "%s:%s check_status() THERM_STAT Normal.\n",program_invocation_short_name,cfg->name);
		}
		else
		{
			fprintf(stdout, "%s:%s check_status() THERM_STAT In Thermal Regulation.\n",program_invocation_short_name,cfg->name);
		}
	}

	if(ncdata.REG08.VBUS_STAT != status_prev.VBUS_STAT)
	{
		BQ2xFLG_VBUS_STAT_t VBUS_STAT = (BQ2xFLG_VBUS_STAT_t)ncdata.REG08.VBUS_STAT;

		switch (VBUS_STAT) {
		case BQ2xFLG_VBUS_STAT_Adapter_port:
			fprintf(stdout, "%s:%s check_status() VBUS_STAT_Adapter_port.\n",program_invocation_short_name,cfg->name);
			break;
		case BQ2xFLG_VBUS_STAT_OTG:
			fprintf(stdout, "%s:%s check_status() VBUS_STAT_OTG.\n",program_invocation_short_name,cfg->name);
			break;
		case BQ2xFLG_VBUS_STAT_USB_host:
			fprintf(stdout, "%s:%s check_status() VBUS_STAT_USB_host.\n",program_invocation_short_name,cfg->name);
			break;
		case BQ2xFLG_VBUS_STAT_Unknown:
			fprintf(stdout, "%s:%s check_status() VBUS_STAT_Unknown.\n",program_invocation_short_name,cfg->name);
			break;
		}
	}

	if(ncdata.REG08.VSYS_STAT != status_prev.VSYS_STAT)
	{
		if(ncdata.REG08.VSYS_STAT)
		{
			fprintf(stdout, "%s:%s check_status() VSYS_STAT  Not in VSYSMIN regulation (BAT>VSYSMIN).\n",program_invocation_short_name,cfg->name);
		}
		else
		{
			fprintf(stdout, "%s:%s check_status() VSYS_STAT In VSYSMIN regulation (BAT<VSYSMIN).\n",program_invocation_short_name,cfg->name);
		}
	}

	memcpy(&status_prev,&ncdata.REG08,sizeof(ncdata.REG08));
	return EXIT_SUCCESS;
}

static int init(void)
{
	char i2c_device_filename[100];
	sprintf(i2c_device_filename,i2c_dev_template,cfg->bus_number);

	if((fd = open_i2c_device(i2c_device_filename))<0){return EXIT_FAILURE;};
	i2c_set_slave(fd,BQ24295_ADDRESS);

	if(init_config() == EXIT_SUCCESS)
	{
		if(load_config() == EXIT_SUCCESS)
		{
			write_registers();
			dest_config();
		}
	}

	if(drv.fp.init !=NULL)
	{
		drv.fp.init(drv.proc.ptr);
	}

	return EXIT_SUCCESS;
}

static int destr(void)
{
	if(drv.fp.destr !=NULL)
	{
		drv.fp.destr(drv.proc.ptr);
	}
	is_running = 0;
	if (close(fd) < 0)
	{
		fprintf(stderr, "%s:%s Failed to close i2c port. Errno = %d: %s.\n",program_invocation_short_name,cfg->name,errno,strerror(errno));
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

static void *readBQ24295(void *thtread_id)
{
	if(thtread_id){};

	if(init()==EXIT_FAILURE)
	{
		return EXIT_SUCCESS;
	}


	while(is_running)
	{
		check_fault();
		check_status();
		display_IINLIM_mode();

		data.REG00.IINLIM = BQ2xFLG_IINLIM_3000mA;
		write_registers();

		read_registers();

		dump_registers();

		if(drv.fp.proc !=NULL)
		{
			drv.fp.proc(drv.proc.ptr);
		}
		usleep(cfg->delay);
		is_running=0;
	}
	destr();

	return EXIT_SUCCESS;
}

int create_BQ24295(DRV_IFACE_t driver_iface)
{
	if(driver_iface.read.ptr == NULL)
	{
		fprintf(stderr, "%s:%s Error create_nav() - bad input data.\n",program_invocation_short_name,cfg->name);
		return EXIT_SUCCESS;
	}
	else
	{
		memcpy(&drv,&driver_iface,sizeof(driver_iface));
	}
	outdata = (DRVD_BQ24295_t *)drv.read.ptr;

	int pthread_create_return = pthread_create(&is_running, NULL, readBQ24295, NULL);
	if(pthread_create_return != 0)
	{
		fprintf(stderr, "%s:%s Error creating thread. Errno = %d: %s.\n",program_invocation_short_name,cfg->name,pthread_create_return,strerror(pthread_create_return));
		return EXIT_FAILURE;
	}
	else
	{
		return EXIT_SUCCESS;
	}
	return EXIT_SUCCESS;
}


int destr_BQ24295(DRV_IFACE_t driver_iface)
{
	if(driver_iface.dev_min_numb){};
	is_running = 0;
	return EXIT_SUCCESS;
}
