/*
    smbus.c - SMBus level access helper functions

    Copyright (C) 1995-1997  Simon G. Vogl
    Copyright (C) 1998-1999  Frodo Looijaard <frodol@dds.nl>
    Copyright (C) 2012-2013  Jean Delvare <jdelvare@suse.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
    MA 02110-1301 USA.
 */

#include <errno.h>
#include <stddef.h>
#include "smbus.h"
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

/* Compatibility defines */
#ifndef I2C_SMBUS_I2C_BLOCK_BROKEN
#define I2C_SMBUS_I2C_BLOCK_BROKEN I2C_SMBUS_I2C_BLOCK_DATA
#endif
#ifndef I2C_FUNC_SMBUS_PEC
#define I2C_FUNC_SMBUS_PEC I2C_FUNC_SMBUS_HWPEC_CALC
#endif

__s32 i2c_smbus_access(int file, char read_write, __u8 command,
		int size, union i2c_smbus_data *data)
{
	struct i2c_smbus_ioctl_data args;
	__s32 err;

	args.read_write = read_write;
	args.command = command;
	args.size = size;
	args.data = data;

	err = ioctl(file, I2C_SMBUS, &args);
	if (err == -1)
		err = -errno;
	return err;
}


__s32 i2c_smbus_write_quick(int file, __u8 value)
{
	return i2c_smbus_access(file, value, 0, I2C_SMBUS_QUICK, NULL);
}

__s32 i2c_smbus_read_byte(int file)
{
	union i2c_smbus_data data;
	int err;

	err = i2c_smbus_access(file, I2C_SMBUS_READ, 0, I2C_SMBUS_BYTE, &data);
	if (err < 0)
		return err;

	return 0x0FF & data.byte;
}

__s32 i2c_smbus_write_byte(int file, __u8 value)
{
	return i2c_smbus_access(file, I2C_SMBUS_WRITE, value,
			I2C_SMBUS_BYTE, NULL);
}

__s32 i2c_smbus_read_byte_data(int file, __u8 command)
{
	union i2c_smbus_data data;
	int err;

	err = i2c_smbus_access(file, I2C_SMBUS_READ, command,
			I2C_SMBUS_BYTE_DATA, &data);
	if (err < 0)
		return err;

	return 0x0FF & data.byte;
}

__s32 i2c_smbus_write_byte_data(int file, __u8 command, __u8 value)
{
	union i2c_smbus_data data;
	data.byte = value;
	return i2c_smbus_access(file, I2C_SMBUS_WRITE, command,
			I2C_SMBUS_BYTE_DATA, &data);
}

__s32 i2c_smbus_read_word_data(int file, __u8 command)
{
	union i2c_smbus_data data;
	int err;

	err = i2c_smbus_access(file, I2C_SMBUS_READ, command,
			I2C_SMBUS_WORD_DATA, &data);
	if (err < 0)
		return err;

	return 0x0FFFF & data.word;
}

__s32 i2c_smbus_write_word_data(int file, __u8 command, __u16 value)
{
	union i2c_smbus_data data;
	data.word = value;
	return i2c_smbus_access(file, I2C_SMBUS_WRITE, command,
			I2C_SMBUS_WORD_DATA, &data);
}

__s32 i2c_smbus_process_call(int file, __u8 command, __u16 value)
{
	union i2c_smbus_data data;
	data.word = value;
	if (i2c_smbus_access(file, I2C_SMBUS_WRITE, command,
			I2C_SMBUS_PROC_CALL, &data))
		return -1;
	else
		return 0x0FFFF & data.word;
}

/* Returns the number of read bytes */
__s32 i2c_smbus_read_block_data(int file, __u8 command, __u8 *values)
{
	union i2c_smbus_data data;
	int i, err;

	err = i2c_smbus_access(file, I2C_SMBUS_READ, command,
			I2C_SMBUS_BLOCK_DATA, &data);
	if (err < 0)
		return err;

	for (i = 1; i <= data.block[0]; i++)
		values[i-1] = data.block[i];
	return data.block[0];
}

__s32 i2c_smbus_write_block_data(int file, __u8 command, __u8 length,
		const __u8 *values)
{
	union i2c_smbus_data data;
	int i;
	if (length > I2C_SMBUS_BLOCK_MAX)
		length = I2C_SMBUS_BLOCK_MAX;
	for (i = 1; i <= length; i++)
		data.block[i] = values[i-1];
	data.block[0] = length;
	return i2c_smbus_access(file, I2C_SMBUS_WRITE, command,
			I2C_SMBUS_BLOCK_DATA, &data);
}

/* Returns the number of read bytes */
/* Until kernel 2.6.22, the length is hardcoded to 32 bytes. If you
   ask for less than 32 bytes, your code will only work with kernels
   2.6.23 and later. */
__s32 i2c_smbus_read_i2c_block_data(int file, __u8 command, __u8 length,
		__u8 *values)
{
	union i2c_smbus_data data;
	int i, err;

	if (length > I2C_SMBUS_BLOCK_MAX)
		length = I2C_SMBUS_BLOCK_MAX;
	data.block[0] = length;

	err = i2c_smbus_access(file, I2C_SMBUS_READ, command,
			length == 32 ? I2C_SMBUS_I2C_BLOCK_BROKEN :
					I2C_SMBUS_I2C_BLOCK_DATA, &data);
	if (err < 0)
		return err;

	for (i = 1; i <= data.block[0]; i++)
		values[i-1] = data.block[i];
	return data.block[0];
}

__s32 i2c_smbus_write_i2c_block_data(int file, __u8 command, __u8 length,
		const __u8 *values)
{
	union i2c_smbus_data data;
	int i;
	if (length > I2C_SMBUS_BLOCK_MAX)
		length = I2C_SMBUS_BLOCK_MAX;
	for (i = 1; i <= length; i++)
		data.block[i] = values[i-1];
	data.block[0] = length;
	return i2c_smbus_access(file, I2C_SMBUS_WRITE, command,
			I2C_SMBUS_I2C_BLOCK_BROKEN, &data);
}

/* Returns the number of read bytes */
__s32 i2c_smbus_block_process_call(int file, __u8 command, __u8 length,
		__u8 *values)
{
	union i2c_smbus_data data;
	int i, err;

	if (length > I2C_SMBUS_BLOCK_MAX)
		length = I2C_SMBUS_BLOCK_MAX;
	for (i = 1; i <= length; i++)
		data.block[i] = values[i-1];
	data.block[0] = length;

	err = i2c_smbus_access(file, I2C_SMBUS_WRITE, command,
			I2C_SMBUS_BLOCK_PROC_CALL, &data);
	if (err < 0)
		return err;

	for (i = 1; i <= data.block[0]; i++)
		values[i-1] = data.block[i];
	return data.block[0];
}

//Non standart functions
//======================================================================================================================================
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <error.h>
extern char *program_invocation_short_name;
/* List the most up-to-date list of functionality constants */
__s32 i2c_smbus_list_adapter_functionality(int file)
{
	int funcs;
	/*
  I2C_FUNC_I2C                    Plain i2c-level commands (Pure SMBus
                                  adapters typically can not do these)
  I2C_FUNC_10BIT_ADDR             Handles the 10-bit address extensions
  I2C_FUNC_PROTOCOL_MANGLING      Knows about the I2C_M_IGNORE_NAK,
                                  I2C_M_REV_DIR_ADDR and I2C_M_NO_RD_ACK
                                  flags (which modify the I2C protocol!)
  I2C_FUNC_NOSTART                Can skip repeated start sequence
  I2C_FUNC_SMBUS_QUICK            Handles the SMBus write_quick command
  I2C_FUNC_SMBUS_READ_BYTE        Handles the SMBus read_byte command
  I2C_FUNC_SMBUS_WRITE_BYTE       Handles the SMBus write_byte command
  I2C_FUNC_SMBUS_READ_BYTE_DATA   Handles the SMBus read_byte_data command
  I2C_FUNC_SMBUS_WRITE_BYTE_DATA  Handles the SMBus write_byte_data command
  I2C_FUNC_SMBUS_READ_WORD_DATA   Handles the SMBus read_word_data command
  I2C_FUNC_SMBUS_WRITE_WORD_DATA  Handles the SMBus write_byte_data command
  I2C_FUNC_SMBUS_PROC_CALL        Handles the SMBus process_call command
  I2C_FUNC_SMBUS_READ_BLOCK_DATA  Handles the SMBus read_block_data command
  I2C_FUNC_SMBUS_WRITE_BLOCK_DATA Handles the SMBus write_block_data command
  I2C_FUNC_SMBUS_READ_I2C_BLOCK   Handles the SMBus read_i2c_block_data command
  I2C_FUNC_SMBUS_WRITE_I2C_BLOCK  Handles the SMBus write_i2c_block_data command
	 */
	fprintf(stdout,"%s:Adapter functionality:",program_invocation_short_name);
	if (ioctl(file, I2C_FUNC_I2C, &funcs) < 0){
		fprintf(stdout,"%s:Not support:I2C_FUNC_I2C Plain i2c-level commands.",program_invocation_short_name);
	}
	else{
		fprintf(stdout,"%s:Supports   :I2C_FUNC_I2C Plain i2c-level commands.",program_invocation_short_name);
	}

	if (ioctl(file, I2C_FUNC_10BIT_ADDR, &funcs) < 0){
		fprintf(stdout,"%s:Not support:I2C_FUNC_10BIT_ADDR Not handles the 10-bit address extensions.",program_invocation_short_name);
	}
	else{
		fprintf(stdout,"%s:Supports   :I2C_FUNC_10BIT_ADDR Handles the 10-bit address extensions.",program_invocation_short_name);
	}

	if (ioctl(file, I2C_FUNC_PROTOCOL_MANGLING, &funcs) < 0){
		fprintf(stdout,"%s:Not support:I2C_FUNC_PROTOCOL_MANGLING.",program_invocation_short_name);
	}
	else{
		fprintf(stdout,"%s:Supports   :I2C_FUNC_PROTOCOL_MANGLING.",program_invocation_short_name);
	}

//	if (ioctl(file, I2C_FUNC_NOSTART, &funcs) < 0){
//		fprintf(stdout,"%s:Not support:I2C_FUNC_NOSTART Can not skip repeated start sequence.",program_invocation_short_name);
//	}
//	else{
//		fprintf(stdout,"%s:Supports   :I2C_FUNC_NOSTART Can skip repeated start sequence.",program_invocation_short_name);
//	}

	if (ioctl(file, I2C_FUNC_SMBUS_QUICK, &funcs) < 0){
		fprintf(stdout,"%s:Not support:I2C_FUNC_SMBUS_QUICK Not handles the SMBus write_quick command.",program_invocation_short_name);
	}
	else{
		fprintf(stdout,"%s:Supports   :I2C_FUNC_SMBUS_QUICK Handles the SMBus write_quick command.",program_invocation_short_name);
	}

	if (ioctl(file, I2C_FUNC_SMBUS_READ_BYTE, &funcs) < 0){
		fprintf(stdout,"%s:Not support:I2C_FUNC_SMBUS_READ_BYTE Not Handles the SMBus read_byte command.",program_invocation_short_name);
	}
	else{
		fprintf(stdout,"%s:Supports   :I2C_FUNC_SMBUS_READ_BYTE Handles the SMBus read_byte command.",program_invocation_short_name);
	}

	if (ioctl(file, I2C_FUNC_SMBUS_WRITE_BYTE, &funcs) < 0){
		fprintf(stdout,"%s:Not support:I2C_FUNC_SMBUS_WRITE_BYTE Not Handles the SMBus write_byte command.",program_invocation_short_name);
	}
	else{
		fprintf(stdout,"%s:Supports   :I2C_FUNC_SMBUS_WRITE_BYTE Handles the SMBus write_byte command.",program_invocation_short_name);
	}

	if (ioctl(file, I2C_FUNC_SMBUS_READ_BYTE_DATA, &funcs) < 0){
		fprintf(stdout,"%s:Not support:I2C_FUNC_SMBUS_READ_BYTE_DATA Not Handles the SMBus read_byte_data command.",program_invocation_short_name);
	}
	else{
		fprintf(stdout,"%s:Supports   :I2C_FUNC_SMBUS_READ_BYTE_DATA Handles the SMBus read_byte_data command.",program_invocation_short_name);
	}

	if (ioctl(file, I2C_FUNC_SMBUS_WRITE_BYTE_DATA, &funcs) < 0){
		fprintf(stdout,"%s:Not support:I2C_FUNC_SMBUS_WRITE_BYTE_DATA Not Handles the SMBus write_byte_data command.",program_invocation_short_name);
	}
	else{
		fprintf(stdout,"%s:Supports   :I2C_FUNC_SMBUS_WRITE_BYTE_DATA Handles the SMBus write_byte_data command.",program_invocation_short_name);
	}

	if (ioctl(file, I2C_FUNC_SMBUS_READ_WORD_DATA, &funcs) < 0){
		fprintf(stdout,"%s:Not support:I2C_FUNC_SMBUS_READ_WORD_DATA Not Handles the SMBus read_word_data command.",program_invocation_short_name);
	}
	else{
		fprintf(stdout,"%s:Supports   :I2C_FUNC_SMBUS_READ_WORD_DATA Handles the SMBus read_word_data command.",program_invocation_short_name);
	}

	if (ioctl(file, I2C_FUNC_SMBUS_WRITE_WORD_DATA, &funcs) < 0){
		fprintf(stdout,"%s:Not support:I2C_FUNC_SMBUS_WRITE_WORD_DATA Not Handles the SMBus write_byte_data command.",program_invocation_short_name);
	}
	else{
		fprintf(stdout,"%s:Supports   :I2C_FUNC_SMBUS_WRITE_WORD_DATA Handles the SMBus write_byte_data command.",program_invocation_short_name);
	}

	if (ioctl(file, I2C_FUNC_SMBUS_PROC_CALL, &funcs) < 0){
		fprintf(stdout,"%s:Not support:I2C_FUNC_SMBUS_PROC_CALL Not Handles the SMBus process_call command.",program_invocation_short_name);
	}
	else{
		fprintf(stdout,"%s:Supports   :I2C_FUNC_SMBUS_PROC_CALL Handles the SMBus process_call command.",program_invocation_short_name);
	}

	if (ioctl(file, I2C_FUNC_SMBUS_READ_BLOCK_DATA, &funcs) < 0){
		fprintf(stdout,"%s:Not support:I2C_FUNC_SMBUS_READ_BLOCK_DATA Not Handles the SMBus read_block_data command.",program_invocation_short_name);
	}
	else{
		fprintf(stdout,"%s:Supports   :I2C_FUNC_SMBUS_READ_BLOCK_DATA Handles the SMBus read_block_data command.",program_invocation_short_name);
	}

	if (ioctl(file, I2C_FUNC_SMBUS_WRITE_BLOCK_DATA, &funcs) < 0){
		fprintf(stdout,"%s:Not support:I2C_FUNC_SMBUS_WRITE_BLOCK_DATA Not Handles the SMBus write_block_data command.",program_invocation_short_name);
	}
	else{
		fprintf(stdout,"%s:Supports   :I2C_FUNC_SMBUS_WRITE_BLOCK_DATA Handles the SMBus write_block_data command.",program_invocation_short_name);
	}

	if (ioctl(file, I2C_FUNC_SMBUS_READ_I2C_BLOCK, &funcs) < 0){
		fprintf(stdout,"%s:Not support:I2C_FUNC_SMBUS_READ_I2C_BLOCK Not Handles the SMBus read_i2c_block_data command.",program_invocation_short_name);
	}
	else{
		fprintf(stdout,"%s:Supports   :I2C_FUNC_SMBUS_READ_I2C_BLOCK Handles the SMBus read_i2c_block_data command.",program_invocation_short_name);
	}

	if (ioctl(file, I2C_FUNC_SMBUS_WRITE_I2C_BLOCK, &funcs) < 0){
		fprintf(stdout,"%s:Not support:I2C_FUNC_SMBUS_WRITE_I2C_BLOCK Not Handles the SMBus write_i2c_block_data command.",program_invocation_short_name);
	}
	else{
		fprintf(stdout,"%s:Supports   :I2C_FUNC_SMBUS_WRITE_I2C_BLOCK Handles the SMBus write_i2c_block_data command.",program_invocation_short_name);
	}

	return 0;
}


__s32 open_i2c_device(char *i2c_smbus_device_filename)
{
	int file;
	if ((file = open(i2c_smbus_device_filename, O_RDWR)) < 0)
	{
		fprintf(stderr, "%s:Failed to open i2c port %s. Errno = %d: %s.\n",program_invocation_short_name,i2c_smbus_device_filename,errno,strerror(errno));
		return -1;
	}
	else
	{
		fprintf(stdout, "%s:Starting on i2c port %s.\n",program_invocation_short_name,i2c_smbus_device_filename);
	}
	return file;
}

__s32 i2c_set_slave(int file, __u8 SLAVE_ADDR)
{
	// Set the port options and set the address of the device we wish to speak to
	if (ioctl(file, I2C_SLAVE, SLAVE_ADDR) < 0)
	{
		fprintf(stderr, "%s:Unable to get bus access to talk to slave. Errno = %d: %s.\n",program_invocation_short_name,errno,strerror(errno));
		return -1;
	}
	return 0;
}
