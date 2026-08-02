/*
 * Day04_Task04.c
 *
 * Created: 2026-08-02 오후 5:17:34
 * Author : lkh06
 */ 
#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "adc.h"
#include "uart.h"


// 센서 측정 주기 설정
#define MEASURE_INTERVAL_MS 200UL


// Timer1 분주비 설정
#define TIMER1_PRESCALER 1024UL



// Timer1 CTC 모드에서 사용할 비교값 계산
// Timer1은 OCR1A 값에 도달하면 인터럽트를 발생시킨다.
// 원하는 측정 주기를 만들기 위해 자동으로 OCR 값을 계산한다.
// 계산 과정
// F_CPU / 분주비 = Timer 입력 주파수
// 입력 주파수 / 1000 × 시간(ms) - 1
#define OCR1A_VALUE ((F_CPU / TIMER1_PRESCALER / 1000UL) * MEASURE_INTERVAL_MS - 1UL)



// PSD 센서가 연결된 ADC 채널
// PSD 센서는 아날로그 출력값을 가지므로
// ADC를 이용해 거리 정보를 얻는다.
#define PSD_ADC_CHANNEL 1



// PSD 센서 거리 변환 보정 계수
// PSD 센서는 ADC 값과 거리가 완전한 선형 관계가 아니기 때문에
// 실측 데이터를 기반으로 보정식을 적용한다.
//
// 기본 형태
// distance = A / (ADC + B) - C
//
// 센서마다 특성이 다르므로 실제 측정값으로 계수를 조정한다.
#define DIST_COEFF_A   27222.0f
#define DIST_COEFF_B   0.0f
#define DIST_OFFSET_C  27.1f



// 센서 정상 동작 범위 설정
// 센서가 측정 가능한 범위를 벗어나면
// 잘못된 값으로 판단하여 오류 처리한다.
#define ADC_MIN_VALID   140
#define ADC_MAX_VALID   700


// 실제 거리 기준 정상 범위
#define DIST_MIN_CM     15.0f
#define DIST_MAX_CM     150.0f



// 가중 이동 평균 필터 설정
// PSD 센서는 측정값에 순간적인 노이즈가 발생할 수 있기 때문에
// 여러 번 측정한 값을 평균 내어 안정적인 값을 얻는다.
// 일반 평균(SMA)은 모든 데이터에 같은 비중을 주지만,
// 가중 이동 평균(WMA)은 최근 데이터에 더 큰 비중을 줘서
// 변화에 빠르게 반응하면서 노이즈를 줄이는 방식이다.
#define FILTER_WINDOW_SIZE 5



// 가중치 배열
//
// 오래된 데이터부터 최신 데이터 순서로 저장한다.
//
// 예)
// 데이터 : x1 x2 x3 x4 x5
// 가중치 : 1  2  3  4  5
//
// 최근 데이터(x5)의 영향이 가장 크도록 설정한다.
static const uint16_t FILTER_WEIGHTS[FILTER_WINDOW_SIZE] =
{
    1, 2, 3, 4, 5
};



/*
 * 전역 변수
 */


// Timer1 인터럽트 발생 여부 저장
// 인터럽트에서는 단순히 측정 필요 여부만 표시하고,
// 실제 ADC 측정과 UART 출력은 메인 루프에서 수행한다.
// 이렇게 하면 인터럽트 처리 시간을 짧게 유지할 수 있다.
volatile uint8_t measure_flag = 0;



// 가중 이동 평균 필터용 데이터 저장 배열
// 최근 ADC 측정값을 저장하는 공간이다.
// adc_window[0] : 가장 오래된 값
// adc_window[마지막] : 가장 최근 값
static uint16_t adc_window[FILTER_WINDOW_SIZE] = {0};



// 현재 저장된 실제 데이터 개수
// 전원이 켜진 직후에는 배열이 0으로 초기화되어 있기 때문에
// 아직 데이터가 충분하지 않은 상태이다.
// 이 값을 이용해 의미 없는 초기 0값이 평균 계산에 포함되지 않도록 한다.
static uint8_t window_count = 0;



// Timer1 초기화 함수
// 일정한 주기로 센서 측정을 실행하기 위해 사용한다.
// CTC 모드를 이용하여 정확한 시간 간격을 만든다.
static void Timer1_Init(void)
{
    // Timer1 동작 모드 초기화
    TCCR1A = 0x00;


    // CTC 모드 설정
    // WGM12 : OCR1A 비교 모드
    // CS12, CS10 : 분주비 1024
    TCCR1B =
        (1 << WGM12) |
        (1 << CS12) |
        (1 << CS10);


    // 계산된 비교값 적용
    OCR1A = (uint16_t)OCR1A_VALUE;

    // Compare Match A 인터럽트 허용
    TIMSK |= (1 << OCIE1A);
}


// Timer1 인터럽트 함수
// 설정한 주기마다 실행된다.
// 센서 측정을 여기서 직접 하지 않고
// flag만 변경하여 메인 루프에서 처리하도록 한다.
ISR(TIMER1_COMPA_vect)
{
    measure_flag = 1;
}

// 가중 이동 평균(Weighted Moving Average) 필터 함수
// PSD 센서의 ADC 값은 주변 빛, 센서 흔들림 등의 영향으로
// 순간적으로 값이 튀는 노이즈가 발생할 수 있다.
// 따라서 최근 측정값 여러 개를 저장하고,
// 각각 다른 가중치를 적용하여 보다 안정적인 값을 만든다.
// 동작 과정
// 1. 기존 데이터들을 한 칸씩 앞으로 이동
//    → 가장 오래된 데이터 제거
// 2. 새로운 ADC 값을 가장 마지막 위치에 저장
//    → 최신 데이터 유지
// 3. 각 데이터에 가중치를 곱한 뒤 평균 계산
// 최근 데이터일수록 높은 가중치를 가지기 때문에
// 단순 평균보다 센서 변화에 빠르게 반응한다.
static uint16_t Apply_Weighted_Moving_Average(uint16_t new_raw)
{
    uint8_t i;

    // 계산 결과가 커질 수 있기 때문에
    // 16비트보다 큰 32비트 변수 사용
    uint32_t weighted_sum = 0;

    uint32_t weight_sum = 0;


    // 기존 측정값을 왼쪽으로 이동
    //
    // 예)
    // 기존 : [100][110][120][130][140]
    //
    // 이동 후:
    //       [110][120][130][140][새값]
    //
    // 가장 오래된 값은 제거되고 최신 값이 추가된다.
    for (i = 0; i < FILTER_WINDOW_SIZE - 1; i++)
    {
        adc_window[i] = adc_window[i + 1];
    }


    // 가장 마지막 위치에 새로운 ADC 값 저장
    adc_window[FILTER_WINDOW_SIZE - 1] = new_raw;



    // 초기에는 배열이 모두 채워져 있지 않기 때문에
    // 실제 측정한 데이터 개수를 증가시킨다.
    // 예)
    // 전원 ON 직후
    // [0][0][0][0][새값]
    // 이런 0값들이 평균 계산에 들어가면
    // 실제보다 작은 값이 나오므로 제외해야 한다.
    if (window_count < FILTER_WINDOW_SIZE)
    {
        window_count++;
    }



    // 아직 채워지지 않은 앞쪽 데이터 제외 위치 계산
    // index 3,4만 사용한다.
    uint8_t start_index =
        FILTER_WINDOW_SIZE - window_count;



    // 가중 평균 계산
    // 최근 데이터일수록 큰 가중치를 가지므로
    // 변화에 빠르게 반응할 수 있다.
    for (i = start_index; i < FILTER_WINDOW_SIZE; i++)
    {
        weighted_sum +=
            (uint32_t)adc_window[i] * FILTER_WEIGHTS[i];


        weight_sum += FILTER_WEIGHTS[i];
    }



    // 최종 필터링된 ADC 값 반환
    return (uint16_t)(weighted_sum / weight_sum);
}

// ADC 값을 실제 거리로 변환하는 함수
//실측 데이터를 기반으로 만든 보정식을 사용한다.
// 사용한 공식
// distance(cm) = A / (ADC + B) - C
// 계산 결과는 소수점 한 자리까지 표현하기 위해
// 실제 거리의 10배 값으로 반환한다.
// 예)
// 25.3cm → 253
static int16_t Convert_ADC_To_Distance10(uint16_t adc_raw)
{
    float distance_cm;

    // ADC 값을 거리(cm)로 변환
    distance_cm =
        (DIST_COEFF_A / ((float)adc_raw + DIST_COEFF_B))
        - DIST_OFFSET_C;

    // 소수점 첫째 자리 표현을 위해 10배 후 반올림
    return (int16_t)(distance_cm * 10.0f + 0.5f);
}

// 측정값의 정상 여부를 판단하는 함수
// 1) ADC 값 자체가 정상 범위인지
// 2) 변환된 거리가 측정 가능한 범위인지
// 두 가지 조건을 모두 확인한다.
// 반환값
// 1 : 정상 데이터
// 0 : 오류 데이터
static uint8_t Is_Valid_Reading(uint16_t adc_raw, int16_t dist10)
{
    // ADC 원본 값이 센서 범위를 벗어난 경우
    // 너무 낮거나 높은 ADC 값은
    // 센서 측정 한계를 의미할 수 있다.
    if (adc_raw <= ADC_MIN_VALID ||
        adc_raw >= ADC_MAX_VALID)
    {
        return 0;
    }

    // 변환된 거리값이 실제 센서 범위를 벗어난 경우
    // 예)
    // 음수 거리
    // 150cm 초과 거리
    // 이런 값은 정상 측정값으로 보기 어렵다.
    if (dist10 < (int16_t)(DIST_MIN_CM * 10) ||
        dist10 > (int16_t)(DIST_MAX_CM * 10))
    {
        return 0;
    }

    return 1;
}

// 센서 측정 후 결과 출력 함수
// 전체 측정 흐름
// 1. ADC 센서값 읽기
// 2. 가중 이동 평균 적용
// 3. 필터링된 값으로 거리 계산
// 4. UART 출력
// RAW 값과 FILTERED 값을 같이 출력하여
// 필터 적용 전후 차이를 확인할 수 있도록 했다.
static void Measure_And_Print(void)
{
    // 필터 적용 전 원본 ADC 값
    uint16_t adc_raw =
        ADC_Read(PSD_ADC_CHANNEL);

    // 노이즈 제거를 위한 필터 적용 값
    uint16_t adc_filtered =
        Apply_Weighted_Moving_Average(adc_raw);

    // 실제 거리 계산은 필터링된 값을 사용
    // 이유:
    // RAW 값은 순간적인 노이즈가 포함될 수 있기 때문에
    // 안정적인 거리 측정을 위해 FILTERED 값을 사용한다.
    int16_t dist10 =
        Convert_ADC_To_Distance10(adc_filtered);

    // UART로 원본 ADC 값 출력
    UART0_Print("RAW: ");
    UART0_PrintNumber((int16_t)adc_raw);

    // UART로 필터 적용 후 ADC 값 출력
    UART0_Print(" | FILTERED: ");
    UART0_PrintNumber((int16_t)adc_filtered);

    UART0_Print(" | ");



    // 이상 여부 판단은 RAW 값을 기준으로 한다.
    // 이유:
    // 필터링된 값으로 검사하면 순간적인 이상값도
    // 평균에 의해 완화되어 센서 이상을 놓칠 수 있다.
    // 따라서
    // - 거리 계산 : FILTERED 사용
    // - 센서 이상 판단 : RAW 사용
    // 으로 역할을 나누었다.
    if (Is_Valid_Reading(adc_raw, dist10))
    {
        UART0_Print("DISTANCE: ");

        UART0_PrintDecimal1(dist10);

        UART0_Println("cm");
    }
    else
    {
        UART0_Println("ERROR: Out of range");
    }
}

int main(void) 
{ 
    ADC_Init(); //ADC 초기화
    UART0_Init(9600); //UART0 통신속도 9600 설정
    Timer1_Init(); 
    sei(); 
    UART0_Println("PSD Distance Measurement Start");
    while (1) 
    { 
        if (measure_flag)
        { 
            measure_flag = 0; 
            Measure_And_Print(); 
        } 
    } 
}