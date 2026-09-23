#include "SYSTICK.h"

#define SYSTICK_CTRL  (*(volatile uint32_t *)0xE000E010UL)
#define SYSTICK_LOAD  (*(volatile uint32_t *)0xE000E014UL)
#define SYSTICK_VAL   (*(volatile uint32_t *)0xE000E018UL)

#define SYSTEM_CLOCK 72000000UL

static volatile uint32_t g_tick = 0;

void SysTick_Init(void)
{
    SYSTICK_LOAD = (SYSTEM_CLOCK / 1000UL) - 1UL;
    SYSTICK_VAL = 0;

    SYSTICK_CTRL =
        (1UL << 2) |
        (1UL << 1) |
        (1UL << 0);
}

void SysTick_Handler(void)
{
    g_tick++;
}

uint32_t SysTick_GetTick(void)
{
    return g_tick;
}

void SysTick_DelayMs(uint32_t ms)
{
    uint32_t start = g_tick;

    while((g_tick - start) < ms)
    {
    }
}