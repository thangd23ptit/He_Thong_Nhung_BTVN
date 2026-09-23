#include "UART.h"
#include "RCC.h"
#include "GPIO.h"

void UART1_Init(uint32_t baudrate)
{
    RCC_Enable_UART1();
    RCC_Enable_PortA();

    GPIO_Config(
        GPIO_PORT_A,
        GPIO_PIN_9,
        GPIO_MODE_AF_PP
    );

    GPIO_Config(
        GPIO_PORT_A,
        GPIO_PIN_10,
        GPIO_MODE_INPUT_FLOATING
    );

    USART1_CR1 = 0;

    USART1_CR2 = 0;

    USART1_CR3 = 0;

    USART1_BRR =
        (72000000UL + baudrate / 2UL)
        / baudrate;

    USART1_CR1 |= USART_CR1_TE;

    USART1_CR1 |= USART_CR1_RE;

    USART1_CR1 |= USART_CR1_UE;
}

void UART1_SendChar(char c)
{
    while (!(USART1_SR & USART_SR_TXE))
    {
    }

    USART1_DR = (uint32_t)c;
}

void UART1_SendString(
    const char *str
)
{
    while (*str)
    {
        UART1_SendChar(*str);
        str++;
    }
}

uint8_t UART1_Available(void)
{
    return (
        USART1_SR & USART_SR_RXNE
    ) ? 1 : 0;
}

char UART1_ReadChar(void)
{
    while (!(USART1_SR & USART_SR_RXNE))
    {
    }

    return (char)(
        USART1_DR & 0xFF
    );
}

void UART1_Send2(
    uint8_t value
)
{
    UART1_SendChar(
        (char)(
            (value / 10) + '0'
        )
    );

    UART1_SendChar(
        (char)(
            (value % 10) + '0'
        )
    );
}