#ifndef __SYSTICK_H
#define __SYSTICK_H

#include <stdint.h>

void SysTick_Init(void);
void SysTick_Handler(void);
uint32_t SysTick_GetTick(void);
void SysTick_DelayMs(uint32_t ms);

#endif