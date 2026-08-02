#ifndef I2C_H_
#define I2C_H_

#include <avr/io.h>

#ifndef F_CPU
#define F_CPU 16000000UL   // 시스템 클럭
#endif

// I2C 통신 속도 (Hz) - 표준모드 100kHz, 고속모드 400kHz
#define I2C_BITRATE 100000UL

// TWI 상태코드 (필요한 것만)
#define TWI_START_OK        0x08
#define TWI_MT_SLA_ACK      0x18
#define TWI_MT_DATA_ACK     0x28
#define TWI_MR_SLA_ACK      0x40

void    I2C_Init(void);
uint8_t I2C_Start(uint8_t address_rw);  // address_rw = (7bit 주소<<1)|R/W
void    I2C_Stop(void);
uint8_t I2C_Write(uint8_t data);
uint8_t I2C_ReadAck(void);
uint8_t I2C_ReadNack(void);

#endif /* I2C_H_ */
