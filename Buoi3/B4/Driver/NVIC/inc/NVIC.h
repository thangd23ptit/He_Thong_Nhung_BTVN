#ifndef __NVIC_H
#define __NVIC_H

#include <stdint.h>

#define NVIC_ISER0 (*(volatile uint32_t *)0xE000E100UL)
#define NVIC_ISER1 (*(volatile uint32_t *)0xE000E104UL)

#define NVIC_ICER0 (*(volatile uint32_t *)0xE000E180UL)
#define NVIC_ICER1 (*(volatile uint32_t *)0xE000E184UL)

void NVIC_EnableIRQ(uint8_t IRQn);
void NVIC_DisableIRQ(uint8_t IRQn);

#endif