#ifndef __SPI_H
#define __SPI_H

#include "stm32f10x.h"

/*
 * SPI1 (Master, chi truyen - 1 line) tren STM32F103C8:
 *   PA5 = SCK  (AF push-pull)
 *   PA7 = MOSI (AF push-pull)
 *   PA6 = MISO khong dung (man hinh TFT chi nhan du lieu)
 */
void    SPI1_Init(void);
void    SPI1_Write(uint8_t data);      /* Gui 1 byte (poll TXE)         */
void    SPI1_WaitDone(void);           /* Cho SPI gui xong hoan toan    */

#endif
