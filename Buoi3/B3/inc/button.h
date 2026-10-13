#ifndef __BUTTON_H
#define __BUTTON_H

#include "stm32f10x.h"

void Button_Init(void);
uint8_t Button_IsPressed(void);
void Led_Init(void);
void Led_Set(uint8_t on);

#endif