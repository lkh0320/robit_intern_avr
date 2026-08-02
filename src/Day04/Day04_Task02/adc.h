#ifndef ADC_H_
#define ADC_H_

#include <avr/io.h>

void     ADC_Init(void);
uint16_t ADC_Read(uint8_t channel); // channel: 0~7 (PF0~PF7)

#endif /* ADC_H_ */
