#include "AFIO.h"

void AFIO_EXTI_Config(uint8_t port, uint8_t pin)
{
    uint32_t shift = (pin % 4U) * 4U;

    if (pin < 4U)
    {
        AFIO->EXTICR1.REG &= ~(0xFU << shift);
        AFIO->EXTICR1.REG |= ((uint32_t)port << shift);
    }
    else if (pin < 8U)
    {
        AFIO->EXTICR2.REG &= ~(0xFU << shift);
        AFIO->EXTICR2.REG |= ((uint32_t)port << shift);
    }
    else if (pin < 12U)
    {
        AFIO->EXTICR3.REG &= ~(0xFU << shift);
        AFIO->EXTICR3.REG |= ((uint32_t)port << shift);
    }
    else
    {
        AFIO->EXTICR4.REG &= ~(0xFU << shift);
        AFIO->EXTICR4.REG |= ((uint32_t)port << shift);
    }
}

void AFIO_USART1_Remap(uint8_t enable)
{
    if (enable)
    {
        AFIO->MAPR |= AFIO_REMAP_USART1;
    }
    else
    {
        AFIO->MAPR &= ~AFIO_REMAP_USART1;
    }
}