#include <stdint.h>

#include "RCC.h"
#include "GPIO.h"
#include "AFIO.h"
#include "EXTI.h"
#include "UART.h"
#include "DMA.h"
#include "SYSTICK.h"

#define BUTTON_DEBOUNCE_MS 100

volatile uint32_t button_count = 0;
volatile uint8_t button_event = 0;

static uint8_t dma_buffer[64];

uint32_t String_Append(
    char *dest,
    uint32_t index,
    const char *src
)
{
    uint32_t i = 0;

    while (src[i] != '\0')
    {
        dest[index++] = src[i++];
    }

    return index;
}

uint32_t UInt_To_String(
    uint32_t value,
    char *buffer
)
{
    char temp[12];

    uint32_t i = 0;
    uint32_t j = 0;

    if (value == 0)
    {
        buffer[0] = '0';
        buffer[1] = '\0';
        return 1;
    }

    while (value > 0)
    {
        temp[i++] = (char)('0' + (value % 10));
        value /= 10;
    }

    while (i > 0)
    {
        buffer[j++] = temp[--i];
    }

    buffer[j] = '\0';

    return j;
}

void Create_Message(uint32_t value)
{
    uint32_t index = 0;
    char number[12];

    if (DMA1_Channel4_IsBusy())
    {
        return;
    }

    index = String_Append(
        (char *)dma_buffer,
        index,
        "C01G01:BTN:"
    );

    UInt_To_String(
        value,
        number
    );

    index = String_Append(
        (char *)dma_buffer,
        index,
        number
    );

    dma_buffer[index++] = '\n';
    dma_buffer[index++] = '\r';

    DMA1_Channel4_Send(
        dma_buffer,
        (uint16_t)index
    );
}

void EXTI0_Callback(void)
{
    button_event = 1;
}

int main(void)
{
    uint32_t press_start;
    uint8_t checking_button = 0;

    RCC_Config_72Mhz();

    RCC_Enable_PortA();
    RCC_Enable_AFIO();

    SysTick_Init();

    GPIO_Config(
        GPIOA,
        GPIO_PIN_0,
        GPIO_MODE_INPUT_PU
    );

    UART1_Init(115200);

    DMA1_Channel4_Init();

    EXTI_Init(
        GPIOA,
        GPIO_PIN_0,
        EXTI_FALLING_MODE
    );

    while (1)
    {
        if (button_event)
        {
            button_event = 0;

            if (!checking_button)
            {
                checking_button = 1;

                press_start = SysTick_GetTick();
            }
        }

        if (checking_button)
        {
            if (GPIO_Read_Pin(GPIOA, GPIO_PIN_0) != 0)
            {
                if ((SysTick_GetTick() - press_start)
                    >= BUTTON_DEBOUNCE_MS)
                {
                    checking_button = 0;

                    button_count++;

                    Create_Message(button_count);
                }
            }
            else
            {
                press_start = SysTick_GetTick();
            }
        }
    }
}