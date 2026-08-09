# ATmega128 과제 및 프로젝트

> **광운대학교 (로보학부)**
> **작성자:** (이규환)
> **제출일:** (2026.00.00)

---

## 1. 개요 (Overview)

본 과제는 ATmega128 마이크로컨트롤러의 Timer1과 PWM 기능을 이용하여 두 개의 DC 모터의 회전 방향과 속도를 제어하는 것을 목표로 함.

이번 과제에서는 모터 드라이버를 사용하여 Motor A와 Motor B를 제어하였으며, 각 모터의 방향은 PORTB의 일반 입출력 핀을 이용하여 설정하고, 모터 구동을 위한 PWM 신호는 Timer1의 OC1A와 OC1B 핀을 사용하여 출력하도록 구현함.

프로그램은 전원이 인가된 후 두 모터를 일정 시간 동안 정회전시키고, 잠시 정지한 후 역회전시키는 동작을 반복함. 이를 통해 Timer1의 Fast PWM 모드, 분주비 설정, TOP 값 설정 및 OCR 레지스터를 이용한 듀티비 제어 방법을 학습하고, 함수로 모터의 동작을 구분하여 프로그램을 구성하는 방법을 확인함.

### 핵심 목표

- ATmega128 Timer1 레지스터 설정을 통한 PWM 신호 출력
- Fast PWM 모드와 ICR1을 이용한 PWM 주파수 설정
- OCR1A, OCR1B를 이용한 PWM 듀티비 설정
- 모터 드라이버의 방향 제어 핀을 이용한 정회전 및 역회전 구현
- 모터 동작을 함수로 분리하여 정회전, 역회전, 정지 기능 구현

---

## 2. 개발 환경 (Environment)

| 항목 | 내용 |
| :--- | :--- |
| **MCU** | ATmega128A (16MHz External Crystal) |
| **IDE / Compiler** | Microchip Studio 7.0 / Microchip AVR GCC |
| **Flasher Tool** | USBISP / STK500 |
| **언어** | C Language |
| **주요 부품** | ATmega128 개발보드, DC 모터 2개, 모터 드라이버 모듈 |

---

## 3. 하드웨어 구성 및 핀 맵 (Hardware Structure)

### Pin Configuration

```text
[ATmega128]                    [Target Component]

PB0                     ----->   Motor A 방향 제어 입력 1
PB1                     ----->   Motor A 방향 제어 입력 2

PB2                     ----->   Motor B 방향 제어 입력 1
PB3                     ----->   Motor B 방향 제어 입력 2

PB5 (OC1A)              ----->   Motor A PWM 제어
PB6 (OC1B)              ----->   Motor B PWM 제어

Timer1                  ----->   Fast PWM 신호 생성
```

### 주요 회로 특징

- **전원:** ATmega128 및 모터 드라이버에 필요한 전원 공급
- **모터 방향 제어:** PB0~PB3을 이용하여 각 모터의 회전 방향 설정
- **PWM 출력:** Timer1의 OC1A(PB5), OC1B(PB6) 핀을 사용하여 PWM 신호 출력
- **PWM 주파수:** 약 5kHz
- **PWM 듀티비:** 약 50%
- **주의사항:** 모터는 동작 시 순간적으로 비교적 큰 전류를 사용할 수 있으므로 ATmega128 핀에 직접 연결하지 않고 모터 드라이버를 통해 제어함

---

## 4. 프로젝트 구조 (Directory Structure)

> 구현부(.c), 선언부(.h)만 구조에 표기함.

```text
├── Day06_Task01/
│   ├── Day06_Task01.c # 메인 제어 루프, Timer1 PWM 설정 및 모터 제어
│   └── README.md      # 과제 동작 및 구현 내용 정리
├── docs/
│   └── motor_connection.pdf # 모터 및 ATmega128 연결 회로도
└── README.md
```

본 과제는 하나의 소스 파일에서 구현하였으며, 프로그램의 가독성을 높이기 위해 모터의 정회전, 역회전, 정지 기능을 각각 함수로 분리함.

---

## 5. 핵심 코드 및 레지스터 설정 (Key Implementation)

### Timer1 및 PWM 초기화 (`Day06_Task01.c`)

```c
TCCR1A = (1 << COM1A1) |
         (1 << COM1B1) |
         (1 << WGM11);

TCCR1B = (1 << WGM13) |
         (1 << WGM12) |
         (1 << CS11);

ICR1 = 399;

OCR1A = 199;
OCR1B = 199;
```

Timer1은 Fast PWM 모드로 설정함. `WGM13`, `WGM12`, `WGM11` 비트를 설정하여 ICR1을 TOP 값으로 사용하는 PWM 모드로 구성함.

`CS11` 비트를 설정하여 Timer1의 분주비를 8로 설정함. ATmega128의 시스템 클럭이 16MHz이므로 Timer1에는 다음과 같은 클럭이 사용됨.

```text
Timer1 Clock = 16MHz / 8
             = 2MHz
```

PWM 주파수는 ICR1 값을 이용하여 설정함.

```text
PWM 주파수 = F_CPU / (분주비 × (ICR1 + 1))

           = 16,000,000 / (8 × (399 + 1))

           = 5,000Hz

           = 5kHz
```

따라서 `ICR1 = 399`로 설정하여 약 5kHz의 PWM 신호가 출력되도록 함.

`OCR1A`와 `OCR1B`는 각각 OC1A와 OC1B의 PWM 듀티비를 결정함.

```c
OCR1A = 199;
OCR1B = 199;
```

TOP 값이 399이므로 약 50% 듀티비의 PWM 신호를 출력하도록 설정함. 두 모터에 동일한 PWM 값을 적용하여 두 모터가 같은 PWM 조건으로 동작하도록 구성함.

### 모터 방향 제어 함수 (`Day06_Task01.c`)

```c
void Motor_Forward(void)
{
    PORTB |= (1 << PB0);
    PORTB &= ~(1 << PB1);

    PORTB |= (1 << PB2);
    PORTB &= ~(1 << PB3);
}

void Motor_Reverse(void)
{
    PORTB &= ~(1 << PB0);
    PORTB |= (1 << PB1);

    PORTB &= ~(1 << PB2);
    PORTB |= (1 << PB3);
}

void Motor_Stop(void)
{
    PORTB &= ~((1 << PB0) |
               (1 << PB1) |
               (1 << PB2) |
               (1 << PB3));
}
```

`Motor_Forward()` 함수에서는 Motor A와 Motor B의 방향 제어 핀을 각각 HIGH, LOW로 설정하여 정회전하도록 구성함.

```text
Motor A : PB0 = HIGH, PB1 = LOW
Motor B : PB2 = HIGH, PB3 = LOW
```

`Motor_Reverse()` 함수에서는 정회전과 반대되는 값으로 방향 제어 핀을 설정함.

```text
Motor A : PB0 = LOW, PB1 = HIGH
Motor B : PB2 = LOW, PB3 = HIGH
```

`Motor_Stop()` 함수에서는 PB0~PB3의 방향 제어 핀을 모두 LOW로 설정하여 두 모터가 정지하도록 구성함.

---

## 6. 동작 설명 및 결과 (Results)

### 동작 시나리오

1. 시스템 전원 인가 후 PB0, PB1, PB2, PB3, PB5, PB6 핀을 출력으로 설정함
2. PB0~PB3의 방향 제어 핀을 LOW로 설정하여 초기 상태에서 모터가 동작하지 않도록 설정함
3. Timer1을 Fast PWM 모드로 설정하고 분주비를 8로 설정함
4. ICR1 값을 399로 설정하여 약 5kHz의 PWM 신호가 출력되도록 함
5. OCR1A와 OCR1B 값을 199로 설정하여 두 PWM 출력의 듀티비를 약 50%로 설정함
6. `Motor_Forward()` 함수를 실행하여 Motor A와 Motor B를 정회전시킴
7. 두 모터가 정회전 상태로 약 3초 동안 동작함
8. `Motor_Stop()` 함수를 실행하여 두 모터를 정지시키고 약 1초 동안 유지함
9. `Motor_Reverse()` 함수를 실행하여 두 모터를 역회전시킴
10. 두 모터가 역회전 상태로 약 3초 동안 동작함
11. 다시 모터를 정지시키고 약 1초 동안 유지함
12. 위 동작을 `while(1)` 반복문을 통해 계속 반복함

이번 과제를 통해 단순히 모터를 ON/OFF하는 것이 아니라, 모터의 **회전 방향은 방향 제어 핀으로 설정하고 모터의 구동 정도는 PWM 신호의 듀티비로 제어할 수 있다는 점**을 확인함.

또한 처음에는 Timer1의 여러 레지스터와 비트 설정이 복잡하게 느껴졌지만, `TCCR1A`와 `TCCR1B`에서 PWM 모드와 분주비를 설정하고, `ICR1`에서 PWM의 주기를 결정하며, `OCR1A`, `OCR1B`에서 듀티비를 설정한다는 구조로 이해하니 각각의 역할을 구분할 수 있었음.

### 동작 사진 / 영상

| 정면 동작 모습 |
| :---: |
| (https://drive.google.com/file/d/1uQYtQ8YdHrGh9g2zuJvUBuW9rcCooI_f/view?usp=sharing) |

---

## 7. AI 툴 활용 명시 (AI Tools Declaration)

본 과제 작성 및 구현 과정에서 활용한 AI 도구(Generative AI)의 사용 현황 및 목적은 다음과 같음.

| 도구명 (Tool) | 활용 영역 | 세부 사용 목적 및 내용 |
| :--- | :--- | :--- |
| **ChatGPT / Claude** | 코드 주석 & 문서화 | - Timer1, PWM 및 모터 제어 코드의 동작 과정 정리<br>- 레지스터 설정에 대한 학습<br>- 과제 보고서 내용 참고 |

### AI 활용 및 검증 원칙

1. **코드 검증:** Timer1의 PWM 설정, 분주비, ICR1 값 및 OCR 값은 ATmega128의 Timer/Counter 관련 설정 내용을 기준으로 확인하고 실제 모터 동작을 통해 테스트함.
2. **학습 주도성:** 모터의 정회전, 역회전, 정지와 같은 핵심 제어 구조는 직접 작성하였으며, AI는 개념 이해 및 보고서 문서화 과정에서 보조 도구로 활용함.
