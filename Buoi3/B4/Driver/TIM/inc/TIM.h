#ifndef __TIM_H
#define __TIM_H

#include <stdint.h>

#define TIM3_BASE 0x40000400UL

#define TIM3_CR1  (*(volatile uint32_t *)(TIM3_BASE + 0x00UL))
#define TIM3_CR2  (*(volatile uint32_t *)(TIM3_BASE + 0x04UL))
#define TIM3_SMCR (*(volatile uint32_t *)(TIM3_BASE + 0x08UL))
#define TIM3_DIER (*(volatile uint32_t *)(TIM3_BASE + 0x0CUL))
#define TIM3_SR   (*(volatile uint32_t *)(TIM3_BASE + 0x10UL))
#define TIM3_EGR  (*(volatile uint32_t *)(TIM3_BASE + 0x14UL))
#define TIM3_CNT  (*(volatile uint32_t *)(TIM3_BASE + 0x24UL))
#define TIM3_PSC  (*(volatile uint32_t *)(TIM3_BASE + 0x28UL))
#define TIM3_ARR  (*(volatile uint32_t *)(TIM3_BASE + 0x2CUL))

#define TIM_CR1_CEN  (1U << 0)

#define TIM_EGR_UG   (1U << 0)

#define TIM_TRGO_RESET  0U
#define TIM_TRGO_UPDATE 2U

void TIM3_Init(
    uint16_t prescaler,
    uint16_t period
);

void TIM3_Set_TRGO_Update(void);

void TIM3_Start(void);
void TIM3_Stop(void);

#endif