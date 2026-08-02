# ATmega128 과제 및 프로젝트

> **광운대학교 (로보학부)**  
> **작성자:** (이규환)  
> **제출일:** (2026.08.02)

---

# 1. 개요 (Overview)

본 과제는 ATmega128의 GPIO를 이용하여 소프트웨어 방식의 UART(Software UART)를 구현하고, PC로 문자열을 전송하는 프로그램을 작성하는 것을 목표로 한다.

ATmega128은 하드웨어 USART를 내장하고 있지만, 본 과제에서는 USART 모듈을 사용하지 않고 일반 GPIO 핀(PD3)을 직접 제어하여 UART 통신을 구현하였다.

UART 프로토콜의 Start Bit, Data Bit, Stop Bit를 소프트웨어적으로 생성하여 문자열을 전송하며, 일정 시간마다 "HelloWorld!" 문자열을 반복 출력하도록 구현하였다.

### 핵심 목표

* Software UART(비트뱅잉) 구현
* GPIO를 이용한 UART TX 신호 생성
* Start Bit, Data Bit, Stop Bit 직접 구현
* 문자열 송신 함수 구현
* 9600bps UART 통신 원리 이해

---

# 2. 개발 환경 (Environment)

| 항목 | 내용 |
| :--- | :--- |
| **MCU** | ATmega128A (16MHz External Crystal) |
| **IDE / Compiler** | Microchip Studio 7.0 / Microchip AVR GCC |
| **언어** | C Language |
| **통신 방식** | Software UART (Bit Banging) |
| **Baud Rate** | 9600bps |
| **주요 부품** | ATmega128 개발보드, USB-UART 모듈 |

---

# 3. 하드웨어 구성 및 핀 맵 (Hardware Structure)

## Pin Configuration

```text
              [PC]
                │
          USB to UART
                │
               RX
                │
            PD3 (TX)
                │
          [ATmega128]
```

### 주요 회로 특징

* PD3를 Software UART의 TX 핀으로 사용하였다.
* 일반 GPIO를 이용하여 UART 신호를 직접 생성하였다.
* UART 수신은 구현하지 않고 송신(TX)만 구현하였다.
* USB-UART 모듈을 이용하여 PC의 시리얼 모니터로 데이터를 확인하였다.

---

# 4. 프로젝트 구조 (Directory Structure)

```text
Day03_Task04/

├── main.c
│   ├── Software UART 초기화
│   ├── 문자 송신 함수
│   ├── 문자열 송신 함수
│   └── 메인 루프
└── README.md
```

---

# 5. 핵심 코드 및 레지스터 설정 (Key Implementation)

## Software UART 초기화

Software UART에서 사용할 TX 핀을 출력으로 설정하고 기본 상태인 High로 유지하였다.

```c
static void SoftUART_Init(void)
{
    DDRD |= (1 << UART_TX_PIN);
    PORTD |= (1 << UART_TX_PIN);
}
```

UART는 Idle 상태에서 항상 High를 유지하므로 초기 출력도 High로 설정하였다.

### 사용 레지스터

| 레지스터 | 기능 |
| :--- | :--- |
| DDRD | PD3를 출력으로 설정 |
| PORTD | TX 핀의 출력 상태 제어 |

---

## UART 비트 전송 시간 설정

UART 전송 속도는 9600bps로 설정하였다.

```c
#define BIT_DELAY_US 104
```

9600bps에서는 1비트 전송 시간이 다음과 같이 계산된다.

```text
1 / 9600

= 0.000104초

≈ 104us
```

따라서 각 비트마다 약 104μs 동안 상태를 유지하도록 구현하였다.

---

## 문자 송신 함수

Software UART 방식으로 문자 1개를 전송하였다.

```c
static void SoftUART_SendByte(uint8_t data)
```

UART 프레임은 다음과 같은 순서로 전송된다.

```text
Start Bit

↓

Data Bit (8bit)

↓

Stop Bit
```

### Start Bit

UART 통신의 시작을 알리기 위해 TX 핀을 Low로 출력하였다.

```c
PORTD &= ~(1 << UART_TX_PIN);
_delay_us(BIT_DELAY_US);
```

---

### Data Bit

8비트 데이터를 LSB(Least Significant Bit)부터 차례대로 전송하였다.

```c
for (uint8_t i = 0; i < 8; i++)
{
    if (data & (1 << i))
        PORTD |= (1 << UART_TX_PIN);
    else
        PORTD &= ~(1 << UART_TX_PIN);

    _delay_us(BIT_DELAY_US);
}
```

예를 들어 문자 'A'(0x41)를 전송하면 다음과 같이 LSB부터 전송된다.

```text
'A'

0x41

01000001

↓

1 → 0 → 0 → 0 → 0 → 0 → 1 → 0
```

---

### Stop Bit

데이터 전송이 끝나면 Stop Bit인 High를 출력하여 프레임 종료를 알렸다.

```c
PORTD |= (1 << UART_TX_PIN);
_delay_us(BIT_DELAY_US);
```

---

## 문자열 송신 함수

문자열의 끝(NULL 문자)이 나올 때까지 한 글자씩 반복하여 전송하였다.

```c
static void SoftUART_SendString(const char *str)
{
    while (*str)
    {
        SoftUART_SendByte((uint8_t)*str);
        str++;
    }
}
```

문자열 전송 과정은 다음과 같다.

```text
"H"

↓

"e"

↓

"l"

↓

"l"

↓

"o"

↓

...

↓

'\0'

↓

전송 종료
```

---

## 메인 루프

메인 루프에서는 "HelloWorld!" 문자열을 1초마다 반복 전송하였다.

```c
while (1)
{
    SoftUART_SendString("HelloWorld!\r\n");

    _delay_ms(1000);
}
```

"\r\n"을 함께 전송하여 시리얼 모니터에서 문자열이 줄바꿈되도록 하였다.

---

# 6. 동작 설명 및 결과 (Results)

### 동작 시나리오

1. 시스템 전원 인가 후 Software UART를 초기화한다.
2. PD3를 UART TX 출력 핀으로 설정한다.
3. TX 핀을 Idle 상태인 High로 유지한다.
4. "HelloWorld!" 문자열을 한 글자씩 UART 프레임으로 전송한다.
5. 문자열 끝에 CR(Carriage Return)과 LF(Line Feed)를 전송하여 줄바꿈을 수행한다.
6. 1초 동안 대기한다.
7. 위 과정을 반복 수행한다.

### UART 프레임 구조

```text
Idle

↓

Start Bit (Low)

↓

Data Bit 0

↓

Data Bit 1

↓

Data Bit 2

↓

Data Bit 3

↓

Data Bit 4

↓

Data Bit 5

↓

Data Bit 6

↓

Data Bit 7

↓

Stop Bit (High)

↓

Idle
```

### 실행 결과

| 동작 | 결과 |
| :--- | :--- |
| 프로그램 시작 | Software UART 초기화 |
| 문자열 전송 | "HelloWorld!" 출력 |
| 전송 완료 | 1초 대기 |
| 반복 실행 | 문자열 반복 출력 |

### 동작 사진 / 영상

| 정면 동작 모습 | 센서 측정 및 시리얼 출력 |
| :---: | :---: |
| ![Hardware Setup](https://drive.google.com/file/d/1Q4KotIACsG3GlAOkOwiuvbKuUvRQgI70/view?usp=sharing) | ![Serial Monitor](https://drive.google.com/file/d/1v7EeVqQh1L9nkFspxXUiWLG42oJIUN1S/view?usp=sharing) |

---

# 7. 사용한 함수 및 레지스터

## 주요 함수

| 함수 | 기능 |
| :--- | :--- |
| SoftUART_Init() | TX 핀 초기화 |
| SoftUART_SendByte() | 문자 1Byte 송신 |
| SoftUART_SendString() | 문자열 송신 |

## 사용 레지스터

| 레지스터 | 기능 |
| :--- | :--- |
| DDRD | PD3 출력 설정 |
| PORTD | TX 신호 출력 |

---

# 8. Software UART와 Hardware UART 비교

| 항목 | Software UART | Hardware UART |
| :--- | :--- | :--- |
| 구현 방식 | GPIO 직접 제어 | USART 하드웨어 사용 |
| CPU 사용률 | 높음 | 낮음 |
| 정확도 | Delay 함수에 의존 | 매우 높음 |
| 송수신 속도 | 제한적 | 고속 통신 가능 |
| 구현 난이도 | 높음 | 비교적 쉬움 |

---

# 9. AI 툴 활용 명시 (AI Tools Declaration)

본 과제 작성 및 구현 과정에서 활용한 AI 도구(Generative AI)의 사용 현황 및 목적은 다음과 같다.

| 도구명 | 활용 영역 | 세부 사용 목적 및 내용 |
| :--- | :--- | :--- |
| **ChatGPT** | 코드 디버깅 | Software UART 동작 원리 정리 |

### AI 활용 및 검증 원칙
1. **코드 검증:** AI가 생성한 레지스터 설정 및 함수 코드는 데이터시트(ATmega128 Datasheet)와 비교 검증한 후 실제 오실로스코프/시리얼 모니터링을 거쳐 직접 수정 및 테스트하였습니다.
2. **학습 주도성:** 코드의 핵심 제어 로직 설계는 직접 작성하였으며, AI는 보조 도구(디버깅, 문서화)로만 활용하였습니다.