#ifndef __DS1307_H
#define __DS1307_H

#include <stdint.h>

typedef struct
{
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t date;
    uint8_t month;
    uint8_t year;
} DS1307_Time;

void DS1307_Init(void);

uint8_t DS1307_SetTime(
    uint8_t second,
    uint8_t minute,
    uint8_t hour,
    uint8_t date,
    uint8_t month,
    uint8_t year
);

uint8_t DS1307_ReadTime(DS1307_Time *time);

uint8_t DS1307_WriteReg(uint8_t reg, uint8_t data);

uint8_t DS1307_ReadReg(
    uint8_t reg,
    uint8_t *data
);

#endif