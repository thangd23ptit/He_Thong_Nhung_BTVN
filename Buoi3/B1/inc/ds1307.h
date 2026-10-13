#ifndef __DS1307_H
#define __DS1307_H

#include "stm32f10x.h"
#include "i2c.h"

#define DS1307_I2C_ADDR 0x68 // Địa chỉ 7-bit mặc định của DS1307

// Cấu trúc lưu trữ thời gian
typedef struct {
    uint8_t sec;   // 00 - 59
    uint8_t min;   // 00 - 59
    uint8_t hour;  // 00 - 23 (Chế độ 24h)
    uint8_t day;   // 1 - 7 (Thứ: 1=CN, 2=T2,...)
    uint8_t date;  // 01 - 31
    uint8_t month; // 01 - 12
    uint8_t year;  // 00 - 99 (Vi dụ: 24 = 2024)
} DS1307_Time;

void DS1307_Init(void);
void DS1307_SetTime(DS1307_Time *time);
void DS1307_GetTime(DS1307_Time *time);
uint8_t DS1307_IsHalted(void);                    // 1 = bit CH đang bật (đồng hồ chưa chạy)
void DS1307_GetBuildTime(DS1307_Time *time);      // Thời điểm biên dịch firmware

#endif