#include "uart.h"
#include <stdlib.h>

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

void UART0_Init(uint32_t baud)
{
    uint16_t ubrr = (uint16_t)((F_CPU / 16UL / baud) - 1);

    UBRR0H = (uint8_t)(ubrr >> 8);
    UBRR0L = (uint8_t)(ubrr);

    UCSR0B = (1 << TXEN0) | (1 << RXEN0);   // 송신/수신 활성화
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8비트 데이터, No parity, 1 stop bit
}

void UART0_Transmit(uint8_t data)
{
    while (!(UCSR0A & (1 << UDRE0))); // 송신 버퍼 빌 때까지 대기
    UDR0 = data;
}

void UART0_Print(const char *str)
{
    while (*str)
        UART0_Transmit((uint8_t)(*str++));
}

void UART0_Println(const char *str)
{
    UART0_Print(str);
    UART0_Transmit('\r');
    UART0_Transmit('\n');
}

void UART0_PrintNumber(int16_t num)
{
    char buf[8];
    itoa(num, buf, 10);
    UART0_Print(buf);
}

void UART0_PrintDecimal1(int16_t scaled)
{
    int16_t whole, frac;

    if (scaled < 0)
    {
        UART0_Transmit('-');
        scaled = -scaled;
    }

    whole = scaled / 10;
    frac  = scaled % 10;

    UART0_PrintNumber(whole);
    UART0_Transmit('.');
    UART0_Transmit((uint8_t)('0' + frac));
}
