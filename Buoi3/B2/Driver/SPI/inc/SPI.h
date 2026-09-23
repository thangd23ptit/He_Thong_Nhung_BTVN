#ifndef __SPI_H
#define __SPI_H

#include <stdint.h>

#define SPI1_BAUDRATE_DIV2      0
#define SPI1_BAUDRATE_DIV4      1
#define SPI1_BAUDRATE_DIV8      2
#define SPI1_BAUDRATE_DIV16     3
#define SPI1_BAUDRATE_DIV32     4
#define SPI1_BAUDRATE_DIV64     5
#define SPI1_BAUDRATE_DIV128    6
#define SPI1_BAUDRATE_DIV256    7

void SPI1_Init(void);
uint8_t SPI1_Transfer(uint8_t data);
void SPI1_SendByte(uint8_t data);
uint8_t SPI1_ReadByte(void);

#endif