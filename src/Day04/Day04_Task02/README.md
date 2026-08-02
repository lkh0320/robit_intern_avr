# ATmega128 과제 및 프로젝트

> **광운대학교 (로보학부)**  
> **작성자:** (이규환)  
> **제출일:** (2026.08.02)

---

# 1. 개요 (Overview)

본 과제는 **ATmega128 마이크로컨트롤러를 이용하여 실시간 디지털 시계(Real Time Clock, RTC)를 구현하는 것**을 목표로 한다.

프로그램은 **I2C 통신을 이용한 16×2 LCD 출력**, **ADC를 이용한 날짜 및 시간 설정**, **Timer1 Compare Match 인터럽트를 이용한 시간 증가**, 그리고 **State Machine 기반의 동작 제어**를 하나의 시스템으로 통합하여 구현하였다.

시간 설정은 가변저항(ADC 입력)을 이용하여 연도, 월, 일, 시, 분, 초를 순서대로 선택하도록 설계하였으며, 두 개의 Push Button을 이용하여 설정 단계 이동과 시계 시작 기능을 구현하였다.

시계가 시작된 이후에는 Timer1 인터럽트가 10ms마다 발생하여 시간을 자동으로 증가시키며, LCD에는 현재 날짜와 시간이 실시간으로 출력된다.

또한 윤년 계산과 월별 날짜 계산 기능을 구현하여 실제 달력과 동일하게 날짜가 증가하도록 설계하였다.

이번 과제를 통해 Timer 인터럽트, ADC, I2C 통신, LCD 제어, 상태 머신(State Machine) 및 구조체를 활용한 임베디드 시스템 설계 방법을 학습하였다.

---

## 핵심 목표

* I2C(TWI)를 이용한 LCD 제어
* ADC를 이용한 날짜 및 시간 설정
* Timer1 Compare Match 인터럽트를 이용한 RTC 구현
* State Machine을 이용한 프로그램 흐름 제어
* 구조체를 이용한 날짜 및 시간 관리
* 윤년 및 월별 날짜 계산 기능 구현
* LCD를 이용한 실시간 날짜 및 시간 출력

---

# 2. 개발 환경 (Environment)

| 항목 | 내용 |
| :--- | :--- |
| **MCU** | ATmega128A (16MHz External Crystal) |
| **IDE / Compiler** | Microchip Studio 7.0 / Microchip AVR GCC |
| **언어** | C Language |
| **통신 방식** | I2C(TWI) |
| **LCD** | 16×2 Character LCD (PCF8574 I2C Module) |
| **입력 장치** | Push Button ×2, 가변저항(ADC) |
| **타이머** | Timer1 Compare Match Interrupt |
| **주요 모듈** | I2C Driver, LCD Driver, ADC Driver |

---

# 3. 프로젝트 목적

본 프로젝트는 여러 개의 주변장치를 동시에 사용하는 임베디드 시스템을 구현하는 것을 목적으로 한다.

프로그램은 다음과 같은 기능을 수행한다.

1. ADC를 이용하여 연도, 월, 일, 시, 분, 초를 설정한다.
2. Push Button을 이용하여 설정 단계를 변경한다.
3. 모든 설정이 완료되면 Timer1 인터럽트를 이용하여 시간이 자동으로 증가한다.
4. 증가하는 시간을 I2C 방식의 LCD에 실시간으로 출력한다.
5. 월별 날짜와 윤년을 계산하여 실제 달력과 동일하게 날짜를 변경한다.

---

# 4. 시스템 구성도 (System Architecture)

```text
                ┌────────────────────────────┐
                │        ATmega128           │
                │                            │
                │   Timer1 Interrupt         │
                │            │               │
                │            ▼               │
                │     RTC(Time Update)       │
                │            │               │
                │            ▼               │
                │      LCD Display           │
                │                            │
                │ ADC0(PF0)                  │
                │      ▲                     │
                │      │                     │
                │  Variable Resistor         │
                │                            │
                │ PC0 -------- SW1           │
                │ PC1 -------- SW2           │
                │                            │
                │ SDA -------- LCD Module    │
                │ SCL -------- LCD Module    │
                └────────────────────────────┘
```

---

# 5. 하드웨어 구성 및 핀 맵 (Hardware Structure)

## Pin Configuration

| 핀 | 연결 장치 | 기능 |
| :--- | :--- | :--- |
| PC0 | Push Button 1 | 날짜 및 시간 설정 단계 이동 |
| PC1 | Push Button 2 | 시계 시작 |
| PF0 (ADC0) | 가변저항 | 날짜 및 시간 설정 |
| PC1 (SDA) | I2C LCD | 데이터 전송 |
| PC0 (SCL) | I2C LCD | 클럭 신호 |
| VCC | 전원 | +5V |
| GND | 공통 접지 | GND |

---

## 시스템 입출력 구성

```text
             +-------------------+
             |     ATmega128     |
             |                   |
ADC0(PF0) <--| 가변저항 입력      |
             |                   |
PC0 <--------| SW1               |
             |                   |
PC1 <--------| SW2               |
             |                   |
SDA -------->| I2C LCD           |
SCL -------->| I2C LCD           |
             +-------------------+
```

---

## 주요 하드웨어 특징

### ATmega128

프로그램의 전체 동작을 제어하는 메인 컨트롤러이다.

모든 주변장치(I2C, ADC, Timer1)를 초기화하며 인터럽트를 이용하여 시간을 계산한다.

---

### 16×2 I2C LCD

LCD는 PCF8574 I/O Expander를 사용하는 I2C 방식의 LCD를 사용하였다.

기존 LCD는 많은 GPIO가 필요하지만 I2C 방식을 사용하면 두 개의 신호선(SDA, SCL)만으로 LCD를 제어할 수 있다.

LCD에는 다음 정보가 출력된다.

* 현재 날짜(YYMMDD)
* 현재 시간(HH:MM:SS.CS)

---

### Push Button

총 두 개의 Push Button을 사용하였다.

| 버튼 | 기능 |
| :--- | :--- |
| SW1 | 현재 설정 완료 후 다음 설정 단계 이동 |
| SW2 | 시간 설정 완료 후 시계 시작 |

버튼은 내부 Pull-up 저항을 사용하므로 평상시에는 HIGH 상태이며 버튼을 누르면 LOW가 입력된다.

또한 채터링(Chattering)을 방지하기 위해 소프트웨어 디바운싱을 적용하였다.

---

### 가변저항(ADC)

ADC0 채널에 연결된 가변저항을 이용하여 날짜와 시간을 설정하였다.

ADC 값은 0~1023 범위를 가지며 프로그램 내부에서 원하는 범위로 변환하여 사용하였다.

예를 들면

| 설정 항목 | 변환 범위 |
| :--- | :---: |
| 연도 | 0 ~ 99 |
| 월 | 1 ~ 12 |
| 일 | 1 ~ 31 |
| 시 | 0 ~ 23 |
| 분 | 0 ~ 59 |
| 초 | 0 ~ 59 |

---

# 6. 프로젝트 구조 (Directory Structure)

```text
Day04_Task02/

├── main.c
│   ├── RTC 구조체 정의
│   ├── State Machine 구현
│   ├── Timer1 인터럽트
│   ├── 버튼 처리
│   ├── 날짜 계산
│   ├── LCD 출력
│   └── 메인 제어 루프
│
├── i2c.c
│   ├── TWI(I2C) 초기화
│   ├── Start Condition
│   ├── Stop Condition
│   ├── 데이터 송수신
│   └── ACK 처리
│
├── i2c.h
│   ├── I2C 속도 설정
│   ├── TWI 상태 코드 정의
│   └── 함수 원형 선언
│
├── lcd_i2c.c
│   ├── LCD 초기화
│   ├── LCD 명령 전송
│   ├── 문자열 출력
│   ├── 커서 이동
│   ├── 백라이트 제어
│   └── I2C LCD 드라이버
│
├── lcd_i2c.h
│   ├── LCD 주소 정의
│   ├── LCD 제어 비트 정의
│   └── 함수 원형 선언
│
├── adc.c
│   ├── ADC 초기화
│   └── ADC 값 읽기
│
├── adc.h
│   └── ADC 함수 선언
│
└── README.md
```

---

# 7. 전체 프로그램 동작 순서

프로그램은 다음 순서로 동작한다.

```text
전원 인가

↓

I2C 초기화

↓

LCD 초기화

↓

ADC 초기화

↓

Timer1 초기화

↓

전역 인터럽트 허용

↓

연도 설정

↓

월 설정

↓

일 설정

↓

시 설정

↓

분 설정

↓

초 설정

↓

SW2 입력 대기

↓

시계 시작

↓

10ms Timer1 인터럽트 발생

↓

시간 증가

↓

LCD 실시간 출력
```

---


# 8. I2C(TWI) 통신 구현

본 프로젝트에서는 16×2 LCD를 제어하기 위해 ATmega128의 **TWI(Two Wire Interface)** 기능을 사용하였다.

ATmega128에서 TWI는 I2C와 동일한 통신 방식으로 동작하며, 두 개의 신호선(SDA, SCL)을 이용하여 데이터를 송수신한다.

I2C 방식은 여러 개의 장치를 두 개의 통신선만으로 연결할 수 있기 때문에 GPIO를 절약할 수 있으며, LCD와 같은 저속 주변장치를 제어하는 데 많이 사용된다.

---

## I2C 통신 구조

```text
        ATmega128
      ┌──────────────┐
      │              │
      │   SDA (PC1) ─────────────┐
      │                          │
      │   SCL (PC0) ─────────────┤
      │                          │
      └──────────────────────────┘
                      │
                PCF8574
                      │
                16×2 LCD
```

---

## I2C 초기화

I2C 초기화는 `I2C_Init()` 함수에서 수행하였다.

```c
void I2C_Init(void)
{
    TWSR = 0x00;

    TWBR = (uint8_t)(((F_CPU / I2C_BITRATE) - 16) / 2);

    TWCR = (1 << TWEN);
}
```

먼저 TWSR의 Prescaler를 1로 설정한 후 TWBR 값을 계산하여 SCL 속도를 설정하였다.

마지막으로 TWEN 비트를 활성화하여 TWI 모듈을 사용할 수 있도록 설정하였다.

---

## 사용 레지스터

| 레지스터 | 기능 |
| :--- | :--- |
| TWBR | I2C 통신 속도 설정 |
| TWSR | Prescaler 및 상태 코드 저장 |
| TWCR | TWI 제어 |
| TWDR | 송수신 데이터 저장 |

---

## SCL 속도 계산

본 프로젝트에서는 I2C 표준모드(Standard Mode)인 **100kHz**를 사용하였다.

```c
#define I2C_BITRATE 100000UL
```

TWBR은 다음 식으로 계산된다.

```text
SCL Frequency

=

F_CPU

────────────────────────────

16 + 2 × TWBR × Prescaler
```

본 프로젝트에서는

```text
F_CPU = 16MHz

Prescaler = 1

SCL = 100kHz
```

이므로 TWBR이 자동 계산된다.

---

## START Condition

I2C 통신을 시작하기 위해 START Condition을 생성하였다.

```c
TWCR = (1 << TWINT)
     | (1 << TWSTA)
     | (1 << TWEN);
```

START가 완료되면 TWINT 비트가 1이 될 때까지 대기한다.

```c
while (!(TWCR & (1 << TWINT)));
```

그 후 Slave 주소와 Read/Write 비트를 전송한다.

```c
TWDR = address_rw;
```

---

## STOP Condition

모든 데이터 전송이 끝나면 STOP 신호를 발생시켜 통신을 종료한다.

```c
TWCR = (1 << TWINT)
     | (1 << TWEN)
     | (1 << TWSTO);
```

STOP 비트가 클리어될 때까지 대기하여 통신 종료를 확인하였다.

---

## 데이터 송신

데이터는 TWDR 레지스터를 이용하여 전송하였다.

```c
TWDR = data;

TWCR = (1 << TWINT)
     | (1 << TWEN);
```

전송 완료 후 ACK를 확인하여 정상적으로 전송되었는지 검사하였다.

```c
(TWSR & 0xF8)
```

상태 코드가

```text
0x28
```

이면 Slave가 데이터를 정상적으로 수신한 것이다.

---

## 데이터 수신

Slave에서 데이터를 읽을 때는 ACK 또는 NACK 방식을 선택하여 사용하였다.

ACK 방식

```c
TWCR =
    (1 << TWINT)
  | (1 << TWEN)
  | (1 << TWEA);
```

NACK 방식

```c
TWCR =
    (1 << TWINT)
  | (1 << TWEN);
```

마지막 데이터를 읽을 때는 NACK를 사용하여 통신 종료를 알렸다.

---

# 9. LCD(I2C) 드라이버 구현

본 프로젝트에서는 **PCF8574 I/O Expander**가 장착된 16×2 LCD를 사용하였다.

일반 LCD는 6~10개의 GPIO가 필요하지만 I2C LCD는 두 개의 통신선(SDA, SCL)만으로 LCD를 제어할 수 있다.

---

## LCD 초기화

LCD 초기화는 `LCD_Init()` 함수에서 수행하였다.

```c
LCD_Init();
```

초기화 순서는 다음과 같다.

```text
I2C 초기화

↓

LCD 전원 안정화

↓

4Bit Mode 진입

↓

Display OFF

↓

Clear Display

↓

Entry Mode 설정

↓

Display ON
```

---

## LCD 4Bit Mode

LCD는 8Bit 데이터를 한 번에 전송하지 않고

상위 4Bit

↓

하위 4Bit

순서로 두 번 나누어 전송하였다.

```c
LCD_Send(value, mode);
```

예를 들어

```text
0x41
```

을 전송하면

```text
0100

↓

0001
```

순으로 전송된다.

---

## Enable Pulse

LCD는 Enable 신호가 High에서 Low로 변경되는 순간 데이터를 읽는다.

이를 위해 다음 함수를 구현하였다.

```c
LCD_PulseEnable(data);
```

동작 순서는 다음과 같다.

```text
EN = High

↓

1us 유지

↓

EN = Low

↓

50us 대기
```

---

## LCD 명령 전송

명령(Command)은 RS=0으로 전송하였다.

```c
LCD_Command(cmd);
```

대표적으로 사용한 명령은 다음과 같다.

| 명령 | 기능 |
| :---: | :--- |
| 0x01 | 화면 전체 삭제 |
| 0x02 | 커서 Home |
| 0x06 | 커서 자동 증가 |
| 0x0C | Display ON |
| 0x28 | 4Bit, 2Line 모드 |

---

## 문자 출력

문자 출력은 RS 비트를 1로 설정하여 수행하였다.

```c
LCD_Data(data);
```

문자열 출력은 다음 함수를 이용하였다.

```c
LCD_String(str);
```

문자열 끝(NULL 문자)이 나올 때까지 반복하여 LCD에 출력한다.

---

## 커서 이동

원하는 위치에 문자열을 출력하기 위해 커서를 이동하였다.

```c
LCD_SetCursor(row, col);
```

예를 들어

```c
LCD_SetCursor(1,5);
```

는

```text
2행

↓

6번째 칸
```

으로 커서를 이동한다.

---

## 문자열 출력

프로젝트에서는

```c
LCD_StringXY(row,col,str);
```

함수를 사용하여

커서 이동과 문자열 출력을 동시에 수행하였다.

예를 들면

```c
LCD_StringXY(0,0,"DATE");
```

는 첫 번째 줄 첫 번째 위치에 "DATE"를 출력한다.

---

## 백라이트 제어

LCD 백라이트도 소프트웨어적으로 제어하였다.

```c
LCD_Backlight(1);
```

↓

백라이트 ON

```c
LCD_Backlight(0);
```

↓

백라이트 OFF

---

# 10. ADC 구현

날짜와 시간을 설정하기 위해 ADC를 이용하여 가변저항 값을 읽었다.

ADC는 아날로그 전압을 10Bit 디지털 값으로 변환하는 기능이다.

변환된 값은

```text
0

↓

1023
```

범위를 가진다.

---

## ADC 초기화

ADC 초기화는 `ADC_Init()` 함수에서 수행하였다.

```c
ADMUX = (1 << REFS0);

ADCSRA =
    (1 << ADEN)
  | (1 << ADPS2)
  | (1 << ADPS1)
  | (1 << ADPS0);
```

---

## 사용 레지스터

| 레지스터 | 기능 |
| :--- | :--- |
| ADMUX | 기준 전압 및 채널 선택 |
| ADCSRA | ADC 활성화 및 분주비 설정 |
| ADC | 변환 결과 저장 |

---

## 기준 전압 설정

```c
ADMUX = (1 << REFS0);
```

AVCC(+5V)를 ADC 기준 전압으로 사용하였다.

---

## ADC 클럭 설정

ADC는 너무 빠른 클럭에서 정확도가 떨어지므로 분주비를 128로 설정하였다.

```text
16MHz

↓

128 분주

↓

125kHz
```

ADC 권장 동작 범위인 50~200kHz를 만족한다.

---

## ADC 변환

ADC 값을 읽는 과정은 다음과 같다.

```text
채널 선택

↓

ADSC = 1

↓

변환 시작

↓

변환 완료 대기

↓

ADC 값 읽기
```

코드에서는

```c
ADCSRA |= (1 << ADSC);

while (ADCSRA & (1 << ADSC));

return ADC;
```

순서로 구현하였다.

---

## 채널 선택

원하는 ADC 채널은 다음 코드로 선택하였다.

```c
ADMUX =
(ADMUX & 0xE0)

|

(channel & 0x1F);
```

REFS 비트는 유지하면서 채널만 변경하도록 구현하였다.

---

## ADC 활용

프로젝트에서는 ADC 값을 이용하여

* 연도
* 월
* 일
* 시
* 분
* 초

값을 선택하였다.

가변저항을 회전시키면 ADC 값이 변하고, 프로그램 내부에서 원하는 범위로 변환하여 날짜와 시간 설정에 사용하였다.

---

# 11. Timer1 인터럽트 및 RTC 구현

본 프로젝트에서는 **ATmega128의 Timer1 Compare Match 인터럽트**를 이용하여 실시간 시계(RTC)를 구현하였다.

시간 계산은 메인 루프에서 수행하지 않고 인터럽트에서 처리하였다. 따라서 메인 프로그램이 다른 작업을 수행하는 동안에도 일정한 주기로 시간이 증가하도록 설계하였다.

RTC는 다음과 같은 단위로 시간을 관리한다.

```text
년(Year)

↓

월(Month)

↓

일(Day)

↓

시(Hour)

↓

분(Minute)

↓

초(Second)

↓

1/100초(Centi Second)
```

시간 정보는 하나의 구조체(RTC_Time)에 저장하여 관리하였다.

```c
typedef struct
{
    uint8_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t min;
    uint8_t sec;
    uint8_t csec;
} RTC_Time;
```

구조체를 사용함으로써 날짜와 시간을 하나의 변수처럼 관리할 수 있으며, 프로그램의 가독성과 유지보수성이 향상된다.

---

# 12. Timer1 초기화

Timer1은 Compare Match(CTC) 모드로 설정하여 일정한 주기마다 인터럽트가 발생하도록 구성하였다.

초기화 과정에서는 다음과 같은 작업을 수행하였다.

* Timer1 동작 모드 설정
* 비교값(OCR1A) 설정
* 인터럽트 허용
* 분주비(Prescaler) 설정

---

## Timer1 동작 원리

```text
16MHz Clock

↓

Prescaler

↓

Timer1 Counter 증가

↓

OCR1A 도달

↓

Compare Match 발생

↓

인터럽트 실행

↓

Counter 초기화

↓

반복
```

CTC(Clear Timer on Compare Match) 모드를 사용하면 OCR1A 값에 도달할 때마다 자동으로 카운터가 초기화되어 일정한 주기의 인터럽트를 생성할 수 있다.

---

## 사용 레지스터

| 레지스터 | 기능 |
| :--- | :--- |
| TCCR1A | Timer1 동작 모드 설정 |
| TCCR1B | 분주비 및 CTC 모드 설정 |
| OCR1A | 비교값 저장 |
| TIMSK | Compare Match 인터럽트 허용 |
| TCNT1 | Timer Counter |

---

# 13. Timer1 Compare Match 인터럽트

인터럽트는 Timer1 Compare Match가 발생할 때마다 실행된다.

인터럽트 내부에서는 시간을 증가시키고 LCD를 다시 출력해야 함을 메인 프로그램에 알려준다.

프로그램에서는 인터럽트 내부에서 LCD를 직접 제어하지 않고

```text
display_update_flag
```

변수만 변경하였다.

이 방식은 인터럽트 실행 시간을 최소화하여 시스템의 안정성을 높여준다.

---

## 인터럽트 동작 순서

```text
Timer1 Compare Match

↓

ISR 실행

↓

timer_running 확인

↓

1/100초 증가

↓

초 증가

↓

분 증가

↓

시 증가

↓

날짜 증가

↓

display_update_flag = 1

↓

ISR 종료
```

---

# 14. 시간 증가 알고리즘

시간은 가장 작은 단위인 1/100초부터 증가하도록 구현하였다.

```text
Centi Second

↓

Second

↓

Minute

↓

Hour

↓

Day

↓

Month

↓

Year
```

예를 들어

```text
23 : 59 : 59 : 99
```

다음 인터럽트가 발생하면

```text
00 : 00 : 00 : 00
```

으로 변경되며 날짜도 하루 증가한다.

---

## 초 증가

```text
99 Centi Second

↓

100

↓

0

↓

Second++
```

---

## 분 증가

```text
59 Second

↓

60

↓

0

↓

Minute++
```

---

## 시 증가

```text
59 Minute

↓

60

↓

0

↓

Hour++
```

---

## 날짜 증가

```text
23 Hour

↓

24

↓

0

↓

Day++
```

이후 월별 마지막 날짜를 검사하여 월과 연도를 변경한다.

---

# 15. 윤년 계산

2월의 마지막 날짜를 결정하기 위해 윤년 계산 함수를 구현하였다.

```c
static uint8_t Is_Leap_Year(uint16_t year)
```

윤년이면

```text
2월 = 29일
```

평년이면

```text
2월 = 28일
```

을 사용한다.

이를 통해 실제 달력과 동일한 날짜 계산이 가능하도록 구현하였다.

---

# 16. 월별 날짜 계산

프로그램은 각 월의 마지막 날짜를 계산하여 자동으로 다음 달로 넘어가도록 설계하였다.

| 월 | 마지막 날짜 |
| :---: | :---: |
| 1 | 31 |
| 2 | 28 또는 29 |
| 3 | 31 |
| 4 | 30 |
| 5 | 31 |
| 6 | 30 |
| 7 | 31 |
| 8 | 31 |
| 9 | 30 |
| 10 | 31 |
| 11 | 30 |
| 12 | 31 |

예를 들어

```text
2026-01-31

↓

다음 날

↓

2026-02-01
```

과 같이 자동으로 변경된다.

---

# 17. State Machine 구현

프로그램은 State Machine을 이용하여 현재 동작 단계를 관리하였다.

상태값은 다음과 같이 정의하였다.

```c
typedef enum
{
    SET_YEAR,
    SET_MONTH,
    SET_DAY,
    SET_HOUR,
    SET_MIN,
    SET_SEC,
    WAIT_RUN,
    RUNNING
} SystemState;
```

각 상태는 하나의 기능만 수행하도록 설계하여 프로그램 구조를 단순하게 만들었다.

---

## 상태 전이 과정

```text
SET_YEAR

↓

SET_MONTH

↓

SET_DAY

↓

SET_HOUR

↓

SET_MIN

↓

SET_SEC

↓

WAIT_RUN

↓

RUNNING
```

---

## SET_YEAR

가변저항(ADC)을 이용하여 연도를 설정한다.

SW1 버튼을 누르면 다음 단계로 이동한다.

---

## SET_MONTH

월을 설정한다.

ADC 값을 1~12 범위로 변환하여 사용한다.

---

## SET_DAY

일을 설정한다.

설정 가능한 최대 날짜는 현재 월에 따라 달라진다.

---

## SET_HOUR

시(0~23)를 설정한다.

---

## SET_MIN

분(0~59)를 설정한다.

---

## SET_SEC

초(0~59)를 설정한다.

---

## WAIT_RUN

모든 설정이 끝난 후 SW2 버튼 입력을 기다리는 상태이다.

SW2를 누르면

```text
timer_running = 1
```

이 되어 Timer1 인터럽트에서 시간이 증가하기 시작한다.

---

## RUNNING

실제 시계가 동작하는 상태이다.

인터럽트에서 시간이 증가하며 LCD에는 현재 시간이 실시간으로 출력된다.

---

# 18. 버튼 처리

프로그램에서는 두 개의 Push Button을 사용하였다.

| 버튼 | 기능 |
| :--- | :--- |
| SW1 | 다음 설정 단계 이동 |
| SW2 | 시계 시작 |

버튼은 Edge Detection 방식을 사용하여 한 번만 입력되도록 구현하였다.

```text
버튼 누름

↓

이전 상태 확인

↓

처음 눌린 경우만 처리

↓

버튼에서 손을 떼면 다시 입력 가능
```

이를 통해 버튼을 계속 누르고 있어도 여러 번 입력되는 현상을 방지하였다.

---

# 19. LCD 갱신 방식

인터럽트 내부에서는 LCD를 직접 제어하지 않았다.

대신

```c
display_update_flag
```

를 1로 변경하고,

메인 루프에서

```text
display_update_flag 확인

↓

LCD 출력

↓

display_update_flag = 0
```

순서로 화면을 갱신하였다.

이 방식은 인터럽트 실행 시간을 최소화하여 시스템의 안정성을 높이고 LCD 통신으로 인해 인터럽트가 지연되는 문제를 방지할 수 있다.

---

# 20. 프로그램 전체 동작 과정

프로그램은 시스템 초기화 이후 사용자가 날짜와 시간을 설정하고, 설정이 완료되면 Timer1 인터럽트를 이용하여 실시간 시계를 동작시키도록 설계하였다.

전체 동작 순서는 다음과 같다.

```text
전원 인가

↓

시스템 초기화

↓

I2C(TWI) 초기화

↓

LCD 초기화

↓

ADC 초기화

↓

Timer1 초기화

↓

전역 인터럽트 허용

↓

연도 설정

↓

월 설정

↓

일 설정

↓

시 설정

↓

분 설정

↓

초 설정

↓

SW2 입력 대기

↓

Timer 시작

↓

Timer1 Compare Match 인터럽트

↓

시간 증가

↓

LCD 갱신

↓

반복 수행
```

---

# 21. 함수별 역할

프로그램은 기능별로 모듈을 분리하여 작성하였다.

## main.c

메인 프로그램으로 시스템 초기화와 상태(State)를 관리한다.

주요 기능

* 시스템 초기화
* State Machine 실행
* 버튼 입력 처리
* LCD 출력
* Timer 시작 및 종료
* 인터럽트 활성화

---

## i2c.c

ATmega128의 TWI 모듈을 이용하여 I2C 통신을 수행한다.

주요 기능

* I2C 초기화
* START Condition 생성
* STOP Condition 생성
* Slave Address 전송
* 데이터 송수신
* ACK/NACK 처리

---

## lcd_i2c.c

I2C LCD를 제어하는 드라이버이다.

주요 기능

* LCD 초기화
* 명령(Command) 전송
* 문자(Data) 전송
* 문자열 출력
* 커서 이동
* 화면 삭제
* 백라이트 제어

---

## adc.c

ADC를 이용하여 가변저항 값을 읽는다.

주요 기능

* ADC 초기화
* ADC 변환 시작
* ADC 값 반환

---

# 22. 프로그램의 특징

본 프로젝트는 여러 개의 ATmega128 주변장치를 동시에 사용하는 통합 프로젝트이다.

구현한 주요 기능은 다음과 같다.

### I2C(TWI)

* LCD 제어
* 두 개의 통신선만 사용
* PCF8574 기반 LCD 제어

---

### ADC

* 가변저항 입력
* 날짜 설정
* 시간 설정

---

### Timer1

* Compare Match Interrupt 사용
* 일정한 시간 간격 생성
* RTC 구현

---

### 인터럽트

* 시간 계산 수행
* 메인 프로그램과 독립적으로 동작

---

### State Machine

* 프로그램 흐름 관리
* 단계별 설정 구현
* 유지보수성 향상

---

### RTC

* 실시간 시간 증가
* 날짜 자동 변경
* 윤년 계산
* 월별 날짜 계산

---

# 23. 실행 결과

프로그램을 실행하면 LCD에는 날짜와 시간이 실시간으로 출력된다.

예시 화면은 다음과 같다.

```text
2026-08-02

13:45:18.25
```

가변저항을 회전하면 현재 설정 중인 항목의 값이 변경되며,

SW1 버튼을 누르면 다음 설정 단계로 이동한다.

모든 설정이 완료된 후 SW2 버튼을 누르면 Timer1 인터럽트가 시작되고 시간이 자동으로 증가한다.

또한 월의 마지막 날짜가 되면 다음 달로 변경되며,

2월은 윤년 여부에 따라 28일 또는 29일까지 자동으로 계산된다.

프로그램을 장시간 동작시켜도 Timer1 인터럽트를 이용하여 일정한 주기로 시간이 증가하는 것을 확인하였다.

---

# 24. 프로젝트 수행 결과

| 기능 | 구현 여부 |
| :--- | :---: |
| I2C(TWI) 통신 | ○ |
| LCD 출력 | ○ |
| ADC 입력 | ○ |
| Timer1 Compare Match | ○ |
| 인터럽트 처리 | ○ |
| 날짜 계산 | ○ |
| 윤년 계산 | ○ |
| RTC 구현 | ○ |
| State Machine | ○ |
| 버튼 입력 처리 | ○ |

---

# 25. 프로젝트를 수행하며 배운 점

이번 프로젝트를 수행하면서 단순히 주변장치를 사용하는 방법뿐만 아니라, 여러 개의 하드웨어 기능을 하나의 프로그램으로 통합하여 제어하는 방법을 학습할 수 있었다.

특히 Timer1 인터럽트를 이용하여 일정한 주기로 시간을 계산하는 방법과, 인터럽트와 메인 루프의 역할을 분리하여 프로그램을 구성하는 방법을 이해할 수 있었다.

또한 ADC를 이용한 아날로그 입력 처리, I2C 기반 LCD 제어, State Machine을 활용한 프로그램 흐름 제어 등을 구현하면서 임베디드 시스템의 기본적인 설계 방법을 익힐 수 있었다.

윤년 계산과 월별 날짜 계산 기능을 추가하여 실제 달력과 동일한 동작을 구현함으로써 단순한 시간 출력이 아닌 실시간 시계 시스템을 완성할 수 있었다.

---

# 26. 개선 사항

현재 프로그램은 기본적인 RTC 기능을 구현하였지만, 다음과 같은 기능을 추가하면 더욱 완성도 높은 시스템을 구현할 수 있다.

* 외부 RTC 모듈(DS1307, DS3231) 연동
* EEPROM을 이용한 날짜 및 시간 저장
* 알람(Alarm) 기능 추가
* 요일(Week) 계산 기능 구현
* 12시간 / 24시간 표시 기능
* 버튼 디바운싱 알고리즘 개선
* LCD 메뉴 인터페이스 추가

---

# 27. 결론

본 프로젝트에서는 **ATmega128의 다양한 주변장치(Timer1, ADC, I2C, GPIO, Interrupt)**를 활용하여 실시간 디지털 시계(RTC)를 구현하였다.

I2C(TWI)를 이용하여 16×2 LCD에 날짜와 시간을 출력하였으며, ADC를 이용한 사용자 입력을 통해 연도, 월, 일, 시, 분, 초를 설정할 수 있도록 구현하였다.

또한 Timer1 Compare Match 인터럽트를 이용하여 일정한 시간 간격으로 시간을 증가시키고, 윤년 및 월별 날짜 계산 기능을 적용하여 실제 달력과 동일한 방식으로 날짜가 변경되도록 구현하였다.

State Machine 구조를 적용하여 날짜와 시간 설정 과정을 단계별로 관리함으로써 프로그램의 가독성과 유지보수성을 향상시켰으며, 인터럽트와 메인 루프를 분리하여 안정적인 동작을 구현할 수 있었다.

이번 과제를 통해 ATmega128의 주요 주변장치 사용법뿐만 아니라, 여러 기능을 통합한 임베디드 시스템 설계 방법과 실시간 제어 프로그램의 구현 과정을 이해할 수 있었다.

---

# 28. 동작 사진 / 영상

| RTC 동작 화면 | 날짜 및 시간 설정 화면 |
| :---: | :---: |
| ![RTC Display](https://drive.google.com/file/d/1hlPAsxZlM2BseFCw6dvEE6K9wW755MN1/view?usp=sharing) | ![Setting Mode](https://drive.google.com/file/d/1HqN-2lUJL4Ro3TajFslMKZ2yfFk-sAd6/view?usp=sharing) |

---

# 29. AI 툴 활용 명시 (AI Tools Declaration)

본 과제 작성 및 구현 과정에서 활용한 AI 도구(Generative AI)의 사용 현황 및 목적은 다음과 같다.

| 도구명 (Tool) | 활용 영역 | 세부 사용 목적 및 내용 |
| :--- | :--- | :--- |
| **Claude** | 디버깅 | - State Machine 구조 검토<br>- 함수별 역할 및 프로그램 흐름 분석 |
| **Gemini** | 디버깅 | - RTC 구현 방식 및 윤년 계산 원리 참고 |

---

### AI 활용 및 검증 원칙
1. **코드 검증:** AI가 생성한 레지스터 설정 및 함수 코드는 데이터시트(ATmega128 Datasheet)와 비교 검증한 후 실제 오실로스코프/시리얼 모니터링을 거쳐 직접 수정 및 테스트하였습니다.
2. **학습 주도성:** 코드의 핵심 제어 로직 설계는 직접 작성하였으며, AI는 보조 도구(디버깅, 문서화)로만 활용하였습니다.
