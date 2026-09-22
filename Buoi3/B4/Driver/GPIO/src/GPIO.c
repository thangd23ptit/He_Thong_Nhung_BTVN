#include "GPIO.h"

void GPIO_Config(
    volatile GPIO_Typedef *GPIOx,
    uint16_t pin,
    uint32_t mode
)
{
    uint32_t position;
    uint32_t config;

    for (position = 0; position < 16; position++)
    {
        if (pin & (1U << position))
        {
            config = 0;

            if (position < 8)
            {
                GPIOx->CRL.REG &= ~(0xFU << (position * 4U));
            }
            else
            {
                GPIOx->CRH.REG &= ~(0xFU << ((position - 8U) * 4U));
            }

            switch (mode)
            {
                case GPIO_MODE_OUTPUT_PP:
                    config = 0x03;
                    break;

                case GPIO_MODE_OUTPUT_OD:
                    config = 0x07;
                    break;

                case GPIO_MODE_AF_PP:
                    config = 0x0B;
                    break;

                case GPIO_MODE_AF_OD:
                    config = 0x0F;
                    break;

                case GPIO_MODE_INPUT_ANALOG:
                    config = 0x00;
                    break;

                case GPIO_MODE_INPUT_FLOATING:
                    config = 0x04;
                    break;

                case GPIO_MODE_INPUT_PU:
                    config = 0x08;
                    GPIOx->ODR.REG |= (1U << position);
                    break;

                case GPIO_MODE_INPUT_PD:
                    config = 0x08;
                    GPIOx->ODR.REG &= ~(1U << position);
                    break;

                default:
                    config = 0;
                    break;
            }

            if (position < 8)
            {
                GPIOx->CRL.REG |=
                    config << (position * 4U);
            }
            else
            {
                GPIOx->CRH.REG |=
                    config << ((position - 8U) * 4U);
            }
        }
    }
}

void GPIO_Write_Pin(
    volatile GPIO_Typedef *GPIOx,
    uint16_t pin,
    uint8_t state
)
{
    if (state)
    {
        GPIOx->BSRR.REG = pin;
    }
    else
    {
        GPIOx->BRR.REG = pin;
    }
}

uint8_t GPIO_Read_Pin(
    volatile GPIO_Typedef *GPIOx,
    uint16_t pin
)
{
    return (GPIOx->IDR.REG & pin) ? 1U : 0U;
}

void GPIO_Toggle_Pin(
    volatile GPIO_Typedef *GPIOx,
    uint16_t pin
)
{
    GPIOx->ODR.REG ^= pin;
}