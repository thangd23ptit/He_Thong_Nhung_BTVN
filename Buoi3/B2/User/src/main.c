#include "RCC.h"
#include "GPIO.h"
#include "UART.h"
#include "SPI.h"
#include "SD.h"
#include "FAT32.h"

static uint16_t UART_ReadCommand(
    char *buffer,
    uint16_t max
)
{
    uint16_t index = 0;

    while (1)
    {
        char c;

        c = UART1_ReadChar();

        if (c == '!')
        {
            UART1_SendString("\r\n");
            break;
        }

        if (c == '\r' || c == '\n')
        {
            UART1_SendString("\r\n");
            break;
        }

        if (c == '\b')
        {
            if (index > 0)
            {
                index--;

                UART1_SendString("\b \b");
            }

            continue;
        }

        if (index < max - 1)
        {
            buffer[index++] = c;
            UART1_SendChar(c);
        }
    }

    buffer[index] = '\0';

    return index;
}

static void UART_SendHexByte(
    uint8_t value
)
{
    const char hex[] =
        "0123456789ABCDEF";

    UART1_SendChar(
        hex[(value >> 4) & 0x0F]
    );

    UART1_SendChar(
        hex[value & 0x0F]
    );
}

static void SD_TestSector0(void)
{
    uint8_t buffer[512];
    uint16_t i;

    UART1_SendString(
        "\r\n"
        "READ SECTOR 0...\r\n"
    );

    if (
        SD_ReadBlock(
            0,
            buffer
        ) != SD_OK
    )
    {
        UART1_SendString(
            "SECTOR 0 READ ERROR\r\n"
        );

        return;
    }

    UART1_SendString(
        "SECTOR 0 READ OK\r\n"
    );

    UART1_SendString(
        "\r\nSECTOR DATA:\r\n\r\n"
    );

    for (i = 0; i < 64; i++)
    {
        UART_SendHexByte(
            buffer[i]
        );

        UART1_SendChar(' ');

        if ((i & 0x0F) == 0x0F)
        {
            UART1_SendString(
                "\r\n"
            );
        }
    }

    UART1_SendString(
        "\r\nBYTE 510 = "
    );

    UART_SendHexByte(
        buffer[510]
    );

    UART1_SendString(
        "\r\nBYTE 511 = "
    );

    UART_SendHexByte(
        buffer[511]
    );

    UART1_SendString(
        "\r\n"
    );

    UART1_SendString(
        "\r\nBPB:\r\n"
    );

    UART1_SendString(
        "BYTE 0 = "
    );

    UART_SendHexByte(
        buffer[0]
    );

    UART1_SendString(
        "\r\n"
    );

    UART1_SendString(
        "BYTE 11-12 = "
    );

    UART_SendHexByte(
        buffer[11]
    );

    UART1_SendChar(' ');

    UART_SendHexByte(
        buffer[12]
    );

    UART1_SendString(
        "\r\n"
    );

    UART1_SendString(
        "BYTE 13 = "
    );

    UART_SendHexByte(
        buffer[13]
    );

    UART1_SendString(
        "\r\n"
    );

    UART1_SendString(
        "BYTE 14-15 = "
    );

    UART_SendHexByte(
        buffer[14]
    );

    UART1_SendChar(' ');

    UART_SendHexByte(
        buffer[15]
    );

    UART1_SendString(
        "\r\n"
    );

    UART1_SendString(
        "BYTE 16 = "
    );

    UART_SendHexByte(
        buffer[16]
    );

    UART1_SendString(
        "\r\n"
    );

    UART1_SendString(
        "BYTE 32-35 = "
    );

    UART_SendHexByte(
        buffer[32]
    );

    UART1_SendChar(' ');

    UART_SendHexByte(
        buffer[33]
    );

    UART1_SendChar(' ');

    UART_SendHexByte(
        buffer[34]
    );

    UART1_SendChar(' ');

    UART_SendHexByte(
        buffer[35]
    );

    UART1_SendString(
        "\r\n"
    );

    UART1_SendString(
        "BYTE 36-39 = "
    );

    UART_SendHexByte(
        buffer[36]
    );

    UART1_SendChar(' ');

    UART_SendHexByte(
        buffer[37]
    );

    UART1_SendChar(' ');

    UART_SendHexByte(
        buffer[38]
    );

    UART1_SendChar(' ');

    UART_SendHexByte(
        buffer[39]
    );

    UART1_SendString(
        "\r\n"
    );

    UART1_SendString(
        "BYTE 44-47 = "
    );

    UART_SendHexByte(
        buffer[44]
    );

    UART1_SendChar(' ');

    UART_SendHexByte(
        buffer[45]
    );

    UART1_SendChar(' ');

    UART_SendHexByte(
        buffer[46]
    );

    UART1_SendChar(' ');

    UART_SendHexByte(
        buffer[47]
    );

    UART1_SendString(
        "\r\n"
    );
}

static void PrintHelp(void)
{
    UART1_SendString(
        "\r\n"
        "COMMAND:\r\n"
        "W:<DATA>!\r\n"
        "R!\r\n"
        "S!\r\n"
        "H!\r\n"
    );
}

int main(void)
{
    char command[128];
    uint16_t length;

    RCC_Config_72Mhz();

    UART1_Init(115200);

    SPI1_Init();

    UART1_SendString(
        "\r\n"
        "==============================\r\n"
        "STM32F103C8T6\r\n"
        "UART + SPI + FAT32\r\n"
        "==============================\r\n"
    );

    UART1_SendString(
        "CLOCK: 72MHz\r\n"
    );

    UART1_SendString(
        "UART: 115200\r\n"
    );

    UART1_SendString(
        "SPI INIT OK\r\n"
    );

    UART1_SendString(
        "SD INIT...\r\n"
    );

    if (
        SD_Init() != SD_OK
    )
    {
        UART1_SendString(
            "SD INIT ERROR\r\n"
        );

        while (1)
        {
        }
    }

    UART1_SendString(
        "SD INIT OK\r\n"
    );

    if (
        SD_GetType() == SD_SDHC
    )
    {
        UART1_SendString(
            "SD TYPE: SDHC/SDXC\r\n"
        );
    }
    else
    {
        UART1_SendString(
            "SD TYPE: SDSC\r\n"
        );
    }

    SD_TestSector0();

    UART1_SendString(
        "\r\n"
        "FAT32 INIT...\r\n"
    );

    if (
        FAT32_Init() != FAT32_OK
    )
    {
        UART1_SendString(
            "FAT32 INIT ERROR\r\n"
        );

        UART1_SendString(
            "\r\n"
            "SYSTEM STOP\r\n"
        );

        while (1)
        {
        }
    }

    UART1_SendString(
        "FAT32 OK\r\n"
    );

    FAT32_PrintStatus();

    PrintHelp();

    UART1_SendString(
        "\r\n> "
    );

    while (1)
    {
        length =
            UART_ReadCommand(
                command,
                sizeof(command)
            );

        if (length == 0)
        {
            UART1_SendString(
                "\r\n> "
            );

            continue;
        }

        if (
            command[0] == 'W' &&
            command[1] == ':'
        )
        {
            if (
                length <= 2
            )
            {
                UART1_SendString(
                    "EMPTY DATA\r\n"
                );
            }
            else
            {
                if (
                    FAT32_AppendFile(
                        "DATA.TXT",
                        &command[2],
                        length - 2
                    ) != FAT32_OK
                )
                {
                    UART1_SendString(
                        "WRITE ERROR\r\n"
                    );
                }
            }
        }
        else if (
            command[0] == 'R' &&
            command[1] == '\0'
        )
        {
            if (
                FAT32_ReadFile(
                    "DATA.TXT"
                ) != FAT32_OK
            )
            {
                UART1_SendString(
                    "READ ERROR\r\n"
                );
            }
        }
        else if (
            command[0] == 'S' &&
            command[1] == '\0'
        )
        {
            FAT32_PrintStatus();
        }
        else if (
            command[0] == 'H' &&
            command[1] == '\0'
        )
        {
            PrintHelp();
        }
        else
        {
            UART1_SendString(
                "UNKNOWN COMMAND\r\n"
            );
        }

        UART1_SendString(
            "\r\n> "
        );
    }
}