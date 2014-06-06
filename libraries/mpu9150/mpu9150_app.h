#ifndef MPU9150_APP_H
#define MPU9150_APP_H


#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <getopt.h>
#include <errno.h>

#include "mpu9150.h"
#include "linux_glue.h"
#include "local_defaults.h"

#ifdef __cplusplus
extern "C" {
#endif

int set_cal(int mag, char *cal_file);
void read_loop(unsigned int sample_rate);
void print_fused_euler_angles(mpudata_t *mpu);
void print_fused_quaternion(mpudata_t *mpu);
void print_calibrated_accel(mpudata_t *mpu);
void print_calibrated_mag(mpudata_t *mpu);
void register_sig_handler();
void sigint_handler(int sig);

/*************************************/

void mpu9150_simple_init(void);

/*************************************/

extern int done;

#define READ_DELAY ((1000 / DEFAULT_SAMPLE_RATE_HZ) - 2)

#ifdef __cplusplus
}
#endif

#endif // MPU9150_APP_H
