#ifndef __UART_H
#define __UART_H

#include "stm32f10x.h"
#include <stdint.h>

/* Khởi tạo UART1 với Baudrate 9600 (PCLK2 = 72MHz) */
void UART1_Init(void);

/* Gửi 1 ký tự qua UART1 */
void UART1_SendChar(char c);

/* Gửi 1 chuỗi ký tự qua UART1 */
void UART1_SendString(const char *str);

/* Nhận 1 ký tự qua UART1 (chế độ Polling / chờ cờ RXNE) */
char UART1_ReceiveChar(void);

/* Kiểm tra xem có dữ liệu sẵn sàng trong thanh ghi nhận chưa (1: có, 0: chưa) */
uint8_t UART1_IsDataAvailable(void);

#endif /* __UART_H */