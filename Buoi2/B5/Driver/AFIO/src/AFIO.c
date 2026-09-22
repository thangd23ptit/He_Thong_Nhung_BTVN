#include "AFIO.h"
#include "RCC.h"


void AFIO_Init(void)
{
    RCC_Enable_AFIO();
}


void AFIO_EXTI_Config(uint8_t port, uint8_t pin)
{
    uint8_t index;
    uint8_t shift;

    if (pin > 15)
    {
        return;
    }

    if (port > AFIO_PORT_E)
    {
        return;
    }

    index = pin / 4;
    shift = (pin % 4) * 4;

    AFIO->EXTICR[index] &= ~(0xFU << shift);

    AFIO->EXTICR[index] |=
        ((uint32_t)port << shift);
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


void AFIO_USART2_Remap(uint8_t enable)
{
    if (enable)
    {
        AFIO->MAPR |= AFIO_REMAP_USART2;
    }
    else
    {
        AFIO->MAPR &= ~AFIO_REMAP_USART2;
    }
}


void AFIO_USART3_Remap(uint8_t enable)
{
    if (enable)
    {
        AFIO->MAPR |= AFIO_REMAP_USART3;
    }
    else
    {
        AFIO->MAPR &= ~AFIO_REMAP_USART3;
    }
}


void AFIO_SPI1_Remap(uint8_t enable)
{
    if (enable)
    {
        AFIO->MAPR |= AFIO_REMAP_SPI1;
    }
    else
    {
        AFIO->MAPR &= ~AFIO_REMAP_SPI1;
    }
}


void AFIO_I2C1_Remap(uint8_t enable)
{
    if (enable)
    {
        AFIO->MAPR |= AFIO_REMAP_I2C1;
    }
    else
    {
        AFIO->MAPR &= ~AFIO_REMAP_I2C1;
    }
}


void AFIO_TIM1_Remap(uint8_t value)
{
    AFIO->MAPR &= ~(3U << 6);

    AFIO->MAPR |=
        ((uint32_t)(value & 0x03) << 6);
}


void AFIO_TIM2_Remap(uint8_t value)
{
    AFIO->MAPR &= ~(3U << 8);

    AFIO->MAPR |=
        ((uint32_t)(value & 0x03) << 8);
}


void AFIO_TIM3_Remap(uint8_t value)
{
    AFIO->MAPR &= ~(3U << 10);

    AFIO->MAPR |=
        ((uint32_t)(value & 0x03) << 10);
}


void AFIO_CAN_Remap(uint8_t enable)
{
    if (enable)
    {
        AFIO->MAPR |= AFIO_REMAP_CAN;
    }
    else
    {
        AFIO->MAPR &= ~AFIO_REMAP_CAN;
    }
}


void AFIO_SWJ_Config(uint32_t mode)
{
    AFIO->MAPR &= ~AFIO_REMAP_SWJ_MASK;

    AFIO->MAPR |= mode;
}