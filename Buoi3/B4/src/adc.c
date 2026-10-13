#include "adc.h"

/* Delay tam bang SysTick (dung khi cho ADC on dinh nguon) */
void ADC_DelayMs(uint32_t ms)
{
    SysTick->LOAD = 72000 - 1;              // 1ms @ 72MHz
    SysTick->CTRL = (1 << 0) | (1 << 2);    // Enable, clock = AHB
    while (ms--)
    {
        SysTick->VAL = 0;
        while (!(SysTick->CTRL & (1 << 16))); // cho COUNTFLAG
    }
    SysTick->CTRL = 0;
}

/*
 * ADC1, channel 5 (PA5), trigger bang TIM3_TRGO, ket qua day sang DMA.
 * Goi ham nay SAU khi DMA da cau hinh, va TRUOC khi start Timer.
 */
void ADC1_Init_TimerTrigger(void)
{
    /* 1. ADCCLK = PCLK2 / 6 = 12MHz (ADCPRE = 10b) */
    RCC->CFGR &= ~(0x3 << 14);
    RCC->CFGR |=  (0x2 << 14);

    /* 2. Cap xung ADC1 (bit 9) va GPIOA (bit 2) */
    RCC->APB2ENR |= (1 << 9) | (1 << 2);

    /* 3. PA5 = Analog input (MODE = 00, CNF = 00) */
    GPIOA->CRL &= ~(0xF << 20);

    /* 4. CR1 = 0: khong scan, khong ngat ADC, che do doc lap */
    ADC1->CR1 = 0;

    /* 5. Sample time channel 5 = 239.5 cycles (111b) */
    ADC1->SMPR2 &= ~(0x7 << 15);
    ADC1->SMPR2 |=  (0x7 << 15);

    /* 6. 1 channel duy nhat: L = 0, SQ1 = 5 */
    ADC1->SQR1 &= ~(0xF << 20);
    ADC1->SQR3 &= ~(0x1F << 0);
    ADC1->SQR3 |=  (5 << 0);

    /* 7. CR2: EXTSEL = 100 (TIM3_TRGO), EXTTRIG = 1, DMA = 1, CONT = 0.
     *    Ghi TRUOC khi bat ADON de tranh vo tinh kich chuyen doi.
     */
    ADC1->CR2 = 0;
    ADC1->CR2 |= (0x4 << 17);   // EXTSEL[19:17] = 100
    ADC1->CR2 |= (1 << 20);     // EXTTRIG
    ADC1->CR2 |= (1 << 8);      // DMA
    /* CONT (bit 1) = 0: moi lan trigger chi chuyen doi 1 lan */

    /* 8. Bat ADC (ADON), doi tSTAB */
    ADC1->CR2 |= (1 << 0);
    ADC_DelayMs(1);

    /* 9. Calibration: RSTCAL roi CAL */
    ADC1->CR2 |= (1 << 3);
    while (ADC1->CR2 & (1 << 3));
    ADC1->CR2 |= (1 << 2);
    while (ADC1->CR2 & (1 << 2));

    /* Tu day ADC san sang, cho moi canh TRGO tu TIM3 se chuyen doi 1 mau
     * va DMA tu chep ADC1->DR vao RAM. */
}
