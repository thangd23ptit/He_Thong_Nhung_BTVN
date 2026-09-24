#include "UART.h"
#include "GPIO.h"
#include "RCC.h"


volatile char uart_cmd_buffer[UART_CMD_BUFFER_SIZE];

volatile uint8_t uart_cmd_ready = 0;

static volatile uint8_t uart_cmd_index = 0;


void UART1_Init(uint32_t baudrate)
{
    RCC_Enable_PortA();
    RCC_Enable_UART1();


    // PA9 = TX
    // PA10 = RX

    GPIO_Config(GPIOA,
                GPIO_PIN_9,
                GPIO_MODE_AF_PP);

    GPIO_Config(GPIOA,
                GPIO_PIN_10,
                GPIO_MODE_INPUT_FLOATING);


    // USART1 clock = 72 MHz

    USART1_BRR =
        (72000000UL + baudrate / 2) / baudrate;


    USART1_CR1 = 0;


    // UE
    USART1_CR1 |= (1 << 13);

    // TE
    USART1_CR1 |= (1 << 3);

    // RE
    USART1_CR1 |= (1 << 2);


    // =================================================
    // RXNE interrupt enable
    // =================================================

    USART1_CR1 |= (1 << 5);


    // =================================================
    // Enable USART1 interrupt
    //
    // USART1 IRQ = 37
    // ISER1 bit = 5
    // =================================================

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


// =========================================================
// USART1 INTERRUPT
// =========================================================

void USART1_IRQHandler(void)
{
    if (USART1_SR & (1 << 5))
    {
        char c;

        c = (char)(USART1_DR & 0xFF);


        // Command kết thúc bằng !
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
                // Buffer overflow
                uart_cmd_index = 0;
            }
        }
    }
}