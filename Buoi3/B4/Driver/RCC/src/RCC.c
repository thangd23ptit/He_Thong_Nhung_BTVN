#include "RCC.h"

void RCC_Config_72Mhz(void)
{
    RCC_CR |= (1U << 16);

    while (!(RCC_CR & (1U << 17)))
    {
    }

    FLASH_ACR &= ~(7U << 0);
    FLASH_ACR |=  (2U << 0);

    RCC_CFGR &= ~(1U << 16);
    RCC_CFGR |=  (1U << 16);

    RCC_CFGR &= ~(0xFU << 18);
    RCC_CFGR |=  (7U << 18);

    RCC_CFGR &= ~(0xFU << 4);
    RCC_CFGR |=  (0U << 4);

    RCC_CFGR &= ~(7U << 8);
    RCC_CFGR |=  (4U << 8);

    RCC_CFGR &= ~(7U << 11);
    RCC_CFGR |=  (0U << 11);

    RCC_CR |= (1U << 24);

    while (!(RCC_CR & (1U << 25)))
    {
    }

    RCC_CFGR &= ~(3U << 0);
    RCC_CFGR |=  (2U << 0);

    while (((RCC_CFGR >> 2) & 3U) != 2U)
    {
    }
}

void RCC_Enable_DMA1(void)
{
    RCC_AHBENR |= (1U << 0);
}

void RCC_Enable_AFIO(void)
{
    RCC_APB2ENR |= (1U << 0);
}

void RCC_Enable_PortA(void)
{
    RCC_APB2ENR |= (1U << 2);
}

void RCC_Enable_PortB(void)
{
    RCC_APB2ENR |= (1U << 3);
}

void RCC_Enable_PortC(void)
{
    RCC_APB2ENR |= (1U << 4);
}

void RCC_Enable_ADC1(void)
{
    RCC_APB2ENR |= (1U << 9);
}

void RCC_Enable_ADC2(void)
{
    RCC_APB2ENR |= (1U << 10);
}

void RCC_Enable_TIM2(void)
{
    RCC_APB1ENR |= (1U << 0);
}

void RCC_Enable_TIM3(void)
{
    RCC_APB1ENR |= (1U << 1);
}

void RCC_Enable_TIM4(void)
{
    RCC_APB1ENR |= (1U << 2);
}

void RCC_Enable_UART1(void)
{
    RCC_APB2ENR |= (1U << 14);
}

void RCC_Enable_UART2(void)
{
    RCC_APB1ENR |= (1U << 17);
}

void RCC_Enable_UART3(void)
{
    RCC_APB1ENR |= (1U << 18);
}

void RCC_Enable_SPI1(void)
{
    RCC_APB2ENR |= (1U << 12);
}

void RCC_Enable_SPI2(void)
{
    RCC_APB1ENR |= (1U << 14);
}

void RCC_Enable_I2C1(void)
{
    RCC_APB1ENR |= (1U << 21);
}

void RCC_Enable_I2C2(void)
{
    RCC_APB1ENR |= (1U << 22);
}

void RCC_Enable_USB(void)
{
    RCC_APB1ENR |= (1U << 23);
}