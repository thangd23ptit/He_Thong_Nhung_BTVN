#include "TIM.h"
#include "GPIO.h"

void TIM2_PWM_Init(void)
{
    TIM2->PSC = 71;

    TIM2->ARR = 999;

    TIM2->CCMR1 = 0;
    TIM2->CCMR2 = 0;

    TIM2->CCMR1 |= (6 << 4);
    TIM2->CCMR1 |= (1 << 3);

    TIM2->CCMR1 |= (6 << 12);
    TIM2->CCMR1 |= (1 << 11);

    TIM2->CCMR2 |= (6 << 4);
    TIM2->CCMR2 |= (1 << 3);

    TIM2->CCMR2 |= (6 << 12);
    TIM2->CCMR2 |= (1 << 11);

    TIM2->CCER |= (1 << 0);
    TIM2->CCER |= (1 << 4);
    TIM2->CCER |= (1 << 8);
    TIM2->CCER |= (1 << 12);

    TIM2->CR1 |= (1 << 7);

    TIM2->EGR |= (1 << 0);

    TIM2_Start();
}


void TIM2_PWM_SetDuty(uint8_t channel, uint8_t duty)
{
    uint16_t value;

    if (duty > 100)
        duty = 100;

    value = ((TIM2->ARR + 1) * duty) / 100;

    switch (channel)
    {
        case TIM2_CH1:
            TIM2->CCR1 = value;
            break;

        case TIM2_CH2:
            TIM2->CCR2 = value;
            break;

        case TIM2_CH3:
            TIM2->CCR3 = value;
            break;

        case TIM2_CH4:
            TIM2->CCR4 = value;
            break;

        default:
            break;
    }
}


void TIM2_Start(void)
{
    TIM2->CR1 |= (1 << 0);
}


void TIM2_Stop(void)
{
    TIM2->CR1 &= ~(1 << 0);
}