#include "ds1307.h"

// Chuyển đổi từ BCD sang Decimal
static uint8_t BCD2DEC(uint8_t bcd)
{
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

// Chuyển đổi từ Decimal sang BCD
static uint8_t DEC2BCD(uint8_t dec)
{
    return ((dec / 10) << 4) | (dec % 10);
}

// Cờ Clock Halt (bit 7 thanh ghi giây) đọc được ở lần GetTime gần nhất
static volatile uint8_t ds1307_halted = 0;

uint8_t DS1307_IsHalted(void)
{
    return ds1307_halted;
}

// Lấy thời điểm biên dịch (macro __DATE__ = "Sep 24 2026", __TIME__ = "09:37:12")
void DS1307_GetBuildTime(DS1307_Time *time)
{
    static const char names[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
    const char *d = __DATE__;
    const char *t = __TIME__;
    uint8_t month = 1;

    for (uint8_t i = 0; i < 12; i++)
    {
        if (d[0] == names[i * 3] && d[1] == names[i * 3 + 1] && d[2] == names[i * 3 + 2])
        {
            month = i + 1;
            break;
        }
    }

    uint8_t  date = ((d[4] == ' ') ? 0 : (d[4] - '0')) * 10 + (d[5] - '0');
    uint16_t year = (d[7] - '0') * 1000 + (d[8] - '0') * 100 + (d[9] - '0') * 10 + (d[10] - '0');

    time->hour  = (t[0] - '0') * 10 + (t[1] - '0');
    time->min   = (t[3] - '0') * 10 + (t[4] - '0');
    time->sec   = (t[6] - '0') * 10 + (t[7] - '0');
    time->date  = date;
    time->month = month;
    time->year  = (uint8_t)(year % 100);

    // Tính thứ trong tuần (Sakamoto): 0 = Chủ nhật -> DS1307 dùng 1 = Chủ nhật
    static const uint8_t tbl[12] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
    uint16_t y = year;
    if (month < 3) y--;
    uint8_t dow = (y + y / 4 - y / 100 + y / 400 + tbl[month - 1] + date) % 7;
    time->day = dow + 1;
}

void DS1307_Init(void)
{
    // Khởi tạo bộ ngoại vi I2C1
    I2C_Init_Config(1);
}

void DS1307_SetTime(DS1307_Time *time)
{
    I2C_Start(I2C1);
    if (I2C_GetLastError() != 0) return;
    I2C_SendAddress(I2C1, DS1307_I2C_ADDR, 0); // Write = 0
    if (I2C_GetLastError() != 0) return;
    I2C_WriteByte(I2C1, 0x00);                // Ghi vào thanh ghi bắt đầu 0x00 (Giây)

    // Bit 7 của thanh ghi Giây (CH - Clock Halt): =0 để kích hoạt RTC chạy
    I2C_WriteByte(I2C1, DEC2BCD(time->sec) & 0x7F); 
    I2C_WriteByte(I2C1, DEC2BCD(time->min));
    I2C_WriteByte(I2C1, DEC2BCD(time->hour) & 0x3F); // Chế độ 24 giờ
    I2C_WriteByte(I2C1, DEC2BCD(time->day));
    I2C_WriteByte(I2C1, DEC2BCD(time->date));
    I2C_WriteByte(I2C1, DEC2BCD(time->month));
    I2C_WriteByte(I2C1, DEC2BCD(time->year));

    I2C_Stop(I2C1);
}

void DS1307_GetTime(DS1307_Time *time)
{
    // 1. Đặt con trỏ thanh ghi DS1307 về 0x00
    I2C_Start(I2C1);
    if (I2C_GetLastError() != 0) return; // Bus lỗi ngay từ START -> dừng sớm

    I2C_SendAddress(I2C1, DS1307_I2C_ADDR, 0); // Write = 0
    if (I2C_GetLastError() != 0) return;       // Không ACK địa chỉ (STOP đã được gửi trong SendAddress)

    I2C_WriteByte(I2C1, 0x00);
    if (I2C_GetLastError() != 0) { I2C_Stop(I2C1); return; }

    // 2. Restart và chuyển sang chế độ Read
    // ACK phải được bật trước khi gửi địa chỉ đọc để STM32 tiếp tục nhận byte.
    I2C1->CR1 |= (1 << 10);
    I2C_Start(I2C1);
    if (I2C_GetLastError() != 0) return;

    I2C_SendAddress(I2C1, DS1307_I2C_ADDR, 1); // Read = 1
    if (I2C_GetLastError() != 0) return;

    // 3. Đọc dữ liệu 7 thanh ghi liên tiếp
    uint8_t rawSec = I2C_ReadByte(I2C1, 1);              // Gửi ACK
    ds1307_halted  = (rawSec >> 7) & 0x01;               // Bit 7 = CH: 1 nghĩa là đồng hồ đang DỪNG
    time->sec      = BCD2DEC(rawSec & 0x7F);
    time->min   = BCD2DEC(I2C_ReadByte(I2C1, 1));        // Gửi ACK
    time->hour  = BCD2DEC(I2C_ReadByte(I2C1, 1) & 0x3F); // Gửi ACK
    time->day   = BCD2DEC(I2C_ReadByte(I2C1, 1));        // Gửi ACK
    time->date  = BCD2DEC(I2C_ReadByte(I2C1, 1));        // Gửi ACK
    time->month = BCD2DEC(I2C_ReadByte(I2C1, 1));        // Byte áp út: vẫn gửi ACK

    // QUAN TRỌNG: theo đúng thủ tục trong Reference Manual (RM0008), để tạo
    // xung NACK cho byte CUỐI, bit ACK phải được xoá và STOP phải được yêu
    // cầu NGAY SAU KHI đọc xong byte áp út (month) - trước khi chờ byte cuối
    // (year), chứ không phải sau khi đọc xong byte cuối như bản cũ.
    I2C1->CR1 &= ~(1 << 10); // Xoá ACK -> byte kế tiếp (cuối cùng) sẽ bị NACK
    I2C1->CR1 |=  (1 << 9);  // Yêu cầu STOP ngay từ bây giờ

    time->year  = BCD2DEC(I2C_ReadByte(I2C1, 0)); // Đọc byte cuối (đã NACK)

    // Chờ STOP hoàn tất để bus rảnh hẳn trước lần START kế tiếp
    {
        uint32_t t = 1000000UL;
        while ((I2C1->CR1 & (1 << 9)) && --t);
    }

    // Chuẩn bị ACK=1 cho lần đọc kế tiếp (không gọi lại I2C_Stop ở đây vì
    // STOP đã được yêu cầu ở trên).
    I2C1->CR1 |= (1 << 10);
}