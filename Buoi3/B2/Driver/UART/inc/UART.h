#ifndef __UART_H
#define __UART_H

#include <stdint.h>

#define UART1_BASE          0x40013800UL

#define USART1_SR           (*(volatile uint32_t *)(UART1_BASE + 0x00UL))
#define USART1_DR           (*(volatile uint32_t *)(UART1_BASE + 0x04UL))
#define USART1_BRR          (*(volatile uint32_t *)(UART1_BASE + 0x08UL))
#define USART1_CR1          (*(volatile uint32_t *)(UART1_BASE + 0x0CUL))
#define USART1_CR2          (*(volatile uint32_t *)(UART1_BASE + 0x10UL))
#define USART1_CR3          (*(volatile uint32_t *)(UART1_BASE + 0x14UL))
#define USART1_GTPR         (*(volatile uint32_t *)(UART1_BASE + 0x18UL))

#define USART_CR1_UE        (1U << 13)
#define USART_CR1_TE        (1U << 3)
#define USART_CR1_RE        (1U << 2)

#define USART_CR3_DMAT      (1U << 7)

void UART1_Init(uint32_t baudrate);

void UART1_DMA_Enable(void);
void UART1_DMA_Disable(void);

void UART1_SendChar(char c);
void UART1_SendString(const char *str);
char UART1_ReadChar(void);
void UART1_SendNumber(uint32_t number);
uint8_t UART1_Available(void);

#endif