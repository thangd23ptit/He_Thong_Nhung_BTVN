#ifndef __DELAY_H
#define __DELAY_H

#include "stm32f10x.h"

void SystemClock_Config(void);   /* HSE 8MHz -> PLL x9 = 72MHz */
void delay_ms(uint32_t ms);      /* Tao tre bang SysTick (72MHz) */

#endif
