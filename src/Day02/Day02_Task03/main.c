/*
 * Day02_Task03.c
 *
 * Created: 2026-07-30 오후 3:23:04
 * Author : lkh06
 */

#define F_CPU 16000000UL

#include <avr/io.h>              
#include <util/delay.h>        
#include <stdio.h>              
#include "lcd_i2c.h"            
#include "i2c.h"                 

int main(void)
{
	int A = 0;          // 첫 번째 피연산자
	int B = 0;          // 두 번째 피연산자

	int op = 0;         // 현재 선택된 연산자
	                    // 0 = +
	                    // 1 = -
	                    // 2 = *
	                    // 3 = /

	char ops[4] = {'+', '-', '*', '/'};
	                    // op 값에 따라 출력할 연산자를 저장한 배열

	char msg[17];        // LCD에 출력할 문자열 저장 공간

	int result = 0;      // 계산 결과 저장 변수


	//버튼 초기화
	// PC0, PC1을 입력으로 설정한다.
	// &= ~를 사용하는 이유는 다른 비트는 그대로 유지하고
	// 해당 비트만 입력(0)으로 변경하기 위해서이다.
	DDRC &= ~(1 << PC0);
	DDRC &= ~(1 << PC1);

	// 내부 풀업 저항을 활성화한다.
	// 버튼을 누르지 않았을 때 HIGH,
	// 버튼을 누르면 LOW가 입력된다.
	PORTC |= (1 << PC0);
	PORTC |= (1 << PC1);

	// PD2, PD3도 입력으로 설정한다.
	DDRD &= ~(1 << PD2);
	DDRD &= ~(1 << PD3);

	// PD2, PD3 역시 내부 풀업 저항을 사용한다.
	PORTD |= (1 << PD2);
	PORTD |= (1 << PD3);


	//LCD 초기화

	LCD_Init();          // LCD를 사용할 수 있도록 초기화한다.
	LCD_Clear();         // LCD 화면을 모두 지운다.

	// 첫 번째 줄에 이름(이니셜)을 출력한다.
	LCD_StringXY(0, 0, "LKH");


	while (1)
	{
		//bt1
		// 버튼을 누를 때마다 A 값을 1씩 증가시킨다.
		// 버튼은 Active Low 방식이므로 LOW가 입력되면 눌린 상태이다.
		if ((PINC & (1 << PC0)) == 0)
		{
			A = A + 1;

			// 버튼을 한 번 눌렀을 때 여러 번 입력되는
			// 채터링 현상을 줄이기 위해 잠시 대기한다.
			_delay_ms(200);
		}


		//bt2
		// 연산자를 변경한다.
		if ((PINC & (1 << PC1)) == 0)
		{
			op = op + 1;

			// 마지막 연산자(/)까지 선택되면
			// 다시 + 연산으로 돌아간다.
			if (op > 3)
			{
				op = 0;
			}

			_delay_ms(200);
		}


		//bt3
		// 버튼을 누를 때마다 B 값을 1씩 증가시킨다.
		if ((PIND & (1 << PD2)) == 0)
		{
			B = B + 1;
			_delay_ms(200);
		}


		//bt4
		// 버튼을 누르고 있는 동안 계산 결과를 표시한다.
		if ((PIND & (1 << PD3)) == 0)
		{
			// 나눗셈에서 0으로 나누는 경우는 계산이 불가능하므로
			// 오류 메시지를 출력한다.
			if (op == 3 && B == 0)
			{
				sprintf(msg, "%d %c %d = err  ", A, ops[op], B);
			}
			else
			{
				// 현재 선택된 연산자에 따라 계산을 수행한다.

				if (op == 0)
					result = A + B;

				if (op == 1)
					result = A - B;

				if (op == 2)
					result = A * B;

				if (op == 3)
					// 정수형 변수이므로 나눗셈 결과의 소수점은 버려진다.
					result = A / B;

				// 계산 결과를 LCD에 출력할 문자열로 만든다.
				sprintf(msg, "%d %c %d = %d  ", A, ops[op], B, result);
			}
		}

		// bt4를 누르지 않은 경우에는
		// 현재 입력된 식만 LCD에 표시한다.
		else
		{
			sprintf(msg, "%d %c %d      ", A, ops[op], B);
		}


		// 두 번째 줄에 계산식 또는 결과를 출력한다.
		LCD_StringXY(1, 0, msg);

		// LCD를 너무 자주 갱신하지 않도록 약간 대기한다.
		_delay_ms(50);
	}

	return 0;
}