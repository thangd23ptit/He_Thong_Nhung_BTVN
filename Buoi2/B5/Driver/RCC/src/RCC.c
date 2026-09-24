#include "RCC.h"

void RCC_Config_72Mhz(void)
{
    /* =========================
     * Enable HSE = 8 MHz
     * ========================= */
    RCC->CR.BITS.HSEON = 1;

    while (!RCC->CR.BITS.HSERDY)
    {
    }

    /* =========================
     * Flash latency = 2 WS
     * ========================= */
    ACR->BITS.LATENCY = 2;

    /* =========================
     * AHB = SYSCLK / 1
     * ========================= */
    RCC->CFGR.BITS.HPRE = 0;

    /* =========================
     * APB1 = HCLK / 2
     * 72 MHz / 2 = 36 MHz
     * ========================= */
    RCC->CFGR.BITS.PPRE1 = 4;

    /* =========================
     * APB2 = HCLK / 1
     * 72 MHz / 1 = 72 MHz
     * ========================= */
    RCC->CFGR.BITS.PPRE2 = 0;

    /* =========================
     * PLL source = HSE
     * PLL = HSE x 9
     * ========================= */
    RCC->CFGR.BITS.PLLSRC = 1;
    RCC->CFGR.BITS.PLLXTPRE = 0;
    RCC->CFGR.BITS.PLLMUL = 7;

    /* =========================
     * Enable PLL
     * ========================= */
    RCC->CR.BITS.PLLON = 1;

    while (!RCC->CR.BITS.PLLRDY)
    {
    }

    /* =========================
     * SYSCLK = PLL
     * ========================= */
    RCC->CFGR.BITS.SW = 2;

    while (RCC->CFGR.BITS.SWS != 2)
    {
    }
}

void RCC_Enable_PortA(void){
	RCC->APB2ENR.BITS.IOPAEN = 1;
}

void RCC_Enable_PortB(void){
	RCC->APB2ENR.BITS.IOPBEN = 1;
}

void RCC_Enable_PortC(void){
	RCC->APB2ENR.BITS.IOPCEN = 1;
}
void RCC_Enable_AFIO(void){
	RCC->APB2ENR.BITS.AFIOEN = 1;
}

void RCC_Enable_TIM2(void){
	RCC->APB1ENR.BITS.TIM2EN = 1;
}

void RCC_Enable_ADC01(void){
	RCC->APB2ENR.BITS.ADC1EN = 1;
}

void RCC_Enable_UART1(void){
	RCC->APB2ENR.BITS.USART1EN = 1;
}
void RCC_Enable_SPI1(void){
	RCC->APB2ENR.BITS.SPI1EN = 1;
}

void RCC_Enable_I2C1(void){
	RCC->APB1ENR.BITS.I2C1EN = 1;
}
void RCC_Enable_USB(void){
	RCC->APB1ENR.BITS.USBEN = 1;
}

void RCC_Enable_UART2(void)
{
    RCC->APB1ENR.BITS.USART2EN = 1;
}