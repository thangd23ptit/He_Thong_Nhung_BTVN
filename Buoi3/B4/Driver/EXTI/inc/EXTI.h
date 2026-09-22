#ifndef __EXTI_H
#define __EXTI_H

#include <stdint.h>
#include "GPIO.h"

#define EXTI_BASE 0x40010400UL

#define NVIC_ISER0 (*(volatile uint32_t *)0xE000E100UL)
#define NVIC_ISER1 (*(volatile uint32_t *)0xE000E104UL)

#define EXTI_RISING_MODE  0x00
#define EXTI_FALLING_MODE 0x01
#define EXTI_BOTH_MODE    0x02

typedef struct
{
    volatile uint32_t REG;
} EXTI_REG;

typedef struct
{
    EXTI_REG IMR;
    EXTI_REG EMR;
    EXTI_REG RTSR;
    EXTI_REG FTSR;
    EXTI_REG SWIER;
    EXTI_REG PR;
} EXTI_TypeDef;

#define EXTI ((volatile EXTI_TypeDef *)EXTI_BASE)

void EXTI_Init(
    volatile GPIO_Typedef *GPIOx,
    uint16_t pin,
    uint8_t mode
);

void EXTI0_Callback(void);
void EXTI1_Callback(void);
void EXTI2_Callback(void);
void EXTI3_Callback(void);
void EXTI4_Callback(void);

#endif