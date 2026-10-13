#include "timer.h"

/*
 * TIM3 nam tren APB1. Vi PPRE1 = /2 (APB1 = 36MHz) nen clock cap cho Timer = 2 x 36 = 72MHz.
 *   f_update = 72MHz / (PSC+1) / (ARR+1) = 72MHz / 7200 / 100 = 100Hz
 * Moi lan Update event -> TRGO (MMS = 010) -> kich ADC1 (EXTSEL = 100 = TIM3_TRGO)
 */
void TIM3_Init_100Hz(void)
{
    /* 1. Cap xung TIM3 (APB1ENR bit 1) */
    RCC->APB1ENR |= (1 << 1);

    /* 2. Reset cau hinh */
    TIM3->CR1 = 0;
    TIM3->CR2 = 0;

    /* 3. Chia tan so: 72MHz / 7200 = 10kHz, dem den 100 -> 100Hz */
    TIM3->PSC = 7200 - 1;
    TIM3->ARR = 100 - 1;

    /* 4. CR2.MMS[6:4] = 010: Update event dung lam TRGO */
    TIM3->CR2 &= ~(0x7 << 4);
    TIM3->CR2 |=  (0x2 << 4);

    /* 5. Tao update event de nap PSC/ARR vao shadow register, roi xoa co */
    TIM3->EGR |= (1 << 0);      // UG
    TIM3->SR   = 0;
}

void TIM3_Start(void)
{
    TIM3->CNT = 0;
    TIM3->CR1 |= (1 << 0);      // CEN
}
