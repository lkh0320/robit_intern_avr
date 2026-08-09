# ATmega128 과제 및 프로젝트

> **광운대학교 (로보학부)**  
> **작성자:** (이규환)  
> **제출일:** (2026.00.00)

---

## 1. 개요 (Overview)

본 과제는 ATmega128 마이크로컨트롤러의 ADC와 UART 통신 기능을 이용하여 라인 트레이서에 연결된 6개의 IR 센서 값을 읽고, 측정한 데이터를 시리얼 통신으로 확인하는 것을 목표로 함.

IR 센서는 주변의 반사 정도에 따라 아날로그 전압을 출력하며, ATmega128은 ADC 기능을 이용하여 이 아날로그 값을 0~1023 범위의 디지털 값으로 변환함. 변환된 센서 값은 UART0을 통해 PC의 시리얼 모니터로 전송하여 각 센서의 현재 상태를 확인할 수 있도록 구현함.

이번 과제에서는 센서가 ADC0부터 ADC5까지 순서대로 연결된 것이 아니라 ADC2, ADC4, ADC6, ADC7, ADC5, ADC3에 각각 연결되어 있기 때문에, 센서 번호와 실제 ADC 채널 번호를 배열로 저장하여 관리함. 이를 통해 여러 개의 센서 값을 반복문으로 읽고 출력하는 방법을 학습함.

### 핵심 목표

- ATmega128의 ADC 기능을 이용하여 IR 센서의 아날로그 값 읽기
- ADC 기준전압과 분주비 설정 방법 이해
- UART0을 이용한 PC와의 시리얼 통신 구현
- 9600bps 통신 속도 설정 방법 이해
- 여러 개의 센서를 배열과 반복문으로 관리
- `sprintf()` 함수를 이용하여 센서 값을 문자열로 변환
- 측정한 센서 데이터를 UART를 통해 시리얼 모니터에 출력

---

## 2. 개발 환경 (Environment)

| 항목 | 내용 |
| :--- | :--- |
| **MCU** | ATmega128A (16MHz External Crystal) |
| **IDE / Compiler** | Microchip Studio 7.0 / Microchip AVR GCC |
| **Flasher Tool** | USBISP / STK500 |
| **언어** | C Language |
| **주요 부품** | ATmega128 개발보드, 라인 트레이서용 IR 센서 6개, USB to Serial 또는 UART 통신 모듈 |

---

## 3. 하드웨어 구성 및 핀 맵 (Hardware Structure)

### Pin Configuration

```text
[ATmega128]                    [Target Component]

PF2 (ADC2)              <-----   IR Sensor 1
PF4 (ADC4)              <-----   IR Sensor 2
PF6 (ADC6)              <-----   IR Sensor 3
PF7 (ADC7)              <-----   IR Sensor 4
PF5 (ADC5)              <-----   IR Sensor 5
PF3 (ADC3)              <-----   IR Sensor 6

PE1 (TXD0)              ----->   PC / USB to Serial Module

AVCC                     ----->   ADC 기준전압
GND                      ------   공통 접지
```

### 센서 채널 구성

프로그램에서는 센서의 번호와 실제 ADC 채널 번호를 다음과 같이 배열로 저장함.

```c
uint8_t sensor_pin[6] = {2, 4, 6, 7, 5, 3};
```

각 배열의 위치와 ADC 채널은 다음과 같음.

| 센서 번호 | ADC 채널 | ATmega128 핀 |
| :---: | :---: | :---: |
| Sensor 1 | ADC2 | PF2 |
| Sensor 2 | ADC4 | PF4 |
| Sensor 3 | ADC6 | PF6 |
| Sensor 4 | ADC7 | PF7 |
| Sensor 5 | ADC5 | PF5 |
| Sensor 6 | ADC3 | PF3 |

### 주요 회로 특징

- **IR 센서:** 주변 물체 또는 바닥의 반사 정도에 따라 아날로그 전압 출력
- **ADC 입력:** ATmega128의 PORTF 핀을 ADC 입력으로 사용
- **UART 출력:** 측정한 센서 값을 UART0을 통해 PC로 전송
- **통신 속도:** 9600bps
- **ADC 기준전압:** AVCC 사용
- **ADC 분주비:** 128
- **센서 개수:** 총 6개

---

## 4. 프로젝트 구조 (Directory Structure)

> 구현부(.c), 선언부(.h)만 구조에 표기함.

```text
├── Day06_Task02/
│   ├── Day06_Task02.c # UART 초기화, ADC 초기화 및 IR 센서 값 측정
│   └── README.md      # 과제 동작 및 구현 내용 정리
├── docs/
│   └── sensor_connection.pdf # IR 센서 및 ATmega128 연결 회로도
└── README.md
```

이번 과제는 하나의 C 소스 파일에서 구현함. UART와 ADC 기능을 각각 함수로 분리하여 초기화 및 데이터 처리 과정을 이해하기 쉽게 구성함.

주요 함수는 다음과 같음.

- `UART0_init()` : UART0 통신 초기화
- `UART0_transmit()` : 문자 1개 전송
- `UART0_print()` : 문자열 전송
- `ADC_init()` : ADC 초기화
- `ADC_read()` : 지정한 ADC 채널의 값 읽기

---

## 5. 핵심 코드 및 레지스터 설정 (Key Implementation)

### UART0 통신 속도 설정

```c
#define BAUD 9600
#define UBRR_VALUE (F_CPU / 16 / BAUD - 1)
```

UART 통신 속도를 9600bps로 설정함.

ATmega128의 시스템 클럭이 16MHz이고 일반 비동기 통신 모드를 사용하는 경우 UBRR 값은 다음과 같이 계산됨.

```text
UBRR = F_CPU / (16 × BAUD) - 1

     = 16,000,000 / (16 × 9,600) - 1

     ≈ 103
```

계산된 UBRR 값은 16비트 레지스터이므로 상위 8비트와 하위 8비트로 나누어 설정함.

```c
UBRR0H = (unsigned char)(UBRR_VALUE >> 8);
UBRR0L = (unsigned char)UBRR_VALUE;
```

### UART0 초기화

```c
void UART0_init(void)
{
    UBRR0H = (unsigned char)(UBRR_VALUE >> 8);
    UBRR0L = (unsigned char)UBRR_VALUE;

    UCSR0B = (1 << TXEN0);

    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}
```

`UCSR0B` 레지스터의 `TXEN0` 비트를 1로 설정하여 UART0의 송신 기능을 활성화함.

```c
UCSR0B = (1 << TXEN0);
```

이번 프로그램에서는 센서 값을 PC로 보내는 기능만 필요하기 때문에 송신 기능만 사용함.

`UCSR0C` 레지스터에서는 UART의 데이터 형식을 설정함.

```c
UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
```

`UCSZ01`과 `UCSZ00`을 모두 1로 설정하여 8비트 데이터 형식을 사용함.

---

### UART 문자 전송

```c
void UART0_transmit(unsigned char data)
{
    while (!(UCSR0A & (1 << UDRE0)));

    UDR0 = data;
}
```

UART로 데이터를 전송하기 전에 `UDRE0` 비트를 확인함.

`UDRE0`은 UART 송신 데이터 레지스터가 비어 있는지를 나타내는 비트임.

```c
while (!(UCSR0A & (1 << UDRE0)));
```

송신 데이터 레지스터가 비어 있지 않으면 이전 데이터가 아직 전송 중일 수 있으므로 기다림.

송신 데이터 레지스터가 비면 전송할 데이터를 `UDR0`에 저장함.

```c
UDR0 = data;
```

데이터를 `UDR0`에 저장하면 UART 하드웨어가 자동으로 데이터를 전송함.

---

### UART 문자열 전송

```c
void UART0_print(char *str)
{
    while (*str)
    {
        UART0_transmit(*str++);
    }
}
```

문자열은 여러 개의 문자로 이루어져 있기 때문에 문자열의 첫 번째 문자부터 하나씩 UART로 전송함.

`*str`이 0이 되면 문자열의 끝인 NULL 문자에 도달한 것이므로 반복문을 종료함.

이 함수 덕분에 다음과 같이 문자열을 한 번에 전송할 수 있음.

```c
UART0_print("Line Tracer IR Sensor Start\r\n");
```

`\r\n`은 시리얼 모니터에서 다음 줄로 이동하기 위한 문자임.

---

### ADC 초기화

```c
void ADC_init(void)
{
    ADMUX = (1 << REFS0);

    ADCSRA = (1 << ADEN) |
             (1 << ADPS2) |
             (1 << ADPS1) |
             (1 << ADPS0);
}
```

ADC의 기준전압은 `ADMUX` 레지스터의 `REFS0` 비트를 이용하여 AVCC로 설정함.

```c
ADMUX = (1 << REFS0);
```

ATmega128 보드에서 AVCC가 5V인 경우 ADC는 약 0V부터 5V 범위의 아날로그 전압을 디지털 값으로 변환할 수 있음.

ADC는 10비트 분해능을 사용하므로 변환된 값의 범위는 다음과 같음.

```text
0V에 가까운 경우    → ADC 값 약 0
5V에 가까운 경우    → ADC 값 약 1023
```

`ADCSRA` 레지스터에서는 ADC 기능을 활성화하고 ADC 클럭의 분주비를 설정함.

```c
ADCSRA = (1 << ADEN) |
         (1 << ADPS2) |
         (1 << ADPS1) |
         (1 << ADPS0);
```

`ADEN` 비트를 1로 설정하여 ADC 기능을 활성화함.

`ADPS2`, `ADPS1`, `ADPS0`을 모두 1로 설정하여 ADC 클럭의 분주비를 128로 설정함.

```text
ADC Clock = CPU Clock / 128

          = 16MHz / 128

          = 125kHz
```

따라서 ADC가 동작하기 위한 클럭으로 약 125kHz를 사용하도록 설정함.

---

### ADC 채널 선택 및 데이터 읽기

```c
uint16_t ADC_read(uint8_t channel)
{
    ADMUX = (ADMUX & 0xE0) | (channel & 0x0F);

    ADCSRA |= (1 << ADSC);

    while (ADCSRA & (1 << ADSC));

    return ADC;
}
```

함수를 호출할 때 전달받은 `channel` 값을 이용하여 ADC 채널을 선택함.

```c
ADMUX = (ADMUX & 0xE0) | (channel & 0x0F);
```

기존 ADMUX 레지스터의 기준전압 설정 부분은 유지하면서 하위 비트에 ADC 채널 번호를 설정함.

그 다음 `ADSC` 비트를 1로 설정하여 ADC 변환을 시작함.

```c
ADCSRA |= (1 << ADSC);
```

ADC 변환이 진행되는 동안에는 `ADSC` 비트가 1로 유지됨.

```c
while (ADCSRA & (1 << ADSC));
```

따라서 변환이 완료될 때까지 기다린 후 ADC 값을 반환함.

```c
return ADC;
```

반환되는 ADC 값은 10비트 데이터이므로 `uint16_t` 자료형을 사용함.

---

### 6개의 IR 센서 데이터 읽기

```c
uint8_t sensor_pin[6] = {2, 4, 6, 7, 5, 3};
uint16_t sensor_value[6];
```

`sensor_pin` 배열에는 각 센서가 연결된 ADC 채널 번호를 저장함.

`sensor_value` 배열에는 ADC를 통해 읽은 실제 센서 값을 저장함.

반복문을 이용하여 6개의 센서 값을 순서대로 읽음.

```c
for (uint8_t i = 0; i < 6; i++)
{
    sensor_value[i] = ADC_read(sensor_pin[i]);
}
```

예를 들어 `i = 0`일 때는 다음과 같이 동작함.

```text
sensor_pin[0] = 2

ADC_read(2) 실행

PF2에 연결된 ADC2 값을 읽음

sensor_value[0]에 저장
```

이 방법을 사용하면 센서의 개수가 여러 개여도 같은 코드를 반복해서 작성하지 않고 배열과 반복문으로 처리할 수 있음.

---

### 센서 값 UART 출력

```c
for (uint8_t i = 0; i < 6; i++)
{
    sprintf(buf, "S%d:%4u ", i + 1, sensor_value[i]);

    UART0_print(buf);
}
```

`sprintf()` 함수를 사용하여 센서 번호와 센서 값을 문자열로 변환함.

예를 들어 Sensor 1의 값이 523인 경우 다음과 같이 출력될 수 있음.

```text
S1: 523
```

6개의 센서 값을 모두 출력하면 시리얼 모니터에서 다음과 같이 확인할 수 있음.

```text
S1: 523 S2: 487 S3: 615 S4: 702 S5: 456 S6: 389
```

센서 값을 모두 출력한 후 줄을 바꾸기 위해 다음 코드를 사용함.

```c
UART0_print("\r\n");
```

그 후 200ms 동안 대기함.

```c
_delay_ms(200);
```

따라서 약 0.2초 간격으로 6개의 센서 값이 반복적으로 시리얼 모니터에 출력됨.

---

## 6. 동작 설명 및 결과 (Results)

### 동작 시나리오

1. 시스템 전원이 인가되면 ATmega128이 동작을 시작함
2. `UART0_init()` 함수를 실행하여 UART0 송신 기능을 초기화함
3. 통신 속도를 9600bps로 설정함
4. `ADC_init()` 함수를 실행하여 ADC 기능을 활성화함
5. ADC의 기준전압을 AVCC로 설정함
6. ADC 클럭의 분주비를 128로 설정함
7. UART를 통해 `"Line Tracer IR Sensor Start"` 메시지를 출력함
8. 반복문에서 6개의 IR 센서 값을 순서대로 읽음
9. 센서 1은 ADC2(PF2)의 값을 읽음
10. 센서 2는 ADC4(PF4)의 값을 읽음
11. 센서 3은 ADC6(PF6)의 값을 읽음
12. 센서 4는 ADC7(PF7)의 값을 읽음
13. 센서 5는 ADC5(PF5)의 값을 읽음
14. 센서 6은 ADC3(PF3)의 값을 읽음
15. 읽은 ADC 값을 `sensor_value` 배열에 저장함
16. `sprintf()` 함수를 이용하여 센서 번호와 값을 문자열로 변환함
17. UART를 통해 6개의 센서 값을 PC로 전송함
18. 센서 값을 모두 출력한 후 다음 줄로 이동함
19. 200ms 대기 후 다시 6개의 센서 값을 측정함
20. 위 과정을 `while(1)` 반복문을 통해 계속 반복함

이번 과제를 통해 IR 센서가 출력하는 아날로그 값을 ATmega128의 ADC를 이용하여 읽을 수 있다는 것을 확인함. 처음에는 센서가 6개이기 때문에 각각의 센서 값을 하나씩 읽는 코드를 따로 작성해야 할 것이라고 생각했지만, 센서가 연결된 ADC 채널을 배열로 저장하고 반복문을 사용하면 비교적 간단하게 처리할 수 있었음.

또한 ADC에서 읽은 숫자만 내부에서 사용하는 것이 아니라 UART를 통해 PC의 시리얼 모니터로 직접 확인할 수 있다는 점을 학습함. 실제 라인 트레이서의 센서 값을 확인하면서 센서가 바닥의 상태에 따라 어떤 값으로 변화하는지 관찰할 수 있었음.

특히 이번 실습을 통해 ADC는 아날로그 값을 디지털 값으로 변환하는 역할을 하고, UART는 이렇게 읽은 데이터를 외부 장치로 전송하는 역할을 한다는 점을 이해할 수 있었음. 이후 라인 트레이서의 실제 주행 제어를 구현할 때 각 센서 값의 크기를 비교하여 로봇이 어느 방향으로 이동해야 하는지 판단하는 데 사용할 수 있을 것으로 생각함.

### 동작 사진 / 영상

| 정면 동작 모습 |
| :---: |
| ![Hardware Setup](https://drive.google.com/file/d/1NiPMitHNKPDEAiLq4hgnhN2I2Sf9tbPl/view?usp=sharing) |
---

## 7. AI 툴 활용 명시 (AI Tools Declaration)

본 과제 작성 및 구현 과정에서 활용한 AI 도구(Generative AI)의 사용 현황 및 목적은 다음과 같음.

| 도구명 (Tool) | 활용 영역 | 세부 사용 목적 및 내용 |
| :--- | :--- | :--- |
| **ChatGPT / Claude** | 개념정리 | - ADC 값 처리 및 시리얼 출력 과정 이해 |

### AI 활용 및 검증 원칙

1. **코드 검증:** UART 통신 속도, ADC 기준전압, ADC 분주비 및 채널 설정은 ATmega128 관련 설정 내용을 기준으로 확인하고 실제 시리얼 모니터에서 센서 값이 정상적으로 출력되는지 테스트함.
2. **학습 주도성:** 센서 채널 배열 구성, ADC 데이터 읽기 및 UART 출력과 같은 핵심 프로그램 구조는 직접 작성하였으며, AI는 개념 이해 및 보고서 문서화 과정에서 보조 도구로 활용함.