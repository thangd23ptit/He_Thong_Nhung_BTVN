#include "I2C.h"
#include "RCC.h"
#include "GPIO.h"

uint8_t I2C1_Init(void)
{
    RCC_Enable_I2C1();
    RCC_Enable_PortB();

    GPIO_Config(
        GPIO_PORT_B,
        GPIO_PIN_6,
        GPIO_MODE_AF_OD
    );

    GPIO_Config(
        GPIO_PORT_B,
        GPIO_PIN_7,
        GPIO_MODE_AF_OD
    );

    I2C1_CR1 =
        I2C_CR1_SWRST;

    I2C1_CR1 = 0;

    I2C1_CR2 = 36;

    I2C1_OAR1 =
        (1U << 14);

    I2C1_OAR2 = 0;

    I2C1_CCR = 180;

    I2C1_TRISE = 37;

    I2C1_CR1 =
        I2C_CR1_PE |
        I2C_CR1_ACK;

    return 1;
}

uint8_t I2C1_Start(void)
{
    uint32_t timeout = 1000000UL;

    while (
        (I2C1_SR2 & I2C_SR2_BUSY) &&
        timeout
    )
    {
        timeout--;
    }

    if (timeout == 0)
    {
        return 0;
    }

    I2C1_CR1 |=
        I2C_CR1_START;

    timeout = 1000000UL;

    while (
        !(I2C1_SR1 & I2C_SR1_SB) &&
        timeout
    )
    {
        timeout--;
    }

    if (timeout == 0)
    {
        return 0;
    }

    return 1;
}

void I2C1_Stop(void)
{
    I2C1_CR1 |=
        I2C_CR1_STOP;
}

uint8_t I2C1_SendAddress(
    uint8_t address
)
{
    uint32_t timeout = 1000000UL;

    I2C1_DR = address;

    while (
        !(I2C1_SR1 & I2C_SR1_ADDR) &&
        timeout
    )
    {
        if (I2C1_SR1 & I2C_SR1_AF)
        {
            I2C1_SR1 &=
                ~I2C_SR1_AF;

            return 0;
        }

        timeout--;
    }

    if (timeout == 0)
    {
        return 0;
    }

    (void)I2C1_SR1;
    (void)I2C1_SR2;

    return 1;
}

uint8_t I2C1_WriteByte(
    uint8_t data
)
{
    uint32_t timeout;

    I2C1_DR = data;

    timeout = 1000000UL;

    while (
        !(I2C1_SR1 & I2C_SR1_TXE) &&
        timeout
    )
    {
        if (I2C1_SR1 & I2C_SR1_AF)
        {
            I2C1_SR1 &=
                ~I2C_SR1_AF;

            return 0;
        }

        timeout--;
    }

    if (timeout == 0)
    {
        return 0;
    }

    timeout = 1000000UL;

    while (
        !(I2C1_SR1 & I2C_SR1_BTF) &&
        timeout
    )
    {
        if (I2C1_SR1 & I2C_SR1_AF)
        {
            I2C1_SR1 &=
                ~I2C_SR1_AF;

            return 0;
        }

        timeout--;
    }

    if (timeout == 0)
    {
        return 0;
    }

    return 1;
}

uint8_t I2C1_ReadByte(
    uint8_t ack
)
{
    uint32_t timeout =
        1000000UL;

    uint8_t data;

    if (ack)
    {
        I2C1_CR1 |=
            I2C_CR1_ACK;
    }
    else
    {
        I2C1_CR1 &=
            ~I2C_CR1_ACK;
    }

    while (
        !(I2C1_SR1 & I2C_SR1_RXNE) &&
        timeout
    )
    {
        timeout--;
    }

    if (timeout == 0)
    {
        I2C1_CR1 |=
            I2C_CR1_ACK;

        return 0;
    }

    data =
        (uint8_t)I2C1_DR;

    return data;
}