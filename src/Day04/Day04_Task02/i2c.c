#include "i2c.h"

void I2C_Init(void)
{
    // TWBR 계산: TWBR = (F_CPU / SCL - 16) / (2 * Prescaler)
    // Prescaler = 1 (TWPS1:0 = 00)
    TWSR = 0x00;
    TWBR = (uint8_t)(((F_CPU / I2C_BITRATE) - 16) / 2);
    TWCR = (1 << TWEN);   // TWI 활성화
}

uint8_t I2C_Start(uint8_t address_rw)
{
    // START 조건 전송
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));

    // SLA+R/W 전송
    TWDR = address_rw;
    TWCR = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));

    // 상태코드 확인 (상위 5비트만 유효)
    uint8_t status = TWSR & 0xF8;
    if (status != TWI_MT_SLA_ACK && status != TWI_MR_SLA_ACK)
        return 1; // 실패

    return 0; // 성공
}

void I2C_Stop(void)
{
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
    while (TWCR & (1 << TWSTO)); // STOP 완료 대기
}

uint8_t I2C_Write(uint8_t data)
{
    TWDR = data;
    TWCR = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));

    if ((TWSR & 0xF8) != TWI_MT_DATA_ACK)
        return 1; // 실패

    return 0; // 성공
}

uint8_t I2C_ReadAck(void)
{
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
    while (!(TWCR & (1 << TWINT)));
    return TWDR;
}

uint8_t I2C_ReadNack(void)
{
    TWCR = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));
    return TWDR;
}
