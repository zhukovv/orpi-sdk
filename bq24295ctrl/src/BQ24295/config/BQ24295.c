/*
 * BQ24295.c
 *
 *  Created on: 07.04.2014
 *      Author: koval
 */
//#include <rios/devices/nav/i2c_BQ24295.h>
#include "../i2c_BQ24295.h"

#define NAME "BQ24295"
#define FREQUENCY 1000000

#define BUS_NUMBER 1

DRVD_BQ24295_CONF_t conf_BQ24295 =
{
		NAME,
		/*
		 * double freq - will be overriden if sample rate divider is not integer
		 */
		FREQUENCY,
		/*
		 *int bus_number
		 */
		BUS_NUMBER//int bus_number
};
