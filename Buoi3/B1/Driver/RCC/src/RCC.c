#include "RCC.h"

void RCC_Config_72Mhz(void)
{
    uint32_t temp;

    RCC_CR |= RCC_CR_HSEON;

    while (!(RCC_CR & RCC_CR_HSERDY))
    {
    }

    temp = RCC_CFGR;

    temp &= ~(0xFU << 4);
    temp &= ~(0x7U << 8);
    temp &= ~(0x7U << 11);
    temp &= ~(1U << 16);
    temp &= ~(1U << 17);
    temp &= ~(0xFU << 18);

    temp |= (0x4U << 8);
    temp |= (0x7U << 18);
    temp |= (1U << 16);

    RCC_CFGR = temp;

    RCC_CR |= RCC_CR_PLLON;

    while (!(RCC_CR & RCC_CR_PLLRDY))
    {
    }

    temp = RCC_CFGR;

    temp &= ~(0x3U << 0);
    temp |= (0x2U << 0);

    RCC_CFGR = temp;

    while (((RCC_CFGR >> 2) & 0x3U) != 0x2U)
    {
    }
}

void RCC_Enable_PortA(void)
{
    RCC_APB2ENR |= RCC_APB2_IOPAEN;
}

void RCC_Enable_PortB(void)
{
    RCC_APB2ENR |= RCC_APB2_IOPBEN;
}

void RCC_Enable_AFIO(void)
{
    RCC_APB2ENR |= RCC_APB2_AFIOEN;
}

void RCC_Enable_UART1(void)
{
    RCC_APB2ENR |= RCC_APB2_USART1EN;
}

void RCC_Enable_I2C1(void)
{
    RCC_APB1ENR |= RCC_APB1_I2C1EN;
}