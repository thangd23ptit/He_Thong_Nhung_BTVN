#ifndef __I2C_H
#define __I2C_H

#include "stm32f10x.h"

// Khởi tạo I2C (I2C1: PB6-SCL, PB7-SDA | I2C2: PB10-SCL, PB11-SDA)
void I2C_Init_Config(uint8_t i2c_ch);

// Các hàm giao tiếp cơ bản
void I2C_Start(I2C_TypeDef *I2Cx);
void I2C_Stop(I2C_TypeDef *I2Cx);
void I2C_SendAddress(I2C_TypeDef *I2Cx, uint8_t address, uint8_t rw);
void I2C_WriteByte(I2C_TypeDef *I2Cx, uint8_t data);
uint8_t I2C_ReadByte(I2C_TypeDef *I2Cx, uint8_t ack);
uint8_t I2C_GetLastError(void);

#endif