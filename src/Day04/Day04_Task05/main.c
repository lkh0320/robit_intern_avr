/*
 * Day04_Task05.c
 *
 * Created: 2026-08-02 오후 5:22:03
 * Author : lkh06
 */ 
#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>

#include "uart.h"

/*
  서보 PWM 설정
  일반적인 아날로그 서보 모터는 약 20ms 주기의 PWM 신호를 사용한다.
  한 주기 안에서 HIGH 상태가 유지되는 시간(펄스 폭)에 따라
  서보의 목표 각도가 결정된다.
  일반적인 기준
  500us  → 약 0도
  1500us → 약 90도
  2400us → 약 180도
  따라서 Timer1을 이용해 20ms 주기의 PWM을 만들고
  OCR1C 값을 변경하여 HIGH 시간을 조절한다.
 */


#define SERVO_PWM_PERIOD_US 20000UL


// 서보가 움직일 수 있는 최소/최대 펄스 폭
// 서보마다 실제 동작 범위가 다르기 때문에
// 동작이 불안정하면 이 값만 조절하면 된다.
#define SERVO_MIN_PULSE_US 500UL
#define SERVO_MAX_PULSE_US 2400UL


// 전원 인가 후 이동할 초기 위치
// 갑자기 움직이는 것을 방지하고
// 항상 일정한 위치에서 시작하기 위해 사용한다.
#define SERVO_ORIGIN_ANGLE 90
/*
  Timer1 tick 계산
  Timer1 분주비를 8로 설정하면
  16MHz / 8 = 2MHz
  즉 1초에 2,000,000번 카운트
  따라서
  1 tick = 0.5us
  1us = 2 tick
  이 값을 이용하여 펄스 폭(us)을
  Timer 비교값(OCR1C)으로 변환한다.
 */
#define TICKS_PER_US ((F_CPU / 8UL) / 1000000UL)

// Fast PWM에서 사용할 TOP 값
// Timer가 ICR1 값까지 증가하면 한 주기가 끝난다.
// 20ms 주기를 만들기 위한 값이다.
#define ICR1_PERIOD_VALUE \
((SERVO_PWM_PERIOD_US * TICKS_PER_US) - 1UL)
/*
  UART 입력 관련 설정
  사용자가 입력한 각도를 문자열로 저장하기 위한 버퍼 크기
  예)
  "-180"
  "90"
  정도의 입력을 저장할 수 있도록 설정한다.
 /
#define LINE_BUF_SIZE 8

// 현재 서보 위치 저장
// 범위를 벗어난 값이 입력되었을 때
// 모터를 움직이지 않고 기존 위치를 유지하기 위해 사용한다.
static uint8_t current_angle = SERVO_ORIGIN_ANGLE;

/*
  Timer1 PWM 초기화 함수
  OC1C(PB7) 핀에서 서보 제어용 PWM 출력 생성
  Fast PWM Mode 14 사용
  TOP = ICR1
  장점:
  - ICR1으로 주기를 정확하게 설정 가능
  - OCR1C로 펄스 폭 변경 가능
 */
static void Servo_PWM_Init(void)
{
    // PB7(OC1C)을 출력 핀으로 설정
    DDRB |= (1 << PB7);

    /*
      Timer1 설정
      COM1C1 = 1
      → 비반전 PWM 출력 사용
      WGM11, WGM12, WGM13
      → Fast PWM Mode 14 설정
      CS11
      → 분주비 8 설정
     */
    TCCR1A =
        (1 << COM1C1) |
        (1 << WGM11);

    TCCR1B =
        (1 << WGM13) |
        (1 << WGM12) |
        (1 << CS11);

    // 20ms 주기 설정
    ICR1 = (uint16_t)ICR1_PERIOD_VALUE;
}

/*
  서보 각도 설정 함수
  입력받은 각도(0~180)를 실제 PWM 펄스 폭으로 변환한다.
  서보는 각도 값을 직접 받는 것이 아니라
  HIGH 펄스의 길이로 위치를 판단하기 때문에
  각도 → 펄스 폭(us) → Timer tick 값
  순서로 변환해야 한다.
 */
static void Set_Servo_Angle(uint8_t angle)
{
    uint32_t pulse_us;
    /*
      각도를 펄스 폭 범위로 선형 변환
      예)
      0도   → 500us
      90도  → 약 1450us
      180도 → 2400us
      각도 변화에 따라 일정하게 PWM 폭이 변하도록 한다.
     */
    pulse_us =
        SERVO_MIN_PULSE_US +
        ((uint32_t)angle *
        (SERVO_MAX_PULSE_US - SERVO_MIN_PULSE_US))
        / 180UL;

    /*
      펄스 폭(us)을 Timer 비교값으로 변환
      Timer1은 0.5us마다 증가하므로
      OCR1C = 펄스폭 × 2
      형태가 된다.
      OCR1C 값만 변경하면 다음 PWM 주기부터
      새로운 각도로 이동한다.
     */
    OCR1C =
        (uint16_t)(pulse_us * TICKS_PER_US);

    // 현재 정상적으로 적용된 각도 저장
    current_angle = angle;
}
/*
  UART 한 줄 입력 함수
  터미널에서 입력한 문자열을 받아서
  Enter 입력 전까지 저장한다.
  기능
  - 문자 입력 에코 출력
    → 사용자가 입력한 내용이 화면에 보임
  - Backspace 처리
    → 입력 실수 수정 가능
  - Enter 입력 시 문자열 종료
 */
static void UART0_ReadLine(char *buf, uint8_t max_len)
{
    uint8_t idx = 0;
    uint8_t c;

    while (1)
    {
        /*
          UART 데이터가 들어올 때까지 대기
          RXC0 비트가 1이면
          수신 데이터가 존재한다는 의미
         */
        while (!(UCSR0A & (1 << RXC0)));
        // 수신 데이터 읽기
        c = UDR0;
        /*
          Enter 입력 처리
          '\r', '\n'은 줄바꿈 문자이므로
          입력 종료로 판단한다.
         */
        if (c == '\r' || c == '\n')
        {
            // 아무것도 입력하지 않은 경우 무시
            if (idx == 0)
                continue;

            // 한 줄 입력 완료
            break;
        }

        /*
          Backspace 처리
          이미 입력한 문자가 있다면
          마지막 문자 제거
         */
        else if (c == 0x08 || c == 0x7F)
        {
            if (idx > 0)
            {
                idx--;

                // 터미널 화면에서도 문자 삭제
                UART0_Print("\b \b");
            }
        }

        /*
          일반 문자 입력
          버퍼 크기를 넘지 않는 경우만 저장한다.
         */
        else if (idx < (max_len - 1))
        {
            buf[idx++] = (char)c;
            // 입력한 내용을 화면에 표시
            UART0_Transmit(c);
        }
    }

    // 문자열 종료 문자 추가
    // C언어 문자열은 마지막에 '\0'이 필요하다.
    buf[idx] = '\0';

    // 입력 완료 후 줄바꿈
    UART0_Println("");
}

/*
  문자열을 정수로 변환하는 함수
  UART로 입력받은 값은 문자 형태이기 때문에
  바로 숫자로 사용할 수 없다.
  예)
  "120"
  문자 '1','2','0'
        ↓
  정수 120
  으로 변환한다.
  반환값
  1 : 정상 변환
  0 : 잘못된 입력
 */
static uint8_t Parse_Int(const char *str, int16_t *out_value)
{
    uint8_t i = 0;

    uint8_t negative = 0;

    int16_t value = 0;



    // 음수 입력 확인
    //
    // 예)
    // "-30"
    if (str[0] == '-')
    {
        negative = 1;
        i = 1;
    }



    // 부호만 입력한 경우 오류
    if (str[i] == '\0')
        return 0;



    // 문자열을 한 자리씩 확인하면서 숫자로 변환
    for (; str[i] != '\0'; i++)
    {
        /*
         * 숫자가 아닌 문자가 포함되면 오류 처리
         *
         * 예)
         * "90a"
         */
        if (str[i] < '0' ||
            str[i] > '9')
        {
            return 0;
        }



        /*
         * 기존 값 × 10 + 현재 숫자
         *
         * 예)
         *
         * "123"
         *
         * 0
         * 0×10+1 = 1
         * 1×10+2 = 12
         * 12×10+3 = 123
         */
        value =
            (int16_t)(value * 10 +
            (str[i] - '0'));
    }



    // 음수 처리
    if (negative)
        *out_value = (int16_t)(-value);
    else
        *out_value = value;



    return 1;
}


int main(void)
{
    char line_buf[LINE_BUF_SIZE];

    int16_t angle;



    /*
      UART 초기화
      PC 터미널과 통신하기 위해 사용한다.
      사용자는 터미널에서
      원하는 각도를 입력한다.
     */
    UART0_Init(9600);



    /*
      Timer1 PWM 초기화
      OC1C 핀에서 50Hz PWM 출력 생성
      서보 모터의 위치 제어에 사용한다.
     */
    Servo_PWM_Init();




    /*
      시스템 시작 시 서보를 원점으로 이동
      전원이 켜졌을 때 서보 위치가 어디인지 알 수 없기 때문에
      지정한 초기 위치로 이동시켜 기준점을 만든다.
     */
    Set_Servo_Angle(SERVO_ORIGIN_ANGLE);



    /*
      서보가 실제로 이동할 시간을 확보
      PWM 신호를 변경한다고 바로 위치가 바뀌는 것이 아니라
      모터가 물리적으로 움직이는 시간이 필요하다.
     */
    _delay_ms(500);

    UART0_Println("Servo Control Ready (0~180)");

    while (1)
    {
        //사용자에게 입력 요청
        UART0_Print("Angle> ");

        //UART로 한 줄 입력 받기
        UART0_ReadLine(line_buf, LINE_BUF_SIZE);

        /*
          문자열을 숫자로 변환
          변환 실패 시
          숫자가 아닌 문자가 포함된 것으로 판단한다.
         */
        if (!Parse_Int(line_buf, &angle))
        {
            UART0_Println(
                "WARNING: Invalid input (not a number). Command ignored."
            );


            // 잘못된 입력이므로 서보 이동하지 않음
            continue;
        }

        /*
          서보 동작 범위 검사
          일반적인 서보는 0~180도 범위에서 동작한다.
          범위를 벗어난 값을 넣으면
          기계적으로 걸리거나 손상될 수 있기 때문에
          실행 전에 검사한다.
         */
        if (angle < 0 || angle > 180)
        {
            UART0_Print(
                "WARNING: Angle out of range (0~180): "
            );
            UART0_PrintNumber(angle);
            UART0_Println(
                " -> Command ignored, motor not moved."
            );
            // 기존 위치 유지
            continue;
        }

        //정상적인 각도라면 서보 위치 변경
        Set_Servo_Angle((uint8_t)angle);

        //정상 동작 메시지 출력
        UART0_Print("OK: Servo -> ");
        UART0_PrintNumber(angle);
        UART0_Println(" deg");
    }
}