#ifndef LCD_I2C_H_
#define LCD_I2C_H_

#include <avr/io.h>
#define LCD_ADDR 0x27

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
