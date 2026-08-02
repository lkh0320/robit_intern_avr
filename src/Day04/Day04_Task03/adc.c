
#include "adc.h"

void ADC_Init(void)
{
    ADMUX  = (1 << REFS0);              // AVCC 기준전압 사용, 채널 0으로 초기화
    ADCSRA = (1 << ADEN)                // ADC 활성화
           | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // 분주비 128 (16MHz/128=125kHz)
}

uint16_t ADC_Read(uint8_t channel)
{
    ADMUX = (ADMUX & 0xE0) | (channel & 0x1F); // 채널 선택 (REFS 비트는 유지)
    ADCSRA |= (1 << ADSC);                     // 변환 시작
    while (ADCSRA & (1 << ADSC));              // 변환 완료 대기
    return ADC;                                // 0 ~ 1023
}
