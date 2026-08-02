/*
 * Day04_Task02.c
 *
 * Created: 2026-08-02 오전 2:28:50
 * Author : lkh06
 */ 
#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "i2c.h"
#include "lcd_i2c.h"
#include "adc.h"

// 스위치가 연결된 핀 정의
// SW1은 시간 설정을 넘기는 버튼
// SW2는 설정이 끝난 후 시계를 시작하는 버튼
#define SW1_PIN (1 << PC0)
#define SW2_PIN (1 << PC1)

// 현재 날짜와 시간을 저장하는 구조체
// 하나의 구조체로 관리하면 시간을 다루기가 편하다.
typedef struct
{
    uint8_t year;      // 연도(00~99)
    uint8_t month;     // 월(1~12)
    uint8_t day;       // 일(1~31)
    uint8_t hour;      // 시(0~23)
    uint8_t min;       // 분(0~59)
    uint8_t sec;       // 초(0~59)
    uint8_t csec;      // 1/100초(10ms)
} RTC_Time;

// 프로그램이 현재 어떤 단계인지 저장하는 상태값
// 상태를 나누어 관리하면 switch문으로 동작을 쉽게 구분할 수 있다.
typedef enum
{
    SET_YEAR = 0,
    SET_MONTH,
    SET_DAY,
    SET_HOUR,
    SET_MIN,
    SET_SEC,
    WAIT_RUN,
    RUNNING
} SystemState;

// 현재 시간을 저장하는 변수
// 기본값은 2000년 1월 1일 00:00:00
volatile RTC_Time now = {0, 1, 1, 0, 0, 0, 0};

// 현재 프로그램 상태
volatile SystemState state = SET_YEAR;

// 1이면 인터럽트에서 시간이 증가하도록 설정
volatile uint8_t timer_running = 0;

// LCD를 다시 출력해야 하는지 알려주는 플래그
// 인터럽트에서 LCD를 직접 출력하지 않고 메인에서 출력하도록 한다.
volatile uint8_t display_update_flag = 0;

// 버튼의 이전 상태 저장
// Edge 검출을 위해 사용한다.
static uint8_t sw1_last = 1;
static uint8_t sw2_last = 1;

// 윤년인지 확인하는 함수
// 2월의 마지막 날짜를 결정할 때 사용한다.
static uint8_t Is_Leap_Year(uint16_t year)
{
    return ((year % 4 == 0) && (year % 100 != 0))
        || (year % 400 == 0);
}

// 입력한 월이 며칠까지 있는지 반환
// 날짜가 범위를 넘지 않도록 하기 위해 사용한다.
static uint8_t Days_In_Month(uint8_t month, uint16_t year)
{
    static const uint8_t dim[12] =
    {
        31,28,31,30,31,30,
        31,31,30,31,30,31
    };

    if (month < 1)
        month = 1;

    if (month > 12)
        month = 12;

    // 윤년이면 2월은 29일까지 존재
    if (month == 2 && Is_Leap_Year(year))
        return 29;

    return dim[month - 1];
}

// Timer1을 10ms마다 인터럽트가 발생하도록 설정
// 일정한 주기로 시간을 증가시키기 위해 사용한다.
static void Timer1_Init(void)
{
    // CTC 모드 사용
    TCCR1A = 0x00;

    // 분주비 64 설정
    // OCR1A에 도달하면 인터럽트 발생
    TCCR1B = (1 << WGM12) | (1 << CS11) | (1 << CS10);

    // 16MHz / 64 = 250000Hz
    // 250000 / 2500 = 100Hz
    // 즉 10ms마다 인터럽트 발생
    OCR1A = 2499;

    // Compare Match A 인터럽트 허용
    TIMSK |= (1 << OCIE1A);
}
// Timer1 Compare Match 인터럽트 함수
// Timer1에서 10ms마다 호출되며 실제 시간을 증가시키는 역할을 한다.
ISR(TIMER1_COMPA_vect)
{
    uint8_t max_day;

    // 아직 시작 버튼을 누르지 않았다면 시간 증가하지 않음
    if (!timer_running)
        return;

    // 10ms마다 센티초 증가
    now.csec++;

    // 100개의 센티초가 모이면 1초 증가
    if (now.csec >= 100)
    {
        now.csec = 0;
        now.sec++;

        // 60초가 지나면 1분 증가
        if (now.sec >= 60)
        {
            now.sec = 0;
            now.min++;

            // 60분이 지나면 1시간 증가
            if (now.min >= 60)
            {
                now.min = 0;
                now.hour++;

                // 24시간이 지나면 날짜 변경
                if (now.hour >= 24)
                {
                    now.hour = 0;

                    // 현재 월의 마지막 날짜 확인
                    max_day = Days_In_Month(now.month, 2000 + now.year);

                    now.day++;

                    // 해당 월의 마지막 날을 넘어가면 다음 달로 이동
                    if (now.day > max_day)
                    {
                        now.day = 1;
                        now.month++;

                        // 12월이 지나면 다음 해로 변경
                        if (now.month > 12)
                        {
                            now.month = 1;
                            now.year++;

                            // 2099년 이후 다시 2000년으로 돌아오도록 설정
                            if (now.year > 99)
                                now.year = 0;
                        }
                    }
                }
            }
        }
    }

    // LCD 출력은 인터럽트에서 하지 않고
    // 메인 루프에서 처리하도록 표시만 변경
    // 인터럽트 시간을 짧게 유지하기 위한 방식
    display_update_flag = 1;
}


// 버튼 입력 확인 함수
// 풀업 저항을 사용하기 때문에
// 평소에는 HIGH, 누르면 LOW가 된다.
// 버튼은 눌렀을 때 여러 번 입력되는 채터링 현상이 발생하기 때문에
// 20ms 대기 후 다시 확인하여 실제 입력인지 판단한다.
static uint8_t Check_Button_Edge(uint8_t pin_mask, uint8_t *last_state)
{
    uint8_t cur = (PINC & pin_mask) ? 1 : 0;
    uint8_t edge = 0;

    // 이전에는 HIGH였고 현재 LOW라면
    // 버튼이 새롭게 눌린 순간으로 판단
    if (cur == 0 && *last_state == 1)
    {
        // 버튼 채터링 방지를 위한 디바운싱
        _delay_ms(20);

        // 20ms 후에도 눌려있다면 정상 입력으로 처리
        if ((PINC & pin_mask) == 0)
            edge = 1;
    }

    // 현재 상태를 다음 비교를 위해 저장
    *last_state = cur;

    return edge;
}


// ADC 값을 원하는 범위의 숫자로 변환하는 함수
// ADC는 0~1023의 값을 가지지만
// 시간 설정에서는 각각 다른 범위가 필요하기 때문에
// 범위에 맞게 변환하여 사용한다.
// 예)
// 연도 : 0~99
// 월 : 1~12
// 시간 : 0~23
static uint16_t Map_ADC(uint16_t adc_val, uint16_t min_v, uint16_t max_v)
{
    uint32_t range = (uint32_t)(max_v - min_v + 1);

    uint32_t mapped = ((uint32_t)adc_val * range) / 1024UL;

    // 계산 결과가 최대 범위를 넘어가지 않도록 제한
    if (mapped > (uint32_t)(max_v - min_v))
        mapped = (uint32_t)(max_v - min_v);

    return (uint16_t)(min_v + mapped);
}


// LCD에 숫자를 두 자리 형태로 출력하는 함수
// 예)
// 1 -> 01
// 9 -> 09
// 시간을 표시할 때 일정한 형식을 유지하기 위해 사용한다.
static void LCD_Print2(uint8_t val)
{
    LCD_Data('0' + (val / 10) % 10);
    LCD_Data('0' + val % 10);
}


// 현재 날짜와 시간을 LCD에 표시하는 함수
// 설정 모드와 실행 모드에서 같은 출력 함수를 사용하기 위해
// 별도의 함수로 분리하였다.
static void LCD_Update_Display(RTC_Time *t)
{
    // 첫 번째 줄에 날짜 출력
    LCD_SetCursor(0, 0);

    LCD_Print2(t->year);
    LCD_Print2(t->month);
    LCD_Print2(t->day);

    // 이전 출력 내용이 남지 않도록 빈칸 출력
    LCD_String("      ");


    // 두 번째 줄에 시간 출력
    LCD_SetCursor(1, 0);

    LCD_Print2(t->hour);
    LCD_Data(':');

    LCD_Print2(t->min);
    LCD_Data(':');

    LCD_Print2(t->sec);
    LCD_Data('.');

    LCD_Print2(t->csec);

    LCD_String("  ");
}
// 시간 설정 모드를 처리하는 함수
// ADC(가변저항)를 이용해 값을 선택하고
// SW1 버튼을 누르면 다음 설정 항목으로 이동한다.
// 설정 순서
// 년 → 월 → 일 → 시 → 분 → 초
// 상태 머신 구조를 사용하여 각 설정 단계를 관리한다.
static void Handle_Setting_State(void)
{
    // 현재 ADC 값 읽기
    // 가변저항의 위치에 따라 설정값이 결정된다.
    uint16_t adc = ADC_Read(0);

    RTC_Time local;


    switch (state)
    {
        // 연도 설정
        // ADC 값을 0~99 범위로 변환하여 저장
        case SET_YEAR:

            now.year = (uint8_t)Map_ADC(adc, 0, 99);

            // SW1을 누르면 다음 설정 단계로 이동
            if (Check_Button_Edge(SW1_PIN, &sw1_last))
                state = SET_MONTH;

            break;


        // 월 설정
        // 1월부터 12월까지 선택 가능
        case SET_MONTH:

            now.month = (uint8_t)Map_ADC(adc, 1, 12);

            if (Check_Button_Edge(SW1_PIN, &sw1_last))
                state = SET_DAY;

            break;


        // 날짜 설정
        case SET_DAY:
        {
            uint8_t max_day;


            // 현재 선택된 월과 연도를 기준으로
            // 가능한 최대 날짜 계산
            //
            // 예)
            // 2월은 28일 또는 29일
            // 4월은 30일까지
            max_day = Days_In_Month(now.month, 2000 + now.year);


            // 계산된 최대 날짜 범위 안에서 설정
            now.day = (uint8_t)Map_ADC(adc, 1, max_day);


            if (Check_Button_Edge(SW1_PIN, &sw1_last))
                state = SET_HOUR;

            break;
        }


        // 시간 설정
        // 0~23시간 범위
        case SET_HOUR:

            now.hour = (uint8_t)Map_ADC(adc, 0, 23);


            if (Check_Button_Edge(SW1_PIN, &sw1_last))
                state = SET_MIN;

            break;


        // 분 설정
        // 0~59분 범위
        case SET_MIN:

            now.min = (uint8_t)Map_ADC(adc, 0, 59);


            if (Check_Button_Edge(SW1_PIN, &sw1_last))
                state = SET_SEC;

            break;


        // 초 설정
        // 0~59초 범위
        case SET_SEC:

            now.sec = (uint8_t)Map_ADC(adc, 0, 59);


            // 초 설정까지 완료하면
            // 실제 시간 흐름 시작 전 대기 상태로 이동
            if (Check_Button_Edge(SW1_PIN, &sw1_last))
                state = WAIT_RUN;

            break;


        default:
            break;
    }


    // 설정 중인 시간을 LCD에 표시하기 위해 복사
    // 설정 단계에서는 timer_running이 0이므로
    // 인터럽트에서 now 값을 변경하지 않는다.
    // 따라서 안전하게 복사 가능하다.
    local = now;


    // 현재 선택된 날짜와 시간을 LCD에 출력
    LCD_Update_Display(&local);
}
int main(void)
{
    RTC_Time local;


    // 스위치 입력 설정
    // DDRC를 0으로 설정하여 입력 모드로 사용한다.
    // 내부 풀업 저항을 활성화하여
    // 버튼을 누르지 않았을 때 HIGH,
    // 버튼을 누르면 LOW가 입력되도록 구성한다.
    DDRC &= ~(SW1_PIN | SW2_PIN);

    PORTC |= (SW1_PIN | SW2_PIN);



    // 주변장치 초기화
    // I2C 통신 초기화
    // LCD가 I2C 방식으로 연결되어 있기 때문에 먼저 설정한다.
    I2C_Init();


    // LCD 초기화
    LCD_Init();


    // ADC 초기화
    // 가변저항 값을 읽어 시간 설정에 사용한다.
    ADC_Init();


    // Timer1 초기화
    // 10ms마다 인터럽트를 발생시켜 시간을 증가시킨다.
    Timer1_Init();


    // LCD 시작 화면 초기화
    LCD_Clear();


    // 전역 인터럽트 허용
    // Timer1 인터럽트가 정상적으로 동작하기 위해 필요하다.
    sei();



    while (1)
    {
        switch (state)
        {

            // 시간 설정 상태
            
            // 가변저항으로 값을 변경하고
            // SW1 버튼으로 다음 항목으로 이동한다.
            case SET_YEAR:
            case SET_MONTH:
            case SET_DAY:
            case SET_HOUR:
            case SET_MIN:
            case SET_SEC:

                Handle_Setting_State();


                // LCD가 너무 빠르게 갱신되는 것을 방지
                _delay_ms(50);

                break;



            // 시간 설정 완료 후
            // SW2 버튼 입력을 기다리는 상태
            case WAIT_RUN:


                // 현재 설정된 시간을 LCD에 표시
                local = now;

                LCD_Update_Display(&local);



                // SW2를 누르면 실제 시간 동작 시작
                if (Check_Button_Edge(SW2_PIN, &sw2_last))
                {
                    // 시작 순간 센티초 초기화
                    now.csec = 0;


                    // Timer1 인터럽트에서 시간 증가 시작
                    timer_running = 1;


                    // 실행 상태로 변경
                    state = RUNNING;
                }


                _delay_ms(50);

                break;




            // 실제 시간이 흐르는 상태
            case RUNNING:


                // Timer1 인터럽트에서 시간이 변경되면
                // display_update_flag가 1이 된다.
                if (display_update_flag)
                {
                    // LCD 업데이트 완료 후 플래그 초기화
                    display_update_flag = 0;



                    // 인터럽트와 동시에 now 값을 읽으면
                    // 데이터가 일부만 변경되는 문제가 발생할 수 있다.
                    //
                    // 예)
                    // 초 증가 중에 값을 읽으면
                    // 초의 일부 바이트만 변경된 상태로 가져올 수 있음
                    //
                    // 따라서 잠시 인터럽트를 막고 안전하게 복사한다.
                    cli();

                    local = now;

                    sei();



                    // 복사한 시간을 LCD에 출력
                    LCD_Update_Display(&local);
                }

                break;



            default:

                // 예외 상황 발생 시 초기 설정 상태로 복귀
                state = SET_YEAR;

                break;
        }
    }
}