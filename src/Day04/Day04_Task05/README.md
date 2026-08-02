# ATmega128 과제 및 프로젝트

> **광운대학교 (로보학부)**  
> **작성자:** (이규환)  
> **제출일:** (2026.08.02)

---

# 1. 개요 (Overview)

본 과제는 ATmega128의 Timer1 PWM 기능을 이용하여 서보 모터를 제어하고, UART를 통해 사용자가 입력한 각도(0~180°)에 따라 서보의 위치를 변경하는 프로그램을 구현하는 것을 목표로 한다.

서보 모터는 일반적인 DC 모터와 달리 PWM 신호의 듀티비가 아닌 펄스 폭(Pulse Width)에 의해 회전 각도가 결정된다. 따라서 Timer1의 Fast PWM Mode를 이용하여 20ms 주기의 PWM을 생성하고, 입력된 각도를 펄스 폭으로 변환하여 OCR1C 레지스터를 변경하는 방식으로 서보를 제어하였다.

또한 UART를 이용하여 사용자의 문자열 입력을 처리하고, 숫자 변환 및 입력 범위 검사를 수행하여 잘못된 입력으로 인한 오동작을 방지하도록 구현하였다.

---

# 2. 개발 환경 (Environment)

| 항목 | 내용 |
| :--- | :--- |
| **MCU** | ATmega128A (16MHz External Crystal) |
| **IDE / Compiler** | Microchip Studio 7.0 / AVR GCC |
| **언어** | C Language |
| **모터** | Servo Motor |
| **통신** | UART0 (9600bps) |
| **주변장치** | Timer1 PWM |

---

# 3. 하드웨어 구성 및 핀 맵 (Hardware Structure)

## Pin Configuration

```text
[ATmega128]

PB7 (OC1C) -------- Servo Motor PWM

PE0 (RXD0)
PE1 (TXD0) -------- PC Serial Monitor
```

### 주요 하드웨어 특징

* Timer1의 OC1C(PB7) 핀에서 PWM 출력
* UART0를 이용한 사용자 입력
* 20ms PWM 주기를 이용한 서보 제어

---

# 4. 프로젝트 구조 (Directory Structure)

```text
Day04_Task05/

├── main.c      # 서보 제어 및 사용자 입력 처리
├── uart.c      # UART 통신
├── uart.h
└── README.md
```

헤더 파일은 UART 함수 원형을 선언하며 실제 구현은 `uart.c`에서 수행하였다.

---

# 5. main.c 구현

## PWM 주기 설정

일반적인 아날로그 서보 모터는 약 20ms 주기의 PWM 신호를 사용한다.

```c
#define SERVO_PWM_PERIOD_US 20000UL
```

20ms마다 PWM 한 주기가 반복되며 HIGH 상태의 유지 시간에 따라 서보의 목표 각도가 결정된다.

---

## Timer1 PWM 초기화

서보 제어를 위해 Timer1의 Fast PWM Mode 14를 사용하였다.

```c
TCCR1A =
    (1 << COM1C1) |
    (1 << WGM11);

TCCR1B =
    (1 << WGM13) |
    (1 << WGM12) |
    (1 << CS11);
```

주요 설정은 다음과 같다.

* Fast PWM Mode 14 사용
* TOP 값을 ICR1로 설정
* 분주비 8 적용
* OC1C(PB7)에서 PWM 출력

이후 ICR1 레지스터에 계산된 값을 저장하여 정확한 20ms PWM 주기를 생성하였다.

---

## 서보 각도 계산

사용자가 입력한 각도는 직접 PWM으로 출력되는 것이 아니라 펄스 폭(us)으로 변환된다.

변환 과정은 다음과 같다.

```text
각도 입력

↓

PWM Pulse Width 계산

↓

Timer Tick 값 계산

↓

OCR1C 변경

↓

서보 회전
```

0~180도를 500us~2400us 범위로 선형 변환하여 서보가 원하는 위치로 이동하도록 구현하였다.

---

## UART 문자열 입력

사용자가 입력한 문자열을 한 줄 단위로 읽기 위해 `UART0_ReadLine()` 함수를 작성하였다.

지원 기능은 다음과 같다.

* Enter 입력 시 문자열 종료
* Backspace 입력 지원
* 입력 문자 Echo 출력
* 문자열 종료 문자('\0') 자동 추가

이를 통해 일반적인 터미널 입력과 동일한 환경을 구현하였다.

---

## 문자열을 숫자로 변환

UART 입력은 문자 형태이므로 바로 숫자로 사용할 수 없다.

`Parse_Int()` 함수에서 문자열을 정수로 변환하였다.

예를 들어

```text
"120"

↓

120
```

과 같은 형태로 변환하며, 숫자가 아닌 문자가 포함되면 오류를 반환하도록 구현하였다.

---

## 입력값 검사

사용자가 입력한 값이 정상 범위인지 검사한다.

검사 항목은 다음과 같다.

* 숫자인지 확인
* 0~180도 범위인지 확인

범위를 벗어난 경우에는 서보를 움직이지 않고 경고 메시지만 출력하도록 구현하였다.

---

## main 함수

메인 함수에서는 UART와 Timer1 PWM을 초기화한 후 서보를 기준 위치(90°)로 이동시킨다.

초기화 순서는 다음과 같다.

```text
UART 초기화

↓

Timer1 PWM 초기화

↓

서보 원점(90°) 이동

↓

500ms 대기

↓

시작 메시지 출력
```

이후 무한 루프에서는 다음 과정을 반복한다.

```text
사용자 입력

↓

문자열 저장

↓

정수 변환

↓

범위 검사

↓

서보 이동

↓

결과 출력
```

입력이 정상인 경우에는 `Set_Servo_Angle()` 함수를 호출하여 OCR1C 값을 변경하고 서보를 이동시킨다.

---

# 6. UART 모듈

UART 모듈은 사용자와의 통신을 담당한다.

주요 기능은 다음과 같다.

* UART 초기화
* 문자 송신
* 문자열 출력
* 정수 출력
* 줄바꿈 출력

통신 속도는 9600bps로 설정하였다.

---

# 7. 프로그램 동작 과정

```text
전원 인가

↓

UART 초기화

↓

Timer1 PWM 초기화

↓

서보 90도 이동

↓

사용자 각도 입력

↓

문자열 → 숫자 변환

↓

입력 범위 검사

↓

OCR1C 변경

↓

서보 회전

↓

결과 출력

↓

반복
```

---

# 8. 실행 결과

프로그램 실행 시 UART 터미널에는 다음과 같이 출력된다.

```text
Servo Control Ready (0~180)

Angle> 30
OK: Servo -> 30 deg

Angle> 120
OK: Servo -> 120 deg
```

잘못된 값을 입력한 경우에는 다음과 같이 출력된다.

```text
Angle> abc

WARNING: Invalid input (not a number). Command ignored.
```

범위를 벗어난 값을 입력한 경우에는 다음과 같이 출력된다.

```text
Angle> 250

WARNING: Angle out of range (0~180): 250 -> Command ignored, motor not moved.
```
### 동작 사진 / 영상

| 정면 동작 모습 | 센서 측정 및 시리얼 출력 |
| :---: | :---: |
| ![Hardware Setup](https://drive.google.com/file/d/1mXuMifVg67ZTxIbDQtaGxmXX-CO8BCjs/view?usp=sharing) | ![Serial Monitor](https://drive.google.com/file/d/1zeTZQwLHFbbCvFYGPAitdg9uFs3Llhx_/view?usp=sharing) |

---

# 9. 프로젝트를 수행하며 배운 점

이번 과제를 수행하면서 Timer1의 Fast PWM Mode를 이용하여 서보 모터를 제어하는 방법을 학습할 수 있었다.

또한 UART를 이용한 문자열 입력 처리와 문자열을 정수로 변환하는 방법을 구현하였으며, 입력값 검증을 통해 잘못된 명령으로부터 시스템을 보호하는 방법을 익힐 수 있었다.

특히 서보 모터는 PWM의 주기가 아니라 펄스 폭에 의해 각도가 결정된다는 점과 OCR1C 값을 변경하여 원하는 위치를 제어하는 원리를 이해할 수 있었다.

---

# 10. AI 툴 활용 명시 (AI Tools Declaration)

| 도구명 | 활용 영역 | 사용 목적 |
| :--- | :--- | :--- |
| ChatGPT | 디버깅 | Timer1 PWM 및 UART 입력 처리 정리, 보고서 작성 |

### AI 활용 및 검증 원칙

1. AI는 코드 분석 및 문서화 보조 도구로만 활용하였다.
2. 프로그램은 실제 개발보드에서 테스트하여 정상 동작을 확인하였다.
3. Timer1 PWM 설정과 UART 동작은 ATmega128 데이터시트를 참고하여 검증하였다.