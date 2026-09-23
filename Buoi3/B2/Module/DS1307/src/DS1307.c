#include "DS1307.h"
#include "I2C.h"

#define DS1307_ADDR 0x68U

static uint8_t DEC_To_BCD(uint8_t value)
{
    return (uint8_t)(
        ((value / 10U) << 4) |
        (value % 10U)
    );
}

static uint8_t BCD_To_DEC(uint8_t value)
{
    return (uint8_t)(
        ((value >> 4) * 10U) |
        (value & 0x0FU)
    );
}

void DS1307_Init(void)
{
}

uint8_t DS1307_WriteReg(uint8_t reg, uint8_t data)
{
    if (!I2C1_Start())
        return 1;

    if (!I2C1_SendAddress((DS1307_ADDR << 1) | 0U))
    {
        I2C1_Stop();
        return 2;
    }

    if (!I2C1_WriteByte(reg))
    {
        I2C1_Stop();
        return 3;
    }

    if (!I2C1_WriteByte(data))
    {
        I2C1_Stop();
        return 4;
    }

    I2C1_Stop();

    return 0;
}

uint8_t DS1307_ReadReg(
    uint8_t reg,
    uint8_t *data
)
{
    if (data == 0)
        return 1;

    if (!I2C1_Start())
        return 2;

    if (!I2C1_SendAddress((DS1307_ADDR << 1) | 0U))
    {
        I2C1_Stop();
        return 3;
    }

    if (!I2C1_WriteByte(reg))
    {
        I2C1_Stop();
        return 4;
    }

    if (!I2C1_ReStart())
    {
        I2C1_Stop();
        return 5;
    }

    if (!I2C1_SendAddress((DS1307_ADDR << 1) | 1U))
    {
        I2C1_Stop();
        return 6;
    }

    if (!I2C1_ReadBytes(data, 1U))
    {
        I2C1_Stop();
        return 7;
    }

    return 0;
}

uint8_t DS1307_SetTime(
    uint8_t second,
    uint8_t minute,
    uint8_t hour,
    uint8_t date,
    uint8_t month,
    uint8_t year
)
{
    if (!I2C1_Start())
        return 1;

    if (!I2C1_SendAddress((DS1307_ADDR << 1) | 0U))
    {
        I2C1_Stop();
        return 2;
    }

    if (!I2C1_WriteByte(0x00U))
    {
        I2C1_Stop();
        return 3;
    }

    if (!I2C1_WriteByte(
        DEC_To_BCD(second) & 0x7FU))
    {
        I2C1_Stop();
        return 4;
    }

    if (!I2C1_WriteByte(
        DEC_To_BCD(minute)))
    {
        I2C1_Stop();
        return 5;
    }

    if (!I2C1_WriteByte(
        DEC_To_BCD(hour)))
    {
        I2C1_Stop();
        return 6;
    }

    if (!I2C1_WriteByte(0x01U))
    {
        I2C1_Stop();
        return 7;
    }

    if (!I2C1_WriteByte(
        DEC_To_BCD(date)))
    {
        I2C1_Stop();
        return 8;
    }

    if (!I2C1_WriteByte(
        DEC_To_BCD(month)))
    {
        I2C1_Stop();
        return 9;
    }

    if (!I2C1_WriteByte(
        DEC_To_BCD(year)))
    {
        I2C1_Stop();
        return 10;
    }

    I2C1_Stop();

    return 0;
}

uint8_t DS1307_ReadTime(DS1307_Time *time)
{
    uint8_t data[7];

    if (time == 0)
        return 1;

    if (!I2C1_Start())
        return 2;

    if (!I2C1_SendAddress((DS1307_ADDR << 1) | 0U))
    {
        I2C1_Stop();
        return 3;
    }

    if (!I2C1_WriteByte(0x00U))
    {
        I2C1_Stop();
        return 4;
    }

    if (!I2C1_ReStart())
    {
        I2C1_Stop();
        return 5;
    }

    if (!I2C1_SendAddress((DS1307_ADDR << 1) | 1U))
    {
        I2C1_Stop();
        return 6;
    }

    if (!I2C1_ReadBytes(data, 7U))
    {
        I2C1_Stop();
        return 7;
    }

    time->second =
        BCD_To_DEC(data[0] & 0x7FU);

    time->minute =
        BCD_To_DEC(data[1] & 0x7FU);

    if (data[2] & 0x40U)
    {
        time->hour =
            BCD_To_DEC(data[2] & 0x1FU);
    }
    else
    {
        time->hour =
            BCD_To_DEC(data[2] & 0x3FU);
    }

    time->date =
        BCD_To_DEC(data[4] & 0x3FU);

    time->month =
        BCD_To_DEC(data[5] & 0x1FU);

    time->year =
        BCD_To_DEC(data[6]);

    return 0;
}