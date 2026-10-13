#include "dma.h"

volatile uint16_t adc_buf[ADC_BUF_SIZE];
volatile uint8_t  dma_half_flag = 0;
volatile uint8_t  dma_full_flag = 0;

/*
 * ADC1 -> DMA1 Channel 1 (theo bang DMA request cua RM0008)
 * Peripheral -> Memory, circular, 16-bit, ngat Half-Transfer + Transfer-Complete
 */
void ADC_DMA_Init(void)
{
    /* 1. Cap xung DMA1 (AHBENR bit 0) */
    RCC->AHBENR |= (1 << 0);

    /* 2. Tat channel truoc khi cau hinh */
    DMA1_Channel1->CCR = 0;

    /* 3. Dia chi nguon (ADC1->DR), dia chi dich (buffer), so phan tu */
    DMA1_Channel1->CPAR  = (uint32_t)&(ADC1->DR);
    DMA1_Channel1->CMAR  = (uint32_t)adc_buf;
    DMA1_Channel1->CNDTR = ADC_BUF_SIZE;

    /* 4. CCR:
     *   bit1  TCIE  = 1 : ngat Transfer-Complete
     *   bit2  HTIE  = 1 : ngat Half-Transfer
     *   bit4  DIR   = 0 : Peripheral -> Memory
     *   bit5  CIRC  = 1 : circular mode (het buffer tu quay lai dau)
     *   bit6  PINC  = 0 : dia chi ADC->DR co dinh
     *   bit7  MINC  = 1 : dia chi buffer tang dan
     *   bit9:8   PSIZE = 01 : 16-bit
     *   bit11:10 MSIZE = 01 : 16-bit
     *   bit13:12 PL    = 10 : priority high
     */
    DMA1_Channel1->CCR = (1 << 1) | (1 << 2) | (1 << 5) | (1 << 7)
                       | (1 << 8) | (1 << 10) | (2 << 12);

    /* 5. Xoa het co cua channel 1 (GIF1, TCIF1, HTIF1, TEIF1) */
    DMA1->IFCR = 0xF << 0;

    /* 6. Bat ngat DMA1_Channel1 trong NVIC (IRQ11), uu tien 1 */
    NVIC_IPR_BASE[DMA1_Channel1_IRQn] = (1 << 4);
    NVIC_ISER0 = (1UL << DMA1_Channel1_IRQn);

    /* 7. Bat DMA channel */
    DMA1_Channel1->CCR |= (1 << 0);
}

/* Ten ham phai trung voi ten trong bang vector (startup_stm32f103.s) */
void DMA1_Channel1_IRQHandler(void)
{
    uint32_t isr = DMA1->ISR;

    if (isr & (1 << 2))                 // HTIF1: DMA vua ghi xong nua dau
    {
        DMA1->IFCR = (1 << 2);          // CHTIF1
        dma_half_flag = 1;
    }
    if (isr & (1 << 1))                 // TCIF1: DMA vua ghi xong nua sau
    {
        DMA1->IFCR = (1 << 1);          // CTCIF1
        dma_full_flag = 1;
    }
}
