#include "SD.h"
#include "SPI.h"
#include "GPIO.h"
#include "UART.h"

#define SD_CS_PIN GPIO_PIN_4

#define CMD0       0
#define CMD8       8
#define CMD9       9
#define CMD12      12
#define CMD17      17
#define CMD24      24
#define CMD55      55
#define CMD58      58
#define ACMD41     41

static uint8_t sd_type = SD_SDSC;

static void SD_CS_Low(void)
{
    GPIO_Write_Pin(GPIOA, SD_CS_PIN, 0);
}

static void SD_CS_High(void)
{
    GPIO_Write_Pin(GPIOA, SD_CS_PIN, 1);
}

static void SD_SendClock(void)
{
    uint8_t i;

    SD_CS_High();

    for (i = 0; i < 10; i++)
    {
        SPI1_Transfer(0xFF);
    }
}

static uint8_t SD_WaitReady(uint32_t timeout)
{
    while (timeout--)
    {
        if (SPI1_Transfer(0xFF) == 0xFF)
        {
            return 1;
        }
    }

    return 0;
}

static uint8_t SD_SendCommand(
    uint8_t command,
    uint32_t argument,
    uint8_t crc
)
{
    uint8_t response;
    uint8_t i;

    SD_CS_High();
    SPI1_Transfer(0xFF);

    SD_CS_Low();

    SPI1_Transfer(0x40 | command);
    SPI1_Transfer((uint8_t)(argument >> 24));
    SPI1_Transfer((uint8_t)(argument >> 16));
    SPI1_Transfer((uint8_t)(argument >> 8));
    SPI1_Transfer((uint8_t)argument);
    SPI1_Transfer(crc);

    for (i = 0; i < 10; i++)
    {
        response = SPI1_Transfer(0xFF);

        if (!(response & 0x80))
        {
            return response;
        }
    }

    return 0xFF;
}

static uint8_t SD_ReadOCR(uint8_t *ocr)
{
    uint8_t r1;
    uint8_t i;

    r1 = SD_SendCommand(
        CMD58,
        0,
        0x01
    );

    if (r1 != 0)
    {
        SD_CS_High();
        SPI1_Transfer(0xFF);
        return SD_ERROR;
    }

    for (i = 0; i < 4; i++)
    {
        ocr[i] = SPI1_Transfer(0xFF);
    }

    SD_CS_High();
    SPI1_Transfer(0xFF);

    return SD_OK;
}

uint8_t SD_Init(void)
{
    uint8_t r1;
    uint8_t ocr[4];
    uint32_t timeout;

    SD_CS_High();

    SD_SendClock();

    r1 = SD_SendCommand(
        CMD0,
        0,
        0x95
    );

    SD_CS_High();
    SPI1_Transfer(0xFF);

    if (r1 != 0x01)
    {
        return SD_ERROR;
    }

    r1 = SD_SendCommand(
        CMD8,
        0x000001AAUL,
        0x87
    );

    if (r1 == 0x01)
    {
        SPI1_Transfer(0xFF);
        SPI1_Transfer(0xFF);
        SPI1_Transfer(0x01);
        SPI1_Transfer(0xAA);

        SD_CS_High();
        SPI1_Transfer(0xFF);

        timeout = 1000000;

        while (timeout--)
        {
            r1 = SD_SendCommand(
                CMD55,
                0,
                0x01
            );

            SD_CS_High();
            SPI1_Transfer(0xFF);

            r1 = SD_SendCommand(
                ACMD41,
                0x40000000UL,
                0x01
            );

            SD_CS_High();
            SPI1_Transfer(0xFF);

            if (r1 == 0)
            {
                break;
            }
        }

        if (r1 != 0)
        {
            return SD_ERROR;
        }

        if (SD_ReadOCR(ocr) != SD_OK)
        {
            return SD_ERROR;
        }

        if (ocr[0] & 0x40)
        {
            sd_type = SD_SDHC;
        }
        else
        {
            sd_type = SD_SDSC;
        }

        return SD_OK;
    }

    SD_CS_High();
    SPI1_Transfer(0xFF);

    return SD_ERROR;
}

uint8_t SD_ReadBlock(
    uint32_t sector,
    uint8_t *buffer
)
{
    uint32_t address;
    uint32_t timeout;
    uint8_t r1;
    uint16_t i;

    if (sd_type == SD_SDHC)
    {
        address = sector;
    }
    else
    {
        address = sector * 512UL;
    }

    r1 = SD_SendCommand(
        CMD17,
        address,
        0x01
    );

    if (r1 != 0)
    {
        SD_CS_High();
        SPI1_Transfer(0xFF);
        return SD_ERROR;
    }

    timeout = 200000;

    while (timeout--)
    {
        r1 = SPI1_Transfer(0xFF);

        if (r1 == 0xFE)
        {
            break;
        }
    }

    if (r1 != 0xFE)
    {
        SD_CS_High();
        SPI1_Transfer(0xFF);
        return SD_ERROR;
    }

    for (i = 0; i < 512; i++)
    {
        buffer[i] = SPI1_Transfer(0xFF);
    }

    SPI1_Transfer(0xFF);
    SPI1_Transfer(0xFF);

    SD_CS_High();
    SPI1_Transfer(0xFF);

    return SD_OK;
}

uint8_t SD_WriteBlock(
    uint32_t sector,
    const uint8_t *buffer
)
{
    uint32_t address;
    uint32_t timeout;
    uint8_t r1;
    uint8_t token;
    uint16_t i;

    if (sd_type == SD_SDHC)
    {
        address = sector;
    }
    else
    {
        address = sector * 512UL;
    }

    r1 = SD_SendCommand(
        CMD24,
        address,
        0x01
    );

    if (r1 != 0)
    {
        SD_CS_High();
        SPI1_Transfer(0xFF);
        return SD_ERROR;
    }

    if (!SD_WaitReady(200000))
    {
        SD_CS_High();
        SPI1_Transfer(0xFF);
        return SD_ERROR;
    }

    SPI1_Transfer(0xFF);
    SPI1_Transfer(0xFE);

    for (i = 0; i < 512; i++)
    {
        SPI1_Transfer(buffer[i]);
    }

    SPI1_Transfer(0xFF);
    SPI1_Transfer(0xFF);

    token = SPI1_Transfer(0xFF);

    if ((token & 0x1F) != 0x05)
    {
        SD_CS_High();
        SPI1_Transfer(0xFF);
        return SD_ERROR;
    }

    timeout = 200000;

    while (timeout--)
    {
        if (SPI1_Transfer(0xFF) == 0xFF)
        {
            break;
        }
    }

    SD_CS_High();
    SPI1_Transfer(0xFF);

    if (timeout == 0)
    {
        return SD_ERROR;
    }

    return SD_OK;
}

uint8_t SD_GetType(void)
{
    return sd_type;
}