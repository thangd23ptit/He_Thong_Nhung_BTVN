#include "button.h"

void Button_Init(void) {
    // Bật clock cho GPIOA (chân PA0 là nút bấm trên board STM32F103C8)
    RCC->APB2ENR |= (1 << 2);

    // Cấu hình PA0 là input với pull-up nội:
    // MODE = 00 (input), CNF = 10 (pull-up / pull-down)
    GPIOA->CRL &= ~(0xF << 0);
    GPIOA->CRL |=  (0x8 << 0);

    // Mức mặc định là HIGH, khi ấn nút sẽ kéo xuống GND
    GPIOA->ODR |= (1 << 0);
}

// Hàm kiểm tra nút nhấn
// Trả về 1 nếu nút đang nhấn, 0 nếu không
uint8_t Button_IsPressed(void) {
    return ((GPIOA->IDR & (1 << 0)) == 0U) ? 1U : 0U;
}

void Led_Init(void) {
    // Bật clock cho GPIOC
    RCC->APB2ENR |= (1 << 4);

    // Cấu hình PC13 là output push-pull 50MHz
    GPIOC->CRH &= ~(0xF << 20);
    GPIOC->CRH |=  (0x3 << 20);

    // LED trên Blue Pill thường hoạt động active-low
    GPIOC->ODR |= (1 << 13);
}

void Led_Set(uint8_t on) {
    if (on) {
        // Active-low: 0V -> LED sáng
        GPIOC->ODR &= ~(1 << 13);
    } else {
        GPIOC->ODR |= (1 << 13);
    }
}