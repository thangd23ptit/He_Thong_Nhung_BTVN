#include "NVIC.h"

void NVIC_EnableIRQ(uint8_t IRQn)
{
    if (IRQn < 32U)
    {
        NVIC_ISER0 |= (1UL << IRQn);
    }
    else
    {
        NVIC_ISER1 |= (1UL << (IRQn - 32U));
    }
}

void NVIC_DisableIRQ(uint8_t IRQn)
{
    if (IRQn < 32U)
    {
        NVIC_ICER0 |= (1UL << IRQn);
    }
    else
    {
        NVIC_ICER1 |= (1UL << (IRQn - 32U));
    }
}