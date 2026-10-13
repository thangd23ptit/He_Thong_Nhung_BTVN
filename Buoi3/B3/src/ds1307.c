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

void DS1307_Init(void)
{
    // Khởi tạo bộ ngoại vi I2C1
    I2C_Init_Config(1);
}

void DS1307_SetTime(DS1307_Time *time)
{
    I2C_Start(I2C1);
    I2C_SendAddress(I2C1, DS1307_I2C_ADDR, 0); // Write = 0
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
    I2C_SendAddress(I2C1, DS1307_I2C_ADDR, 0); // Write = 0
    I2C_WriteByte(I2C1, 0x00);

    // 2. Restart và chuyển sang chế độ Read
    // ACK phải được bật trước khi gửi địa chỉ đọc để STM32 tiếp tục nhận byte.
    I2C1->CR1 |= (1 << 10);
    I2C_Start(I2C1);
    I2C_SendAddress(I2C1, DS1307_I2C_ADDR, 1); // Read = 1

    // 3. Đọc dữ liệu 7 thanh ghi liên tiếp
    time->sec   = BCD2DEC(I2C_ReadByte(I2C1, 1) & 0x7F); // Gửi ACK
    time->min   = BCD2DEC(I2C_ReadByte(I2C1, 1));        // Gửi ACK
    time->hour  = BCD2DEC(I2C_ReadByte(I2C1, 1) & 0x3F); // Gửi ACK
    time->day   = BCD2DEC(I2C_ReadByte(I2C1, 1));        // Gửi ACK
    time->date  = BCD2DEC(I2C_ReadByte(I2C1, 1));        // Gửi ACK
    time->month = BCD2DEC(I2C_ReadByte(I2C1, 1));        // Gửi ACK
    time->year  = BCD2DEC(I2C_ReadByte(I2C1, 0));        // Gửi NACK cho byte cuối

    I2C_Stop(I2C1);
    // Chuẩn bị ACK cho lần đọc kế tiếp.
    I2C1->CR1 |= (1 << 10);
}