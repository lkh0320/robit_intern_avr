
#ifndef UART_H_
#define UART_H_

#include <avr/io.h>
#include <stdint.h>

void UART0_Init(uint32_t baud);
void UART0_Transmit(uint8_t data);
void UART0_Print(const char *str);
void UART0_Println(const char *str);     // 문자열 + 개행(\r\n)
void UART0_PrintNumber(int16_t num);      // 정수 출력 (부호 포함)
void UART0_PrintDecimal1(int16_t scaled); // scaled = 실제값*10 (소수점 1자리로 출력)

#endif /* UART_H_ */
