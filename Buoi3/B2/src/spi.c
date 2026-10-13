#include "spi.h"

void SPI1_Init(void) {
    // 1. Bật clock cho GPIOA (bit 2) và SPI1 (bit 12) trên bus APB2
    RCC->APB2ENR |= (1 << 12) | (1 << 2);

    // 2. Cấu hình PA5 (SCK) là Alternate Function Output Push-Pull 50MHz (0xB)
    GPIOA->CRL &= ~(0xF << 20);
    GPIOA->CRL |=  (0xB << 20);

    // 3. Cấu hình PA7 (MOSI) là Alternate Function Output Push-Pull 50MHz (0xB)
    GPIOA->CRL &= ~(0xF << 28);
    GPIOA->CRL |=  (0xB << 28);

    // 4. Xóa cấu hình cũ của SPI1
    SPI1->CR1 = 0;
    SPI1->CR2 = 0;

    // 5. Cấu hình SPI1 - CR1:
    //    bit 0  CPHA    = 1  (lấy mẫu ở cạnh lên - kết hợp CPOL=1 là mode 3)
    //    bit 1  CPOL    = 1  (clock idle mức cao)
    //    bit 2  MSTR    = 1  (chế độ Master)
    //    bit 5:3 BR     = 001 (chia 4: 72MHz / 4 = 18MHz)
    //    bit 7  LSBFIRST= 0  (MSB truyền trước)
    //    bit 8  SSI     = 1  \ quản lý NSS bằng phần mềm
    //    bit 9  SSM     = 1  /
    //    bit 11 DFF     = 0  (khung dữ liệu 8 bit)
    //    bit 14 BIDIOE  = 1  \ 1 đường truyền, chỉ gửi (không cần MISO)
    //    bit 15 BIDIMODE= 1  /
    SPI1->CR1 = (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) |
                (1 << 8) | (1 << 9) | (1 << 14) | (1 << 15);

    // 6. Bật SPI: SPE (bit 6)
    SPI1->CR1 |= (1 << 6);
}

void SPI1_Write(uint8_t data) {
    while (!(SPI1->SR & (1 << 1)));   // Chờ TXE (bit 1) = 1: buffer TX trống
    SPI1->DR = data;
}

void SPI1_WaitDone(void) {
    while (!(SPI1->SR & (1 << 1)));   // Chờ TXE = 1
    while (SPI1->SR & (1 << 7));      // Chờ BSY (bit 7) = 0: SPI gửi xong hẳn
}
