#ifndef __AFIO_H
#define __AFIO_H

#include <stdint.h>

typedef struct
{
    uint32_t EVCR;
    uint32_t MAPR;
    uint32_t EXTICR[4];
    uint32_t MAPR2;

} AFIO_Typedef;


#define AFIO_BASE       0x40010000UL
#define AFIO            ((volatile AFIO_Typedef *)AFIO_BASE)


#define AFIO_PORT_A     0
#define AFIO_PORT_B     1
#define AFIO_PORT_C     2
#define AFIO_PORT_D     3
#define AFIO_PORT_E     4


#define AFIO_REMAP_SPI1       (1U << 0)
#define AFIO_REMAP_I2C1       (1U << 1)
#define AFIO_REMAP_USART1     (1U << 2)
#define AFIO_REMAP_USART2     (1U << 3)
#define AFIO_REMAP_USART3     (1U << 4)

#define AFIO_REMAP_TIM1       (1U << 6)
#define AFIO_REMAP_TIM2       (1U << 8)
#define AFIO_REMAP_TIM3       (1U << 10)
#define AFIO_REMAP_TIM4       (1U << 12)

#define AFIO_REMAP_CAN        (1U << 13)

#define AFIO_REMAP_PD01       (1U << 15)

#define AFIO_REMAP_SWJ_MASK   (3U << 24)

#define AFIO_SWJ_FULL        (0U << 24)
#define AFIO_SWJ_NOJTAG      (2U << 24)
#define AFIO_SWJ_DISABLED    (4U << 24)


void AFIO_Init(void);

void AFIO_EXTI_Config(uint8_t port, uint8_t pin);

void AFIO_USART1_Remap(uint8_t enable);
void AFIO_USART2_Remap(uint8_t enable);
void AFIO_USART3_Remap(uint8_t enable);

void AFIO_SPI1_Remap(uint8_t enable);
void AFIO_I2C1_Remap(uint8_t enable);

void AFIO_TIM1_Remap(uint8_t value);
void AFIO_TIM2_Remap(uint8_t value);
void AFIO_TIM3_Remap(uint8_t value);

void AFIO_CAN_Remap(uint8_t enable);

void AFIO_SWJ_Config(uint32_t mode);

#endif