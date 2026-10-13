#include "i2c.h"

#define I2C_TIMEOUT 1000000UL

static volatile uint8_t i2c_last_error;

// ==========================================================================
// I2C BUS RECOVERY
// ==========================================================================
// STM32F1 có nhược điểm phần cứng đã biết: nếu bus I2C bị lỗi giữa chừng
// (mất ACK, nhiễu, hoặc Slave giữ SDA ở mức thấp lúc cấp nguồn), cờ BUSY
// nội bộ của khối I2C có thể bị "kẹt" ở mức 1 vĩnh viễn. Khi đó I2C_Start()
// sẽ không bao giờ thấy cờ SB được set (vì phần cứng từ chối phát START khi
// thấy bus "đang bận"), khiến chương trình phải bấm nút Reset cứng mới chạy
// lại được. Hàm dưới đây "bit-bang" thủ công bằng GPIO thường (không qua
// khối I2C phần cứng) để: (1) tạo tối đa 9 xung SCL nhằm ép Slave nhả SDA,
// (2) phát 1 điều kiện STOP thủ công để đưa bus về trạng thái rảnh (idle)
// trước khi cấu hình lại chân về chế độ Alternate Function cho khối I2C.
static void I2C1_Bus_Recovery(void)
{
    RCC->APB2ENR |= (1 << 3); // IOPBEN: bật clock GPIOB

    // Cấu hình tạm PB6 (SCL), PB7 (SDA) làm GPIO Output Open-Drain 50MHz
    // (MODE=11, CNF=01 => giá trị 0x7 cho mỗi chân) để có thể điều khiển tay
    GPIOB->CRL &= ~(0xFFUL << 24);
    GPIOB->CRL |=  (0x77UL << 24);

    // Thả cả 2 chân lên mức cao (open-drain: thả = kéo lên nhờ pull-up)
    GPIOB->BSRR = (1 << 6) | (1 << 7);
    for (volatile int d = 0; d < 2000; d++);

    // Nếu SDA (PB7) đang bị kéo xuống thấp bởi Slave, tạo tối đa 9 xung
    // clock trên SCL (PB6) để ép Slave nhả SDA ra (theo chuẩn hồi phục I2C)
    for (int i = 0; i < 9; i++)
    {
        if (GPIOB->IDR & (1 << 7)) break; // SDA đã lên cao -> bus đã rảnh

        GPIOB->BSRR = (1 << 22); // SCL = 0  (bit reset của BS6, offset +16)
        for (volatile int d = 0; d < 500; d++);
        GPIOB->BSRR = (1 << 6);  // SCL = 1
        for (volatile int d = 0; d < 500; d++);
    }

    // Phát 1 điều kiện STOP thủ công (SDA đi từ thấp lên cao trong khi SCL cao)
    GPIOB->BSRR = (1 << 23); // SDA = 0 (bit reset của BS7, offset +16)
    for (volatile int d = 0; d < 500; d++);
    GPIOB->BSRR = (1 << 6);  // đảm bảo SCL = 1
    for (volatile int d = 0; d < 500; d++);
    GPIOB->BSRR = (1 << 7);  // SDA = 1 -> STOP, bus trở về idle
    for (volatile int d = 0; d < 500; d++);
}

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

        // 1b. Giải phóng bus trước khi cấu hình phần cứng I2C (xem giải thích ở trên)
        I2C1_Bus_Recovery();

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
    uint32_t timeout = I2C_TIMEOUT;

    // Bit 9: STOP generation
    I2Cx->CR1 |= (1 << 9);

    // Chờ phần cứng xóa bit STOP (bus đã thực sự về trạng thái rảnh)
    while ((I2Cx->CR1 & (1 << 9)) && --timeout);
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
        if (I2Cx->SR1 & (1 << 10))       // Bit 10 AF: Slave không ACK địa chỉ
        {
            i2c_last_error = 2;
            I2Cx->SR1 &= ~(1 << 10);     // Xóa cờ AF (nếu không xóa sẽ lỗi mãi)
            I2C_Stop(I2Cx);
            return;
        }
    }
    if (!timeout)
    {
        i2c_last_error = 3;
        I2C_Stop(I2Cx);
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

// ==========================================================================
// HÀM CHẨN ĐOÁN
// ==========================================================================
// Đọc mức logic thực tế của PB6 (SCL) và PB7 (SDA).
// bit0 = SCL, bit1 = SDA. Khi bus rảnh và có điện trở kéo lên thì cả 2 phải = 1.
uint8_t I2C1_GetLineState(void)
{
    return (uint8_t)((GPIOB->IDR >> 6) & 0x03);
}

// Thăm dò 1 địa chỉ 7-bit trên bus.
// Trả về: 1 = có thiết bị ACK, 0 = không ai ACK, 2 = bus lỗi/kẹt (không phát được START, mất trọng tài...)
uint8_t I2C_Probe(I2C_TypeDef *I2Cx, uint8_t address)
{
    uint32_t timeout;
    uint8_t  result = 0;
    volatile uint32_t temp;

    I2Cx->CR1 |= (1 << 8);                          // START
    timeout = 100000UL;
    while (!(I2Cx->SR1 & (1 << 0)) && --timeout);   // Chờ SB
    if (!timeout)
    {
        I2Cx->SR1 &= ~((1 << 8) | (1 << 9));
        return 2;                                   // Không phát được START -> bus kẹt
    }

    I2Cx->DR = (uint8_t)(address << 1);             // Gửi địa chỉ + bit Write

    // Chờ 1 trong các cờ: ADDR (bit 1), BERR (bit 8), ARLO (bit 9), AF (bit 10)
    timeout = 100000UL;
    while (!(I2Cx->SR1 & ((1 << 1) | (1 << 8) | (1 << 9) | (1 << 10))) && --timeout);

    if (I2Cx->SR1 & (1 << 1))                       // ADDR = 1: có Slave ACK
    {
        temp = I2Cx->SR1;
        temp = I2Cx->SR2;                           // Đọc SR1 rồi SR2 để xóa ADDR
        (void)temp;
        result = 1;
    }
    else if (I2Cx->SR1 & ((1 << 8) | (1 << 9)) || !timeout)
    {
        result = 2;                                 // BERR / ARLO / timeout: bus lỗi
    }
    // Trường hợp còn lại: AF = 1 -> không ai ACK, result = 0

    I2Cx->SR1 &= ~((1 << 8) | (1 << 9) | (1 << 10)); // Xóa BERR, ARLO, AF
    I2C_Stop(I2Cx);
    return result;
}
