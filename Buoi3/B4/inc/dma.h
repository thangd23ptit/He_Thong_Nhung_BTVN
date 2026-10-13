#ifndef __DMA_H
#define __DMA_H

#include "stm32f10x.h"

/* 100Hz x 1s = 100 mau. Buffer chia 2 nua, moi nua 50 mau (0.5s) */
#define ADC_BUF_SIZE    100
#define ADC_HALF_SIZE   (ADC_BUF_SIZE / 2)

extern volatile uint16_t adc_buf[ADC_BUF_SIZE];
extern volatile uint8_t  dma_half_flag;   // nua dau [0..49] da day
extern volatile uint8_t  dma_full_flag;   // nua sau [50..99] da day

void ADC_DMA_Init(void);

#endif
