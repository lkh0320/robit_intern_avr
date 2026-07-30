#include "I2C_LCD.h"
#include <util/delay.h>
#include <stdio.h>

#define LCD_ADDR 0x27

#define LCD_BACKLIGHT 0x08
#define EN 0x04
#define RW 0x02
#define RS 0x01

static void LCD_Write(unsigned char data)
{
	TWI_Start();
	TWI_Write(LCD_ADDR<<1);
	TWI_Write(data|LCD_BACKLIGHT);
	TWI_Stop();
}

static void LCD_Enable(unsigned char data)
{
	LCD_Write(data|EN);
	_delay_us(1);
	LCD_Write(data&~EN);
	_delay_us(50);
}

static void LCD_Send4(unsigned char nibble,unsigned char mode)
{
	unsigned char data;

	data=(nibble&0xF0);

	if(mode)
	data|=RS;

	LCD_Enable(data);
}

void lcdCommand(unsigned char cmd)
{
	LCD_Send4(cmd,0);
	LCD_Send4(cmd<<4,0);
	_delay_ms(2);
}

void lcdData(unsigned char data)
{
	LCD_Send4(data,1);
	LCD_Send4(data<<4,1);
}

void lcdInit(void)
{
	_delay_ms(50);

	LCD_Send4(0x30,0);
	_delay_ms(5);

	LCD_Send4(0x30,0);
	_delay_us(150);

	LCD_Send4(0x30,0);
	LCD_Send4(0x20,0);

	lcdCommand(0x28);
	lcdCommand(0x0C);
	lcdCommand(0x06);
	lcdCommand(0x01);

	_delay_ms(2);
}

void lcdClear(void)
{
	lcdCommand(0x01);
}

static void lcdGoto(unsigned char line,unsigned char col)
{
	if(line==0)
	lcdCommand(0x80+col);
	else
	lcdCommand(0xC0+col);
}

void lcdString(unsigned char line,unsigned char col,char *str)
{
	lcdGoto(line,col);

	while(*str)
	{
		lcdData(*str++);
	}
}

void lcdNumber(unsigned char line,unsigned char col,int num)
{
	char buf[10];

	sprintf(buf,"%d",num);

	lcdString(line,col,buf);
}