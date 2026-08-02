#include "lcd_i2c.h"
#include "i2c.h"
#include <util/delay.h>

static uint8_t backlight_state = LCD_BL; // 기본값: 백라이트 켬

// PCF8574로 1바이트 그대로 전송
static void LCD_I2C_Write(uint8_t data)
{
    I2C_Start((LCD_ADDR << 1) | 0); // Write 모드
    I2C_Write(data);
    I2C_Stop();
}

static void LCD_PulseEnable(uint8_t data)
{
    LCD_I2C_Write(data | LCD_EN);   // EN = 1
    _delay_us(1);
    LCD_I2C_Write(data & ~LCD_EN);  // EN = 0
    _delay_us(50);
}

// 4비트(상위 니블) 데이터 전송
static void LCD_Write4Bits(uint8_t nibble)
{
    uint8_t data = (nibble & 0xF0) | backlight_state;
    LCD_I2C_Write(data);
    LCD_PulseEnable(data);
}

// 실제 커맨드/데이터 전송 (8비트를 4비트씩 2번 전송)
static void LCD_Send(uint8_t value, uint8_t mode) // mode: 0=Command, 1=Data
{
    uint8_t rs = mode ? LCD_RS : 0x00;
    uint8_t high = (value & 0xF0) | rs | backlight_state;
    uint8_t low  = ((value << 4) & 0xF0) | rs | backlight_state;

    LCD_I2C_Write(high);
    LCD_PulseEnable(high);

    LCD_I2C_Write(low);
    LCD_PulseEnable(low);
}

void LCD_Command(uint8_t cmd)
{
    LCD_Send(cmd, 0);
    if (cmd == 0x01 || cmd == 0x02) // Clear / Home 명령은 시간이 더 필요
        _delay_ms(2);
}

void LCD_Data(uint8_t data)
{
    LCD_Send(data, 1);
}

void LCD_Init(void)
{
    I2C_Init();
    _delay_ms(50); // LCD 전원 안정화 대기

    // HD44780 4비트 모드 초기화 시퀀스
    LCD_Write4Bits(0x30);
    _delay_ms(5);
    LCD_Write4Bits(0x30);
    _delay_us(150);
    LCD_Write4Bits(0x30);
    _delay_us(150);
    LCD_Write4Bits(0x20); // 4비트 모드로 전환

    LCD_Command(0x28); // 4bit, 2line, 5x8 font
    LCD_Command(0x08); // Display OFF
    LCD_Command(0x01); // Clear
    _delay_ms(2);
    LCD_Command(0x06); // Entry mode: 커서 자동 증가
    LCD_Command(0x0C); // Display ON, Cursor OFF, Blink OFF
}

void LCD_Clear(void)
{
    LCD_Command(0x01);
    _delay_ms(2);
}

void LCD_SetCursor(uint8_t row, uint8_t col)
{
    uint8_t row_offsets[] = {0x00, 0x40, 0x14, 0x54}; // 16x2 / 20x4 대응
    LCD_Command(0x80 | (col + row_offsets[row]));
}

void LCD_String(const char *str)
{
    while (*str)
    {
        LCD_Data((uint8_t)(*str));
        str++;
    }
}

void LCD_StringXY(uint8_t row, uint8_t col, const char *str)
{
    LCD_SetCursor(row, col);
    LCD_String(str);
}

void LCD_Backlight(uint8_t on)
{
    backlight_state = on ? LCD_BL : 0x00;
    LCD_I2C_Write(backlight_state); // 즉시 반영
}
