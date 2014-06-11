/*
 ============================================================================
 Name        : BQ24295ctrl.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description :
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <devices.h>
#include <i2c_BQ24295.h>

static DRV_IFACE_t bq2x;
static DRVD_BQ24295_t bq2x_data;

int main(void)
{
	bq2x.read.ptr = &bq2x_data;
	bq2x.read.size = sizeof(bq2x_data);
	create_BQ24295(bq2x);

//	pause();
usleep(2000000);
	return EXIT_SUCCESS;
}
