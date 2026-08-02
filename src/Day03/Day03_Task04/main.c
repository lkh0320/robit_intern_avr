/*
 * Day03_Task04.c
 *
 * Created: 2026-08-02 오전 12:55:11
 * Author : lkh06
 */ 
#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>

// 소프트웨어 UART에서 사용할 TX 핀과 전송 속도 설정
// 9600bps 기준으로 1비트 전송 시간을 약 104us로 설정
#define UART_TX_PIN PD3
#define BIT_DELAY_US 104

// TX 핀을 출력으로 설정하고
// UART의 대기 상태인 High로 만들어 둠
static void SoftUART_Init(void)
{
    DDRD |= (1 << UART_TX_PIN);
    PORTD |= (1 << UART_TX_PIN);
}

// 소프트웨어 UART 방식으로 문자 1개를 전송
// Start Bit → Data 8bit → Stop Bit 순서로 전송
static void SoftUART_SendByte(uint8_t data)
{
    // Start Bit는 Low
    PORTD &= ~(1 << UART_TX_PIN);
    _delay_us(BIT_DELAY_US);

    // 데이터는 LSB부터 차례대로 전송
    for (uint8_t i = 0; i < 8; i++)
    {
        if (data & (1 << i))
            PORTD |= (1 << UART_TX_PIN);      // 비트가 1이면 High 출력
        else
            PORTD &= ~(1 << UART_TX_PIN);     // 비트가 0이면 Low 출력

        _delay_us(BIT_DELAY_US);
    }

    // Stop Bit는 High
    PORTD |= (1 << UART_TX_PIN);
    _delay_us(BIT_DELAY_US);
}

// 문자열의 끝('\0')이 나올 때까지
// 한 글자씩 전송하는 함수
static void SoftUART_SendString(const char *str)
{
    while (*str)
    {
        SoftUART_SendByte((uint8_t)*str);
        str++;
    }
}

int main(void)
{
    // 소프트웨어 UART 초기 설정
    SoftUART_Init();

    while (1)
    {
        // "HelloWorld!" 문자열을 전송
        SoftUART_SendString("HelloWorld!\r\n");

        // 1초 대기 후 다시 전송
        _delay_ms(1000);
    }
}