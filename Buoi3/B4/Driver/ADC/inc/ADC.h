#ifndef __ADC_H
#define __ADC_H

#include <stdint.h>

#define ADC1_BASE 0x40012400UL

#define ADC1_SR     (*(volatile uint32_t *)(ADC1_BASE + 0x00UL))
#define ADC1_CR1    (*(volatile uint32_t *)(ADC1_BASE + 0x04UL))
#define ADC1_CR2    (*(volatile uint32_t *)(ADC1_BASE + 0x08UL))
#define ADC1_SMPR1  (*(volatile uint32_t *)(ADC1_BASE + 0x0CUL))
#define ADC1_SMPR2  (*(volatile uint32_t *)(ADC1_BASE + 0x10UL))
#define ADC1_JOFR1  (*(volatile uint32_t *)(ADC1_BASE + 0x14UL))
#define ADC1_JOFR2  (*(volatile uint32_t *)(ADC1_BASE + 0x18UL))
#define ADC1_JOFR3  (*(volatile uint32_t *)(ADC1_BASE + 0x1CUL))
#define ADC1_JOFR4  (*(volatile uint32_t *)(ADC1_BASE + 0x20UL))
#define ADC1_HTR    (*(volatile uint32_t *)(ADC1_BASE + 0x24UL))
#define ADC1_LTR    (*(volatile uint32_t *)(ADC1_BASE + 0x28UL))
#define ADC1_SQR1   (*(volatile uint32_t *)(ADC1_BASE + 0x2CUL))
#define ADC1_SQR2   (*(volatile uint32_t *)(ADC1_BASE + 0x30UL))
#define ADC1_SQR3   (*(volatile uint32_t *)(ADC1_BASE + 0x34UL))
#define ADC1_JSQR   (*(volatile uint32_t *)(ADC1_BASE + 0x38UL))
#define ADC1_JDR1   (*(volatile uint32_t *)(ADC1_BASE + 0x3CUL))
#define ADC1_JDR2   (*(volatile uint32_t *)(ADC1_BASE + 0x40UL))
#define ADC1_JDR3   (*(volatile uint32_t *)(ADC1_BASE + 0x44UL))
#define ADC1_JDR4   (*(volatile uint32_t *)(ADC1_BASE + 0x48UL))
#define ADC1_DR     (*(volatile uint32_t *)(ADC1_BASE + 0x4CUL))

#define ADC_CHANNEL_0    0U
#define ADC_CHANNEL_1    1U
#define ADC_CHANNEL_2    2U
#define ADC_CHANNEL_3    3U
#define ADC_CHANNEL_4    4U
#define ADC_CHANNEL_5    5U
#define ADC_CHANNEL_6    6U
#define ADC_CHANNEL_7    7U
#define ADC_CHANNEL_8    8U
#define ADC_CHANNEL_9    9U
#define ADC_CHANNEL_10   10U
#define ADC_CHANNEL_11   11U
#define ADC_CHANNEL_12   12U
#define ADC_CHANNEL_13   13U
#define ADC_CHANNEL_14   14U
#define ADC_CHANNEL_15   15U
#define ADC_CHANNEL_16   16U
#define ADC_CHANNEL_17   17U

#define ADC_SAMPLE_1_5    0U
#define ADC_SAMPLE_7_5    1U
#define ADC_SAMPLE_13_5   2U
#define ADC_SAMPLE_28_5   3U
#define ADC_SAMPLE_41_5   4U
#define ADC_SAMPLE_55_5   5U
#define ADC_SAMPLE_71_5   6U
#define ADC_SAMPLE_239_5  7U

#define ADC_CR2_ADON       (1U << 0)
#define ADC_CR2_CAL        (1U << 2)
#define ADC_CR2_RSTCAL     (1U << 3)
#define ADC_CR2_DMA        (1U << 8)
#define ADC_CR2_EXTTRIG    (1U << 20)

#define ADC_EXTSEL_TIM3_TRGO (4U << 17)

void ADC1_Init(void);

void ADC1_Config_Channel(
    uint8_t channel,
    uint8_t sample_time
);

void ADC1_Config_Trigger_TIM3(void);

void ADC1_DMA_Enable(void);

void ADC1_Calibrate(void);

uint16_t ADC1_Read(void);

#endif