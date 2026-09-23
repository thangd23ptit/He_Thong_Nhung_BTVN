#include "UART.h"
#include "RCC.h"
#include "GPIO.h"

void UART1_Init(uint32_t baudrate)
{
    RCC_Enable_UART1();
    RCC_Enable_PortA();

    GPIO_Config(
        GPIOA,
        GPIO_PIN_9,
        GPIO_MODE_AF_PP
    );

    GPIO_Config(
        GPIOA,
        GPIO_PIN_10,
        GPIO_MODE_INPUT_FLOATING
    );

    USART1_CR1 = 0;
    USART1_CR2 = 0;
    USART1_CR3 = 0;

    USART1_BRR =
        (72000000UL + baudrate / 2U) / baudrate;

    USART1_CR1 |= USART_CR1_UE;
    USART1_CR1 |= USART_CR1_TE;
    USART1_CR1 |= USART_CR1_RE;
}

void UART1_DMA_Enable(void)
{
    USART1_CR3 |= USART_CR3_DMAT;
}

void UART1_DMA_Disable(void)
{
    USART1_CR3 &= ~USART_CR3_DMAT;
}

void UART1_SendChar(char c)
{
    while (!(USART1_SR & (1U << 7)))
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
    while (!(USART1_SR & (1U << 5)))
    {
    }

    return (char)(USART1_DR & 0xFFU);
}

uint8_t UART1_Available(void)
{
    return (USART1_SR & (1U << 5)) ? 1U : 0U;
}

void UART1_SendNumber(uint32_t number)
{
    char buffer[11];
    uint8_t index = 0;

    if (number == 0)
    {
        UART1_SendChar('0');
        return;
    }

    while (number > 0)
    {
        buffer[index++] =
            (char)('0' + (number % 10));

        number /= 10;
    }

    while (index > 0)
    {
        UART1_SendChar(
            buffer[--index]
        );
    }
}