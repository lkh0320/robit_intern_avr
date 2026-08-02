# ATmega128 과제 및 프로젝트

> **광운대학교 (로보학부)**  
> **작성자:** (이규환)  
> **제출일:** (2026.08.02)

---

# 1. 개요 (Overview)

본 과제는 ATmega128의 ADC와 Timer1 인터럽트를 이용하여 PSD 거리센서의 거리를 일정한 주기로 측정하고, UART를 통해 PC로 측정 결과를 출력하는 프로그램을 구현하는 것을 목표로 한다.

PSD 센서는 거리와 출력 전압이 선형 관계가 아니기 때문에 실측 데이터를 기반으로 한 보정식을 사용하여 실제 거리(cm)를 계산하였다. 또한 Timer1 Compare Match 인터럽트를 이용하여 일정한 주기로 센서를 측정하도록 구성하여 메인 루프의 부하를 줄이고 안정적인 측정이 가능하도록 설계하였다.

측정된 값은 정상 범위 여부를 검사한 후 UART를 통해 거리 값을 출력하며, 범위를 벗어난 경우에는 오류 메시지를 출력하도록 구현하였다.

---

# 2. 개발 환경 (Environment)

| 항목 | 내용 |
| :--- | :--- |
| **MCU** | ATmega128A (16MHz External Crystal) |
| **IDE / Compiler** | Microchip Studio 7.0 / AVR GCC |
| **언어** | C Language |
| **센서** | PSD Distance Sensor |
| **통신** | UART (9600bps) |
| **주변장치** | ADC, Timer1 Interrupt |

---

# 3. 하드웨어 구성 및 핀 맵 (Hardware Structure)

## Pin Configuration

```text
[ATmega128]

PF1(ADC1) -------- PSD Distance Sensor

PE0(RXD0)
PE1(TXD0) -------- PC Serial Monitor
```

---

## 주요 하드웨어 특징

* PSD 센서의 아날로그 출력값을 ADC1 채널에서 측정
* Timer1 Compare Match 인터럽트를 이용하여 200ms마다 측정 수행
* UART를 통해 거리 데이터를 PC로 출력

---

# 4. 프로젝트 구조 (Directory Structure)

```text
Day04_Task03/

├── main.c      # 메인 프로그램 및 Timer1 제어
├── adc.c       # ADC 초기화 및 데이터 읽기
├── uart.c      # UART 통신
├── adc.h
├── uart.h
└── README.md
```

헤더 파일은 함수 원형과 매크로를 선언하기 위한 파일이며 실제 기능 구현은 `.c` 파일에서 수행하였다.

---

# 5. main.c 구현

## 측정 주기 설정

프로그램은 Timer1 Compare Match 인터럽트를 이용하여 일정한 주기로 센서를 측정한다.

```c
#define MEASURE_INTERVAL_MS 200UL
```

측정 주기를 매크로로 정의하여 프로그램 수정 없이 손쉽게 측정 시간을 변경할 수 있도록 하였다.

---

## Timer1 비교값 계산

```c
#define OCR1A_VALUE ((F_CPU / TIMER1_PRESCALER / 1000UL) * MEASURE_INTERVAL_MS - 1UL)
```

CPU 클럭과 분주비를 이용하여 OCR1A 값을 자동으로 계산하도록 구현하였다.

따라서 측정 주기가 변경되어도 OCR1A 값을 다시 계산할 필요가 없다.

---

## PSD 센서 보정식

PSD 센서는 출력 전압과 거리가 선형 관계가 아니므로 실측 데이터를 기반으로 한 보정식을 사용하였다.

```c
distance_cm =
(DIST_COEFF_A / ((float)adc_raw + DIST_COEFF_B))
- DIST_OFFSET_C;
```

보정식을 이용하여 ADC 값을 실제 거리(cm)로 변환하였다.

---

## Timer1 초기화

Timer1은 CTC(Clear Timer on Compare Match) 모드로 설정하였다.

```c
TCCR1A = 0x00;

TCCR1B =
(1<<WGM12)
|
(1<<CS12)
|
(1<<CS10);
```

CTC 모드를 사용하면 OCR1A에 도달할 때마다 자동으로 카운터가 초기화되어 일정한 주기의 인터럽트를 생성할 수 있다.

이후 OCR1A 값을 설정하고 Compare Match 인터럽트를 허용하였다.

```c
OCR1A = OCR1A_VALUE;

TIMSK |= (1<<OCIE1A);
```

---

## Timer1 인터럽트

인터럽트에서는 센서를 직접 측정하지 않고 측정 플래그만 변경하였다.

```c
ISR(TIMER1_COMPA_vect)
{
    measure_flag = 1;
}
```

인터럽트 처리 시간을 최소화하여 시스템의 안정성을 높였다.

---

## 거리 계산 함수

ADC 값을 입력받아 실제 거리(cm)로 변환한다.

```c
Convert_ADC_To_Distance10()
```

소수점 첫째 자리까지 표현하기 위해 거리값에 10을 곱하여 정수형으로 저장하였다.

예를 들면

```text
25.3cm

↓

253
```

으로 저장하여 float 출력 없이도 소수점 표현이 가능하도록 구현하였다.

---

## 측정값 검증

측정값이 정상 범위인지 확인하는 함수를 작성하였다.

```c
Is_Valid_Reading()
```

검사 항목

* ADC 정상 범위 확인
* 거리 정상 범위 확인

정상 범위를 벗어나면 오류로 판단하여 거리 대신 오류 메시지를 출력하였다.

---

## 거리 측정 함수

센서 측정 과정은 하나의 함수로 구성하였다.

```text
ADC 읽기

↓

거리 계산

↓

정상 여부 검사

↓

UART 출력
```

이 과정을

```c
Measure_And_Print()
```

함수에서 수행하여 메인 함수의 구조를 단순하게 만들었다.

---

## main 함수

메인 함수에서는 주변장치를 초기화한 후 무한 루프를 실행한다.

초기화 순서는 다음과 같다.

```text
ADC 초기화

↓

UART 초기화

↓

Timer1 초기화

↓

전역 인터럽트 허용

↓

시작 메시지 출력
```

이후 무한 루프에서는

```c
if(measure_flag)
```

를 검사하여 Timer1 인터럽트에서 측정 요청이 발생한 경우에만 거리 측정을 수행한다.

이 구조를 사용함으로써 인터럽트는 매우 짧게 실행되고 실제 측정은 메인 루프에서 수행하도록 구현하였다.

---

# 6. ADC 모듈

ADC 모듈에서는 PSD 센서의 아날로그 전압을 디지털 값으로 변환한다.

주요 기능

* ADC 초기화
* ADC 채널 선택
* ADC 변환 시작
* 변환 완료 대기
* ADC 값 반환

---

# 7. UART 모듈

UART 모듈은 측정된 거리 데이터를 PC로 전송하는 기능을 수행한다.

주요 기능

* UART 초기화
* 문자열 출력
* 거리 출력
* 오류 메시지 출력

통신 속도는 9600bps를 사용하였다.

---

# 8. 프로그램 동작 과정

```text
전원 인가

↓

ADC 초기화

↓

UART 초기화

↓

Timer1 초기화

↓

전역 인터럽트 허용

↓

200ms Timer1 인터럽트 발생

↓

measure_flag 설정

↓

ADC 측정

↓

거리 계산

↓

정상 여부 확인

↓

UART 출력

↓

반복
```

---

# 9. 실행 결과

프로그램 실행 시 UART 시리얼 모니터에는 다음과 같이 측정된 거리가 출력된다.

```text
PSD Distance Measurement Start

25.4 cm

26.1 cm

27.0 cm

24.9 cm
```

센서의 측정 범위를 벗어난 경우에는

```text
ERROR : Out of range
```

메시지를 출력하여 비정상적인 데이터를 구분할 수 있도록 하였다.

### 동작 사진 / 영상

| 정면 동작 모습 | 센서 측정 및 시리얼 출력 |
| :---: | :---: |
| ![Hardware Setup](https://drive.google.com/file/d/1PmC0-kvKy7ghLo7Bx3ypJfteSUd-3RyL/view?usp=sharing) | ![Serial Monitor](https://drive.google.com/file/d/1L_fw2J6RvZO70NjI9HNeoZroGbNUF3Bc/view?usp=sharing) |

---

# 10. 프로젝트를 수행하며 배운 점

이번 과제를 수행하면서 ADC를 이용한 아날로그 센서 입력 처리와 Timer1 Compare Match 인터럽트를 이용한 주기적인 작업 수행 방법을 학습할 수 있었다.

또한 PSD 센서의 출력 특성이 선형이 아니라는 점을 고려하여 보정식을 적용하는 방법을 이해할 수 있었으며, 인터럽트에서는 플래그만 변경하고 실제 작업은 메인 루프에서 수행하는 구조를 구현하여 효율적인 프로그램을 작성할 수 있었다.

---

# 11. AI 툴 활용 명시 (AI Tools Declaration)

| 도구명 | 활용 영역 | 사용 목적 |
| :--- | :--- | :--- |
| ChatGPT | 코드 분석 및 문서화 | Timer1, ADC, UART 동작 원리 정리 |
| Claude | 디버깅 | 프로그램 구조 및 함수 설명 검토 |

### AI 활용 및 검증 원칙
1. **코드 검증:** AI가 생성한 레지스터 설정 및 함수 코드는 데이터시트(ATmega128 Datasheet)와 비교 검증한 후 실제 오실로스코프/시리얼 모니터링을 거쳐 직접 수정 및 테스트하였습니다.
2. **학습 주도성:** 코드의 핵심 제어 로직 설계는 직접 작성하였으며, AI는 보조 도구(디버깅, 문서화)로만 활용하였습니다.