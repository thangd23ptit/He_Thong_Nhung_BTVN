#include "DS1307.h"
#include "I2C.h"

static uint8_t DEC_To_BCD(
    uint8_t value
)
{
    return (uint8_t)(
        ((value / 10U) << 4) |
        (value % 10U)
    );
}

static uint8_t BCD_To_DEC(
    uint8_t value
)
{
    return (uint8_t)(
        ((value >> 4) * 10U) +
        (value & 0x0FU)
    );
}

void DS1307_Init(void)
{
    I2C1_Init();
}

uint8_t DS1307_IsReady(void)
{
    if (!I2C1_Start())
    {
        return 0;
    }

    if (!I2C1_SendAddress(
            (DS1307_ADDRESS << 1) | 0
        ))
    {
        I2C1_Stop();

        return 0;
    }

    I2C1_Stop();

    return 1;
}

uint8_t DS1307_WriteReg(
    uint8_t reg,
    uint8_t data
)
{
    if (!I2C1_Start())
    {
        return 0;
    }

    if (!I2C1_SendAddress(
            (DS1307_ADDRESS << 1) | 0
        ))
    {
        I2C1_Stop();

        return 0;
    }

    if (!I2C1_WriteByte(reg))
    {
        I2C1_Stop();

        return 0;
    }

    if (!I2C1_WriteByte(data))
    {
        I2C1_Stop();

        return 0;
    }

    I2C1_Stop();

    return 1;
}

uint8_t DS1307_ReadReg(
    uint8_t reg
)
{
    uint8_t data;

    if (!I2C1_Start())
    {
        return 0xFF;
    }

    if (!I2C1_SendAddress(
            (DS1307_ADDRESS << 1) | 0
        ))
    {
        I2C1_Stop();

        return 0xFF;
    }

    if (!I2C1_WriteByte(reg))
    {
        I2C1_Stop();

        return 0xFF;
    }

    I2C1_CR1 |=
        I2C_CR1_START;

    while (
        !(I2C1_SR1 & I2C_SR1_SB)
    )
    {
    }

    if (!I2C1_SendAddress(
            (DS1307_ADDRESS << 1) | 1
        ))
    {
        I2C1_Stop();

        return 0xFF;
    }

    I2C1_CR1 &=
        ~I2C_CR1_ACK;

    data =
        I2C1_ReadByte(0);

    I2C1_Stop();

    I2C1_CR1 |=
        I2C_CR1_ACK;

    return data;
}

uint8_t DS1307_SetTime(
    uint8_t date,
    uint8_t month,
    uint8_t year,
    uint8_t hour,
    uint8_t minute,
    uint8_t second
)
{
    if (!I2C1_Start())
    {
        return 0;
    }

    if (!I2C1_SendAddress(
            (DS1307_ADDRESS << 1) | 0
        ))
    {
        I2C1_Stop();

        return 0;
    }

    if (!I2C1_WriteByte(
            DS1307_SECONDS
        ))
    {
        I2C1_Stop();

        return 0;
    }

    if (!I2C1_WriteByte(
            DEC_To_BCD(second) & 0x7F
        ))
    {
        I2C1_Stop();

        return 0;
    }

    if (!I2C1_WriteByte(
            DEC_To_BCD(minute)
        ))
    {
        I2C1_Stop();

        return 0;
    }

    if (!I2C1_WriteByte(
            DEC_To_BCD(hour) & 0x3F
        ))
    {
        I2C1_Stop();

        return 0;
    }

    if (!I2C1_WriteByte(1))
    {
        I2C1_Stop();

        return 0;
    }

    if (!I2C1_WriteByte(
            DEC_To_BCD(date)
        ))
    {
        I2C1_Stop();

        return 0;
    }

    if (!I2C1_WriteByte(
            DEC_To_BCD(month)
        ))
    {
        I2C1_Stop();

        return 0;
    }

    if (!I2C1_WriteByte(
            DEC_To_BCD(year)
        ))
    {
        I2C1_Stop();

        return 0;
    }

    I2C1_Stop();

    return 1;
}

uint8_t DS1307_ReadTime(
    DS1307_Time *time
)
{
    if (time == 0)
    {
        return 0;
    }

    if (!I2C1_Start())
    {
        return 0;
    }

    if (!I2C1_SendAddress(
            (DS1307_ADDRESS << 1) | 0
        ))
    {
        I2C1_Stop();

        return 0;
    }

    if (!I2C1_WriteByte(
            DS1307_SECONDS
        ))
    {
        I2C1_Stop();

        return 0;
    }

    I2C1_CR1 |=
        I2C_CR1_START;

    while (
        !(I2C1_SR1 & I2C_SR1_SB)
    )
    {
    }

    if (!I2C1_SendAddress(
            (DS1307_ADDRESS << 1) | 1
        ))
    {
        I2C1_Stop();

        return 0;
    }

    I2C1_CR1 |=
        I2C_CR1_ACK;

    time->second =
        I2C1_ReadByte(1);

    time->minute =
        I2C1_ReadByte(1);

    time->hour =
        I2C1_ReadByte(1);

    time->day =
        I2C1_ReadByte(1);

    time->date =
        I2C1_ReadByte(1);

    time->month =
        I2C1_ReadByte(1);

    I2C1_CR1 &=
        ~I2C_CR1_ACK;

    time->year =
        I2C1_ReadByte(0);

    I2C1_Stop();

    I2C1_CR1 |=
        I2C_CR1_ACK;

    time->second =
        BCD_To_DEC(
            time->second & 0x7F
        );

    time->minute =
        BCD_To_DEC(
            time->minute & 0x7F
        );

    time->hour =
        BCD_To_DEC(
            time->hour & 0x3F
        );

    time->day =
        BCD_To_DEC(
            time->day & 0x07
        );

    time->date =
        BCD_To_DEC(
            time->date & 0x3F
        );

    time->month =
        BCD_To_DEC(
            time->month & 0x1F
        );

    time->year =
        BCD_To_DEC(
            time->year
        );

    return 1;
}