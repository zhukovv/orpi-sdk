#ifdef __cplusplus
extern "C" {
#endif

#include "mpu9150_app.h"
int done;
/***************************************/
void mpu9150_simple_init(void)
{
//	int opt, len;
	int i2c_bus = DEFAULT_I2C_BUS;
	int sample_rate = DEFAULT_SAMPLE_RATE_HZ;
	int yaw_mix_factor = DEFAULT_YAW_MIX_FACTOR;
	int verbose = 0;
	char *mag_cal_file = NULL;
	char *accel_cal_file = NULL;


	register_sig_handler();

	mpu9150_set_debug(verbose);

	if (mpu9150_init(i2c_bus, sample_rate, yaw_mix_factor))
		exit(1);

	set_cal(0, accel_cal_file);
	set_cal(1, mag_cal_file);

	if (accel_cal_file)
		free(accel_cal_file);

	if (mag_cal_file)
		free(mag_cal_file);

    linux_delay_ms(READ_DELAY);

    printf("..done!\n\n");
}

/***************************************/

void read_loop(unsigned int sample_rate)
{
	unsigned long loop_delay;
	mpudata_t mpu;

	memset(&mpu, 0, sizeof(mpudata_t));

	if (sample_rate == 0)
		return;

	loop_delay = (1000 / sample_rate) - 2;

	printf("\nEntering read loop (ctrl-c to exit)\n\n");

	linux_delay_ms(loop_delay);

	while (!done) {
		if (mpu9150_read(&mpu) == 0) {
			print_fused_euler_angles(&mpu);
			// printf_fused_quaternions(&mpu);
			// print_calibrated_accel(&mpu);
			// print_calibrated_mag(&mpu);
		}

		linux_delay_ms(loop_delay);
	}

	printf("\n\n");
}

void print_fused_euler_angles(mpudata_t *mpu)
{
	printf("\rX: %0.0f Y: %0.0f Z: %0.0f        ",
			mpu->fusedEuler[VEC3_X] * RAD_TO_DEGREE,
			mpu->fusedEuler[VEC3_Y] * RAD_TO_DEGREE,
			mpu->fusedEuler[VEC3_Z] * RAD_TO_DEGREE);

	fflush(stdout);
}

void print_fused_quaternions(mpudata_t *mpu)
{
	printf("\rW: %0.2f X: %0.2f Y: %0.2f Z: %0.2f        ",
			mpu->fusedQuat[QUAT_W],
			mpu->fusedQuat[QUAT_X],
			mpu->fusedQuat[QUAT_Y],
			mpu->fusedQuat[QUAT_Z]);

	fflush(stdout);
}

void print_calibrated_accel(mpudata_t *mpu)
{
	printf("\rX: %05d Y: %05d Z: %05d        ",
			mpu->calibratedAccel[VEC3_X],
			mpu->calibratedAccel[VEC3_Y],
			mpu->calibratedAccel[VEC3_Z]);

	fflush(stdout);
}

void print_calibrated_mag(mpudata_t *mpu)
{
	printf("\rX: %03d Y: %03d Z: %03d        ",
			mpu->calibratedMag[VEC3_X],
			mpu->calibratedMag[VEC3_Y],
			mpu->calibratedMag[VEC3_Z]);

	fflush(stdout);
}

int set_cal(int mag, char *cal_file)
{
	int i;
	FILE *f;
	char buff[32];
	long val[6];
	caldata_t cal;

	if (cal_file) {
		f = fopen(cal_file, "r");

		if (!f) {
			perror("open(<cal-file>)");
			return -1;
		}
	}
	else {
		if (mag) {
			f = fopen("/usr/src/magcal.txt", "r");

			if (!f) {
				printf("Default magcal.txt not found\n");
				return 0;
			}
		}
		else {
			f = fopen("/usr/src/accelcal.txt", "r");

			if (!f) {
				printf("Default accelcal.txt not found\n");
				return 0;
			}
		}
	}

	memset(buff, 0, sizeof(buff));

	for (i = 0; i < 6; i++) {
		if (!fgets(buff, 20, f)) {
			printf("Not enough lines in calibration file\n");
			break;
		}

		val[i] = atoi(buff);

		if (val[i] == 0) {
			printf("Invalid cal value: %s\n", buff);
			break;
		}
	}

	fclose(f);

	if (i != 6)
		return -1;

	cal.offset[0] = (short)((val[0] + val[1]) / 2);
	cal.offset[1] = (short)((val[2] + val[3]) / 2);
	cal.offset[2] = (short)((val[4] + val[5]) / 2);

	cal.range[0] = (short)(val[1] - cal.offset[0]);
	cal.range[1] = (short)(val[3] - cal.offset[1]);
	cal.range[2] = (short)(val[5] - cal.offset[2]);

	if (mag)
		mpu9150_set_mag_cal(&cal);
	else
		mpu9150_set_accel_cal(&cal);

	return 0;
}

void register_sig_handler()
{
	struct sigaction sia;

	bzero(&sia, sizeof sia);
	sia.sa_handler = sigint_handler;

	if (sigaction(SIGINT, &sia, NULL) < 0) {
		perror("sigaction(SIGINT)");
		exit(1);
	}
}

void sigint_handler(int sig)
{
	done = 1;
}

#ifdef __cplusplus
}
#endif

