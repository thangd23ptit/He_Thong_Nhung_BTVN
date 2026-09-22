#ifndef __DMA_H
#define __DMA_H

#include <stdint.h>

#define DMA1_BASE 0x40020000UL

#define DMA1_ISR  (*(volatile uint32_t *)(DMA1_BASE + 0x00UL))
#define DMA1_IFCR (*(volatile uint32_t *)(DMA1_BASE + 0x04UL))

#define DMA1_CCR1  (*(volatile uint32_t *)(DMA1_BASE + 0x08UL))
#define DMA1_CNDTR1 (*(volatile uint32_t *)(DMA1_BASE + 0x0CUL))
#define DMA1_CPAR1  (*(volatile uint32_t *)(DMA1_BASE + 0x10UL))
#define DMA1_CMAR1  (*(volatile uint32_t *)(DMA1_BASE + 0x14UL))

#define DMA1_CCR4  (*(volatile uint32_t *)(DMA1_BASE + 0x44UL))
#define DMA1_CNDTR4 (*(volatile uint32_t *)(DMA1_BASE + 0x48UL))
#define DMA1_CPAR4  (*(volatile uint32_t *)(DMA1_BASE + 0x4CUL))
#define DMA1_CMAR4  (*(volatile uint32_t *)(DMA1_BASE + 0x50UL))

#define DMA_CCR_EN      (1U << 0)
#define DMA_CCR_TCIE    (1U << 1)
#define DMA_CCR_HTIE    (1U << 2)
#define DMA_CCR_TEIE    (1U << 3)
#define DMA_CCR_DIR     (1U << 4)
#define DMA_CCR_CIRC    (1U << 5)
#define DMA_CCR_MINC    (1U << 7)

#define DMA_CCR_PSIZE_8BIT   (0U << 8)
#define DMA_CCR_PSIZE_16BIT  (1U << 8)
#define DMA_CCR_PSIZE_32BIT  (2U << 8)

#define DMA_CCR_MSIZE_8BIT   (0U << 10)
#define DMA_CCR_MSIZE_16BIT  (1U << 10)
#define DMA_CCR_MSIZE_32BIT  (2U << 10)

#define DMA_CCR_PL_LOW        (0U << 12)
#define DMA_CCR_PL_MEDIUM     (1U << 12)
#define DMA_CCR_PL_HIGH       (2U << 12)
#define DMA_CCR_PL_VERY_HIGH  (3U << 12)

#define DMA1_CHANNEL1_IRQ 11U
#define DMA1_CHANNEL4_IRQ 14U

#define DMA1_CH1_GIF   (1U << 0)
#define DMA1_CH1_TCIF  (1U << 1)
#define DMA1_CH1_HTIF  (1U << 2)
#define DMA1_CH1_TEIF  (1U << 3)

#define DMA1_CH4_GIF   (1U << 12)
#define DMA1_CH4_TCIF  (1U << 13)
#define DMA1_CH4_HTIF  (1U << 14)
#define DMA1_CH4_TEIF  (1U << 15)

#define DMA1_CH4_MAX_SIZE 65535U

void DMA1_Channel1_Init(
    volatile uint16_t *buffer,
    uint16_t size
);

void DMA1_Channel1_Start(void);
void DMA1_Channel1_Stop(void);

uint8_t DMA1_Channel1_HalfComplete(void);
uint8_t DMA1_Channel1_TransferComplete(void);

void DMA1_Channel1_IRQHandler(void);

void DMA1_Channel4_Init(void);

uint8_t DMA1_Channel4_Send(
    const uint8_t *buffer,
    uint16_t length
);

uint8_t DMA1_Channel4_IsBusy(void);

void DMA1_Channel4_Stop(void);

void DMA1_Channel4_IRQHandler(void);

#endif