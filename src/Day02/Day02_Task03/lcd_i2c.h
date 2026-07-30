/*******************************************************
 * lcd_i2c.h
 * PCF8574T I2C 백팩 + HD44780 LCD (16x2 / 20x4) 드라이버
 * ATmega128 + i2c.h/i2c.c 필요
 *******************************************************/
#ifndef LCD_I2C_H_
#define LCD_I2C_H_

#include <avr/io.h>

// PCF8574T의 7비트 I2C 슬레이브 주소
// 보드에 따라 0x27 또는 0x3F(PCF8574A) 임. 안 되면 이 값부터 바꿔보세요.
#define LCD_ADDR 0x27

// PCF8574 핀 <-> LCD 핀 매핑 (일반적인 백팩 배선 기준)
// P0=RS, P1=RW, P2=EN, P3=Backlight, P4~P7 = D4~D7
#define LCD_RS   0x01
#define LCD_RW   0x02
#define LCD_EN   0x04
#define LCD_BL   0x08   // Backlight ON

void LCD_Init(void);
void LCD_Command(uint8_t cmd);
void LCD_Data(uint8_t data);
void LCD_Clear(void);
void LCD_SetCursor(uint8_t row, uint8_t col); // row: 0~1(16x2 기준), col: 0~15
void LCD_String(const char *str);
void LCD_StringXY(uint8_t row, uint8_t col, const char *str);
void LCD_Backlight(uint8_t on); // 1: 켜기, 0: 끄기

#endif /* LCD_I2C_H_ */
