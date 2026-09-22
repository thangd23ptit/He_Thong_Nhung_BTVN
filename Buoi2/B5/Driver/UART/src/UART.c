#include "UART.h"
#include "GPIO.h"
#include "RCC.h"


volatile char uart_cmd_buffer[UART_CMD_BUFFER_SIZE];

volatile uint8_t uart_cmd_ready = 0;

static volatile uint8_t uart_cmd_index = 0;


void UART1_Init(uint32_t baudrate)
{
    USART1_BRR =(72000000UL + baudrate / 2) / baudrate;
    USART1_CR1 = 0;
    USART1_CR1 |= (1 << 13);
    USART1_CR1 |= (1 << 3);
    USART1_CR1 |= (1 << 2);

    USART1_CR1 |= (1 << 5);
    NVIC_ISER1 |= (1 << 5);
}


void UART1_SendChar(char c)
{
    while (!(USART1_SR & (1 << 7)))
    {
    }

    USART1_DR = (uint32_t)c;
}


void UART1_SendString(const char *str)
{
    while (*str)
    {
        UART1_SendChar(*str++);
    }
}


char UART1_ReadChar(void)
{
    while (!(USART1_SR & (1 << 5)))
    {
    }

    return (char)(USART1_DR & 0xFF);
}

void USART1_IRQHandler(void)
{
    if (USART1_SR & (1 << 5))
    {
        char c;
        c = (char)(USART1_DR & 0xFF);
        if (c == '!')
        {
            uart_cmd_buffer[uart_cmd_index] = '\0';
            uart_cmd_ready = 1;
            uart_cmd_index = 0;
        }
        else
        {
            if (uart_cmd_index <
                UART_CMD_BUFFER_SIZE - 1)
            {
                uart_cmd_buffer[uart_cmd_index++] = c;
            }
            else
            {
                uart_cmd_index = 0;
            }
        }
    }
}