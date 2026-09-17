#include "RCC.h"
#include "GPIO.h"
#include "SYSTICK.h"

#define LED_01HZ GPIO_PIN_3
#define LED_1HZ  GPIO_PIN_4
#define LED_10HZ GPIO_PIN_5

int main(void)
{
    uint32_t last_01Hz = 0;
    uint32_t last_1Hz = 0;
    uint32_t last_10Hz = 0;

    RCC_Config_72Mhz();

    RCC_Enable_PortA();
    GPIO_Config(GPIOA, LED_01HZ, GPIO_MODE_OUTPUT_PP);
    GPIO_Config(GPIOA, LED_1HZ, GPIO_MODE_OUTPUT_PP);
    GPIO_Config(GPIOA, LED_10HZ, GPIO_MODE_OUTPUT_PP);

    GPIO_Write_Pin(GPIOA, LED_01HZ, 0);
    GPIO_Write_Pin(GPIOA, LED_1HZ, 0);
    GPIO_Write_Pin(GPIOA, LED_10HZ, 0);

    SysTick_Init();

    while(1)
    {
        uint32_t tick = SysTick_GetTick();

        if((tick - last_01Hz) >= 5000)
        {
            last_01Hz = tick;
            GPIO_Toggle_Pin(GPIOA, LED_01HZ);
        }

        if((tick - last_1Hz) >= 500)
        {
            last_1Hz = tick;
            GPIO_Toggle_Pin(GPIOA, LED_1HZ);
        }

        if((tick - last_10Hz) >= 50)
        {
            last_10Hz = tick;
            GPIO_Toggle_Pin(GPIOA, LED_10HZ);
        }
    }
}