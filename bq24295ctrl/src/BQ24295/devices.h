/*
 * devices.h
 *
 *  Created on: 15.03.2014
 *      Author: koval
 */

#ifndef DEVICES_H_
#define DEVICES_H_


typedef int DRV_FUNC_t(void *data_ptr);

typedef struct data_source
{
	void *ptr;
	int offset;
	int size;
} DRV_SRC_t;

typedef struct driver_data_process_funcs
{
	DRV_FUNC_t *init;
	DRV_FUNC_t *proc;
	DRV_FUNC_t *destr;
} DRV_FUNC_PROC_t;

typedef struct driver_data_funcs
{
	DRV_FUNC_t *init;
	DRV_FUNC_t *read;
	DRV_FUNC_t *write;
	DRV_FUNC_t *destr;
} DRV_FUNC_DATA_t;

typedef struct driver_data_interface
{
	DRV_SRC_t read;
	DRV_SRC_t write;
	DRV_SRC_t proc;
	DRV_SRC_t sett;

	unsigned int dev_min_numb;//Номер первого устройства
	unsigned int dev_max_numb;//Номер последнего устройства

	DRV_FUNC_DATA_t fd;//Функции для чтения-записи данных
	DRV_FUNC_PROC_t fp;//Функции для обработки данных - если определены запускаются внутри драйвера
} DRV_IFACE_t;

int create_drv(DRV_IFACE_t driver_iface);
int destr_drv(DRV_IFACE_t driver_iface);


#endif /* DEVICES_H_ */
