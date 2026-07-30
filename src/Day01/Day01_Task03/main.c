/*
 * Day01_Task01.c
 *
 * Created: 2026-07-29 오후 4:57:33
 * Author : lkh06
 */

#define F_CPU 16000000UL          

#include <avr/io.h>               
#include <util/delay.h>           
#include <avr/interrupt.h>        

volatile uint8_t counter = 0x00;  // LED에 출력할 값을 저장하는 변수
                                  // volatile은 인터럽트에서도 값이 변경될 수 있기 때문에
                                  // 컴파일러가 최적화를 하지 않도록 하기 위해 사용한다.

int main(void)
{
    DDRA = 0xFF;                  // PORTA의 모든 핀을 출력으로 설정한다.

    DDRD = 0x00;                  // PORTD의 모든 핀을 입력으로 설정한다.
                                  // 외부 인터럽트(INT0~INT3)를 사용하기 위한 설정이다.

    PORTC = 0x00;                 // PORTC의 내부 풀업 저항을 사용하지 않는다.
    PORTD = 0x00;                 // PORTD도 내부 풀업 저항을 사용하지 않는다.

    PORTA = 0xFF;                 

    EIMSK = 0b00001111;           // INT0, INT1, INT2, INT3 외부 인터럽트를 모두 허용한다.

    EICRA = 0x02;                 // 외부 인터럽트의 동작 조건을 설정한다.
                                  // 현재 값으로는 INT0만 Falling Edge 방식으로 설정된다.

    sei();                        // 전역 인터럽트를 허용한다.
                                  // 이 함수가 실행되어야 인터럽트가 발생한다.

    while(1)
    {
        // counter 값을 0부터 255까지 1씩 증가시키며 LED에 출력한다.
        for (counter = 0x00; counter != 0b11111111; counter = counter + 1)
        {
            PORTA = ~counter;     // counter 값을 반전하여 LED에 출력한다.
                                  // Active Low 방식이므로 비트를 반전해야 원하는 LED가 켜진다.

            _delay_ms(100);       // 숫자가 너무 빨리 증가하지 않도록 0.1초 대기한다.
        }
    }
}


// INT0 인터럽트가 발생하면 실행되는 함수
ISR(INT0_vect)
{
    uint8_t move = 0x07;          // 처음에는 하위 3개의 LED를 켠 상태로 시작한다.
    int count = 1;                // LED가 몇 바퀴 이동했는지 세기 위한 변수

    while(1)
    {
        // LED가 두 바퀴 이동하면 반복을 종료한다.
        if (count == 3)
        {
            break;
        }

        // 이동하다가 0xE0 위치까지 오면 한 바퀴를 완료한 것으로 판단한다.
        else if (move == 0xE0)
        {
            count += 1;
        }

        PORTA = ~move;            // 현재 LED 위치를 출력한다.
        _delay_ms(500);           // 이동하는 모습을 보기 위해 0.5초 동안 유지한다.

        // LED 패턴을 왼쪽으로 한 칸 회전시킨다.
        // 가장 왼쪽 비트는 다시 오른쪽 끝으로 이동한다.
        move = (move << 1) | (move >> 7);
    }
}


// INT1 인터럽트가 발생하면 실행되는 함수
ISR(INT1_vect)
{
    uint8_t move = 0xE0;          // 상위 3개의 LED가 켜진 상태에서 시작한다.
    int count = 1;                // 회전 횟수를 저장한다.

    while(1)
    {
        // 두 바퀴 회전하면 종료한다.
        if (count == 3)
        {
            break;
        }

        // 처음 위치(0x07)로 돌아오면 한 바퀴 완료
        else if (move == 0x07)
        {
            count += 1;
        }

        PORTA = ~move;            // 현재 LED 상태를 출력한다.
        _delay_ms(500);           // LED 이동 속도를 조절한다.

        // LED 패턴을 오른쪽으로 한 칸 회전시킨다.
        // 가장 오른쪽 비트는 다시 왼쪽 끝으로 이동한다.
        move = (move >> 1) | (move << 7);
    }
}


// INT2 인터럽트가 발생하면 실행되는 함수
ISR(INT2_vect)
{
    int16_t move;

    // LED를 왼쪽에서 오른쪽 방향으로 하나씩 이동시킨다.
    for (move = 0x80; move != 0b00000001; move = move >> 1)
    {
        PORTA = ~move;
        _delay_ms(100);
    }

    // 마지막 LED를 출력한다.
    PORTA = ~move;
    _delay_ms(100);

    // 이번에는 오른쪽에서 왼쪽 방향으로 다시 이동한다.
    for (move = 0x01; move != 0b10000000; move = move << 1)
    {
        PORTA = ~move;
        _delay_ms(100);
    }

    // 마지막 LED를 출력한 후 인터럽트를 종료한다.
    PORTA = ~move;
    _delay_ms(100);
}


// INT3 인터럽트가 발생하면 실행되는 함수
ISR(INT3_vect)
{
    // counter 값을 0으로 초기화한다.
    // 메인 반복문에서 다시 0부터 LED 카운트가 시작된다.
    counter = 0x00;
}