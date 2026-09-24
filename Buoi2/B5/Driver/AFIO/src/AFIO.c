#include "AFIO.h"

void AFIO_EXTI_Config(uint8_t port, uint8_t pin)
{
    uint8_t index;
    uint8_t shift;

    index = pin / 4;
    shift = (pin % 4) * 4;

    AFIO->EXTICR[index] &= ~(0xFU << shift);

    AFIO->EXTICR[index] |= ((uint32_t)port << shift);
}

void AFIO_USART1_Remap(uint8_t enable)
{
    if (enable)
        AFIO->MAPR |= AFIO_REMAP_USART1;
    else
        AFIO->MAPR &= ~AFIO_REMAP_USART1;
}