#include "I2C.h"
#include "RCC.h"
#include "GPIO.h"

static uint8_t I2C1_WaitBusyClear(void)
{
    uint32_t timeout = 1000000U;

    while (I2C1_SR2 & I2C_SR2_BUSY)
    {
        if (--timeout == 0U)
            return 0;
    }

    return 1;
}

static uint8_t I2C1_WaitSB(void)
{
    uint32_t timeout = 1000000U;

    while (!(I2C1_SR1 & I2C_SR1_SB))
    {
        if (--timeout == 0U)
            return 0;
    }

    return 1;
}

static uint8_t I2C1_WaitADDR(void)
{
    uint32_t timeout = 1000000U;

    while (!(I2C1_SR1 & I2C_SR1_ADDR))
    {
        if (I2C1_SR1 & I2C_SR1_AF)
        {
            I2C1_SR1 &= ~I2C_SR1_AF;
            return 0;
        }

        if (--timeout == 0U)
            return 0;
    }

    return 1;
}

static uint8_t I2C1_WaitTXE(void)
{
    uint32_t timeout = 1000000U;

    while (!(I2C1_SR1 & I2C_SR1_TXE))
    {
        if (I2C1_SR1 & I2C_SR1_AF)
        {
            I2C1_SR1 &= ~I2C_SR1_AF;
            return 0;
        }

        if (--timeout == 0U)
            return 0;
    }

    return 1;
}

static uint8_t I2C1_WaitBTF(void)
{
    uint32_t timeout = 1000000U;

    while (!(I2C1_SR1 & I2C_SR1_BTF))
    {
        if (I2C1_SR1 & I2C_SR1_AF)
        {
            I2C1_SR1 &= ~I2C_SR1_AF;
            return 0;
        }

        if (--timeout == 0U)
            return 0;
    }

    return 1;
}

static uint8_t I2C1_WaitRXNE(void)
{
    uint32_t timeout = 1000000U;

    while (!(I2C1_SR1 & I2C_SR1_RXNE))
    {
        if (--timeout == 0U)
            return 0;
    }

    return 1;
}

void I2C1_Init(void)
{
    RCC_Enable_I2C1();
    RCC_Enable_PortB();

    GPIO_Config(
        GPIOB,
        GPIO_PIN_6,
        GPIO_MODE_AF_OD
    );

    GPIO_Config(
        GPIOB,
        GPIO_PIN_7,
        GPIO_MODE_AF_OD
    );

    I2C1_CR1 = I2C_CR1_SWRST;

    for (volatile uint32_t i = 0; i < 10000U; i++)
    {
    }

    I2C1_CR1 = 0;

    I2C1_CR2 = 36U;

    I2C1_OAR1 = (1U << 14);

    I2C1_OAR2 = 0;

    I2C1_CCR = 180U;

    I2C1_TRISE = 37U;

    I2C1_FLTR = 0;

    I2C1_CR1 = I2C_CR1_PE;

    I2C1_CR1 |= I2C_CR1_ACK;
}

uint8_t I2C1_Start(void)
{
    if (!I2C1_WaitBusyClear())
        return 0;

    I2C1_CR1 |= I2C_CR1_START;

    if (!I2C1_WaitSB())
        return 0;

    return 1;
}

uint8_t I2C1_ReStart(void)
{
    I2C1_CR1 |= I2C_CR1_START;

    if (!I2C1_WaitSB())
        return 0;

    return 1;
}

void I2C1_Stop(void)
{
    I2C1_CR1 |= I2C_CR1_STOP;
}

uint8_t I2C1_SendAddress(uint8_t address)
{
    volatile uint32_t temp;

    I2C1_DR = address;

    if (!I2C1_WaitADDR())
        return 0;

    temp = I2C1_SR1;
    temp = I2C1_SR2;

    (void)temp;

    return 1;
}

uint8_t I2C1_WriteByte(uint8_t data)
{
    I2C1_DR = data;

    if (!I2C1_WaitTXE())
        return 0;

    if (!I2C1_WaitBTF())
        return 0;

    return 1;
}

uint8_t I2C1_ReadBytes(uint8_t *data, uint8_t length)
{
    uint8_t i;

    if (data == 0)
        return 0;

    if (length == 0)
        return 0;

    if (length == 1)
    {
        I2C1_CR1 &= ~I2C_CR1_ACK;
        I2C1_CR1 |= I2C_CR1_STOP;

        if (!I2C1_WaitRXNE())
        {
            I2C1_CR1 |= I2C_CR1_ACK;
            return 0;
        }

        data[0] = (uint8_t)I2C1_DR;

        I2C1_CR1 |= I2C_CR1_ACK;

        return 1;
    }

    if (length == 2)
    {
        I2C1_CR1 &= ~I2C_CR1_ACK;

        if (!I2C1_WaitBTF())
        {
            I2C1_CR1 |= I2C_CR1_ACK;
            return 0;
        }

        I2C1_CR1 |= I2C_CR1_STOP;

        data[0] = (uint8_t)I2C1_DR;
        data[1] = (uint8_t)I2C1_DR;

        I2C1_CR1 |= I2C_CR1_ACK;

        return 1;
    }

    I2C1_CR1 |= I2C_CR1_ACK;

    for (i = 0; i < (uint8_t)(length - 3U); i++)
    {
        if (!I2C1_WaitRXNE())
        {
            I2C1_CR1 |= I2C_CR1_ACK;
            return 0;
        }

        data[i] = (uint8_t)I2C1_DR;
    }

    if (!I2C1_WaitBTF())
    {
        I2C1_CR1 |= I2C_CR1_ACK;
        return 0;
    }

    I2C1_CR1 &= ~I2C_CR1_ACK;

    data[length - 3U] = (uint8_t)I2C1_DR;

    I2C1_CR1 |= I2C_CR1_STOP;

    data[length - 2U] = (uint8_t)I2C1_DR;

    if (!I2C1_WaitRXNE())
    {
        I2C1_CR1 |= I2C_CR1_ACK;
        return 0;
    }

    data[length - 1U] = (uint8_t)I2C1_DR;

    I2C1_CR1 |= I2C_CR1_ACK;

    return 1;
}