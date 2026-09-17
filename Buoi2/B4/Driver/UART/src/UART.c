#include "UART.h"
#include "GPIO.h"
#include "RCC.h"

void UART1_Init(uint32_t baudrate)
{
    RCC_Enable_PortA();
    RCC_Enable_UART1();
    GPIO_Config(GPIOA, GPIO_PIN_9, GPIO_MODE_AF_PP);
    GPIO_Config(GPIOA, GPIO_PIN_10, GPIO_MODE_INPUT_FLOATING);
    USART1_BRR = (72000000UL + baudrate / 2) / baudrate;
    USART1_CR1 = 0;
    USART1_CR1 |= (1 << 13);
    USART1_CR1 |= (1 << 3);
    USART1_CR1 |= (1 << 2);
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