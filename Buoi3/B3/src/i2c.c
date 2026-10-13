#include "i2c.h"

#define I2C_TIMEOUT 1000000UL

static volatile uint8_t i2c_last_error;

void I2C_Init_Config(uint8_t i2c_ch)
{
    i2c_last_error = 0;

    // Bật clock cho AFIO (Alternate Function IO)
    RCC->APB2ENR |= (1 << 0); // Bit 0: AFIOEN

    if (i2c_ch == 1)
    {
        // 1. Cấp clock cho I2C1 (APB1) và GPIOB (APB2)
        RCC->APB1ENR |= (1 << 21); // Bit 21: I2C1EN
        RCC->APB2ENR |= (1 << 3);  // Bit 3: IOPBEN

        // 2. Cấu hình chân GPIOB: PB6 (SCL), PB7 (SDA)
        // Alternate Function Open-Drain, Speed 50MHz -> CNF=11, MODE=11 (0xF)
        GPIOB->CRL &= ~(0xFF << 24); // Xóa cấu hình cũ PB6, PB7
        GPIOB->CRL |=  (0xFF << 24); // PB6 = 0xF (bits 24-27), PB7 = 0xF (bits 28-31)

        // 3. Reset ngoại vi I2C1 để đảm bảo trạng thái ban đầu sạch
        I2C1->CR1 |= (1 << 15);  // Bit 15 SWRST = 1 (Software Reset)
        I2C1->CR1 &= ~(1 << 15); // Bit 15 SWRST = 0 (Thoát Reset)

        // 4. Cấu hình tần số bus APB1 cho I2C (Ví dụ: APB1 = 36 MHz)
        I2C1->CR2 &= ~(0x3F);    // Xóa FREQ[5:0]
        I2C1->CR2 |= 36;         // Gán tần số 36 MHz

        // 5. Cấu hình Tốc độ Chuẩn (Standard Mode 100 kHz)
        // CCR = PCLK1 / (2 * 100kHz) = 36,000,000 / 200,000 = 180 (0xB4)
        I2C1->CCR = 180;

        // 6. Cấu hình thời gian tăng điện áp tối đa TRISE
        // TRISE = (1000ns / (1/36MHz)) + 1 = 36 + 1 = 37
        I2C1->TRISE = 37;

        // 7. Bật module I2C1
        I2C1->CR1 |= (1 << 0);   // Bit 0 PE = 1 (Peripheral Enable)
    }
    else if (i2c_ch == 2)
    {
        // Cấp clock cho I2C2 và GPIOB
        RCC->APB1ENR |= (1 << 22); // Bit 22: I2C2EN
        RCC->APB2ENR |= (1 << 3);  // Bit 3: IOPBEN

        // PB10 (SCL), PB11 (SDA)
        GPIOB->CRH &= ~(0xFF << 8);  // Xóa cấu hình PB10, PB11
        GPIOB->CRH |=  (0xFF << 8);  // Alternate Function Open-Drain 50MHz

        I2C2->CR1 |= (1 << 15);
        I2C2->CR1 &= ~(1 << 15);

        I2C2->CR2 |= 36;
        I2C2->CCR = 180;
        I2C2->TRISE = 37;

        I2C2->CR1 |= (1 << 0);
    }
}

void I2C_Start(I2C_TypeDef *I2Cx)
{
    uint32_t timeout = I2C_TIMEOUT;

    i2c_last_error = 0;
    // Bit 8: START generation
    I2Cx->CR1 |= (1 << 8);

    // Chờ bit SB (Start Bit) trong SR1 lên 1 (xác nhận tín hiệu START đã phát)
    while (!(I2Cx->SR1 & (1 << 0)) && --timeout);
    if (!timeout)
    {
        i2c_last_error = 1;
    }
}

void I2C_Stop(I2C_TypeDef *I2Cx)
{
    // Bit 9: STOP generation
    I2Cx->CR1 |= (1 << 9);
}

void I2C_SendAddress(I2C_TypeDef *I2Cx, uint8_t address, uint8_t rw)
{
    volatile uint32_t temp;
    uint32_t timeout = I2C_TIMEOUT;

    // Ghi địa chỉ (7 bit) kèm bit R/W (bit 0) vào thanh ghi DR
    I2Cx->DR = (address << 1) | (rw & 0x01);

    // Chờ bit ADDR (bit 1 trong SR1) lên 1 (Thiết bị Slave đã ACK địa chỉ)
    while (!(I2Cx->SR1 & (1 << 1)) && --timeout)
    {
        if (I2Cx->SR1 & (1 << 10))
        {
            i2c_last_error = 2;
            I2Cx->CR1 |= (1 << 9);
            return;
        }
    }
    if (!timeout)
    {
        i2c_last_error = 3;
        I2Cx->CR1 |= (1 << 9);
        return;
    }

    // Xóa cờ ADDR bằng cách đọc SR1 tiếp theo đọc SR2 (theo datasheet STM32)
    temp = I2Cx->SR1;
    temp = I2Cx->SR2;
    (void)temp; // Tránh warning unused variable
}

void I2C_WriteByte(I2C_TypeDef *I2Cx, uint8_t data)
{
    uint32_t timeout = I2C_TIMEOUT;

    // Chờ cho đến khi thanh ghi dữ liệu rỗng (Bit 7 TXE = 1)
    while (!(I2Cx->SR1 & (1 << 7)) && --timeout);
    if (!timeout)
    {
        i2c_last_error = 4;
        return;
    }

    // Nạp dữ liệu vào thanh ghi DR
    I2Cx->DR = data;

    // Chờ quá trình truyền dữ liệu hoàn tất (Bit 2 BTF = 1 - Byte Transfer Finished)
    timeout = I2C_TIMEOUT;
    while (!(I2Cx->SR1 & (1 << 2)) && --timeout);
    if (!timeout)
    {
        i2c_last_error = 5;
    }
}

uint8_t I2C_ReadByte(I2C_TypeDef *I2Cx, uint8_t ack)
{
    uint32_t timeout = I2C_TIMEOUT;

    if (ack)
    {
        // Bật bit ACK (Bit 10 trong CR1) để trả lời Slave tiếp tục gửi
        I2Cx->CR1 |= (1 << 10);
    }
    else
    {
        // Tắt bit ACK để thông báo kết thúc nhận (NACK)
        I2Cx->CR1 &= ~(1 << 10);
    }

    // Chờ dữ liệu về đầy thanh ghi DR (Bit 6 RXNE = 1)
    while (!(I2Cx->SR1 & (1 << 6)) && --timeout);
    if (!timeout)
    {
        i2c_last_error = 6;
        return 0;
    }

    return (uint8_t)(I2Cx->DR & 0xFF);
}

uint8_t I2C_GetLastError(void)
{
    return i2c_last_error;
}