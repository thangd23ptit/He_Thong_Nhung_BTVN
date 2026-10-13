#ifndef __UART_H
#define __UART_H

#include "stm32f10x.h"

// Khởi tạo UART1 và cấu hình DMA1 - Channel 4 cho bộ truyền (TX)
void UART1_DMA_Init(void);

// Hàm kích hoạt DMA để gửi dữ liệu từ buffer chỉ định
void UART1_Send_DMA(uint8_t *buffer, uint16_t len);

#endif