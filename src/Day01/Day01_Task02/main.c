/*
 * Day01_Task01.c
 *
 * Created: 2026-07-29 오후 4:57:33
 * Author : lkh06
 */

#define F_CPU 16000000UL          // CPU 클럭을 16MHz로 설정한다.
                                  // _delay_ms() 함수는 이 값을 기준으로 지연 시간을 계산한다.

#include <avr/io.h>               // ATmega128의 입출력 레지스터를 사용하기 위한 헤더
#include <util/delay.h>           // _delay_ms(), _delay_us() 함수를 사용하기 위한 헤더
#include <avr/interrupt.h>        // 인터럽트 관련 함수와 ISR()을 사용하기 위한 헤더

int main(void)
{
    DDRA = 0xFF;                  // PORTA의 모든 핀을 출력으로 설정한다.
                                  // LED가 연결되어 있으므로 출력 모드로 설정하여 LED를 제어한다.

    DDRC &= ~((1<<PC0) | (1<<PC1));   // PC0, PC1을 입력으로 설정한다.
                                      // DDR에서 0은 입력, 1은 출력이다.
                                      // &= ~를 사용하는 이유는 다른 비트는 그대로 유지하면서
                                      // PC0, PC1만 입력으로 변경하기 위해서이다.

    DDRD &= ~((1<<PD2) | (1<<PD3));   // PD2(INT2), PD3(INT3)을 입력으로 설정한다.
                                      // 외부 인터럽트 입력으로 사용할 핀이므로 입력 모드가 필요하다.

    PORTC = 0x00;                 // PORTC 출력값을 모두 LOW로 설정한다.
                                  // 입력 모드에서는 내부 풀업 저항도 비활성화된다.

    PORTD = 0x00;                 // PORTD도 내부 풀업 저항을 사용하지 않도록 LOW로 설정한다.

    PORTA = 0xFF;                 // 시작할 때 LED를 모두 끈다.
                                  // 사용하는 LED 회로가 Active Low 방식이므로
                                  // HIGH를 출력하면 LED가 꺼지고 LOW를 출력하면 켜진다.

    EIMSK = 0b00001100;           // 외부 인터럽트 INT2와 INT3을 허용한다.
                                  // 비트2 = INT2, 비트3 = INT3

    EICRA = 0x02;                 // INT2를 Falling Edge(하강 에지)에서 동작하도록 설정한다.
                                  // INT3은 설정하지 않았으므로 기본 설정을 사용한다.

    sei();                        // 전역 인터럽트를 허용한다.
                                  // 이 함수가 실행되어야 ISR이 호출된다.

    int Togle = 0;                // LED 점멸 상태를 저장하는 변수
                                  // 0이면 LED OFF, 1이면 LED ON 상태를 의미한다.

    while(1)
    {
        // PC0과 PC1이 모두 눌렸는지 확인한다.
        // 스위치를 누르면 LOW가 입력되므로 !( )를 사용하여 눌림을 검사한다.
        if(!(PINC & (1<<PC0)) && !(PINC & (1<<PC1)))
        {
            PORTA = 0x00;         // LED를 모두 켠다.
            _delay_ms(10);        // 스위치 채터링을 줄이기 위해 짧게 대기한다.
        }

        // PC0만 눌린 경우
        else if(!(PINC & (1<<PC0)))
        {
            PORTA = 0x0F;         // 하위 4개의 LED만 켠다.
            _delay_ms(10);
        }

        // PC1만 눌린 경우
        else if(!(PINC & (1<<PC1)))
        {
            PORTA = 0xF0;         // 상위 4개의 LED만 켠다.
            _delay_ms(10);
        }

        // 아무 버튼도 눌리지 않은 경우
        else
        {
            // Toggle 변수 값을 이용하여 LED를 반복적으로 점멸시킨다.
            if (Togle == 1)
            {
                Togle = 0;        // 다음 반복에서는 OFF 상태가 되도록 변경

                PORTA = 0xFF;     // LED 모두 끄기
                _delay_ms(500);   // 0.5초 동안 유지
            }
            else
            {
                Togle = 1;        // 다음 반복에서는 ON 상태가 되도록 변경

                PORTA = 0x00;     // LED 모두 켜기
                _delay_ms(500);   // 0.5초 동안 유지
            }
        }
    }
}


// INT2 인터럽트가 발생하면 자동으로 실행되는 함수
ISR(INT2_vect)
{
    int16_t move;                 // 이동할 LED 위치를 저장하는 변수

    // 가장 왼쪽 LED부터 오른쪽 방향으로 이동한다.
    // move 값을 오른쪽으로 한 칸씩 이동시키면서 LED 위치를 변경한다.
    for (move = 0x80; move != 0b00000001; move = move >> 1)
    {
        PORTA = ~move;            // Active Low 방식이므로 비트를 반전하여 출력한다.
                                  // move 위치의 LED만 켜지게 된다.

        _delay_ms(100);           // LED 이동 속도를 보기 쉽게 0.1초 대기한다.
    }

    // 반복문이 끝나면 마지막 LED(맨 오른쪽)를 출력한다.
    PORTA = ~move;
    _delay_ms(100);
}


// INT3 인터럽트가 발생하면 자동으로 실행되는 함수
ISR(INT3_vect)
{
    int16_t move;                 // 현재 LED 위치 저장

    // 가장 오른쪽 LED부터 왼쪽 방향으로 이동한다.
    // move 값을 왼쪽으로 한 칸씩 이동시키면서 LED 위치를 변경한다.
    for (move = 0x01; move != 0b10000000; move = move << 1)
    {
        PORTA = ~move;            // 현재 위치의 LED만 켜지도록 출력한다.
        _delay_ms(100);
    }

    // 마지막으로 가장 왼쪽 LED를 출력한다.
    PORTA = ~move;
    _delay_ms(100);
}