#include "DMA.h"
#include "RCC.h"
#include "NVIC.h"
#include "UART.h"

#define ADC1_DR_ADDRESS    0x4001244CUL
#define USART1_DR_ADDRESS  0x40013804UL

static volatile uint8_t dma_ch4_busy = 0;

static volatile uint8_t dma_ch1_half = 0;
static volatile uint8_t dma_ch1_complete = 0;

void DMA1_Channel1_Init(
    volatile uint16_t *buffer,
    uint16_t size
)
{
    RCC_Enable_DMA1();

    DMA1_CCR1 &= ~DMA_CCR_EN;

    DMA1_IFCR =
        DMA1_CH1_GIF |
        DMA1_CH1_TCIF |
        DMA1_CH1_HTIF |
        DMA1_CH1_TEIF;

    DMA1_CPAR1 =
        ADC1_DR_ADDRESS;

    DMA1_CMAR1 =
        (uint32_t)buffer;

    DMA1_CNDTR1 =
        size;

    DMA1_CCR1 =
        DMA_CCR_CIRC |
        DMA_CCR_MINC |
        DMA_CCR_PSIZE_16BIT |
        DMA_CCR_MSIZE_16BIT |
        DMA_CCR_PL_HIGH |
        DMA_CCR_HTIE |
        DMA_CCR_TCIE |
        DMA_CCR_TEIE;

    dma_ch1_half = 0;
    dma_ch1_complete = 0;

    NVIC_EnableIRQ(DMA1_CHANNEL1_IRQ);

    DMA1_CCR1 |= DMA_CCR_EN;
}

void DMA1_Channel1_Start(void)
{
    DMA1_CCR1 |= DMA_CCR_EN;
}

void DMA1_Channel1_Stop(void)
{
    DMA1_CCR1 &= ~DMA_CCR_EN;
}

uint8_t DMA1_Channel1_HalfComplete(void)
{
    uint8_t status;

    status = dma_ch1_half;

    dma_ch1_half = 0;

    return status;
}

uint8_t DMA1_Channel1_TransferComplete(void)
{
    uint8_t status;

    status = dma_ch1_complete;

    dma_ch1_complete = 0;

    return status;
}

void DMA1_Channel1_IRQHandler(void)
{
    uint32_t status;

    status = DMA1_ISR;

    if (status & DMA1_CH1_HTIF)
    {
        DMA1_IFCR = DMA1_CH1_HTIF;

        dma_ch1_half = 1;
    }

    if (status & DMA1_CH1_TCIF)
    {
        DMA1_IFCR = DMA1_CH1_TCIF;

        dma_ch1_complete = 1;
    }

    if (status & DMA1_CH1_TEIF)
    {
        DMA1_IFCR = DMA1_CH1_TEIF;
    }
}

void DMA1_Channel4_Init(void)
{
    RCC_Enable_DMA1();

    DMA1_CCR4 &= ~DMA_CCR_EN;

    DMA1_IFCR =
        DMA1_CH4_TCIF |
        DMA1_CH4_TEIF;

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

    NVIC_EnableIRQ(DMA1_CHANNEL4_IRQ);

    UART1_DMA_Enable();

    dma_ch4_busy = 0;
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

    if (length > DMA1_CH4_MAX_SIZE)
    {
        return 0;
    }

    if (dma_ch4_busy)
    {
        return 0;
    }

    DMA1_CCR4 &= ~DMA_CCR_EN;

    DMA1_IFCR =
        DMA1_CH4_TCIF |
        DMA1_CH4_TEIF;

    DMA1_CPAR4 =
        USART1_DR_ADDRESS;

    DMA1_CMAR4 =
        (uint32_t)buffer;

    DMA1_CNDTR4 =
        length;

    dma_ch4_busy = 1;

    DMA1_CCR4 |= DMA_CCR_EN;

    return 1;
}

uint8_t DMA1_Channel4_IsBusy(void)
{
    return dma_ch4_busy;
}

void DMA1_Channel4_Stop(void)
{
    DMA1_CCR4 &= ~DMA_CCR_EN;

    DMA1_IFCR =
        DMA1_CH4_TCIF |
        DMA1_CH4_TEIF;

    dma_ch4_busy = 0;
}

void DMA1_Channel4_IRQHandler(void)
{
    uint32_t status;

    status = DMA1_ISR;

    if (status & DMA1_CH4_TCIF)
    {
        DMA1_CCR4 &= ~DMA_CCR_EN;

        DMA1_IFCR =
            DMA1_CH4_TCIF;

        dma_ch4_busy = 0;
    }

    if (status & DMA1_CH4_TEIF)
    {
        DMA1_CCR4 &= ~DMA_CCR_EN;

        DMA1_IFCR =
            DMA1_CH4_TEIF;

        dma_ch4_busy = 0;
    }
}