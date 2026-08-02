/*
 * Day03_Task02.c
 *
 * Created: 2026-07-31 오전 11:40:44
 * Author : lkh06
 */ 
#define F_CPU 16000000              

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

// UART 함수 원형 선언
unsigned char Uart_Getch(void);
void Uart_Putch(unsigned char PutData);
void Uart_Puts(char *str);

int main(void)
{
    //UART 초기 설정
    UBRR0L = 103;                   // Baud Rate 9600bps (16MHz 기준)
    UBRR0H = 0;
    UCSR0A = 0x00;                  // 기본 설정
    UCSR0B = 0x18;                  // 송신(TX), 수신(RX) 활성화
    UCSR0C = 0x06;                  // 8비트 데이터, 패리티 없음, Stop Bit 1개

    // TX 핀 출력 설정
    DDRE = 0x02;

    // 전역 인터럽트 허용
    SREG = 0x80;

    //입출력 설정

    DDRC &= ~(1<<PC0);             
    DDRA = 0xFF;                   

    PORTA = 0xFF;                   // Active Low LED라 처음에는 모두 OFF

    unsigned char LED = 0;          // 현재 LED 상태 저장 변수

    while (1)
    {
        unsigned char recvData = 0;

        // UART로 문자가 들어왔는지 확인
        if (UCSR0A & (1 << RXC0))
        {
            recvData = Uart_Getch();    // 문자 하나 읽기

            // 입력된 문자에 따라 LED 제어
            switch(recvData)
            {
                case '0':
                    LED = 0b00000001;   
                    Uart_Putch(recvData);
                    Uart_Puts(" LED on\r\n");
                    break;

                case '1':
                    LED = 0b00000010;   
                    Uart_Putch(recvData);
                    Uart_Puts(" LED on\r\n");
                    break;

                case '2':
                    LED = 0b00000100;   
                    Uart_Putch(recvData);
                    Uart_Puts(" LED on\r\n");
                    break;

                case '3':
                    LED = 0b00001000;  
                    Uart_Putch(recvData);
                    Uart_Puts(" LED on\r\n");
                    break;

                case '4':
                    LED = 0b00010000;   
                    Uart_Putch(recvData);
                    Uart_Puts(" LED on\r\n");
                    break;

                case '5':
                    LED = 0b00100000;   
                    Uart_Putch(recvData);
                    Uart_Puts(" LED on\r\n");
                    break;

                case '6':
                    LED = 0b01000000;   
                    Uart_Putch(recvData);
                    Uart_Puts(" LED on\r\n");
                    break;

                case '7':
                    LED = 0b10000000;   
                    Uart_Putch(recvData);
                    Uart_Puts(" LED on\r\n");
                    break;

                case '8':
                    // 기존 LED를 오른쪽으로 한 칸 이동시켜
                    // LED가 왼쪽으로 이동하는 것처럼 보이게 함
                    LED = (LED >> 1) | (LED << 7);
                    Uart_Puts("LEFT\r\n");
                    break;

                case '9':
                    // 기존 LED를 왼쪽으로 한 칸 이동시켜
                    // LED가 오른쪽으로 이동하는 것처럼 보이게 함
                    LED = (LED << 1) | (LED >> 7);
                    Uart_Puts("RIGHT\r\n");
                    break;

                default:
                    // 지정하지 않은 문자가 들어오면 오류 출력
                    Uart_Puts("error\r\n");
                    break;
            }
        }

        // 버튼이 계속 눌려있는 동안 여러 번 실행되지 않도록
        // 이전 버튼 상태를 저장
        static uint8_t old = 0;

        // 버튼이 처음 눌렸을 때만 실행 (상승 에지 검출)
        if ((PINC & (1<<PC0)) && !old)
        {
            old = 1;
            LED = 0;                     // 모든 LED OFF
            Uart_Puts("RESET\r\n");
        }
        // 버튼에서 손을 떼면 다시 눌림을 감지할 수 있도록 초기화
        else if (!(PINC & (1<<PC0)))
        {
            old = 0;
        }

        // Active Low LED이므로 비트를 반전시켜 출력
        PORTA = ~LED;
    }
}


// UART 수신 함수
// 수신 버퍼에 데이터가 들어올 때까지 기다렸다가 반환
unsigned char Uart_Getch(void)
{
    while(!(UCSR0A & (1 << RXC0)));
    return UDR0;
}


// UART 송신 함수
// 송신 버퍼가 비어있으면 문자 하나 전송
void Uart_Putch(unsigned char PutData)
{
    while(!(UCSR0A & (1 << UDRE0)));
    UDR0 = PutData;
}


// 문자열 출력 함수
// 문자열 끝('\0')까지 한 글자씩 전송
void Uart_Puts(char *str)
{
    while(*str)
    {
        Uart_Putch(*str++);
    }
}