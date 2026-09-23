#include "GPIO.h"
#include "RCC.h"

static volatile uint32_t *GPIO_CRL(
    GPIO_Port port
)
{
    if (port == GPIO_PORT_A)
        return &GPIOA_CRL;

    if (port == GPIO_PORT_B)
        return &GPIOB_CRL;

    return &GPIOC_CRL;
}

static volatile uint32_t *GPIO_CRH(
    GPIO_Port port
)
{
    if (port == GPIO_PORT_A)
        return &GPIOA_CRH;

    if (port == GPIO_PORT_B)
        return &GPIOB_CRH;

    return &GPIOC_CRH;
}

static volatile uint32_t *GPIO_IDR(
    GPIO_Port port
)
{
    if (port == GPIO_PORT_A)
        return &GPIOA_IDR;

    if (port == GPIO_PORT_B)
        return &GPIOB_IDR;

    return &GPIOC_IDR;
}

static volatile uint32_t *GPIO_ODR(
    GPIO_Port port
)
{
    if (port == GPIO_PORT_A)
        return &GPIOA_ODR;

    if (port == GPIO_PORT_B)
        return &GPIOB_ODR;

    return &GPIOC_ODR;
}

void GPIO_Config(
    GPIO_Port port,
    uint16_t pin,
    uint32_t mode
)
{
    uint8_t position;
    uint32_t config;
    volatile uint32_t *reg;

    if (port == GPIO_PORT_A)
        RCC_Enable_PortA();

    if (port == GPIO_PORT_B)
        RCC_Enable_PortB();

    for (position = 0; position < 16; position++)
    {
        if (!(pin & (1U << position)))
            continue;

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

                *GPIO_ODR(port) |=
                    (1U << position);

                break;

            case GPIO_MODE_INPUT_PD:
                config = 0x08;

                *GPIO_ODR(port) &=
                    ~(1U << position);

                break;

            default:
                config = 0x04;
                break;
        }

        if (position < 8)
        {
            reg = GPIO_CRL(port);

            *reg &= ~(
                0xFU << (position * 4)
            );

            *reg |=
                config << (position * 4);
        }
        else
        {
            reg = GPIO_CRH(port);

            *reg &= ~(
                0xFU << ((position - 8) * 4)
            );

            *reg |=
                config << ((position - 8) * 4);
        }
    }
}

uint8_t GPIO_Read_Pin(
    GPIO_Port port,
    uint16_t pin
)
{
    return (
        (*GPIO_IDR(port) & pin)
        ? 1
        : 0
    );
}

void GPIO_Write_Pin(
    GPIO_Port port,
    uint16_t pin,
    uint8_t state
)
{
    if (state)
        *GPIO_ODR(port) |= pin;
    else
        *GPIO_ODR(port) &= ~pin;
}