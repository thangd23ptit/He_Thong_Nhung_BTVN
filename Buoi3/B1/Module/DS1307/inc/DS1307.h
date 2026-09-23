#ifndef __DS1307_H
#define __DS1307_H

#include <stdint.h>

#define DS1307_ADDRESS  0x68

#define DS1307_SECONDS  0x00
#define DS1307_MINUTES  0x01
#define DS1307_HOURS    0x02
#define DS1307_DAY      0x03
#define DS1307_DATE     0x04
#define DS1307_MONTH    0x05
#define DS1307_YEAR     0x06

typedef struct
{
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t day;
    uint8_t date;
    uint8_t month;
    uint8_t year;
} DS1307_Time;

void DS1307_Init(void);

uint8_t DS1307_IsReady(void);

uint8_t DS1307_WriteReg(
    uint8_t reg,
    uint8_t data
);

uint8_t DS1307_ReadReg(
    uint8_t reg
);

uint8_t DS1307_SetTime(
    uint8_t date,
    uint8_t month,
    uint8_t year,
    uint8_t hour,
    uint8_t minute,
    uint8_t second
);

uint8_t DS1307_ReadTime(
    DS1307_Time *time
);

#endif