/*
 * Day06_Task01.c
 *
 * Created: 2026-08-9 오전 3:01:33
 * Author : lkh06
 */

#define F_CPU 16000000UL      
                             

#include <avr/io.h>        
#include <util/delay.h>    


// 모터를 정회전시키는 함수
void Motor_Forward(void);

// 모터를 역회전시키는 함수
void Motor_Reverse(void);

// 모터를 정지시키는 함수
void Motor_Stop(void);


int main(void)
{
    // PB0, PB1 : Motor A의 회전 방향을 제어한다.
    // PB2, PB3 : Motor B의 회전 방향을 제어한다.
    // PB5, PB6 : Timer1의 PWM 출력 핀으로 사용한다.
    //
    // 모터 드라이버를 제어하기 위해 사용하는 핀이므로
    // 모두 출력 모드로 설정한다.
    DDRB |= (1 << PB0) | (1 << PB1) |
            (1 << PB2) | (1 << PB3) |
            (1 << PB5) | (1 << PB6);


    // 처음 전원이 켜졌을 때 모터가 바로 움직이지 않도록
    // 방향 제어 핀을 모두 LOW로 설정한다.
    //
    // PB0 = 0, PB1 = 0 : Motor A 정지
    // PB2 = 0, PB3 = 0 : Motor B 정지
    PORTB &= ~((1 << PB0) | (1 << PB1) |
               (1 << PB2) | (1 << PB3));


    // Timer1을 Fast PWM 모드로 설정한다.
    //
    // COM1A1, COM1B1을 1로 설정하면
    // OC1A(PB5), OC1B(PB6) 핀으로 PWM 신호가 출력된다.
    //
    // WGM13, WGM12, WGM11을 설정하여
    // ICR1 값을 TOP 값으로 사용하는 Fast PWM 모드로 설정한다.
    TCCR1A = (1 << COM1A1) |
             (1 << COM1B1) |
             (1 << WGM11);

    // CS11 = 1로 설정하여 Timer1의 분주비를 8로 설정한다.
    //
    // ATmega128의 클럭이 16MHz이므로
    // Timer1에는 16MHz / 8 = 2MHz의 클럭이 입력된다.
    TCCR1B = (1 << WGM13) |
             (1 << WGM12) |
             (1 << CS11);


    // PWM의 주기를 결정하는 TOP 값을 설정한다.
    //
    // Fast PWM에서 ICR1을 399로 설정하면
    // Timer1은 0부터 399까지 카운트한다.
    //
    // PWM 주파수 =
    // 16MHz / (8 × (399 + 1))
    // = 5kHz
    ICR1 = 399;


    // PWM의 듀티비를 약 50%로 설정한다.
    //
    // OCR1A와 OCR1B는 각각 OC1A(PB5), OC1B(PB6)의
    // PWM 출력 시간을 결정한다.
    //
    // OCR 값이 199이고 TOP 값이 399이므로
    // 약 50% 듀티비의 PWM 신호가 출력된다.
    //
    // 이 PWM 신호를 이용하여 모터의 속도를 제어할 수 있다.
    OCR1A = 199;
    OCR1B = 199;


    while (1)
    {
        // 두 모터를 같은 방향으로 정회전시킨다.
        Motor_Forward();

        // 정회전 상태를 3초 동안 유지한다.
        _delay_ms(3000);


        // 모터를 모두 정지시킨다.
        Motor_Stop();

        // 정지 상태를 1초 동안 유지한다.
        _delay_ms(1000);


        // 두 모터를 반대 방향으로 역회전시킨다.
        Motor_Reverse();

        // 역회전 상태를 3초 동안 유지한다.
        _delay_ms(3000);


        // 다시 모터를 정지시킨다.
        Motor_Stop();

        // 정지 상태를 1초 동안 유지한 후
        // 다시 while문 처음으로 돌아간다.
        _delay_ms(1000);
    }
}


// 두 모터를 정회전시키는 함수
void Motor_Forward(void)
{
    // Motor A의 회전 방향 설정
    //
    // PB0 = HIGH
    // PB1 = LOW
    //
    // 모터 드라이버의 IN 핀에 서로 다른 값을 입력하여
    // Motor A가 한 방향으로 회전하도록 한다.
    PORTB |= (1 << PB0);
    PORTB &= ~(1 << PB1);


    // Motor B의 회전 방향 설정
    //
    // PB2 = HIGH
    // PB3 = LOW
    //
    // Motor A와 같은 방향으로 Motor B를 회전시킨다.
    PORTB |= (1 << PB2);
    PORTB &= ~(1 << PB3);
}


// 두 모터를 역회전시키는 함수
void Motor_Reverse(void)
{
    // Motor A의 회전 방향을 반대로 설정한다.
    //
    // PB0 = LOW
    // PB1 = HIGH
    //
    // 정회전 때와 반대로 설정하면
    // 모터의 전류 방향이 바뀌어 역방향으로 회전한다.
    PORTB &= ~(1 << PB0);
    PORTB |= (1 << PB1);


    // Motor B도 Motor A와 동일하게
    // 반대 방향으로 회전하도록 설정한다.
    //
    // PB2 = LOW
    // PB3 = HIGH
    PORTB &= ~(1 << PB2);
    PORTB |= (1 << PB3);
}


// 두 모터를 정지시키는 함수
void Motor_Stop(void)
{
    // Motor A와 Motor B의 방향 제어 핀을 모두 LOW로 설정한다.
    //
    // PB0 = 0, PB1 = 0
    // PB2 = 0, PB3 = 0
    //
    // 모터 드라이버에 따라 이 상태에서는
    // 모터에 회전 방향 신호가 입력되지 않아 정지하게 된다.
    PORTB &= ~((1 << PB0) |
               (1 << PB1) |
               (1 << PB2) |
               (1 << PB3));
}