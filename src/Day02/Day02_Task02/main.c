/*
 * Day02_Task02.c
 *
 * Created: 2026-07-30 오전 5:04:35
 * Author : lkh06
 */

#define F_CPU 16000000UL           

#include <avr/io.h>                
#include <util/delay.h>            
#include <stdio.h>                 // sprintf() 함수를 사용하기 위한 헤더
#include "lcd_i2c.h"               // I2C LCD 제어 함수가 들어있는 헤더
#include "i2c.h"                   // I2C 통신을 위한 헤더


//ADC 초기화
void adc_init(void)
{
	// REFS0 = 1 : 기준 전압을 AVCC(5V)로 설정한다.
	// ADC 입력은 기본적으로 ADC0(PF0) 채널을 사용한다.
	ADMUX = (1 << REFS0);

	// ADEN : ADC 기능을 활성화한다.
	// ADPS2~0 : ADC 클럭 분주비를 128로 설정한다.
	// CPU가 16MHz이므로 ADC 클럭은 125kHz가 되어
	// 데이터시트에서 권장하는 동작 범위에 맞는다.
	ADCSRA = (1 << ADEN)
		   | (1 << ADPS2)
		   | (1 << ADPS1)
		   | (1 << ADPS0);
}


//ADC 값 읽기
uint16_t adc_read(uint8_t channel)
{
	// 사용할 ADC 채널을 선택한다.
	// 상위 비트(기준전압 설정)는 그대로 유지하고
	// 하위 5비트만 원하는 채널 번호로 변경한다.
	ADMUX = (ADMUX & 0xE0) | (channel & 0x1F);

	// ADSC 비트를 1로 설정하면 ADC 변환이 시작된다.
	ADCSRA |= (1 << ADSC);

	// ADSC 비트가 0이 될 때까지 기다린다.
	// 변환이 완료되면 하드웨어가 자동으로 ADSC를 0으로 만든다.
	while (ADCSRA & (1 << ADSC));

	// 변환된 10비트 ADC 값을 반환한다.
	// 반환 범위는 0 ~ 1023이다.
	return ADC;
}


//LED 초기화
void led_init(void)
{
	// PORTA의 모든 핀을 출력으로 설정한다.
	// LED를 제어하기 위해 출력 모드가 필요하다.
	DDRA = 0xFF;

	// 모든 핀을 HIGH로 출력한다.
	PORTA = 0xFF;
}


//ADC 값(0~1023)에 따라 LED 위치를 이동시키는 함수
void led_move(uint16_t adc_value)
{
	// ADC 값은 0~1023이므로
	// 이를 LED 개수인 8개에 맞게 0~7 범위로 변환한다.
	uint8_t pos = (uint32_t)adc_value * 8 / 1024;

	// 계산 결과가 혹시 7을 넘어가는 경우를 대비한 안전 코드이다.
	if (pos > 7)
		pos = 7;

	// 계산된 위치의 LED만 켠다.
	// Active Low 방식이므로 비트를 반전하여 출력한다.
	PORTA = ~(1 << pos);
}


int main(void)
{
	// LCD에 출력할 문자열을 저장하는 배열
	// 문자 16개 + 문자열 끝(NULL 문자)까지 저장하기 위해 크기를 17로 선언한다.
	char line1[17];
	char line2[17];

	uint16_t adc_val;        // ADC 원시값 저장
	uint16_t voltage_mV;     // 계산된 전압(mV) 저장
	uint16_t v_int;          // 전압의 정수 부분
	uint16_t v_frac;         // 전압의 소수 부분

	// 사용할 장치들을 초기화한다.
	adc_init();
	led_init();
	LCD_Init();
	LCD_Clear();

	while (1)
	{
		// ADC0(PF0)에서 아날로그 값을 읽는다.
		// 가변저항을 돌리면 0~1023 사이의 값이 저장된다.
		adc_val = adc_read(0);

		// ADC 값을 실제 전압으로 변환한다.
		// 기준전압이 5V이므로
		// 0~1023 → 0~5000mV로 계산한다.
		voltage_mV = (uint32_t)adc_val * 5000 / 1023;

		// 전압의 정수 부분을 구한다.
		// 예) 3.7V이면 3
		v_int = voltage_mV / 1000;

		// 전압의 소수 첫째 자리만 구한다.
		// 예) 3.7V이면 7
		v_frac = (voltage_mV % 1000) / 100;

		// ADC 값에 비례하여 LED 위치를 이동시킨다.
		led_move(adc_val);

		// LCD 첫 번째 줄에 이름(이니셜)을 출력한다.
		sprintf(line1, "LKH");
		LCD_StringXY(0, 0, line1);

		// LCD 두 번째 줄에
		// ADC 값과 계산된 전압을 함께 출력한다.
		// 예) 512  2.5V
		sprintf(line2, "%4d  %d.%1dV", adc_val, v_int, v_frac);
		LCD_StringXY(1, 0, line2);

		// LCD와 LED가 너무 빠르게 변경되지 않도록
		// 0.2초마다 값을 갱신한다.
		_delay_ms(200);
	}

	return 0;
}