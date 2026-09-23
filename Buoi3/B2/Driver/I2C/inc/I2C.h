#ifndef __I2C_H
#define __I2C_H

#include <stdint.h>

#define I2C1_BASE       0x40005400UL

#define I2C1_CR1        (*(volatile uint32_t *)(I2C1_BASE + 0x00UL))
#define I2C1_CR2        (*(volatile uint32_t *)(I2C1_BASE + 0x04UL))
#define I2C1_OAR1       (*(volatile uint32_t *)(I2C1_BASE + 0x08UL))
#define I2C1_OAR2       (*(volatile uint32_t *)(I2C1_BASE + 0x0CUL))
#define I2C1_DR         (*(volatile uint32_t *)(I2C1_BASE + 0x10UL))
#define I2C1_SR1        (*(volatile uint32_t *)(I2C1_BASE + 0x14UL))
#define I2C1_SR2        (*(volatile uint32_t *)(I2C1_BASE + 0x18UL))
#define I2C1_CCR        (*(volatile uint32_t *)(I2C1_BASE + 0x1CUL))
#define I2C1_TRISE      (*(volatile uint32_t *)(I2C1_BASE + 0x20UL))
#define I2C1_FLTR       (*(volatile uint32_t *)(I2C1_BASE + 0x24UL))

#define I2C_CR1_PE      (1U << 0)
#define I2C_CR1_START   (1U << 8)
#define I2C_CR1_STOP    (1U << 9)
#define I2C_CR1_ACK     (1U << 10)
#define I2C_CR1_SWRST   (1U << 15)

#define I2C_SR1_SB      (1U << 0)
#define I2C_SR1_ADDR    (1U << 1)
#define I2C_SR1_BTF     (1U << 2)
#define I2C_SR1_RXNE    (1U << 6)
#define I2C_SR1_TXE     (1U << 7)
#define I2C_SR1_AF      (1U << 10)

#define I2C_SR2_BUSY    (1U << 1)

void I2C1_Init(void);

uint8_t I2C1_Start(void);
uint8_t I2C1_ReStart(void);
void I2C1_Stop(void);

uint8_t I2C1_SendAddress(uint8_t address);
uint8_t I2C1_WriteByte(uint8_t data);

uint8_t I2C1_ReadBytes(uint8_t *data, uint8_t length);

#endif