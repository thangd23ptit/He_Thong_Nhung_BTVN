#include "ADC.h"
#include "RCC.h"

void ADC1_Init(void)
{
    RCC_Enable_ADC1();

    ADC1_CR1 = 0;
    ADC1_CR2 = 0;

    ADC1_SQR1 = 0;
    ADC1_SQR2 = 0;
    ADC1_SQR3 = 0;

    ADC1_CR2 |= ADC_CR2_ADON;

    ADC1_Calibrate();
}

void ADC1_Config_Channel(
    uint8_t channel,
    uint8_t sample_time
)
{
    if (channel <= 9U)
    {
        ADC1_SMPR2 &=
            ~(7U << (channel * 3U));

        ADC1_SMPR2 |=
            ((uint32_t)sample_time << (channel * 3U));
    }
    else if (channel <= 17U)
    {
        uint8_t position = channel - 10U;

        ADC1_SMPR1 &=
            ~(7U << (position * 3U));

        ADC1_SMPR1 |=
            ((uint32_t)sample_time << (position * 3U));
    }

    ADC1_SQR1 &= ~(0xFU << 20);

    ADC1_SQR1 |= (0U << 20);

    ADC1_SQR3 &= ~0x1FU;

    ADC1_SQR3 |= channel;
}

void ADC1_Config_Trigger_TIM3(void)
{
    ADC1_CR2 &= ~(7U << 17);

    ADC1_CR2 |= ADC_EXTSEL_TIM3_TRGO;

    ADC1_CR2 |= ADC_CR2_EXTTRIG;
}

void ADC1_DMA_Enable(void)
{
    ADC1_CR2 |= ADC_CR2_DMA;
}

void ADC1_Calibrate(void)
{
    volatile uint32_t delay;

    for (delay = 0; delay < 1000U; delay++)
    {
    }

    ADC1_CR2 |= ADC_CR2_RSTCAL;

    while (ADC1_CR2 & ADC_CR2_RSTCAL)
    {
    }

    ADC1_CR2 |= ADC_CR2_CAL;

    while (ADC1_CR2 & ADC_CR2_CAL)
    {
    }
}

uint16_t ADC1_Read(void)
{
    ADC1_CR2 |= ADC_CR2_ADON;

    while (!(ADC1_SR & (1U << 1)))
    {
    }

    return (uint16_t)ADC1_DR;
}