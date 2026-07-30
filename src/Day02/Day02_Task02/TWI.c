#include "TWI.h"

void TWI_Init(void)
{
	TWSR = 0x00;
	TWBR = 72;          // 100kHz @16MHz
	TWCR = (1<<TWEN);
}

void TWI_Start(void)
{
	TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
	while(!(TWCR&(1<<TWINT)));
}

void TWI_Stop(void)
{
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
}

void TWI_Write(unsigned char data)
{
	TWDR = data;
	TWCR = (1<<TWINT)|(1<<TWEN);
	while(!(TWCR&(1<<TWINT)));
}

unsigned char TWI_Start_Address(unsigned char address)
{
	// START
	TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);
	while(!(TWCR & (1<<TWINT)));

	// SLA+W
	TWDR = address;
	TWCR = (1<<TWINT) | (1<<TWEN);
	while(!(TWCR & (1<<TWINT)));

	// ACK È®ÀÎ
	if((TWSR & 0xF8) != 0x18)
	{
		// STOP
		TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
		return 0;
	}

	// STOP
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO);

	return 1;
}