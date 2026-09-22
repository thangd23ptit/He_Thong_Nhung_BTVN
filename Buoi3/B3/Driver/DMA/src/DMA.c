#include "DMA.h"
#include "RCC.h"
#include "UART.h"

#define NVIC_ISER0 (*(volatile uint32_t *)0xE000E100UL)

#define USART1_DR_ADDRESS 0x40013804UL

static volatile uint8_t dma_channel4_busy = 0;

void DMA1_Channel4_Init(void)
{
    RCC_Enable_DMA1();

    DMA1_CCR4 &= ~DMA_CCR_EN;

    DMA1_IFCR =
        DMA1_CHANNEL4_TCIF |
        DMA1_CHANNEL4_TEIF;

    DMA1_CPAR4 =
        USART1_DR_ADDRESS;

    DMA1_CNDTR4 = 0;

    DMA1_CMAR4 = 0;

    DMA1_CCR4 =
        DMA_CCR_DIR |
        DMA_CCR_MINC |
        DMA_CCR_PSIZE_8BIT |
        DMA_CCR_MSIZE_8BIT |
        DMA_CCR_PL_HIGH |
        DMA_CCR_TCIE |
        DMA_CCR_TEIE;

    NVIC_ISER0 |=
        (1U << DMA1_CHANNEL4_IRQn);

    UART1_DMA_Enable();

    dma_channel4_busy = 0;
}

uint8_t DMA1_Channel4_Send(
    const uint8_t *buffer,
    uint16_t length
)
{
    if (buffer == 0)
    {
        return 0;
    }

    if (length == 0)
    {
        return 0;
    }

    if (length > DMA1_CHANNEL4_MAX_SIZE)
    {
        return 0;
    }

    if (dma_channel4_busy)
    {
        return 0;
    }

    DMA1_CCR4 &= ~DMA_CCR_EN;

    DMA1_IFCR =
        DMA1_CHANNEL4_TCIF |
        DMA1_CHANNEL4_TEIF;

    DMA1_CPAR4 =
        USART1_DR_ADDRESS;

    DMA1_CMAR4 =
        (uint32_t)buffer;

    DMA1_CNDTR4 =
        length;

    dma_channel4_busy = 1;

    DMA1_CCR4 |= DMA_CCR_EN;

    return 1;
}

uint8_t DMA1_Channel4_IsBusy(void)
{
    return dma_channel4_busy;
}

void DMA1_Channel4_Stop(void)
{
    DMA1_CCR4 &= ~DMA_CCR_EN;

    DMA1_IFCR =
        DMA1_CHANNEL4_TCIF |
        DMA1_CHANNEL4_TEIF;

    dma_channel4_busy = 0;
}

void DMA1_Channel4_IRQHandler(void)
{
    uint32_t status;

    status = DMA1_ISR;

    if (status & DMA1_CHANNEL4_TCIF)
    {
        DMA1_CCR4 &= ~DMA_CCR_EN;

        DMA1_IFCR =
            DMA1_CHANNEL4_TCIF;

        dma_channel4_busy = 0;
    }

    if (status & DMA1_CHANNEL4_TEIF)
    {
        DMA1_CCR4 &= ~DMA_CCR_EN;

        DMA1_IFCR =
            DMA1_CHANNEL4_TEIF;

        dma_channel4_busy = 0;
    }
}