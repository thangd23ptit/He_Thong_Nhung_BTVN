#include "delay.h"

void SystemClock_Config(void)
{
    /* 1. Bật HSE (thạch anh ngoài 8MHz) */
    RCC->CR |= (1 << 16);                       /* HSEON  */
    while (!(RCC->CR & (1 << 17)));             /* HSERDY */

    /* 2. Flash: bật prefetch, 2 wait state (48MHz < SYSCLK <= 72MHz) */
    FLASH->ACR |= (1 << 4);
    FLASH->ACR &= ~(0x7);
    FLASH->ACR |= (0x2);

    /* 3. Prescaler: AHB = /1, APB1 = /2 (36MHz), APB2 = /1 (72MHz) */
    RCC->CFGR &= ~(0xF << 4);                   /* HPRE  = 0    */
    RCC->CFGR &= ~(0x7 << 8);
    RCC->CFGR |=  (0x4 << 8);                   /* PPRE1 = 100  */
    RCC->CFGR &= ~(0x7 << 11);                  /* PPRE2 = 0    */

    /* 4. PLL: nguồn HSE, không chia, nhân 9 => 72MHz */
    RCC->CFGR |=  (1 << 16);                    /* PLLSRC = HSE */
    RCC->CFGR &= ~(1 << 17);                    /* PLLXTPRE = 0 */
    RCC->CFGR &= ~(0xF << 18);
    RCC->CFGR |=  (0x7 << 18);                  /* PLLMUL = x9  */

    /* 5. Bật PLL, chờ khóa */
    RCC->CR |= (1 << 24);
    while (!(RCC->CR & (1 << 25)));

    /* 6. Chọn PLL làm SYSCLK */
    RCC->CFGR &= ~(0x3);
    RCC->CFGR |=  (0x2);
    while (((RCC->CFGR >> 2) & 0x3) != 0x2);
}

void delay_ms(uint32_t ms)
{
    while (ms--) {
        SysTick->LOAD = 72000 - 1;
        SysTick->VAL  = 0;
        SysTick->CTRL = 5;                      /* ENABLE + CLKSOURCE = AHB */
        while (!(SysTick->CTRL & (1 << 16)));   /* COUNTFLAG */
    }
    SysTick->CTRL = 0;
}
