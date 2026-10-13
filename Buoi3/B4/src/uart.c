#include "uart.h"

void UART1_DMA_Init(void) {
    // 1. Bật clock cho GPIOA, USART1 và DMA1
    RCC->APB2ENR |= (1 << 14) | (1 << 2);
    RCC->AHBENR  |= (1 << 0);

    // 2. Cấu hình chân PA9 (TX) là Alternate Function Output Push-Pull
    GPIOA->CRH &= ~(0xF << 4);
    GPIOA->CRH |=  (0xB << 4);

    // 3. Cấu hình chân PA10 (RX) là input floating
    GPIOA->CRH &= ~(0xF << 8);
    GPIOA->CRH |=  (0x4 << 8);

    // 4. Khởi tạo lại các thanh ghi USART1 và DMA trước khi cấu hình
    USART1->CR1 = 0;
    USART1->CR2 = 0;
    USART1->CR3 = 0;
    DMA1_Channel4->CCR = 0;
    DMA1_Channel4->CPAR = 0;
    DMA1_Channel4->CMAR = 0;
    DMA1_Channel4->CNDTR = 0;

    // 5. Cấu hình baudrate 115200 cho PCLK2 = 72MHz
    //    BRR = 72000000 / 115200 = 625 = 0x271
    USART1->BRR = 0x271;

    // 6. Bật UART: UE + TE, cho phép DMA gửi TX
    USART1->CR1 |= (1 << 13) | (1 << 3);
    USART1->CR3 |= (1 << 7);

    // 7. Cấu hình DMA channel 4 cho truyền từ RAM -> UART
    DMA1_Channel4->CPAR = (uint32_t)&(USART1->DR);
    DMA1_Channel4->CCR = (1 << 7) | (1 << 4); // MINC + DIR
}

void UART1_Send_DMA(uint8_t *buffer, uint16_t len) {
    // Tắt DMA trước khi cài lại cấu hình
    DMA1_Channel4->CCR &= ~(1 << 0);

    // Cập nhật địa chỉ nguồn và số byte cần gửi
    DMA1_Channel4->CMAR = (uint32_t)buffer;
    DMA1_Channel4->CNDTR = len;

    // Xóa cờ transfer complete của DMA channel 4
    DMA1->IFCR |= (1 << 13);

    // Bật DMA để bắt đầu gửi
    DMA1_Channel4->CCR |= (1 << 0);
}