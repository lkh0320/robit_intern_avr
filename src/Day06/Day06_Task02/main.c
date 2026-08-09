/*
 * Day06_Task02.c
 *
 * Created: 2026-08-10 오전 3:35:10
 * Author : lkh06
 */

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h> 
#include <stdint.h>

#define BAUD 9600

#define UBRR_VALUE (F_CPU / 16 / BAUD - 1)


// UART0 초기화 함수
void UART0_init(void)
{
    // 계산한 UBRR 값을 상위 8비트와 하위 8비트로 나누어 저장한다.
    UBRR0H = (unsigned char)(UBRR_VALUE >> 8);
    UBRR0L = (unsigned char)UBRR_VALUE;


    // UART0의 송신 기능을 활성화한다.
    UCSR0B = (1 << TXEN0);


    // UART 통신 데이터 형식을 설정한다.

    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}


// UART0으로 문자 1개를 전송하는 함수
void UART0_transmit(unsigned char data)
{
    // UART의 송신 데이터 레지스터가 비어 있을 때까지 기다린다.

    while (!(UCSR0A & (1 << UDRE0)));

    // 전송할 문자 데이터를 UDR0 레지스터에 저장한다.
    // 데이터를 저장하면 UART 하드웨어가 자동으로 데이터를 전송한다.
    UDR0 = data;
}


// 문자열을 UART로 전송하는 함수
void UART0_print(char *str)
{
    // 문자열의 끝(NULL 문자)이 나올 때까지
    // 문자 하나씩 UART0_transmit() 함수로 전송한다.
    while (*str)
    {
        UART0_transmit(*str++);
    }
}


// ADC 초기화 함수
void ADC_init(void)
{
    // REFS0 = 1로 설정하여 ADC의 기준전압을 AVCC로 사용한다.
    //
    // ATmega128 보드에서 AVCC가 5V인 경우
    // ADC는 약 0V ~ 5V 범위의 입력 전압을 측정할 수 있다.
    ADMUX = (1 << REFS0);


    // ADEN = 1
    // → ADC 기능을 활성화한다.
    //
    // ADPS2, ADPS1, ADPS0 = 1
    // → ADC 클럭 분주비를 128로 설정한다.
    //
    // CPU 클럭이 16MHz이므로
    // ADC 클럭은 16MHz / 128 = 125kHz가 된다.
    ADCSRA = (1 << ADEN) |
             (1 << ADPS2) |
             (1 << ADPS1) |
             (1 << ADPS0);
}


// 지정한 ADC 채널의 값을 읽는 함수
uint16_t ADC_read(uint8_t channel)
{
    // 사용할 ADC 채널을 선택한다.
    ADMUX = (ADMUX & 0xE0) | (channel & 0x0F);


    ADCSRA |= (1 << ADSC);

    while (ADCSRA & (1 << ADSC));

    return ADC;
}


int main(void)
{
    // UART로 출력할 문자열을 저장하는 배열
    char buf[16];


    // 라인 트레이서에 연결된 6개의 IR 센서가
    // 각각 어느 ADC 채널에 연결되어 있는지 저장한다.
    //
    // 센서 1 → ADC2 = PF2
    // 센서 2 → ADC4 = PF4
    // 센서 3 → ADC6 = PF6
    // 센서 4 → ADC7 = PF7
    // 센서 5 → ADC5 = PF5
    // 센서 6 → ADC3 = PF3
    uint8_t sensor_pin[6] = {2, 4, 6, 7, 5, 3};

    uint16_t sensor_value[6];

    UART0_init();

    ADC_init();

    UART0_print("Line Tracer IR Sensor Start\r\n");


    while (1)
    {
        // 6개의 IR 센서 값을 순서대로 읽는다.
        for (uint8_t i = 0; i < 6; i++)
        {
            // sensor_pin 배열에 저장된 ADC 채널 번호를 사용하여
            // 각 센서의 아날로그 값을 읽는다.
            sensor_value[i] = ADC_read(sensor_pin[i]);
        }


        // 읽어온 6개의 센서 값을 UART로 출력한다.
        for (uint8_t i = 0; i < 6; i++)
        {
            // 센서 번호와 ADC 값을 문자열로 만든다.
            //
            // %d  : 센서 번호 출력
            // %4u : 부호 없는 정수를 최소 4자리 폭으로 출력
            //
            // 시리얼 모니터에서 값을 보기 편하다.
            sprintf(buf, "S%d:%4u ", i + 1, sensor_value[i]);


            // 만들어진 문자열을 UART를 통해 전송한다.
            UART0_print(buf);
        }

        UART0_print("\r\n");
        _delay_ms(200);
    }
}