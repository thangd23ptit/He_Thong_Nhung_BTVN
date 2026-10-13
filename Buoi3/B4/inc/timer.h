#ifndef __TIMER_H
#define __TIMER_H

#include "stm32f10x.h"

/* TIM3 tao su kien TRGO 100Hz de kich ADC1 */
void TIM3_Init_100Hz(void);
void TIM3_Start(void);

#endif
