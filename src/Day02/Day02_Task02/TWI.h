#ifndef __TWI_H__
#define __TWI_H__

#include <avr/io.h>

void TWI_Init(void);
void TWI_Start(void);
void TWI_Stop(void);
void TWI_Write(unsigned char data);
unsigned char TWI_Start_Address(unsigned char address);
#endif