#include "RCC.h"

void RCC_Config_72Mhz(void)
{
    RCC->CR.BITS.HSEON = 1;

    while (!RCC->CR.BITS.HSERDY)
    {
    }

    ACR->BITS.LATENCY = 2;
    ACR->BITS.PRFTBE = 1;

    RCC->CFGR.BITS.HPRE = 0;
    RCC->CFGR.BITS.PPRE1 = 4;
    RCC->CFGR.BITS.PPRE2 = 0;
    RCC->CFGR.BITS.ADCPRE = 2;

    RCC->CFGR.BITS.PLLSRC = 1;
    RCC->CFGR.BITS.PLLXTPRE = 0;
    RCC->CFGR.BITS.PLLMUL = 7;
    RCC->CFGR.BITS.USBPRE = 0;

    RCC->CR.BITS.PLLON = 1;

    while (!RCC->CR.BITS.PLLRDY)
    {
    }

    RCC->CFGR.BITS.SW = 2;

    while (RCC->CFGR.BITS.SWS != 2)
    {
    }
}


void RCC_Enable_PortA(void)
{
    RCC->APB2ENR.BITS.IOPAEN = 1;
}


void RCC_Enable_PortB(void)
{
    RCC->APB2ENR.BITS.IOPBEN = 1;
}


void RCC_Enable_PortC(void)
{
    RCC->APB2ENR.BITS.IOPCEN = 1;
}


void RCC_Enable_PortD(void)
{
    RCC->APB2ENR.BITS.IOPDEN = 1;
}


void RCC_Enable_PortE(void)
{
    RCC->APB2ENR.BITS.IOPEEN = 1;
}


void RCC_Enable_AFIO(void)
{
    RCC->APB2ENR.BITS.AFIOEN = 1;
}


void RCC_Enable_DMA1(void)
{
    RCC->AHBENR.BITS.DMA1EN = 1;
}


void RCC_Enable_DMA2(void)
{
    RCC->AHBENR.BITS.DMA2EN = 1;
}


void RCC_Enable_TIM2(void)
{
    RCC->APB1ENR.BITS.TIM2EN = 1;
}


void RCC_Enable_TIM3(void)
{
    RCC->APB1ENR.BITS.TIM3EN = 1;
}


void RCC_Enable_TIM4(void)
{
    RCC->APB1ENR.BITS.TIM4EN = 1;
}


void RCC_Enable_ADC1(void)
{
    RCC->APB2ENR.BITS.ADC1EN = 1;
}


void RCC_Enable_ADC2(void)
{
    RCC->APB2ENR.BITS.ADC2EN = 1;
}


void RCC_Enable_UART1(void)
{
    RCC->APB2ENR.BITS.USART1EN = 1;
}


void RCC_Enable_UART2(void)
{
    RCC->APB1ENR.BITS.USART2EN = 1;
}


void RCC_Enable_UART3(void)
{
    RCC->APB1ENR.BITS.USART3EN = 1;
}


void RCC_Enable_SPI1(void)
{
    RCC->APB2ENR.BITS.SPI1EN = 1;
}


void RCC_Enable_SPI2(void)
{
    RCC->APB1ENR.BITS.SPI2EN = 1;
}


void RCC_Enable_I2C1(void)
{
    RCC->APB1ENR.BITS.I2C1EN = 1;
}


void RCC_Enable_I2C2(void)
{
    RCC->APB1ENR.BITS.I2C2EN = 1;
}


void RCC_Enable_USB(void)
{
    RCC->APB1ENR.BITS.USBEN = 1;
}