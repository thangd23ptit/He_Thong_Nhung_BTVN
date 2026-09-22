#include "TIM.h"
#include "RCC.h"

void TIM3_Init(
    uint16_t prescaler,
    uint16_t period
)
{
    RCC_Enable_TIM3();

    TIM3_CR1 = 0;
    TIM3_CR2 = 0;
    TIM3_SMCR = 0;

    TIM3_PSC = prescaler;
    TIM3_ARR = period;

    TIM3_CNT = 0;

    TIM3_EGR = TIM_EGR_UG;
}

void TIM3_Set_TRGO_Update(void)
{
    TIM3_CR2 &= ~(7U << 4);
    TIM3_CR2 |= (TIM_TRGO_UPDATE << 4);
}

void TIM3_Start(void)
{
    TIM3_CR1 |= TIM_CR1_CEN;
}

void TIM3_Stop(void)
{
    TIM3_CR1 &= ~TIM_CR1_CEN;
}