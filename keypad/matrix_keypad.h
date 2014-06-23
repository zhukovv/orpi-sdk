#ifndef MATRIX_KEYPAD_H_INCLUDED
#define MATRIX_KEYPAD_H_INCLUDED

#include <bcm2835.h>

#define C0 RPI_GPIO_P1_12
#define C1 RPI_GPIO_P1_13
#define C2 RPI_GPIO_P1_16
#define R0 RPI_GPIO_P1_18
#define R1 RPI_GPIO_P1_22
#define R2 RPI_GPIO_P1_26
#define R3 RPI_GPIO_P1_11

char get_key(void);
int init_keypad(void);

#endif // MATRIX_KEYPAD_H_INCLUDED
