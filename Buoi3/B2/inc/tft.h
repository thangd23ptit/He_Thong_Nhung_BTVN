#ifndef __TFT_H
#define __TFT_H

#include "stm32f10x.h"

/*
 * Màn hình 1.54" TFT ST7789 240x240 (SPI)
 *
 * Nối dây:
 *   TFT GND -> GND
 *   TFT VCC -> 3.3V
 *   TFT SCL -> PA5  (SPI1_SCK)
 *   TFT SDA -> PA7  (SPI1_MOSI)
 *   TFT RST -> PB1
 *   TFT DC  -> PB0
 *   TFT CS  -> PA4
 *   TFT BL  -> PB10 (hoặc nối thẳng 3.3V)
 *
 * Màu RGB565 (mã hex):
 *   0x0000 đen   0xFFFF trắng  0xF800 đỏ     0x07E0 xanh lá
 *   0x001F xanh dương  0xFFE0 vàng  0x07FF cyan  0xF81F tím  0xFD20 cam
 */

void TFT_Init(void);
void TFT_FillScreen(uint16_t color);
void TFT_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void TFT_DrawPixel(uint16_t x, uint16_t y, uint16_t color);
void TFT_DrawHLine(uint16_t x, uint16_t y, uint16_t w, uint16_t color);
void TFT_DrawVLine(uint16_t x, uint16_t y, uint16_t h, uint16_t color);
void TFT_DrawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void TFT_DrawChar(uint16_t x, uint16_t y, char c, uint16_t fg, uint16_t bg, uint8_t size);
void TFT_DrawString(uint16_t x, uint16_t y, const char *s, uint16_t fg, uint16_t bg, uint8_t size);

#endif
