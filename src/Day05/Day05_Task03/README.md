# TIMER1/3 & PWM 레지스터 정리 보고서

> 2024 로봇게임단 신입생 교육 자료(TIMER1/3, PWM) 기반 + 추가 보충 설명
> 대상 MCU: AVR 계열 (TIMSK/ETIMSK/SFIOR 명칭 사용 — ATmega128 계열 기준)

---

## 1. Timer/Counter 기본 개념

| 용어 | 설명 |
| --- | --- |
| Counter | Clock의 Rising Edge 수를 세는 것 |
| Timer | Counter의 Clock 입력을 일정 주기로 넣어 시간을 측정하는 것 |
| Prescaler(분주비) | Clock Source의 주기를 n배로 늘려 긴 주기의 Timer를 만들 때 사용하는 모듈 |
| BOTTOM | 타이머/카운터가 가질 수 있는 최소값 (0x0000) |
| MAX | 타이머/카운터가 가질 수 있는 최대값 (16bit 기준 0xFFFF) |
| TOP | 동작 모드에 따라 카운터가 실제로 도달하는 최대값 (CTC 모드에서는 TOP = OCR) |

**Timer Period 공식**

```
T = (Prescale × Count) / Freq_MCU
```

- TIMER0/2 : 8bit (0~255)
- TIMER1/3 : 16bit (0~65535) ← 이번 자료의 핵심

**Timer Interrupt 종류**
- Overflow (OVF)
- Compare Match (COMP)
- Capture Event (CAPT)

---

## 2. TIMER1/3 특징

- 16bit 카운터 구조 (0x0000 ~ 0xFFFF)
- 10bit 분주비 (0x0000 ~ 0x3FF)
- 지원 인터럽트: Overflow, Output Compare Match A/B/C, Input Capture
- CTC 모드에서는 Output Compare Match 시 타이머가 자동으로 Clear됨

**동작 방식 요약**
1. 내부 클럭 또는 외부 클럭 중 하나를 기준 클럭으로 선택
2. 0x0000~0xFFFF까지 카운트 후 Overflow → OVF 인터럽트 발생
3. TCNT 값과 OCR 값을 지속적으로 비교 → 일치 시 COMP 인터럽트 발생
4. 외부 ICPn 핀에 트리거 신호 입력 시 TCNTn 값이 ICRn에 저장되며 IC 인터럽트 발생

---

## 3. TIMER1 관련 레지스터 전체 목록

| 레지스터 | 역할 |
| --- | --- |
| TCCR1A / TCCR1B / TCCR1C | Timer/Counter1 제어 레지스터 |
| TCNT1H / TCNT1L | 16bit 카운터 현재값 |
| OCR1AH/L, OCR1BH/L, OCR1CH/L | 채널별 Output Compare 값 (16bit) |
| ICR1H / ICR1L | Input Capture Register (16bit) |
| SFIOR | Special Function IO Register (Prescaler 리셋 등) |
| TIMSK / ETIMSK | Timer Interrupt Mask (Enable) 레지스터 |
| TIFR / ETIFR | Timer Interrupt Flag 레지스터 |

---

## 4. 레지스터 상세

### 4-1. TIMSK (Timer Interrupt Mask Register)

| Bit | 이름 | 설명 |
| --- | --- | --- |
| 5 | TICIE1 | Input Capture Interrupt Enable (1=활성화) |
| 4:3 | OCIE1A, OCIE1B | Output Compare Match A/B Interrupt Enable |
| 2 | TOIE1 | Overflow Interrupt Enable |

> 위 인터럽트들을 실제로 사용하려면 **SREG의 전역 인터럽트 비트(I-bit)** 가 1로 set 되어 있어야 함 (`sei()` 호출 필요).

### 4-2. TCCR1A (Timer/Counter1 Control Register A)

| Bit | 이름 | 설명 |
| --- | --- | --- |
| 7:6 | COM1A1, COM1A0 | 채널 A 비교 출력 모드 (반전/비반전 선택) |
| 5:4 | COM1B1, COM1B0 | 채널 B 비교 출력 모드 |
| 3:2 | COM1C1, COM1C0 | 채널 C 비교 출력 모드 |
| 1:0 | WGM11, WGM10 | Waveform Generation Mode (TCCR1B의 WGM13:12와 결합하여 동작모드 결정) |

### 4-3. TCCR1B (Timer/Counter1 Control Register B)

| Bit | 이름 | 설명 |
| --- | --- | --- |
| 7 | ICNC1 | Input Capture Noise Canceler (1=노이즈 필터 작동, 시스템 클럭 4주기 지연 발생) |
| 6 | ICES1 | Input Capture Edge Select (1=상승 엣지, 0=하강 엣지 검출) |
| 4:3 | WGM13, WGM12 | Waveform Generation Mode (TCCR1A의 WGM11:10과 결합) |
| 2:0 | CS12, CS11, CS10 | Clock Select (분주비 선택) |

**Clock Select (CS12:10) 표**

| CS12 | CS11 | CS10 | 설명 |
| --- | --- | --- | --- |
| 0 | 0 | 0 | 클럭 소스 없음 (Timer/Counter 정지) |
| 0 | 0 | 1 | clk/1 (분주 없음) |
| 0 | 1 | 0 | clk/8 |
| 0 | 1 | 1 | clk/64 |
| 1 | 0 | 0 | clk/256 |
| 1 | 0 | 1 | clk/1024 |
| 1 | 1 | 0 | 외부 클럭(T1 핀), 하강 엣지에서 카운트 |
| 1 | 1 | 1 | 외부 클럭(T1 핀), 상승 엣지에서 카운트 |

**WGM(Waveform Generation Mode) 전체 표 (WGM13:WGM12:WGM11:WGM10)**

| Mode | WGM 3:2:1:0 | 동작 모드 | TOP | OCR 갱신 시점 | TOV Flag Set 시점 |
| --- | --- | --- | --- | --- | --- |
| 0 | 0000 | Normal | 0xFFFF | Immediate | MAX |
| 1 | 0001 | PWM, Phase Correct, 8-bit | 0x00FF | TOP | BOTTOM |
| 2 | 0010 | PWM, Phase Correct, 9-bit | 0x01FF | TOP | BOTTOM |
| 3 | 0011 | PWM, Phase Correct, 10-bit | 0x03FF | TOP | BOTTOM |
| 4 | 0100 | CTC | OCR1A | Immediate | MAX |
| 5 | 0101 | Fast PWM, 8-bit | 0x00FF | TOP | TOP |
| 6 | 0110 | Fast PWM, 9-bit | 0x01FF | TOP | TOP |
| 7 | 0111 | Fast PWM, 10-bit | 0x03FF | TOP | TOP |
| 8 | 1000 | PWM, Phase & Frequency Correct | ICR1 | BOTTOM | BOTTOM |
| 9 | 1001 | PWM, Phase & Frequency Correct | OCR1A | BOTTOM | BOTTOM |
| 10 | 1010 | PWM, Phase Correct | ICR1 | TOP | BOTTOM |
| 11 | 1011 | PWM, Phase Correct | OCR1A | TOP | BOTTOM |
| 12 | 1100 | CTC | ICR1 | Immediate | MAX |
| 13 | 1101 | 예약(Reserved) | - | - | - |
| 14 | 1110 | Fast PWM | ICR1 | TOP | TOP |
| 15 | 1111 | Fast PWM | OCR1A | TOP | TOP |

### 4-4. TCCR1C (Timer/Counter1 Control Register C)

| Bit | 이름 | 설명 |
| --- | --- | --- |
| 7:5 | FOC1A, FOC1B, FOC1C | Force Output Compare — CTC/PWM 모드에서 현재 타이머 값과 무관하게 강제로 출력 비교 동작(핀 토글/set/clear)을 즉시 발생시킴 |

### 4-5. TCNT1H / TCNT1L

- Timer/Counter1의 16bit 카운터 **현재값**을 저장하는 레지스터

### 4-6. OCR1xH / OCR1xL (x = A, B, C)

- TCNT1과 비교되어 OC1x 핀에 출력 신호를 만들기 위한 **16bit 비교값** 레지스터

### 4-7. ICR1 (Input Capture Register)

- ICP1 핀의 입력 캡처 신호에 의해 TCNT1 값이 저장되는 16bit 레지스터
- **Fast PWM 모드에서는 TOP 값으로도 사용 가능** → PWM 주파수를 자유롭게 설정할 때 핵심 레지스터

### 4-8. TIFR (Timer/Counter Interrupt Flag Register)

| Bit | 이름 | 설명 |
| --- | --- | --- |
| 5 | ICF1 | Input Capture Flag. IC 신호 발생 시 1로 set, 인터럽트 처리 시작과 동시에 자동으로 0 clear |

---

## 5. PWM (Pulse Width Modulation)

- 정의: 구동 전압 자체를 변화시키는 아날로그 방식이 아니라, **펄스의 폭(Duty)을 변조**하여 평균 전압/속도를 제어하는 디지털 방식
- 대표 모드 2가지: **Fast PWM**, **Phase Correct PWM**

### 5-1. 주파수/Duty 계산 예시 (20kHz 제어 목표)

| Prescaler | ICR1(TOP) | 비고 |
| --- | --- | --- |
| 1 | 799 | 정수값, 오차 없음 |
| 8 | 99 | 정수값 |
| 64 | 11.5 | 소수 발생 |
| 256 | 2.12 | 소수 발생 |
| 1024 | -0.21 | 음수/소수 → 사용 불가 |

Prescaler=1, ICR1=799일 때 Duty ratio별 OCR1 값 예시:

| Duty(%) | 계산식 | OCR1 값 |
| --- | --- | --- |
| 0 | ICR×0.00 | 0 |
| 25 | ICR×0.25 | 200 |
| 50 | ICR×0.50 | 400 |
| 75 | ICR×0.75 | 600 |
| 100 | ICR×1.00 | 799 |

### 5-2. 참고 초기화 코드 흐름 (Fast PWM, Timer1 예시)

```c
void Timer1_Init(void)
{
    DDRB |= (1 << PB5);              // OC1A 핀을 출력으로 설정

    TCCR1A = (1 << COM1A1) | (1 << WGM11);              // 비반전, Fast PWM
    TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS10);  // WGM=14(ICR1=TOP), No prescaling

    ICR1  = 799;   // 20kHz 목표 → TOP 값
    OCR1A = 0;     // 초기 Duty 0%
}
```

> 실제 과제 제출용 코드는 PPT 본문 및 개인 실습 코드를 기준으로 작성할 것 — 위는 레지스터 설정 흐름 이해를 위한 요약임.

---

## 6. (추가) PPT에 없지만 알아두면 좋은 내용

### 6-1. Fast PWM vs Phase Correct PWM — 주파수 공식 차이
같은 Prescaler(N), 같은 TOP 값이어도 두 모드의 실제 PWM 주파수는 다르다.

- Fast PWM 주파수: `f = F_clk / (N × (1 + TOP))`
- Phase Correct PWM 주파수: `f = F_clk / (2 × N × TOP)`

Phase Correct는 카운터가 0→TOP→0으로 왕복(삼각파)하기 때문에 **같은 TOP 값이라도 Fast PWM보다 주파수가 정확히 절반**이 된다. 또한 상승/하강이 대칭이라 파형이 항상 중앙 정렬(symmetric)되므로 모터 제어처럼 **좌우 대칭 스위칭이 중요한 응용(H-bridge)**에서 노이즈가 적다는 장점이 있다. 반면 Fast PWM은 갱신 주기가 짧아 더 높은 PWM 주파수를 낼 수 있다는 장점이 있다.

### 6-2. 16bit 레지스터 접근 시 반드시 주의할 점 (매우 중요)
AVR은 8bit MCU이기 때문에 TCNT1, OCR1x, ICR1 같은 16bit 레지스터에 한 번의 명령으로 접근할 수 없다. 내부적으로 **하나의 8bit 임시 레지스터(Temporary Register)**를 이용해 상위/하위 바이트를 순차적으로 처리하는데, 이 순서를 지키지 않으면 값이 깨질 수 있다.

- **읽을 때**: 반드시 **Low 바이트를 먼저 읽고**, High 바이트를 나중에 읽어야 한다.
- **쓸 때**: 반대로 **High 바이트를 먼저 쓰고**, Low 바이트를 나중에 써야 한다.
- C 컴파일러(avr-gcc 등)에서 `TCNT1 = 799;` 처럼 그냥 대입하면 컴파일러가 이 순서를 자동으로 처리해주지만, 인터럽트 중간에 접근이 끼어들 수 있는 상황(멀티 바이트 접근 도중 인터럽트 발생)에서는 값이 꼬일 수 있으므로 필요 시 `cli()`/`sei()`로 임계구역을 보호해야 한다.

### 6-3. Timer1과 Timer3의 관계
이번 자료는 TIMER1 레지스터를 기준으로 설명하지만, **TIMER3는 TIMER1과 완전히 동일한 구조를 그대로 복제한 것**이다. 레지스터 이름의 `1`을 `3`으로 바꾸면 그대로 대응된다.

| Timer1 | Timer3 |
| --- | --- |
| TCCR1A/B/C | TCCR3A/B/C |
| TCNT1H/L | TCNT3H/L |
| OCR1A/B/C | OCR3A/B/C |
| ICR1 | ICR3 |

단, 인터럽트 Enable/Flag 비트는 TIMSK/TIFR가 아니라 **ETIMSK / ETIFR (Extended)** 레지스터에 들어있다는 점이 다르다 (ATmega128 기준).

### 6-4. MCU 종류에 따른 레지스터 명칭 차이 (호환성 주의)
이 자료의 레지스터 명칭(TIMSK, ETIMSK, TIFR, ETIFR, SFIOR 하나로 통합)은 **ATmega128 계열**의 방식이다. ATmega2560, ATmega328 등 이후 세대 AVR에서는 타이머별로 레지스터가 완전히 분리되어 있다.

| 구분 | ATmega128 방식 | ATmega2560/328 방식 |
| --- | --- | --- |
| 인터럽트 Enable | TIMSK(공용) + ETIMSK | TIMSK1, TIMSK3 (타이머별 개별) |
| 인터럽트 Flag | TIFR(공용) + ETIFR | TIFR1, TIFR3 (타이머별 개별) |
| Prescaler 리셋 | SFIOR | GTCCR |

datasheet를 볼 때 **자신이 사용하는 정확한 MCU 모델명**을 확인하고 해당 데이터시트의 레지스터 맵을 참고해야 한다.

### 6-5. SFIOR (Special Function IO Register)
PPT 목록에는 있지만 세부 설명이 빠져 있는 레지스터. 여러 타이머의 **Prescaler를 동시에 리셋**하는 PSR321/PSR010 비트 등이 포함되어 있다. 여러 타이머를 위상 동기화(phase-sync)해서 동작시키고 싶을 때(예: 다축 모터 동시 구동) 사용한다.

### 6-6. Duty=100%/0% 근처의 실무 팁
- Fast PWM에서 `OCR1A = ICR1(TOP)`으로 설정하면 이론상 Duty 100%이지만, 실제로는 한 클럭 구간 동안 핀이 순간적으로 반대 상태가 되는 **글리치(glitch)**가 발생할 수 있는 모드가 있다. 완전한 0%/100%가 필요한 응용(밝기 완전 소등/최대 등)에서는 datasheet의 Edge case 설명을 반드시 확인할 것.
- 모터 제어 시 PWM 주파수를 가청주파수(20Hz~20kHz) 밖으로 잡거나(예: 20kHz 이상) 모터 자체의 전기적 시정수(inductance)를 고려하지 않으면 소음/발열/토크 손실이 발생할 수 있다.

### 6-7. H-bridge 모터 드라이버와 정/역 회전
PWM 만으로는 속도만 제어 가능하다. 정/역 회전을 위해서는 **H-bridge 모터 드라이버**(예: L298N, DRV8833 등)를 사용해 전류 방향을 스위칭해야 하며, 이때:
- 상단(High-side)과 하단(Low-side) 스위치가 동시에 켜지면 **관통 전류(shoot-through)**로 소자가 파손될 수 있어, 실무에서는 **데드타임(dead time)**을 두어 스위칭 전환 구간에 잠깐의 여유를 준다.
- 로봇 실습용 저가 드라이버 모듈은 대부분 이 부분이 내부적으로 처리되어 있지만, 원리는 알아두는 것이 좋다.

### 6-8. 인터럽트 서비스 루틴(ISR) 작성 시 참고
- avr-gcc 기준 인터럽트는 `#include <avr/interrupt.h>` 후 `ISR(TIMER1_OVF_vect)`, `ISR(TIMER1_COMPA_vect)`, `ISR(TIMER1_CAPT_vect)` 형태의 벡터 이름으로 작성한다.
- ISR 내부는 최대한 짧게 유지하고(진짜 처리 로직은 메인 루프의 flag 처리로 넘기는 방식 권장), ISR 안에서 다시 `sei()`를 호출하는 중첩 인터럽트는 특별한 이유가 없으면 피한다.

---

## 7. 과제 관련 요약 (참고)
- 과제1: 5kHz Fast PWM / Phase Correct 모드로 모터 정·역 회전 구현
- 과제2: 가변저항 값으로 속도·방향 제어, OCR1 값을 LCD에 표시(역방향은 `-` 표시), 제어 주파수 5kHz
- 과제3: 레지스터 설정에 대한 설명 문서 작성 + 주석 포함 코드(main.c) + 영상 제출