/*
 * Day04_Task03.c
 *
 * Created: 2026-08-02 오후 4:17:43
 * Author : lkh06
 */ 
#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "adc.h"
#include "uart.h"


// 측정 주기 설정
//
// Timer1 인터럽트가 발생하는 주기를 결정한다.
// 이 값을 변경하면 센서 측정 빈도를 쉽게 변경할 수 있도록 구성했다.
#define MEASURE_INTERVAL_MS 200UL


// Timer1의 클럭 분주비 설정
//
// CPU 클럭을 그대로 사용하면 너무 빠르기 때문에
// 분주비를 적용하여 원하는 시간 간격을 만들기 쉽게 한다.
#define TIMER1_PRESCALER 1024UL


// Timer1 CTC 모드에서 사용할 비교값 계산
//
// Timer1은 OCR1A 값에 도달하면 인터럽트를 발생시키므로
// 원하는 시간(ms)에 맞는 값을 자동으로 계산한다.
//
// 예)
// 200ms마다 인터럽트 발생
// F_CPU / 분주비 / 1000 * 시간(ms) - 1
#define OCR1A_VALUE ((F_CPU / TIMER1_PRESCALER / 1000UL) * MEASURE_INTERVAL_MS - 1UL)



// PSD 센서가 연결된 ADC 채널
//
// PSD 센서의 아날로그 출력값을 ADC로 읽어서
// 거리 계산에 사용한다.
#define PSD_ADC_CHANNEL 1



// PSD 센서 거리 변환 계수
//
// PSD 센서는 출력 전압과 거리가 완전히 선형 관계가 아니기 때문에
// 실측 데이터를 기반으로 보정식을 사용한다.
//
// 기본 형태:
// 거리 = A / (ADC값 + B) - C
//
// 실제 센서마다 특성이 다르기 때문에
// 측정한 데이터를 이용하여 A, B, C 값을 조정한다.
#define DIST_COEFF_A   27222.0f
#define DIST_COEFF_B   0.0f
#define DIST_OFFSET_C  27.1f



// ADC 입력값의 정상 범위 설정
//
// 센서가 측정 가능한 범위를 벗어나면
// 잘못된 값이 들어올 수 있기 때문에 예외 처리를 위해 사용한다.
#define ADC_MIN_VALID   140
#define ADC_MAX_VALID   700


// 실제 거리의 정상 범위
//
// 계산된 거리가 센서의 측정 범위를 벗어나면
// 정상적인 측정값으로 판단하지 않는다.
#define DIST_MIN_CM     15.0f
#define DIST_MAX_CM     150.0f



// Timer1 인터럽트 발생 여부를 저장하는 변수
//
// 인터럽트에서는 측정 실행을 직접 하지 않고
// 플래그만 변경한다.
//
// 실제 ADC 측정과 UART 출력은 메인 루프에서 처리하여
// 인터럽트 처리 시간을 짧게 유지한다.
volatile uint8_t measure_flag = 0;



// Timer1 초기 설정 함수
//
// 일정한 시간마다 센서 값을 측정하기 위해 Timer1을 사용한다.
// CTC 모드를 사용하면 OCR1A 값에 도달할 때마다
// 정확한 주기의 인터럽트를 만들 수 있다.
static void Timer1_Init(void)
{
    // 일반 모드 초기화
    TCCR1A = 0x00;


    // CTC 모드 설정
    // WGM12 = 1 : OCR1A 비교 후 자동 초기화
    //
    // CS12, CS10 = 1
    // Timer1 분주비 1024 설정
    TCCR1B = (1 << WGM12) | (1 << CS12) | (1 << CS10);


    // 계산된 비교값 적용
    OCR1A = (uint16_t)OCR1A_VALUE;


    // Output Compare A 인터럽트 허용
    TIMSK |= (1 << OCIE1A);
}



// Timer1 인터럽트 함수
//
// 설정한 주기마다 자동으로 호출된다.
//
// 여기서는 센서 측정을 직접 하지 않고
// 측정이 필요하다는 표시만 남긴다.
ISR(TIMER1_COMPA_vect)
{
    measure_flag = 1;
}

// ADC 값을 실제 거리로 변환하는 함수
//
// PSD 센서는 ADC 값과 거리가 반비례하는 특성을 가지기 때문에
// 단순한 비례식이 아닌 보정식을 사용한다.
//
// 반환값은 거리(cm) * 10 형태로 저장한다.
//
// 예)
// 25.3cm → 253
//
// 이렇게 저장하면 float 출력 없이도
// 소수점 한 자리 표현이 가능하다.
static int16_t Convert_ADC_To_Distance10(uint16_t adc_raw)
{
    float distance_cm;


    // 실측을 통해 얻은 보정식을 이용하여 거리 계산
    distance_cm =
        (DIST_COEFF_A / ((float)adc_raw + DIST_COEFF_B))
        - DIST_OFFSET_C;


    // 소수점 첫째 자리까지 표현하기 위해 10을 곱하고 반올림
    return (int16_t)(distance_cm * 10.0f + 0.5f);
}



// 측정된 값이 정상 범위인지 확인하는 함수
//
// 센서는 주변 환경이나 노이즈에 의해
// 비정상적인 ADC 값이 들어올 수 있기 때문에
// 측정값 검증 과정을 추가하였다.
//
// 반환값
// 1 : 정상적인 측정값
// 0 : 범위를 벗어난 오류값
static uint8_t Is_Valid_Reading(uint16_t adc_raw, int16_t dist10)
{
    // ADC 값이 센서 사용 범위를 벗어난 경우
    if (adc_raw <= ADC_MIN_VALID || adc_raw >= ADC_MAX_VALID)
        return 0;


    // 계산된 거리가 실제 센서 측정 범위를 벗어난 경우
    if (dist10 < (int16_t)(DIST_MIN_CM * 10) ||
        dist10 > (int16_t)(DIST_MAX_CM * 10))
        return 0;


    return 1;
}



// 센서 값을 한 번 측정하고 UART로 출력하는 함수
//
// ADC 읽기 → 거리 변환 → 정상 여부 확인 → 출력
// 과정을 하나의 함수로 묶어서 main의 구조를 단순하게 만든다.
static void Measure_And_Print(void)
{
    uint16_t adc_raw;
    int16_t dist10;


    // PSD 센서의 아날로그 값을 ADC로 읽음
    adc_raw = ADC_Read(PSD_ADC_CHANNEL);


    // ADC 값을 실제 거리 값으로 변환
    dist10 = Convert_ADC_To_Distance10(adc_raw);



    // 측정값이 정상 범위인지 확인
    if (Is_Valid_Reading(adc_raw, dist10))
    {
        // UART로 거리 출력
        //
        // dist10은 소수점 한 자리 표현용 값이므로
        // UART 함수에서 자동으로 변환하여 출력한다.
        UART0_PrintDecimal1(dist10);

        UART0_Println(" cm");
    }
    else
    {
        // 센서 범위를 벗어난 경우 오류 출력
        UART0_Println("  ERROR: Out of range");
    }
}




int main(void)
{
    // ADC 초기화
    //
    // PSD 센서의 아날로그 출력값을 읽기 위해 사용한다.
    ADC_Init();


    // UART 초기화
    //
    // 측정한 거리 데이터를 PC에서 확인하기 위해 사용한다.
    UART0_Init(9600);


    // Timer1 초기화
    //
    // 일정한 주기로 센서를 측정하기 위한 타이머 설정
    Timer1_Init();


    // Timer1 인터럽트 사용을 위한 전역 인터럽트 허용
    sei();



    // 프로그램 시작 메시지 출력
    UART0_Println("PSD Distance Measurement Start");



    while (1)
    {
        // Timer1 인터럽트에서 측정 요청이 발생하면 실행
        if (measure_flag)
        {
            // 플래그 초기화
            measure_flag = 0;


            // 센서 측정 후 UART 출력
            Measure_And_Print();
        }
    }
}