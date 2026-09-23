#include "SPI.h"
#include "RCC.h"
#include "GPIO.h"

#define SPI1_BASE       0x40013000UL

#define SPI1_CR1        (*(volatile uint32_t *)(SPI1_BASE + 0x00UL))
#define SPI1_CR2        (*(volatile uint32_t *)(SPI1_BASE + 0x04UL))
#define SPI1_SR         (*(volatile uint32_t *)(SPI1_BASE + 0x08UL))
#define SPI1_DR         (*(volatile uint32_t *)(SPI1_BASE + 0x0CUL))

#define SPI_CR1_CPHA    (1U << 0)
#define SPI_CR1_CPOL    (1U << 1)
#define SPI_CR1_MSTR    (1U << 2)
#define SPI_CR1_BR_Pos  3
#define SPI_CR1_SPE     (1U << 6)
#define SPI_CR1_LSBFIRST (1U << 7)
#define SPI_CR1_SSI     (1U << 8)
#define SPI_CR1_SSM     (1U << 9)

#define SPI_SR_RXNE     (1U << 0)
#define SPI_SR_TXE      (1U << 1)
#define SPI_SR_BSY      (1U << 7)

void SPI1_Init(void)
{
    RCC_Enable_SPI1();
    RCC_Enable_PortA();

    GPIO_Config(
        GPIOA,
        GPIO_PIN_4,
        GPIO_MODE_OUTPUT_PP
    );

    GPIO_Config(
        GPIOA,
        GPIO_PIN_5,
        GPIO_MODE_AF_PP
    );

    GPIO_Config(
        GPIOA,
        GPIO_PIN_6,
        GPIO_MODE_INPUT_FLOATING
    );

    GPIO_Config(
        GPIOA,
        GPIO_PIN_7,
        GPIO_MODE_AF_PP
    );

    GPIO_Write_Pin(GPIOA, GPIO_PIN_4, 1);

    SPI1_CR1 = 0;
    SPI1_CR2 = 0;

    SPI1_CR1 |= SPI_CR1_MSTR;
    SPI1_CR1 |= SPI_CR1_SSM;
    SPI1_CR1 |= SPI_CR1_SSI;

    SPI1_CR1 |=
        ((uint32_t)SPI1_BAUDRATE_DIV256 << SPI_CR1_BR_Pos);

    SPI1_CR1 &= ~SPI_CR1_CPOL;
    SPI1_CR1 &= ~SPI_CR1_CPHA;
    SPI1_CR1 &= ~SPI_CR1_LSBFIRST;

    SPI1_CR1 |= SPI_CR1_SPE;
}

uint8_t SPI1_Transfer(uint8_t data)
{
    while (!(SPI1_SR & SPI_SR_TXE))
    {
    }

    SPI1_DR = data;

    while (!(SPI1_SR & SPI_SR_RXNE))
    {
    }

    return (uint8_t)SPI1_DR;
}

void SPI1_SendByte(uint8_t data)
{
    SPI1_Transfer(data);
}

uint8_t SPI1_ReadByte(void)
{
    return SPI1_Transfer(0xFF);
}