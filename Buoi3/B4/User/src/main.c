#include <stdint.h>

#include "RCC.h"
#include "GPIO.h"
#include "NVIC.h"
#include "UART.h"
#include "TIM.h"
#include "ADC.h"
#include "DMA.h"

#define ADC_BUFFER_SIZE 100U
#define ADC_HALF_SIZE   50U

static volatile uint16_t adc_buffer[ADC_BUFFER_SIZE];

static void ADC_Buffer_Send(
    uint16_t start,
    uint16_t end
)
{
    uint16_t i;

    for (i = start; i < end; i++)
    {
        UART1_SendNumber(adc_buffer[i]);
    }
}

int main(void)
{
    RCC_Config_72Mhz();

    RCC_Enable_PortA();

    GPIO_Config(
        GPIOA,
        GPIO_PIN_1,
        GPIO_MODE_INPUT_ANALOG
    );

    UART1_Init(115200);

    ADC1_Init();

    ADC1_Config_Channel(
        ADC_CHANNEL_1,
        ADC_SAMPLE_239_5
    );

    ADC1_Config_Trigger_TIM3();

    ADC1_DMA_Enable();

    DMA1_Channel1_Init(
        adc_buffer,
        ADC_BUFFER_SIZE
    );

    TIM3_Init(
        7199,
        99
    );

    TIM3_Set_TRGO_Update();

    TIM3_Start();

    while (1)
    {
        if (DMA1_Channel1_HalfComplete())
        {
            ADC_Buffer_Send(
                0,
                ADC_HALF_SIZE
            );
        }

        if (DMA1_Channel1_TransferComplete())
        {
            ADC_Buffer_Send(
                ADC_HALF_SIZE,
                ADC_BUFFER_SIZE
            );
        }
    }
}