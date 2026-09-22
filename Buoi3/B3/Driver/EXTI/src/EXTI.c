#include "EXTI.h"
#include "AFIO.h"
#include "RCC.h"

__attribute__((weak))
void EXTI0_Callback(void)
{
}

__attribute__((weak))
void EXTI1_Callback(void)
{
}

__attribute__((weak))
void EXTI2_Callback(void)
{
}

__attribute__((weak))
void EXTI3_Callback(void)
{
}

__attribute__((weak))
void EXTI4_Callback(void)
{
}

void EXTI_Init(
    volatile GPIO_Typedef *GPIOx,
    uint16_t pin,
    uint8_t mode
)
{
    uint8_t port;
    uint8_t pin_number;

    if (GPIOx == GPIOA)
    {
        port = 0;
    }
    else if (GPIOx == GPIOB)
    {
        port = 1;
    }
    else if (GPIOx == GPIOC)
    {
        port = 2;
    }
    else
    {
        return;
    }

    if (pin == GPIO_PIN_0)
        pin_number = 0;
    else if (pin == GPIO_PIN_1)
        pin_number = 1;
    else if (pin == GPIO_PIN_2)
        pin_number = 2;
    else if (pin == GPIO_PIN_3)
        pin_number = 3;
    else if (pin == GPIO_PIN_4)
        pin_number = 4;
    else if (pin == GPIO_PIN_5)
        pin_number = 5;
    else if (pin == GPIO_PIN_6)
        pin_number = 6;
    else if (pin == GPIO_PIN_7)
        pin_number = 7;
    else if (pin == GPIO_PIN_8)
        pin_number = 8;
    else if (pin == GPIO_PIN_9)
        pin_number = 9;
    else if (pin == GPIO_PIN_10)
        pin_number = 10;
    else if (pin == GPIO_PIN_11)
        pin_number = 11;
    else if (pin == GPIO_PIN_12)
        pin_number = 12;
    else if (pin == GPIO_PIN_13)
        pin_number = 13;
    else if (pin == GPIO_PIN_14)
        pin_number = 14;
    else if (pin == GPIO_PIN_15)
        pin_number = 15;
    else
        return;

    AFIO_EXTI_Config(
        port,
        pin_number
    );

    EXTI->IMR.REG |=
        (1U << pin_number);

    EXTI->RTSR.REG &=
        ~(1U << pin_number);

    EXTI->FTSR.REG &=
        ~(1U << pin_number);

    if (mode == EXTI_RISING_MODE)
    {
        EXTI->RTSR.REG |=
            (1U << pin_number);
    }
    else if (mode == EXTI_FALLING_MODE)
    {
        EXTI->FTSR.REG |=
            (1U << pin_number);
    }
    else if (mode == EXTI_BOTH_MODE)
    {
        EXTI->RTSR.REG |=
            (1U << pin_number);

        EXTI->FTSR.REG |=
            (1U << pin_number);
    }

    EXTI->PR.REG =
        (1U << pin_number);

    if (pin_number <= 4)
    {
        NVIC_ISER0 |=
            (1U << (6U + pin_number));
    }
    else if (pin_number <= 9)
    {
        NVIC_ISER0 |=
            (1U << 23);
    }
    else
    {
        NVIC_ISER1 |=
            (1U << 8);
    }
}

void EXTI0_IRQHandler(void)
{
    if (EXTI->PR.REG & (1U << 0))
    {
        EXTI->PR.REG = (1U << 0);

        EXTI0_Callback();
    }
}

void EXTI1_IRQHandler(void)
{
    if (EXTI->PR.REG & (1U << 1))
    {
        EXTI->PR.REG = (1U << 1);

        EXTI1_Callback();
    }
}

void EXTI2_IRQHandler(void)
{
    if (EXTI->PR.REG & (1U << 2))
    {
        EXTI->PR.REG = (1U << 2);

        EXTI2_Callback();
    }
}

void EXTI3_IRQHandler(void)
{
    if (EXTI->PR.REG & (1U << 3))
    {
        EXTI->PR.REG = (1U << 3);

        EXTI3_Callback();
    }
}

void EXTI4_IRQHandler(void)
{
    if (EXTI->PR.REG & (1U << 4))
    {
        EXTI->PR.REG = (1U << 4);

        EXTI4_Callback();
    }
}