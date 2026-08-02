# ATmega128 과제 및 프로젝트

> **광운대학교 (로보학부)**  
> **작성자:** (이규환)  
> **제출일:** (2026.08.02)

---

# 1. 개요 (Overview)

본 과제는 ATmega128의 USART0(UART) 통신 기능을 이용하여 PC와 시리얼 통신을 수행하고, 사용자가 입력한 문자에 따라 8개의 LED를 제어하는 시스템을 구현하는 것을 목표로 한다.

UART를 통해 수신한 문자에 따라 지정된 LED를 점등하며, LED 이동 및 초기화 기능을 함께 구현하였다. 또한 Push Button 입력을 이용하여 LED를 초기화하고 UART를 통해 현재 동작 상태를 출력하도록 설계하였다.

### 핵심 목표

* USART0(9600bps) 초기화 및 시리얼 통신 구현
* UART 송신 및 수신 함수 구현
* 문자 입력에 따른 LED 제어
* Active Low 방식 LED 출력
* Push Button을 이용한 LED 초기화
* 비트 시프트(Bit Shift)를 이용한 LED 이동 구현

---

# 2. 개발 환경 (Environment)

| 항목 | 내용 |
| :--- | :--- |
| **MCU** | ATmega128A (16MHz External Crystal) |
| **IDE / Compiler** | Microchip Studio 7.0 / Microchip AVR GCC |
| **언어** | C Language |
| **통신 방식** | USART0 (UART, 9600bps) |
| **주요 부품** | ATmega128 개발보드, 8Bit LED, Push Button |

---

# 3. 하드웨어 구성 및 핀 맵 (Hardware Structure)

## Pin Configuration

```text
               [PC]
                 │
           UART (9600bps)
                 │
        RXD0(PE0) / TXD0(PE1)
                 │
          [ATmega128]
      ┌──────────┴──────────┐
      │                     │
 PORTA(PA0~PA7)        PORTC(PC0)
      │                     │
 8Bit LED(Active Low)   Push Button
```

### 주요 회로 특징

* PORTA는 Active Low 방식의 8Bit LED에 연결하였다.
* PC0는 Push Button 입력으로 사용하였다.
* USART0를 이용하여 PC와 9600bps 속도로 통신하였다.

---

# 4. 프로젝트 구조 (Directory Structure)

```text
Day03_Task02/

├── main.c
│   ├── USART0 초기화
│   ├── UART 송신 함수
│   ├── UART 수신 함수
│   ├── 문자열 출력 함수
│   ├── LED 제어
│   └── Push Button 처리
└── README.md
```

---

# 5. 핵심 코드 및 레지스터 설정 (Key Implementation)

## USART0 초기화

USART0를 이용하여 PC와 9600bps 속도로 통신하도록 초기화하였다.

```c
UBRR0L = 103;
UBRR0H = 0;

UCSR0A = 0x00;
UCSR0B = 0x18;
UCSR0C = 0x06;
```

Baud Rate는 다음 식으로 계산된다.

```text
UBRR = (F_CPU / (16 × BaudRate)) - 1

= (16,000,000 / (16 × 9600)) - 1

≈ 103
```

### 사용 레지스터

| 레지스터 | 기능 |
| :--- | :--- |
| UBRR0H/L | Baud Rate 설정 |
| UCSR0A | USART 상태 설정 |
| UCSR0B | 송신 및 수신 활성화 |
| UCSR0C | 데이터 형식 설정 |

---

## GPIO 설정

LED 출력과 버튼 입력을 설정하였다.

```c
DDRA = 0xFF;

DDRC &= ~(1<<PC0);

DDRE = 0x02;
```

| 포트 | 기능 |
| :--- | :--- |
| PA0~PA7 | LED 출력 |
| PC0 | Push Button 입력 |
| PE1 | UART TX 출력 |

---

## UART 수신 함수

UART 수신 버퍼에 데이터가 들어올 때까지 대기한 후 데이터를 반환한다.

```c
unsigned char Uart_Getch(void)
{
    while(!(UCSR0A & (1 << RXC0)));
    return UDR0;
}
```

---

## UART 송신 함수

송신 버퍼가 비어있을 때 데이터를 전송한다.

```c
void Uart_Putch(unsigned char PutData)
{
    while(!(UCSR0A & (1 << UDRE0)));
    UDR0 = PutData;
}
```

---

## 문자열 출력 함수

문자열 끝(NULL 문자)까지 한 글자씩 전송한다.

```c
void Uart_Puts(char *str)
{
    while(*str)
    {
        Uart_Putch(*str++);
    }
}
```

---

## LED 제어

UART로 입력한 문자에 따라 LED를 점등하였다.

| 입력 문자 | LED 출력 |
| :---: | :--- |
| '0' | PA0 LED ON |
| '1' | PA1 LED ON |
| '2' | PA2 LED ON |
| '3' | PA3 LED ON |
| '4' | PA4 LED ON |
| '5' | PA5 LED ON |
| '6' | PA6 LED ON |
| '7' | PA7 LED ON |

LED는 Active Low 방식이므로 출력 시 비트를 반전하였다.

```c
PORTA = ~LED;
```

---

## LED 이동

문자 '8'과 '9'를 입력하면 비트 시프트를 이용하여 LED를 이동하였다.

왼쪽 이동

```c
LED = (LED >> 1) | (LED << 7);
```

오른쪽 이동

```c
LED = (LED << 1) | (LED >> 7);
```

순환(Rotate) 방식으로 이동하기 때문에 끝에 있는 LED가 반대편으로 이동한다.

예시

```text
00010000

↓

00001000

↓

00000100
```

---

## Push Button 처리

Push Button이 눌리면 모든 LED를 OFF하고 UART로 RESET 문자열을 출력하였다.

```c
static uint8_t old = 0;
```

이전 버튼 상태를 저장하여 버튼을 계속 누르고 있어도 RESET이 반복 출력되지 않도록 Edge Detection 방식을 적용하였다.

---

# 6. 동작 설명 및 결과 (Results)

### 동작 시나리오

1. 시스템 전원 인가 후 USART0를 초기화한다.
2. PORTA를 LED 출력으로 설정한다.
3. UART를 통해 문자 입력을 대기한다.
4. '0' ~ '7'을 입력하면 해당 LED를 점등한다.
5. '8'을 입력하면 LED가 왼쪽으로 순환 이동한다.
6. '9'를 입력하면 LED가 오른쪽으로 순환 이동한다.
7. Push Button을 누르면 모든 LED를 OFF하고 "RESET" 문자열을 출력한다.
8. 정의되지 않은 문자가 입력되면 "error" 문자열을 출력한다.

### 동작 결과

| 입력 | 동작 |
| :---: | :--- |
| 0~7 | 해당 LED 점등 |
| 8 | LED 왼쪽 이동 |
| 9 | LED 오른쪽 이동 |
| Button | LED 초기화 및 RESET 출력 |
| 기타 문자 | error 출력 |

### 동작 사진 / 영상

| 정면 동작 모습 | 센서 측정 및 시리얼 출력 |
| :---: | :---: |
| ![Hardware Setup](https://drive.google.com/file/d/1x4Zg0-RwJuiJP7hhsRspP2A45R3GCOy7/view?usp=sharing) | ![Serial Monitor](https://drive.google.com/file/d/1d8_tMo0oDrhe8a_1rNaSJwFA6xZpPuVA/view?usp=sharing) |


---

# 7. AI 툴 활용 명시 (AI Tools Declaration)

본 과제 작성 및 구현 과정에서 활용한 AI 도구(Generative AI)의 사용 현황 및 목적은 다음과 같다.

| 도구명 | 활용 영역 | 세부 내용 |
| :--- | :--- | :--- |
| ChatGPT | 코드 분석 및 디버깅| USART 레지스터 설명 및 보고서 작성 |

### AI 활용 및 검증 원칙

1. UART 레지스터 설정은 ATmega128 Datasheet를 참고하여 검증하였다.
2. 프로그램은 실제 개발보드에서 테스트하여 LED 동작 및 UART 통신을 확인하였다.
3. AI는 코드 분석,디버깅 보조 도구로만 활용하였다.