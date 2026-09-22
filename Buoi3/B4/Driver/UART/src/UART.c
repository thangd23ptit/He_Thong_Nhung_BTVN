#include "UART.h"
#include "RCC.h"
#include "GPIO.h"

void UART1_Init(uint32_t baudrate)
{
    uint32_t usartdiv;

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

    usartdiv =
        (72000000UL + (baudrate / 2UL))
        / baudrate;

    USART1_BRR = usartdiv;

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
    while (!(USART1_SR & USART_SR_TXE))
    {
    }

    USART1_DR = (uint32_t)c;
}

void UART1_SendString(const char *str)
{
    while (*str)
    {
        UART1_SendChar(*str);
        str++;
    }
}

void UART1_SendNumber(uint16_t value)
{
    char buffer[5];
    uint8_t index = 0;
    uint8_t i;

    if (value == 0)
    {
        UART1_SendChar('0');
        UART1_SendChar('\n');
        UART1_SendChar('\r');
        return;
    }

    while (value > 0)
    {
        buffer[index++] =
            (char)('0' + (value % 10U));

        value /= 10U;
    }

    for (i = index; i > 0; i--)
    {
        UART1_SendChar(buffer[i - 1]);
    }

    UART1_SendChar('\n');
    UART1_SendChar('\r');
}

char UART1_ReadChar(void)
{
    while (!(USART1_SR & USART_SR_RXNE))
    {
    }

    return (char)(USART1_DR & 0xFFU);
}