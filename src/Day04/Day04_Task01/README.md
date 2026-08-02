# 과제 1

# TIMER0 / TIMER2에 대한 보고서
(정의, 원리, 사용법)

---

# 1. TIMER란?

Timer는 마이크로컨트롤러(MCU) 내부에 존재하는 하드웨어 모듈로, 일정한 시간 간격을 측정하거나 특정 시간마다 동작을 수행하기 위해 사용된다.

프로그램에서 단순히 반복문(Delay)을 이용하여 시간을 지연시키는 방법도 있지만, CPU가 계속 반복문만 수행해야 하므로 다른 작업을 동시에 처리하기 어렵다.

반면 Timer는 CPU와 독립적으로 동작하기 때문에 시간이 흐르는 동안에도 CPU는 다른 연산을 수행할 수 있다.

ATmega128에는 여러 개의 Timer/Counter가 내장되어 있으며, 각각의 Timer는 다양한 기능을 수행할 수 있다.

대표적인 기능은 다음과 같다.

- 일정 시간 측정
- 주기적인 인터럽트 발생
- PWM(Pulse Width Modulation) 생성
- 외부 이벤트 카운트
- 시간 지연(Delay)

---

# 2. Timer와 Counter의 차이

ATmega128에서는 Timer와 Counter를 하나의 모듈에서 함께 제공한다.

두 기능의 차이는 클럭의 발생 위치이다.

| 구분 | Timer | Counter |
|---|---|---|
| 클럭 발생 | 내부 클럭 | 외부 입력 |
| 용도 | 시간 측정 | 펄스 개수 측정 |
| 사용 예 | Delay, PWM | 버튼 횟수, 엔코더 |

Timer는 MCU 내부 클럭을 이용하여 시간을 측정하고,

Counter는 외부에서 들어오는 신호를 세어 개수를 계산한다.

---

# 3. ATmega128 Timer 종류

ATmega128에는 총 4개의 Timer가 존재한다.

| Timer | 비트 수 | 특징 |
|---|---|---|
| Timer0 | 8bit | 기본 타이머 |
| Timer1 | 16bit | 고정밀 타이머 |
| Timer2 | 8bit | 비동기 동작 가능 |
| Timer3 | 16bit | Timer1과 유사 |

이번 보고서에서는 Timer0와 Timer2를 중심으로 설명한다.

---

# 4. Timer0란?

Timer0는 ATmega128에 내장된 8비트 타이머이다.

8비트이므로

0부터 255까지 총 256개의 값을 저장할 수 있다.

255 이후에는 다시 0으로 돌아가며 이를 Overflow라고 한다.

Timer0는 가장 많이 사용하는 기본 Timer이며

Delay 생성,

주기적인 인터럽트,

PWM 출력 등에 사용된다.

---

# 5. Timer2란?

Timer2 역시 8비트 Timer이다.

기본적인 동작은 Timer0와 거의 동일하지만,

외부 클럭(32.768kHz 크리스털)을 사용할 수 있다는 특징이 있다.

이 기능을 이용하면

RTC(Real Time Clock),

시계,

시간 측정 기능을 구현할 수 있다.

---

# 6. Timer 동작 원리

Timer는 MCU 내부 클럭을 일정한 속도로 받아 숫자를 증가시킨다.

예를 들어

CPU 클럭이

```
16MHz
```

라면

1초 동안

```
16,000,000번
```

클럭이 발생한다.

하지만 너무 빠르므로

Prescaler를 이용하여 속도를 줄인다.

예를 들어

Prescaler를 64로 설정하면

```
16MHz / 64

= 250,000Hz
```

즉

1초에

250,000번만 증가하게 된다.

이렇게 Timer 값이

255까지 증가하면

다시 0으로 돌아오며

Overflow가 발생한다.

---

# 7. Prescaler란?

Prescaler는 Timer 속도를 느리게 만드는 분주기이다.

클럭을 일정 비율로 나누어 Timer 증가 속도를 조절한다.

ATmega128에서는 다음과 같은 Prescaler를 사용할 수 있다.

| Prescaler | Timer 증가 속도 |
|---|---|
| 1 | 가장 빠름 |
| 8 | 8배 느림 |
| 32 | 32배 느림 (Timer2) |
| 64 | 많이 사용 |
| 128 | Timer2 지원 |
| 256 | 느림 |
| 1024 | 가장 느림 |

Prescaler 값이 클수록 Timer 증가 속도는 느려지고

더 긴 시간을 측정할 수 있다.

---

# 8. Overflow란?

8비트 Timer는

```
0
↓

1

↓

2

↓

...

↓

255
```

까지 증가한다.

그 다음 증가하면

```
255

↓

0
```

으로 다시 돌아간다.

이 순간을

Overflow

라고 한다.

Overflow가 발생하면

인터럽트를 발생시켜

특정 작업을 수행할 수 있다.

예를 들어

- LED 깜빡임
- 시간 측정
- 주기적인 센서 읽기

등에 사용된다.

---

# 9. Compare Match란?

Timer는 특정 값과 비교하여 일치하면 이벤트를 발생시킬 수도 있다.

예를 들어

OCR0 레지스터에

```
100
```

을 저장하면

Timer 값이

100이 되는 순간

Compare Match가 발생한다.

Overflow를 기다리지 않아도

원하는 시점에 정확하게 인터럽트를 발생시킬 수 있다.

---

# 10. Timer0 주요 레지스터

Timer0는 여러 레지스터를 이용하여 제어한다.

---

## 10.1 TCNT0

Timer 현재 값을 저장하는 레지스터이다.

값은

```
0 ~ 255
```

사이에서 계속 증가한다.

예)

```c
TCNT0 = 0;
```

Timer 값을 0부터 시작한다.

---

## 10.2 TCCR0

Timer0 동작을 설정하는 레지스터이다.

설정 가능한 항목은

- Prescaler
- 동작 모드
- PWM 설정

등이다.

대표적인 Prescaler 설정

| CS02 | CS01 | CS00 | 분주비 |
|---|---|---|---|
|0|0|1|1|
|0|1|0|8|
|0|1|1|64|
|1|0|0|256|
|1|0|1|1024|

예)

```c
TCCR0 = (1<<CS01) | (1<<CS00);
```

Prescaler를 64로 설정한다.

---

## 10.3 TIMSK

인터럽트 허용 레지스터이다.

Overflow 인터럽트나 Compare Match 인터럽트를 활성화한다.

예)

```c
TIMSK |= (1<<TOIE0);
```

Timer0 Overflow 인터럽트를 허용한다.

---

## 10.4 TIFR

Timer 인터럽트 발생 여부를 확인하는 레지스터이다.

Overflow 발생 시

TOV0 비트가 1이 된다.

---

## 10.5 OCR0

Compare Match 값을 저장한다.

예)

```c
OCR0 = 200;
```

Timer 값이

200이 되면

Compare Match가 발생한다.

---

# 11. Timer2 주요 레지스터

Timer2 역시 거의 동일한 구조를 가진다.

사용되는 레지스터

- TCNT2
- TCCR2
- OCR2
- ASSR
- TIMSK
- TIFR

ASSR 레지스터는

외부 크리스털 사용 여부를 설정하는 레지스터이다.

Timer2에서만 사용하는 특징적인 레지스터이다.

---

# 12. Timer0 초기화 예제

```c
void Timer0_Init(void)
{
    TCNT0 = 0;
    TCCR0 = (1<<CS01) | (1<<CS00);
}
```

동작 과정

1. Timer 값을 0으로 초기화

2. Prescaler를 64로 설정

3. Timer 시작

---

# 13. Timer0 Overflow 인터럽트 예제

```c
ISR(TIMER0_OVF_vect)
{
    PORTB ^= (1<<0);
}
```

Overflow가 발생할 때마다

PB0에 연결된 LED가 반전되어 깜빡이게 된다.

---

# 14. Timer2 초기화 예제

```c
void Timer2_Init(void)
{
    TCNT2 = 0;
    TCCR2 = (1<<CS22);
}
```

Timer2를 시작하는 가장 기본적인 예제이다.

---

# 15. Timer0와 Timer2 비교

| 항목 | Timer0 | Timer2 |
|---|---|---|
| 비트 수 | 8bit | 8bit |
| 최대 값 | 255 | 255 |
| Overflow | 가능 | 가능 |
| Compare Match | 가능 | 가능 |
| PWM | 가능 | 가능 |
| 외부 크리스털 | 불가능 | 가능 |
| RTC 기능 | 불가능 | 가능 |

---

# 16. Timer의 활용 분야

Timer는 다양한 임베디드 시스템에서 사용된다.

대표적인 활용 예는 다음과 같다.

- LED 점멸
- Delay 생성
- PWM 모터 제어
- 서보모터 제어
- 부저 출력
- 시계(RTC)
- 센서 주기 측정
- 인터럽트 발생
- 주기적인 데이터 송수신

---

# 17. Timer0와 Timer2의 장점 및 단점

## 장점

- CPU와 독립적으로 동작
- 정확한 시간 측정 가능
- 인터럽트 사용 가능
- PWM 생성 가능
- 다양한 분주비(Prescaler) 지원

## 단점

- 8비트이므로 긴 시간 측정이 어려움
- Prescaler 계산이 필요함
- Overflow 계산이 복잡할 수 있음

---

# 18. 결론

Timer0와 Timer2는 ATmega128에서 가장 많이 사용하는 8비트 Timer이다.

두 Timer 모두 내부 클럭을 이용하여 시간을 측정하고, Overflow와 Compare Match 기능을 통해 일정한 시간마다 인터럽트를 발생시킬 수 있다.

특히 Timer0는 일반적인 시간 지연과 PWM 생성에 널리 사용되며, Timer2는 외부 크리스털을 사용할 수 있어 RTC(Real Time Clock)와 같은 시간 관리 기능에 적합하다.

또한 Prescaler를 이용하여 Timer의 속도를 조절할 수 있으며, 다양한 레지스터(TCCR0, TCNT0, OCR0, TIMSK 등)를 설정하여 원하는 방식으로 Timer를 제어할 수 있다.

따라서 Timer0와 Timer2는 LED 제어, 센서 측정, 모터 제어, 주기적인 인터럽트 등 다양한 임베디드 시스템에서 핵심적인 역할을 수행하는 중요한 하드웨어 모듈이다.